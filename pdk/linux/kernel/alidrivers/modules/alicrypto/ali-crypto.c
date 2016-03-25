#include <linux/module.h>    
#include <linux/kernel.h>
#include <crypto/aes.h> 
#include <crypto/algapi.h> 
#include <linux/crypto.h> 
#include <linux/interrupt.h> 
#include <linux/io.h> 
#include <linux/kthread.h> 
#include <linux/platform_device.h> 
#include <linux/scatterlist.h> 
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <crypto/internal/hash.h>
#include <crypto/sha.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <linux/ali_dsc.h> 
#include <linux/ali_ce.h> 
#include <ali_otp_common.h>

//#define ALI_CRYPTO_DBG
#ifdef ALI_CRYPTO_DBG
#define ALI_CRYPTO_PRT(fmt, args...) \
	do { \
		printk(fmt, ##args); \
	} while (0)

#define ALI_CRYPTO_LOG(fmt, args...) \
	do { \
		printk("ALI CRYPTO %s "fmt, __func__, ##args); \
	} while (0)
#define ALI_CRYPTO_DUMP(data,len) \
    do{ \
        int i, l=(len); \
        for(i=0; i<l; i++){ \
            ALI_CRYPTO_PRT("0x%x,",*(data+i)); \
            if((i+1)%16==0) \
                ALI_CRYPTO_PRT("\n");\
        }\
    }while(0)
#else
#define ALI_CRYPTO_PRT(...)
#define ALI_CRYPTO_LOG(...)
#define ALI_CRYPTO_DUMP(...) 
#endif

#define AES_HW_KEY_LEN  16

#define AES_KEY_LEN     (8 * 4)

#ifndef SG_MITER_TO_SG
#define SG_MITER_TO_SG          (1 << 1)        /* flush back to phys on unmap */
#endif

#ifndef SG_MITER_FROM_SG
#define SG_MITER_FROM_SG        (1 << 2)        /* nop */
#endif


/* 
 * STM: 
 * /---------------------------------------\ 
 * | | request complete 
 * \./ | 
 * IDLE -> new request -> BUSY -> done -> DEQUEUE 
 * /бу\ | 
 * | | more scatter entries 
 * \________________/ 
 */ 
enum engine_status { 
    ENGINE_IDLE, 
    ENGINE_BUSY, 
    ENGINE_W_DEQUEUE, 
}; 
 
/** 
 * struct req_progress - used for every crypt request 
 * @src_sg_it: sg iterator for src 
 * @dst_sg_it: sg iterator for dst 
 * @sg_src_left: bytes left in src to process (scatter list) 
 * @src_start: offset to add to src start position (scatter list) 
 * @crypt_len: length of current crypt process 
 * @sg_dst_left: bytes left dst to process in this scatter list 
 * @dst_start: offset to add to dst start position (scatter list) 
 * @total_req_bytes: total number of bytes processed (request). 
 * 
 * sg helper are used to iterate over the scatterlist. Since the size of the 
 * SRAM may be less than the scatter size, this struct struct is used to keep 
 * track of progress within current scatterlist. 
 */ 
struct req_progress { 
    struct sg_mapping_iter src_sg_it; 
    struct sg_mapping_iter dst_sg_it; 
 
    /* src mostly */ 
    int sg_src_left; 
    int src_start; 
    int crypt_len; 
    /* dst mostly */ 
    int sg_dst_left; 
    int dst_start; 
    int total_req_bytes; 
}; 
 
struct ali_ctx { 
    u8 aes_enc_key[32]; 
    u32 aes_dec_key[8]; 
    int key_len; 
    u32 need_calc_aes_dkey;
}; 

struct ali_aes_hw_key { 
    u8 cipher_key[16]; 
    u8 r1[16]; 
    u8 r2[16];
}; 

//define the default data for root key is zero
struct ali_aes_hw_key aes_hw_key={
        /*for OTP root secret key 2 is zero*/
    	.cipher_key	= "\x29\x04\x64\xeb\x94\xdc\x6d\x75\xd8\xc1\xc6\x13\xfa\x01\x65\xca",
        .r1 = "\x0e\xbb\x0e\x72\x47\xca\x22\xd9\xa5\xa3\x3f\xe0\x25\x89\xee\x54",
        .r2 = "\x08\xa2\x97\x8a\x97\x96\x54\x51\x8c\x38\x0c\xde\x60\xb3\xc6\x7e",
};

struct ali_aes_hw_key aes_download_key[2]= {
    {
        /*for OTP root secret key 2 is zero*/
    	.cipher_key	= "\x29\x04\x64\xeb\x94\xdc\x6d\x75\xd8\xc1\xc6\x13\xfa\x01\x65\xca",
        .r1 = "\x0e\xbb\x0e\x72\x47\xca\x22\xd9\xa5\xa3\x3f\xe0\x25\x89\xee\x54",
        .r2 = "\x08\xa2\x97\x8a\x97\x96\x54\x51\x8c\x38\x0c\xde\x60\xb3\xc6\x7e",
    },
    {
        /*for OTP root secret key 2 is M3713 sample key*/
    	.cipher_key	= "\x7e\x19\x22\x3\x9a\xc2\x9a\xdc\xf0\x12\x3e\x54\xb3\x12\xc4\x31",
        .r1 = "\xec\x34\x6e\xdd\x6\x99\x7c\xfa\x56\xe\xe7\xc8\xe3\x2b\x7f\x8d",
        .r2 = "\xe\xb0\x7b\x94\xc3\xef\x12\xc2\x6b\x7\xd4\x7a\xca\x21\x2f\x25",
    }
};

enum crypto_op { 
    COP_AES_ECB, 
    COP_AES_CBC, 
}; 
 
struct ali_req_ctx { 
    enum crypto_op op; 
    int decrypt; 
}; 
 
struct crypto_priv { 
    void __iomem *reg; 
    void __iomem *sram; 
    int irq; 
    struct task_struct *queue_th; 
 
    /* the lock protects queue and eng_st */ 
    spinlock_t lock; 
    struct crypto_queue queue; 
    enum engine_status eng_st; 
    struct ablkcipher_request *cur_req; 
    struct req_progress p; 
    int max_req_size; 
    pAES_DEV dev;
    void *buf_in;
    void *buf_out;
    int stream_create;
    enum crypto_op last_op;
    int last_key_len;
    int new_key_set; 
    int en_hw_key;    //enable hw key
    int hw_key_ready; //hw key already setted
}; 
 
static struct crypto_priv *cpg; 
 
 
static void compute_aes_dec_key(struct ali_ctx *ctx) 
 { 
     if (!ctx->need_calc_aes_dkey) 
         return; 
     memcpy(&ctx->aes_dec_key[0], &ctx->aes_enc_key[0], AES_KEY_LEN);
     ctx->need_calc_aes_dkey = 0; 
 } 
  
 static int ali_setkey_aes(struct crypto_ablkcipher *cipher, const u8 *key, 
         unsigned int len) 
 { 
     static u32 setkey_count = 0;    
     struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher); 
     struct ali_ctx *ctx = crypto_tfm_ctx(tfm); 
     
     switch (len) { 
     case AES_KEYSIZE_128: 
     case AES_KEYSIZE_192: 
     case AES_KEYSIZE_256: 
         break; 
     default: 
         crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN); 
         return -EINVAL; 
     } 
     ctx->key_len = len; 
     ctx->need_calc_aes_dkey = 1; 
     //if((++setkey_count)>8/*(AES_CBC_ENC_TEST_VECTORS+AES_CBC_DEC_TEST_VECTORS)*/)
     {
        cpg->en_hw_key=1;
        ALI_CRYPTO_LOG("Change to AES HW Key\n");
     }
     ALI_CRYPTO_LOG("SET AES TEST Key %d times\n", setkey_count);
     memcpy(ctx->aes_enc_key, key, AES_KEY_LEN); 
     cpg->new_key_set = 1; 
     return 0; 
 } 
  
 static void setup_data_in(struct ablkcipher_request *req) 
 { 
     int ret; 
     void *buf; 
  
     if (!cpg->p.sg_src_left) { 
         ret = sg_miter_next(&cpg->p.src_sg_it); 
         BUG_ON(!ret); 
         cpg->p.sg_src_left = cpg->p.src_sg_it.length; 
         cpg->p.src_start = 0; 
     } 
  
     cpg->p.crypt_len = min(cpg->p.sg_src_left, cpg->max_req_size); 
  
     buf = cpg->p.src_sg_it.addr; 
     buf += cpg->p.src_start; 
  
     cpg->buf_in = buf;
     //memcpy(cpg->sram + SRAM_DATA_IN_START, buf, cpg->p.crypt_len); 
  
     cpg->p.sg_src_left -= cpg->p.crypt_len; 
     cpg->p.src_start += cpg->p.crypt_len; 
} 

#define AES_HW_M2M2_KEY_POS	    	KEY_0_1
#define AES_SYS_KEY_POS				KEY_1_1
#define AES_HW_M2M2_KEY_OTP_ADDR	OTP_ADDESS_2

/*
 * Useful MACROs
 */
#define ALIASIX_CIPHERKEY_ATOH(result, str, condition, idx) \
    do { \
        result <<= 4; \
        if ((str[idx + 2] - '0') <= 9) \
	{ \
            result |= ((str[idx + 2] - '0') & 0x0000000F); \
	} \
        else \
	{ \
            result |= ((str[idx + 2] - 'a' + 10) & 0x0000000F); \
	} \
        idx++; \
    } while (condition)

static int aes_cipherkey_initialized = 0;

/*
 * To get the dmcrypt cipher key from U-Boot param
 * \@ param format:
 * \@ 0x********,0x****
 * \@ addr(8 hex always),size
 */
static int __init aes_cipherkey_setup(char *str)
{
    char *hw_key_opt = str;
    int i = 0;
    u_long addr = 0;
    struct ali_aes_hw_key *hw_key = NULL;
    __u32 otp_value=0;

	if (aes_cipherkey_initialized) return 1;
	aes_cipherkey_initialized = 1;

    ALI_CRYPTO_LOG("Info, dmcrypt cipher  param %s\n", str);

    if (NULL != strstr(str, "internal"))
    {
        ali_otp_hw_init();
        ali_otp_read(AES_HW_M2M2_KEY_OTP_ADDR*4, &otp_value, sizeof(__u32));
        if(0 != otp_value){
           memcpy(&aes_hw_key, &aes_download_key[1], sizeof(struct ali_aes_hw_key));
        }
        else{
           memcpy(&aes_hw_key, &aes_download_key[0], sizeof(struct ali_aes_hw_key));
        }
        return 1;
    }
    
    if (NULL == (hw_key_opt = strstr(str, "0x")))
        return 0;
    ALIASIX_CIPHERKEY_ATOH(addr, hw_key_opt, (hw_key_opt[i + 2] != '\0'), i);
    ALI_CRYPTO_LOG("Info, key addr 0x%x\n", addr);
    hw_key = (struct ali_aes_hw_key *)addr;
    memcpy(aes_hw_key.cipher_key, hw_key->cipher_key, AES_HW_KEY_LEN);
    memcpy(aes_hw_key.r1, hw_key->r1, AES_HW_KEY_LEN);
    memcpy(aes_hw_key.r2, hw_key->r2, AES_HW_KEY_LEN);
    ALI_CRYPTO_LOG("Info, get dmcrypt cipher param from uboot %s\n", str);
    return 1;
}
__setup("DmCipherKey=", aes_cipherkey_setup);

static int aes_prepare_key(struct ali_aes_hw_key *hw_key, u32 *key_pos)
{
    static pCE_DEVICE pCeDev0 = NULL;
	pCeDev0 = (pCE_DEVICE)hld_dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    CE_FOUND_FREE_POS_PARAM key_pos_param;
    u32 f_key_pos=0,s_key_pos=0; 
    u8  tmp_key[3*AES_HW_KEY_LEN];
    u32 i=0;
    int ret=0; 

    if(hw_key)
    {
        memcpy(&tmp_key[0],hw_key->r1,AES_HW_KEY_LEN);
        memcpy(&tmp_key[AES_HW_KEY_LEN],hw_key->r2,AES_HW_KEY_LEN);
        memcpy(&tmp_key[AES_HW_KEY_LEN*2],hw_key->cipher_key,AES_HW_KEY_LEN);
    }
    else
    {
        ALI_CRYPTO_LOG("input parameter error!!\n");
        return -1;
    }

#ifdef ALI_CRYPTO_DBG
    ALI_CRYPTO_LOG("tmp key:\n");
    ALI_CRYPTO_DUMP(tmp_key, 3*AES_HW_KEY_LEN);
#endif    
    if(!pCeDev0)
    {
        ALI_CRYPTO_LOG("crypto engine not avaliable!!\n");
        return -1;
    }    
    f_key_pos = AES_HW_M2M2_KEY_POS;
    s_key_pos = AES_SYS_KEY_POS;
    ret = ali_ce_load_otp_key(f_key_pos);
    if(ret)
    {
        ALI_CRYPTO_LOG("load otp key fail!!\n");
        return -1;
    }
    ret = ali_ce_generate_key_by_aes(&tmp_key[0],f_key_pos,s_key_pos,CE_IS_DECRYPT);
    if(ret){
        ALI_CRYPTO_LOG("generate 1 level key fail!!\n");
        return -1;
    }
    f_key_pos = s_key_pos;
    /*generate two level key*/
    for(i=0;i<2;i++){
    	memset(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    	key_pos_param.ce_key_level = TWO_LEVEL+i;
    	key_pos_param.pos = INVALID_ALI_CE_KEY_POS;
	key_pos_param.root = AES_HW_M2M2_KEY_POS;
    	ret = ali_ce_ioctl(pCeDev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    	if (ret)
    	{
    		ALI_CRYPTO_LOG("find free key pos fail!\n");
    		return INVALID_ALI_CE_KEY_POS;
    	}
        s_key_pos = key_pos_param.pos;
    	ret = ali_ce_generate_key_by_aes(&tmp_key[AES_HW_KEY_LEN*(i+1)],f_key_pos,s_key_pos,CE_IS_DECRYPT);

        if(ret){
            ALI_CRYPTO_LOG("generate %d level key fail!!\n",(i+2));
        }
#ifdef ALI_CRYPTO_DBG
    	CE_DEBUG_KEY_INFO param;
    	param.len = 4 ; 
    	param.sel = CE_KEY_READ ;
    	ali_ce_ioctl(pCeDev0, IO_CRYPT_DEBUG_GET_KEY, &param);  
        ALI_CRYPTO_LOG("the %d times level key:\n", (i+2));
        ALI_CRYPTO_DUMP(param.buffer, 4);
        ALI_CRYPTO_PRT("\n");
#endif
        ali_ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, f_key_pos);
    	if (ret)
    	{
    		ALI_CRYPTO_LOG("CE generate key by AES fail\n");
    		ali_ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, s_key_pos);
    		return INVALID_ALI_CE_KEY_POS;
    	}
        f_key_pos = s_key_pos;
    }
    *key_pos = s_key_pos;
    ALI_CRYPTO_LOG("return key pos is: %x\n",key_pos_param.pos);
    return 0;
}
  
 static void ali_process_current_q(int first_block) 
 { 
    struct ablkcipher_request *req = cpg->cur_req; 
    struct ali_ctx *ctx = crypto_tfm_ctx(req->base.tfm); 
    struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    UINT16 test_pid[1]= {0x1FFF};
    struct AES_256Bit_KEY ali_aes_key;
    static u32 pos=0;
    u32 ret = 0;
    
    /*if(first_block &&
       (cpg->stream_create == 0 ||
        cpg->new_key_set == 1   ||
        cpg->last_key_len != ctx->key_len ||
        cpg->last_op !=  req_ctx->op))*/
    if(first_block)    
    {  
     memset(&aes_param, 0, sizeof(struct aes_init_param));  
     memset(&key_param, 0, sizeof(KEY_PARAM));    
     
     switch (ctx->key_len) { 
     case AES_KEYSIZE_128:
        aes_param.key_mode = AES_128BITS_MODE;
        key_param.key_length = 128;
        break; 
     case AES_KEYSIZE_192:
        aes_param.key_mode = AES_192BITS_MODE;
        key_param.key_length = 192;
        break; 
     case AES_KEYSIZE_256:
        aes_param.key_mode = AES_256BITS_MODE; 
        key_param.key_length = 256;
        break; 
     default: 
         return; 
     }      
     cpg->last_key_len = ctx->key_len;
     switch (req_ctx->op) { 
     case COP_AES_ECB: 
         aes_param.work_mode = WORK_MODE_IS_ECB;
         break; 
     case COP_AES_CBC: 
         aes_param.work_mode = WORK_MODE_IS_CBC;
         break; 
     } 
     cpg->last_op =  req_ctx->op;
     
    if(cpg->en_hw_key){
        if(!cpg->hw_key_ready)
        {
            ret = aes_prepare_key(&aes_hw_key,&pos);
            if(!ret)
            {
                cpg->hw_key_ready = 1;
            }
            else
            {
                ALI_CRYPTO_LOG("Generate aes hw key error!\n");
            }
        }
    }else{
        if (req_ctx->decrypt) { 
            memcpy((void *)(&ali_aes_key.even_key[0]), ctx->aes_dec_key, key_param.key_length/8); 
            memcpy((void *)(&ali_aes_key.even_key[0]) + key_param.key_length/8, ctx->aes_dec_key, key_param.key_length/8);
        } else {
            memcpy((void *)(&ali_aes_key.even_key[0]), ctx->aes_enc_key, key_param.key_length/8);
            memcpy((void *)(&ali_aes_key.even_key[0]) + key_param.key_length/8, ctx->aes_enc_key, key_param.key_length/8);
        }
    }

    if(cpg->stream_create == 1)
    ali_aes_ioctl(cpg->dev ,IO_DELETE_CRYPT_STREAM_CMD , 0);

    aes_param.dma_mode = PURE_DATA_MODE;
    if(cpg->en_hw_key)
        aes_param.key_from = KEY_FROM_CRYPTO;
    else
        aes_param.key_from = KEY_FROM_SRAM;
    
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = 4; 
    aes_param.cbc_cts_enable = 0;
    ali_aes_ioctl(cpg->dev ,IO_INIT_CMD , (UINT32)&aes_param);

    key_param.ctr_counter = NULL ;
    key_param.init_vector = req->info; 
    key_param.pid_len = 1; 
    key_param.pid_list = test_pid;
    key_param.p_aes_iv_info = req->info ;
    if(cpg->en_hw_key)
        key_param.pos = pos;        
    else
        key_param.p_aes_key_info = (AES_KEY_PARAM *)(&ali_aes_key) ;
    
    key_param.stream_id = 4; 
    key_param.force_mode = 0;
    ali_aes_ioctl(cpg->dev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    cpg->stream_create = 1;  
    cpg->new_key_set = 0;      
    }
    
    setup_data_in(req); 

    //printk("%s %p %d %p\n", __func__, cpg->buf_in, cpg->p.crypt_len, cpg->buf_out);
    if (req_ctx->decrypt) { 
    ali_aes_decrypt(cpg->dev, 4, (UINT8 *)cpg->buf_in, (UINT8 *)cpg->buf_out, cpg->p.crypt_len);
    } else {
    ali_aes_encrypt(cpg->dev, 4, (UINT8 *)cpg->buf_in, (UINT8 *)cpg->buf_out, cpg->p.crypt_len);    
    } 
    cpg->eng_st = ENGINE_W_DEQUEUE;

} 
  
 static void ali_crypto_algo_completion(void) 
 { 
     struct ablkcipher_request *req = cpg->cur_req; 
     struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
  
     if (req_ctx->op != COP_AES_CBC) 
         return ; 
#if 1  
     memcpy(req->info, (cpg->buf_out + cpg->p.crypt_len - 16), 16); 
#endif 
 } 
  
 static void dequeue_complete_req(void) 
 { 
     struct ablkcipher_request *req = cpg->cur_req; 
     void *buf; 
     int ret; 
     int off;
     
     off = 0;
     cpg->p.total_req_bytes += cpg->p.crypt_len; 
     do { 
         int dst_copy; 
  
         if (!cpg->p.sg_dst_left) { 
             ret = sg_miter_next(&cpg->p.dst_sg_it); 
             BUG_ON(!ret); 
             cpg->p.sg_dst_left = cpg->p.dst_sg_it.length; 
             cpg->p.dst_start = 0; 
         } 
  
         buf = cpg->p.dst_sg_it.addr; 
         buf += cpg->p.dst_start; 
  
         dst_copy = min(cpg->p.crypt_len, cpg->p.sg_dst_left); 
#if 1  
         memcpy(buf, cpg->buf_out + off, dst_copy); 
         //memcpy(buf, cpg->sram + SRAM_DATA_OUT_START, dst_copy); 
#endif  
         cpg->p.sg_dst_left -= dst_copy; 
         cpg->p.crypt_len -= dst_copy; 
         cpg->p.dst_start += dst_copy; 
         off += dst_copy;
     } while (cpg->p.crypt_len > 0); 
  
     BUG_ON(cpg->eng_st != ENGINE_W_DEQUEUE); 
     if (cpg->p.total_req_bytes < req->nbytes) { 
         /* process next scatter list entry */ 
         cpg->eng_st = ENGINE_BUSY; 
         ali_process_current_q(0); 
     } else { 
         sg_miter_stop(&cpg->p.src_sg_it); 
         sg_miter_stop(&cpg->p.dst_sg_it); 
         ali_crypto_algo_completion(); 
         cpg->eng_st = ENGINE_IDLE; 
         req->base.complete(&req->base, 0); 
     } 
 } 
  
 static int count_sgs(struct scatterlist *sl, unsigned int total_bytes) 
 { 
     int i = 0; 
  
     do { 
         total_bytes -= sl[i].length; 
         i++; 
  
     } while (total_bytes > 0); 
  
     return i; 
 } 
  
 static void ali_enqueue_new_req(struct ablkcipher_request *req) 
 { 
     int num_sgs; 
  #if 1
     cpg->cur_req = req; 
     memset(&cpg->p, 0, sizeof(struct req_progress)); 
  
     num_sgs = count_sgs(req->src, req->nbytes); 
     sg_miter_start(&cpg->p.src_sg_it, req->src, num_sgs, SG_MITER_FROM_SG); 
  
     num_sgs = count_sgs(req->dst, req->nbytes); 
     sg_miter_start(&cpg->p.dst_sg_it, req->dst, num_sgs, SG_MITER_TO_SG); 
  #endif
     ali_process_current_q(1); 
 } 
  
 static int queue_manag(void *data) 
 { 
     cpg->eng_st = ENGINE_IDLE; 
     do { 
         struct ablkcipher_request *req; 
         struct crypto_async_request *async_req = NULL; 
         struct crypto_async_request *backlog; 
  
         //__set_current_state(TASK_INTERRUPTIBLE); 
  
         if (cpg->eng_st == ENGINE_W_DEQUEUE) 
             dequeue_complete_req(); 
  
         spin_lock_irq(&cpg->lock); 
         if (cpg->eng_st == ENGINE_IDLE) { 
             backlog = crypto_get_backlog(&cpg->queue); 
             async_req = crypto_dequeue_request(&cpg->queue); 
             if (async_req) { 
                 BUG_ON(cpg->eng_st != ENGINE_IDLE); 
                 cpg->eng_st = ENGINE_BUSY; 
             } 
         } 
         spin_unlock_irq(&cpg->lock); 
  
         if (backlog) { 
             backlog->complete(backlog, -EINPROGRESS); 
             backlog = NULL; 
         } 
  
         if (async_req) { 
             req = container_of(async_req, 
                     struct ablkcipher_request, base); 
             ali_enqueue_new_req(req); 
             async_req = NULL; 
         } 
  
         //schedule(); 
  
     } while (!kthread_should_stop()); 
     return 0; 
 } 
  
 static int ali_handle_req(struct ablkcipher_request *req) 
 { 
     unsigned long flags; 
     int ret; 
  
     spin_lock_irqsave(&cpg->lock, flags); 
     ret = ablkcipher_enqueue_request(&cpg->queue, req); 
     spin_unlock_irqrestore(&cpg->lock, flags); 
     wake_up_process(cpg->queue_th); 
     return ret; 
 } 
  
 static int ali_enc_aes_ecb(struct ablkcipher_request *req) 
 { 
     struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
  
     req_ctx->op = COP_AES_ECB; 
     req_ctx->decrypt = 0; 
  
     return ali_handle_req(req); 
 } 
  
 static int ali_dec_aes_ecb(struct ablkcipher_request *req) 
 { 
     struct ali_ctx *ctx = crypto_tfm_ctx(req->base.tfm); 
     struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
  
     req_ctx->op = COP_AES_ECB; 
     req_ctx->decrypt = 1; 
  
     compute_aes_dec_key(ctx); 
     return ali_handle_req(req); 
 } 
  
 static int ali_enc_aes_cbc(struct ablkcipher_request *req) 
 { 
     struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
  
     req_ctx->op = COP_AES_CBC; 
     req_ctx->decrypt = 0; 
  
     return ali_handle_req(req); 
 } 
  
 static int ali_dec_aes_cbc(struct ablkcipher_request *req) 
 { 
     struct ali_ctx *ctx = crypto_tfm_ctx(req->base.tfm); 
     struct ali_req_ctx *req_ctx = ablkcipher_request_ctx(req); 
  
     req_ctx->op = COP_AES_CBC; 
     req_ctx->decrypt = 1; 
 
     compute_aes_dec_key(ctx); 
     return ali_handle_req(req); 
 } 
  
 static int ali_cra_init(struct crypto_tfm *tfm) 
 {
	 printk("crypto alg: ali hw aes\n");
	 if (tfm && tfm->__crt_alg)
		 printk("alg name %s %s\n", tfm->__crt_alg->cra_name, \
				 tfm->__crt_alg->cra_driver_name);
     tfm->crt_ablkcipher.reqsize = sizeof(struct ali_req_ctx); 
     return 0; 
 } 
  
 struct crypto_alg ali_aes_alg_ecb = { 
     .cra_name = "ecb(aes)", 
     .cra_driver_name = "ali-ecb-aes", 
     .cra_priority = 300, 
     .cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
     .cra_blocksize = AES_BLOCK_SIZE, 
     .cra_ctxsize = sizeof(struct ali_ctx), 
     .cra_alignmask = 0, 
     .cra_type = &crypto_ablkcipher_type, 
     .cra_module = THIS_MODULE, 
     .cra_init = ali_cra_init, 
     .cra_u = { 
         .ablkcipher = { 
             .min_keysize = AES_MIN_KEY_SIZE, 
             .max_keysize = AES_MAX_KEY_SIZE, 
             .setkey = ali_setkey_aes, 
             .encrypt = ali_enc_aes_ecb, 
             .decrypt = ali_dec_aes_ecb, 
         }, 
     }, 
 }; 
  
 struct crypto_alg ali_aes_alg_cbc = { 
     .cra_name = "cbc(aes)", 
     .cra_driver_name = "ali-cbc-aes", 
     .cra_priority = 300, 
     .cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
     .cra_blocksize = AES_BLOCK_SIZE, 
     .cra_ctxsize = sizeof(struct ali_ctx), 
     .cra_alignmask = 0, 
     .cra_type = &crypto_ablkcipher_type, 
     .cra_module = THIS_MODULE, 
     .cra_init = ali_cra_init, 
     .cra_u = { 
         .ablkcipher = { 
             .ivsize = AES_BLOCK_SIZE, 
             .min_keysize = AES_MIN_KEY_SIZE, 
             .max_keysize = AES_MAX_KEY_SIZE, 
             .setkey = ali_setkey_aes, 
             .encrypt = ali_enc_aes_cbc, 
             .decrypt = ali_dec_aes_cbc, 
         }, 
     }, 
 }; 
  
 static int ali_probe(void) 
 { 
     struct crypto_priv *cp; 
     //struct resource *res; 
     //int irq; 
     int ret=0; 
  
     if (cpg) { 
         printk(KERN_ERR "Second crypto dev?\n"); 
         return -EEXIST; 
     } 
    
     cp = kzalloc(sizeof(*cp), GFP_KERNEL); 
     if (!cp) 
         return -ENOMEM; 
     
     cp->buf_out = kzalloc(0x10000, GFP_KERNEL); 
     if (!cp->buf_out) 
         goto err; 
         
     cp->max_req_size = 0x10000;
     cp->buf_in = 0; 
     cp->dev = (pAES_DEV)(ali_dsc_get_free_sub_device_id(AES));
     cp->dev = hld_dev_get_by_id(HLD_DEV_TYPE_AES, (UINT16)cp->dev);
     cp->stream_create = 0;
     cp->new_key_set = 0;
     cp->en_hw_key = 0;
     cp->hw_key_ready = 0;
     
     spin_lock_init(&cp->lock); 
     crypto_init_queue(&cp->queue, 50); 
 
     cpg = cp; 
  
     
     cp->queue_th = kthread_run(queue_manag, cp, "ali_crypto"); 
     if (IS_ERR(cp->queue_th)) { 
         ret = PTR_ERR(cp->queue_th); 
         goto err_thread; 
     } 
  
     
     ret = crypto_register_alg(&ali_aes_alg_ecb); 
     if (ret) 
         goto err_reg; 
  
     ret = crypto_register_alg(&ali_aes_alg_cbc); 
     if (ret) 
         goto err_unreg_ecb; 
     return 0; 
 err_unreg_ecb: 
     crypto_unregister_alg(&ali_aes_alg_ecb); 
 err_thread:    
 err_reg: 
     kthread_stop(cp->queue_th); 
 err: 
     kfree(cp); 
     cpg = NULL; 
     return ret; 
 } 
  
 static void ali_remove(void) 
 { 
     struct crypto_priv *cp = cpg; 
  
     crypto_unregister_alg(&ali_aes_alg_ecb); 
     crypto_unregister_alg(&ali_aes_alg_cbc); 
     kthread_stop(cp->queue_th); 
     kfree(cp); 
     cpg = NULL; 
     return; 
 } 
  

  
 static int __init ali_crypto_init(void) 
 { 
     return ali_probe();
 } 
 late_initcall_sync(ali_crypto_init); 
  
 static void __exit ali_crypto_exit(void) 
 { 
     ali_remove();
 } 
 module_exit(ali_crypto_exit); 
  
 MODULE_AUTHOR("wen_liu@ali.com.tw"); 
 MODULE_DESCRIPTION("Support for ALI's cryptographic engine"); 
 MODULE_LICENSE("GPL"); 
