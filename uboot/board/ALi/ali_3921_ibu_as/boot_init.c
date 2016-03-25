#include <common.h>
#include <ali/basic_types.h>
#include <asm-generic/gpio.h>
#include <ali_gpio.h>
#include <i2c.h>

#include <nand.h>
#include <linux/mtd/mtd.h>
#include <ali_nand.h>

#include <ali/hld_dev.h>
#include <ali/string.h>
#include <ali/dsc.h>
#include <ali/trng.h>
#include <ali/crypto.h>
#include <ali/rsa_verify.h>
#include <ali/flash_cipher.h>
//#include "ali_pmi_stbid.h"

#include <ali/chunk.h>
#include <ali/sys_parameters.h>
//#include "./include/flash.h"
extern UINT8* g_pmi_blk ; 
extern struct PMI   _gxPMI;
#include <ali/pan.h>
#include <ali/pan_dev.h>
//#include "./panel/ch455/pan_ch455.h"

#define FP_LOCK_GPIO_NUM            4//127
#define FP_STANDBY_GPIO_NUM         127
#define FP_CLOCK_GPIO_NUM           127
#define FP_DATA_GPIO_NUM            127
#define FP_CS_GPIO_NUM            	127
#define FP_KEY1_GPIO_NUM            127
#define FP_COM1_GPIO_NUM            127
#define FP_COM2_GPIO_NUM            127
#define FP_COM3_GPIO_NUM            127
#define FP_COM4_GPIO_NUM            127
#define GPIO_USB_POWER            	127

#if 1
#define BOOT_PRINTF printf
#define BOOT_DUMP(data, len) \
	do { \
		int i, l = (len); \
		for (i = 0; i < l; i++) \
			BOOT_PRINTF(" 0x%02x", *((data) + i)); \
		BOOT_PRINTF("\n"); \
} while (0)

#else
#define BOOT_PRINTF(...) do{}while(0)
#define BOOT_DUMP(...) do{}while(0)
#endif

#if 1
struct pan_hw_info pan_hw_info =
{
	0,				/* type_kb : 2; Key board (array) type */
	1,				/* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,				/* type_key: 1; Key exit or not */
	1,				/* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,				/* type_mcu: 1; MCU exit or not */
	4,				/* num_com: 4; Number of com PIN, 0 to 8 */
	1,				/* Position of colon flag, 0 to 7 */
	1,				/* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,				/* rsvd_bits:6; Reserved bits */
	0,              /* rsvd byte for align pan_info */
	{0, HAL_GPIO_O_DIR, 127},		/* LATCH PIN */
	{0, HAL_GPIO_O_DIR, 	FP_CLOCK_GPIO_NUM},		/* CLOCK PIN */
	{1, HAL_GPIO_O_DIR, 	FP_DATA_GPIO_NUM},		/* DATA PIN */
	{{0, HAL_GPIO_I_DIR, 	FP_KEY1_GPIO_NUM},		/* SCAN1 PIN */
	{0, HAL_GPIO_I_DIR, 127}},		/* SCAN2 PIN */
	{{0, HAL_GPIO_O_DIR, 	FP_COM1_GPIO_NUM},		/* COM1 PIN */
	{0, HAL_GPIO_O_DIR, 	FP_COM2_GPIO_NUM},		/* COM2 PIN */
	{0, HAL_GPIO_O_DIR, 	FP_COM3_GPIO_NUM},		/* COM3 PIN */
	{0, HAL_GPIO_O_DIR, 	FP_COM4_GPIO_NUM},		/* COM4 PIN */
	{0, HAL_GPIO_O_DIR, 127},		/* COM5 PIN */
	{0, HAL_GPIO_O_DIR, 127},		/* COM6 PIN */
	{0, HAL_GPIO_O_DIR, 127},		/* COM7 PIN */
	{0, HAL_GPIO_O_DIR, 127}},		/* COM8 PIN */
	{{0, HAL_GPIO_O_DIR, 	FP_STANDBY_GPIO_NUM},		/* POWER PIN */
	{1, HAL_GPIO_O_DIR, 	FP_LOCK_GPIO_NUM},		/* LOCK PIN */
	{0, HAL_GPIO_O_DIR, 127},		/* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 127}},		/* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 127},       /* rsvd extend function LBD */        
	300,							/* Intv repeat first */
	250,							/* Intv repeat */
	350,							    /* Intv release, 0: disable release key */
	NULL,	 	 	 	 	 	 	/* hook_scan() callback */
	NULL,	 	 	 	 	 	 	/* hook_show() callback */
};
#define bitmap_list				NULL
#define bitmap_list_num		0

struct pan_configuration pan_config = {&pan_hw_info, bitmap_list_num, bitmap_list};
UINT32 g_panel_id = I2C_TYPE_GPIO0;
#endif

void panel_init()
{
    UINT32 data,i;
    struct pan_device *panel_dev = NULL;

    dev_en_remote(0);
	pan_ch455_attach(&pan_config);
    panel_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
    pan_ch455_id_set(panel_dev,g_panel_id);

    if(panel_dev == NULL)
    {
        BOOT_PRINTF("panel attach fail!\n");
        return;
    }
    pan_open(panel_dev);
	pan_display(panel_dev, " ON ", 4);
	dev_en_remote(1);

    printf("panel init OK!\n");
}

void bl_panel_fail()
{
/*
    struct pan_device *panel_dev = NULL;

    panel_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
    return pan_display(panel_dev,"FAIL",4);
*/
}

struct sto_device *flash_dev = NULL;


char ascii0[] = "0123456789ABCDEF";
static void dump_reg(unsigned long addr,unsigned long len)
{
	unsigned long i,j;
	//char* ascii;
	unsigned char index;
	
//	char ascii[] = "0123456789ABCDEF";
	for(i=0;i<len;i++)
	{
		if(i%16 == 0)
		{
			output_char(0x0d);
			output_char(0x0a);
			for(j=0;j<8;j++)
			{
				output_char(ascii0[((addr+i)>>(4*(7-j)))&0xF]);
			}
			output_char(':');
		}
		index = *(unsigned char *)(addr+i);
		output_char(ascii0[(index>>4)&0xF]);
		output_char(ascii0[index&0xF]);
		output_char(' ');
	}
	output_char(0x0d);
	output_char(0x0a);
	return;
}

#ifndef NAND_BOOT
void bl_init_flash()
{
    UINT8  flash_speed;
	UINT32 data[128];
	UINT32 i,j;

	UINT8 *product_sabbat_dual_abs;

    dev_en_remote(0);
#ifdef ALI_ARM_STB
    *(volatile UINT32*)0x1802e098 = 0xc2000d03 ;
#else
    *(volatile UINT32*)0xb802e098 = 0xc2000d03 ;
#endif
#if 0//#ifndef ENABLE_SERIAL_FLASH
	flash_info_pl_init();
	sto_local_flash_attach(NULL);
#else
	flash_info_sl_init();
	sto_local_sflash_attach(NULL);
#endif
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (flash_dev == NULL)
	{
		BOOT_PRINTF("Can't find FLASH device!\n");
		return;
	}
	sto_open(flash_dev);
	sto_chunk_init(0, flash_dev->totol_size);
	sto_get_data(flash_dev, &flash_speed, HW_SET_FLASHSPEED, 1);
/*
	product_sabbat_dual_abs = 0x8b000000;	// CVD load product_sabbat_dual_abs to 0x8b000000

	j=0x100000;
//	sto_put_data(flash_dev, 0,product_sabbat_dual_abs, j);
		
	j=0;
	while(1)
	{
		for(i=0;i<128;i++) data[i] = j*128+i;
		//sto_put_data(flash_dev,  j*128*4,data, 128*4);
		for(i=0;i<128;i++) data[i] = 0;
		sto_get_data(flash_dev, data,  j*128*4, 128*4);
		dump_reg(data,128*4);
		j+=128;
	}
	flash_speed &= 0x3F;
	if (flash_speed == 0)
		flash_speed = HW_FLASH_DEFSPEED;
*/		
//	HAL_ROM_MODE_SET(flash_speed, 0, 1, 0);
 //   flash_info.flash_size=0;
    //flash_identify();
//	BOOT_PRINTF("flash init OK,flash size is:%x!\n",flash_info.flash_size);
    //osal_delay(100000);
    dev_en_remote(1);
}
#else
void bl_init_flash()
{
    dev_en_remote(0);
    sto_local_ram_attach();
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (flash_dev == NULL)
	{
		BOOT_PRINTF("Can't find FLASH device!\n");
	}
	sto_open(flash_dev);
    sto_chunk_init(0, flash_dev->totol_size);
//	BOOT_PRINTF("flash init OK,flash size is:%x!\n",flash_info.flash_size);
    dev_en_remote(1);
}
#endif
/*
int api_store_chunk_to_nandflash(UINT32 id)
{
    UINT32 block_addr = 0, block_len = 0, ret_len = 0, addr = 0, len = 0;
	nand_info_t *nand = &nand_info[nand_curr_device];
    struct Partation *part = NULL;
	int ret =-1;

    if (RET_FAILURE == sto_get_chunk_len(id, &block_addr, &block_len))
    {
        BOOT_PRINTF("sto_get_chunk_len Fail!\n");
        return -1;
    }
    BOOT_PRINTF("[%s, %d]block_addr = 0x%x, block_len = 0x%x\n", __FUNCTION__, __LINE__, block_addr, block_len);

    part = (struct Partation * )malloc(sizeof(struct Partation));
    if (part == NULL)
    {
        BOOT_PRINTF("Error[%s, %d]: out of memory 0x%x \n", __FUNCTION__, __LINE__, sizeof(struct Partation));
        return -1;
    }
    memset(part, 0x0, sizeof(struct Partation));
    // Due to MPtools rule, the bl addr and len need left shift 10
    part->offset = (_gxPMI.Loader_Start << 10) + block_addr;
    part->img_len = block_len;
    addr = RAM_BASE_ADDR + block_addr;

    ret_len = part->img_len;
	ret = nand_write_skip_bad(nand, (loff_t)(part->offset), &(ret_len), addr, 0);

	if (ret < 0) {
		printf("Error (%d) writing offset %d\n", ret,part->offset);
		return -1;
	}

    free(part);
    return 0;
}
*/


#if 0 //ifdef _CAS9_CA_ENABLE_ 

static int NF_LoadMac(UINT8 *buf, UINT32 len)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	int ret = -1 ;
	UINT32 retlen = len;

	ret = nand_read_skip_bad(nand, _gxPMI.Mac_Addr_Start, &retlen, buf);
	if (ret < 0) {
		printf("Error (%d) reading offset %d\n", ret,_gxPMI.Mac_Addr_Start);
		return -1;
	}
	return ret;
}

static int NF_SaveMac(UINT8 *buf, UINT32 len)
{

	nand_info_t *nand = &nand_info[nand_curr_device];
	int ret = -1 ;
	UINT32 retlen = len;

	ret = nand_write_skip_bad(nand, (loff_t)(_gxPMI.Mac_Addr_Start), &(retlen), buf, 0);

	if (ret < 0) {
		printf("Error (%d) writing offset %d\n", ret,_gxPMI.Mac_Addr_Start);
		return -1;
	}
	
	return ret;

}

/*API for mac address hash encrypt
* @ mac       : mac address data buf
* @ len       : mac address data len < 0x10
* mac address output format :
*        mac address (6 bytes + 10 bytes null) 
*      + magic number(4 bytes )
*      + encrypt mac hash (32bytes)
*      + R5+R6 (16 bytes + 16 bytes)   
*
*/
INT32 encrypt_mac_addr(UINT8 *mac, UINT32 len)
{
	RET_CODE ret;
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM otp_info;
	CE_DATA_INFO Ce_data_info;
	UINT8 raw_data[32];
    UINT8 hash_buf[HASH_OUT_LENGTH];
	UINT8 key_pos;
	CE_FOUND_FREE_POS_PARAM key_pos_param;
    AES_KEY_LADDER_BUF_PARAM aes_ce_param;       

	BOOT_PRINTF("encrypt mac address\n");
    if(len > MAC_ADDR_LEN )
    {
        BOOT_PRINTF("Error : mac address length out of range !%s,%d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }

    /*1. generate key pos from dsc*/
	if((ret =trng_get_64bits(raw_data,4)) == RET_FAILURE)
	{   
        BOOT_PRINTF("Error : generate random data error!%s,%d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
	}
	BOOT_PRINTF("raw data:\n");
	BOOT_DUMP(raw_data, 32);

    MEMSET(&aes_ce_param ,0, sizeof(AES_KEY_LADDER_BUF_PARAM));
    aes_ce_param.key_ladder=2;
    aes_ce_param.root_key_pos=KEY_0_1;
    MEMCPY(&aes_ce_param.r[0],raw_data,32);
    if(ret=aes_generate_key_with_multi_keyladder(&aes_ce_param, &key_pos)!=RET_SUCCESS)
    {
        BOOT_PRINTF("Error :generate key error!%s,%d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }

    /*2. calc pure mac address hash */
    UINT8 *tmp_buf = (UINT8*)MALLOC(0x100+0x1f);
    UINT8* nand_mac = (UINT8 *)(((UINT32)tmp_buf + 0x1f)&(0xfffffff0));
    if(nand_mac == NULL )
    {
        BOOT_PRINTF("Error :malloc mac buffer error!%s,%d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }   
    MEMSET(nand_mac,0x0,0x64);

     // pure mac address hash
     memset(hash_buf,0x0,32);
   if (RET_SUCCESS != create_sha_ramdrv(mac, len, hash_buf))
	{
		BOOT_PRINTF("create_sha_ramdrv() failed!\n");
		ret = RET_FAILURE ;
        goto exit;
	}
    BOOT_PRINTF("pure mac address hash data:\n");
	BOOT_DUMP(hash_buf, 32);

    /*3. encrypt hash result*/
    UINT8 *output = nand_mac ;
    *(UINT32*)(output+MAC_ADDR_LEN) = MAC_EN_MAGIC;  // set encrypte done flag
    MEMCPY(output,mac,len);                      // clear mac address
    MEMCPY(output+MAC_ADDR_LEN+4+HASH_OUT_LENGTH, raw_data, 32);  // store the random number
    osal_cache_flush(output,0x100);
    ret = aes_crypt_puredata_with_ce_key(hash_buf,                \   
                                        &output[MAC_ADDR_LEN+4],   \
                                        HMAC_OUT_LENGTH,          \
                                        key_pos,                \
                                        DSC_ENCRYPT);
    if(ret!=RET_SUCCESS)
    {
        BOOT_PRINTF("Error :mac encrypt error! %s\n",__FUNCTION__);
        ret = RET_FAILURE ;
        goto exit;
    }
    BOOT_PRINTF("mac address hash output:\n");
	BOOT_DUMP(output, 0x64);

    /*4.save to nand flash*/
    ret = NF_SaveMac(output,0x64);
    if(ret <0 ){
        BOOT_PRINTF("Error :save cipher mac address failed:\n");
        ret = RET_FAILURE ;
        goto exit;
    }
   
exit:
	ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos);
    FREE(tmp_buf);
	return ret;
}

/*API for mac address hash verify
* @ mac_data  : mac buffer from nand which should include
                mac + magic number + encrypt hash + R5&R6 
* @ len       : mac address data len 0x64
*
*
*/
INT32 decrypt_mac_addr(UINT8 *mac, UINT32 len)
{
	RET_CODE ret;
	pCE_DEVICE pCeDev = (pCE_DEVICE)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;
    UINT8 hash_buf[HASH_OUT_LENGTH],decrypt_buf[HASH_OUT_LENGTH];
	UINT8 key_pos;
	CE_FOUND_FREE_POS_PARAM key_pos_param;
    AES_KEY_LADDER_BUF_PARAM aes_ce_param;

    /*1. generate key pos from dsc*/
    MEMSET(&aes_ce_param ,0, sizeof(AES_KEY_LADDER_BUF_PARAM));
    aes_ce_param.key_ladder=2;
    aes_ce_param.root_key_pos=KEY_0_1;
    MEMCPY(&aes_ce_param.r[0],&mac[MAC_ADDR_LEN+4+HASH_OUT_LENGTH],32);
    if(ret=aes_generate_key_with_multi_keyladder(&aes_ce_param, &key_pos)!=RET_SUCCESS)
    {
        BOOT_PRINTF("generate key error!%s,%d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }

    /*2. decrypt hash result*/

    if(ret=aes_crypt_puredata_with_ce_key(&mac[MAC_ADDR_LEN+4],decrypt_buf,HASH_OUT_LENGTH,key_pos,DSC_DECRYPT)!=RET_SUCCESS)
    {
        BOOT_PRINTF("decrypt error! %s\n",__FUNCTION__);
	    ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos);
        ret= RET_FAILURE;
        goto exit;
    }
    BOOT_PRINTF("decyrpt mac hash result:\n");
	BOOT_DUMP(decrypt_buf, HASH_OUT_LENGTH);

     /*3. calc pure mac address hash*/
    if (RET_SUCCESS != create_sha_ramdrv(mac, 6, hash_buf))
	{
		BOOT_PRINTF("create_sha_ramdrv() failed!\n");
		ret = RET_FAILURE ;
        goto exit ;
	}
    BOOT_PRINTF("calc mac hash result:\n");
	BOOT_DUMP(hash_buf, HASH_OUT_LENGTH); 

    //compare the result, if not equal, system will reboot
    if(MEMCMP(hash_buf,decrypt_buf,HASH_OUT_LENGTH))
    {
        BOOT_PRINTF("mac address verify fail, reboot %s, %d\n",__FUNCTION__, __LINE__);
        BOOT_DUMP(mac, 0x64);        
        hw_watchdog_reboot();
    }
    ret = RET_SUCCESS;
exit:    
	ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos);
	return ret;
}


int encrypt_mac()
{

    int ret=RET_FAILURE; 
    UINT8 * tmp_buf = (UINT8*)malloc(0x100 + 0x1f); 
	UINT8 * mac_buf = (UINT8 *)(((UINT32)tmp_buf + 0x1f)&(0xfffffff0));
	
    ret = NF_LoadMac(mac_buf,0x100);
    if(ret != RET_SUCCESS ){
        BOOT_PRINTF("read mac failure %s,%d\n",__FUNCTION__,__LINE__);
        return ret ;
    }
      BOOT_PRINTF("To be check mac buf : \n");
      BOOT_DUMP(mac_buf,0x100);
    
    if( *(UINT32*)(mac_buf+MAC_ADDR_LEN)  !=  MAC_EN_MAGIC )
    {
        ret = encrypt_mac_addr(mac_buf, 6);
        if(ret != RET_SUCCESS){
            BOOT_PRINTF("Encrypt mac falied %s,%d\n",__FUNCTION__,__LINE__);
            FREE(tmp_buf);
            return ret ;
        }
        BOOT_PRINTF("Encrypt mac success \n");    
    }
    else
    {
        //already encrypt
        ret=RET_SUCCESS; 
    }
    FREE(tmp_buf);
    return ret;
}

int decrypt_mac()
{

    int ret=RET_FAILURE; 

    UINT8 * tmp_buf = (UINT8*)malloc(0x100 + 0x1f); 
	UINT8 * mac_buf = (UINT8 *)(((UINT32)tmp_buf + 0x1f)&(0xfffffff0));
	
	ret = NF_LoadMac(mac_buf,0x100);
	if(ret != RET_SUCCESS )
	{
        BOOT_PRINTF("read mac failure %s,%d\n",__FUNCTION__,__LINE__);
        return ret ;
    }
    BOOT_PRINTF("Cipher mac buf : \n");
	BOOT_DUMP(mac_buf,0x100);
     
    if(*(volatile UINT32*)(mac_buf+MAC_ADDR_LEN)==MAC_EN_MAGIC)
    {
        //decrypt mac and verify it, if it is not pass the integrity check, system will reboot
        ret = decrypt_mac_addr(mac_buf, 0x100);
        if(ret != RET_SUCCESS)
            BOOT_PRINTF("decrypt mac falied %s,%d\n",__FUNCTION__,__LINE__);
        BOOT_PRINTF("decrypt mac success \n");          
    }
    else
    {
        BOOT_PRINTF("cipher mac error %s,%d\n",__FUNCTION__,__LINE__);
        ret=RET_FAILURE;
    }
    FREE(tmp_buf);
    return ret;
}

int verify_mac_addr()
{
    int ret=RET_FAILURE; 
    if((ret=encrypt_mac())!=RET_SUCCESS)
    {
        BOOT_PRINTF("Reboot %s,%d\n",__FUNCTION__, __LINE__);
        hw_watchdog_reboot();
    }
    if((ret=decrypt_mac())!=RET_SUCCESS)
    {
        BOOT_PRINTF("Reboot %s,%d\n",__FUNCTION__, __LINE__);
        hw_watchdog_reboot();
    }
    return ret;
}

    
/*
Verify the signature of input data, if pass verification, return RET_SUCCESS, otherwise, return error NO.
const UINT32 addr: the start address for the check data, 
                   the data format need follow ALi format,use ALi sign tools can generate the special format
const UINT32 len:  data length for verify
*/
static UINT8 buf_ff[256];
static INT32 nand_pmi_hash_status()
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *chip = nand->priv;

    UINT32 ret ,ofset;
    UINT8*  blk_buf ;
    
    blk_buf = g_pmi_blk;
    MEMSET(buf_ff, 0xff, 256);
    if( MEMCMP(buf_ff, blk_buf,256) == 0)
    {
        BOOT_PRINTF("ERROR : pmi is empyt 0xff\n");
        ret = 2 ;
        goto exit; 
    }

    MEMSET(buf_ff, 0x0, 256);
    if( MEMCMP(buf_ff, blk_buf,256) == 0)
    {
        BOOT_PRINTF("ERROR : pmi is empyt\n");
        ret = 2 ;
        goto exit; 
    }
    
    ofset = chip->PMIpos + PMI_HASH_OFFSET; // pmi hash offset by bytes

    BOOT_PRINTF("pmi status buf :");
    BOOT_DUMP((UINT8*)(blk_buf+ofset),32);
    if( *(UINT32*)(blk_buf+ofset) == MAC_EN_MAGIC )
           ret = 1;
    else
           ret = 0;
exit:    
    return ret ;
    
}


static INT32 nand_pmi_hash_crypt(unsigned char * input, unsigned char * output,unsigned int len , unsigned int flag)
{
	UINT32 AesDevId = INVALID_DSC_SUB_DEV_ID;
    pAES_DEV  pAesDev = NULL;
  	UINT16 pid[1] = {0x1234};
	UINT32 stream_id = INVALID_DSC_STREAM_ID;

    struct aes_init_param aes_param;
	KEY_PARAM key_param;
    AES_IV_INFO iv;
    int ret ;

	BOOT_PRINTF("aes_cbc_decrypt_chunk: pos %d\n", g_uk_pos);

    if(flag == DSC_ENCRYPT)
    {
        BOOT_PRINTF("\nencrypt data \n"); 
        BOOT_PRINTF("input data:\n");
	    BOOT_DUMP(input, 32);
    }else
    {
        BOOT_PRINTF("\ndecrypt data \n"); 
        BOOT_PRINTF("input data:\n");
	    BOOT_DUMP(input, 32);
    }
    //config aes

	if ((AesDevId = dsc_get_free_sub_device_id(AES)) == INVALID_DSC_SUB_DEV_ID)
	{
		BOOT_PRINTF("dsc_get_free_sub_device_id() failed\n");
		return -1;
	}
	
	if ((pAesDev = dev_get_by_id(HLD_DEV_TYPE_AES, AesDevId)) == NULL)
	{
		BOOT_PRINTF("%s() get AES device %d failed!\n", __FUNCTION__, AesDevId);
		dsc_set_sub_device_id_idle(AES, AesDevId);
		return -1;
	}
	
	if ((stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
		BOOT_PRINTF("%s() get free stream id failed!\n", __FUNCTION__);
		dsc_set_sub_device_id_idle(AES, AesDevId);		
		return -1;
	}

    MEMSET(iv.even_iv, 0, sizeof(iv.even_iv));
	MEMSET(iv.odd_iv, 0, sizeof(iv.odd_iv));   	
	MEMSET(&aes_param, 0, sizeof(struct aes_init_param));
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0 ;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC ;
    aes_param.cbc_cts_enable = 0;
	aes_ioctl(pAesDev ,IO_INIT_CMD , (UINT32)&aes_param);

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL ; 
	key_param.init_vector = &iv ;
	key_param.key_length = 128;  
	key_param.pid_len = 1;
	key_param.pid_list = pid;
	key_param.p_aes_iv_info = &iv ;
	key_param.p_aes_key_info = NULL;
	key_param.stream_id = stream_id; //PURE_ID; /*DMx 0*/ /*0-3 for dmx id , 4-7 for pure data mode*/
	key_param.force_mode = 1;
	key_param.pos = g_uk_pos;
	
	aes_ioctl(pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);	

    if(flag == DSC_ENCRYPT)
	    ret = aes_encrypt(pAesDev, stream_id, input, output, len);
    else if(flag == DSC_DECRYPT )
        ret = aes_decrypt(pAesDev, stream_id, input, output, len);
    else{
        BOOT_PRINTF("dsc flag set error\n");
        return -1 ;
    }
     if(flag == DSC_ENCRYPT){
        BOOT_PRINTF("cipher pmi hash result:\n");
    	BOOT_DUMP(output, len);
     }else{
        BOOT_PRINTF("clear pmi hash result:\n");
    	BOOT_DUMP(output, len);
     }
     
	aes_ioctl(pAesDev ,IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, AesDevId);

	return (ret == RET_SUCCESS) ? 0 : -1;
}


static unsigned char hash_out_buf[HASH_OUT_LENGTH] __attribute__((  aligned( 16 )));
/* create pmi hash
@ index :  the PMI index, such as 1,2,3,4, pmi ofset is 256 pages 
*
* 2k pmi + 2k ddr + 4k bbt + 4byte flag + 32 bytes encyrpt pmi hash 
*/
static INT32 nand_pmi_hash_create( )
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *chip = nand->priv;

    UINT32 blk_len;
    UINT8*  blk_buf ,
          *  tmp_buf1,
          *  en_out_buf;
    INT32 ret;
    
    blk_len = nand->erasesize;
    blk_buf = g_pmi_blk;    
    
    ret = create_sha_ramdrv((UINT8*)((UINT32)blk_buf|0xa0000000), PMI_SIZE, hash_out_buf);   
    if(ret != RET_SUCCESS)
    {
       BOOT_PRINTF("pmi hash calculate failed \n");       
       return -1 ;
    }
    
    tmp_buf1= (UINT8*)malloc(HASH_OUT_LENGTH+0x20); // encrypt output 
    if(!tmp_buf1)
    {
       BOOT_PRINTF("Error: nand_pmi_hash_create malloc tmp_buf1: 0x%x failed \n",HASH_OUT_LENGTH+0x20);
        return -1 ;      
    }
    en_out_buf = (UINT8*)(((UINT32)tmp_buf1+0x1f)&0xfffffff0); // encrypt output 

    ret = nand_pmi_hash_crypt(hash_out_buf,en_out_buf, HASH_OUT_LENGTH,DSC_ENCRYPT) ;
    if(ret <0 )
    {
       BOOT_PRINTF("pmi hash encrypt failed \n");
       goto exit;
    }
        
    UINT32 mgc_flg = MAC_EN_MAGIC;
    MEMCPY(blk_buf+PMI_HASH_OFFSET,  &mgc_flg, 4 );
    MEMCPY(blk_buf+PMI_HASH_OFFSET+4,  en_out_buf, HASH_OUT_LENGTH );
    
    BOOT_PRINTF("pmi hash :\n");
	BOOT_DUMP((blk_buf+PMI_HASH_OFFSET), HASH_OUT_LENGTH +4);

	BOOT_PRINTF("blk_buf 0x%x\n",blk_buf);
	BOOT_DUMP(blk_buf,0x100);
	BOOT_PRINTF("save pmi hash to flash, ofset :0x%x , len: 0x%x \n", chip->PMIpos, blk_len);
	UINT32 ret_len = HASH_OUT_LENGTH +4;
    ret = nand_write_skip_bad(nand, (loff_t)(chip->PMIpos+PMI_HASH_OFFSET), &ret_len, (blk_buf+PMI_HASH_OFFSET), 0);
    if(ret <0 )
    {   
       BOOT_PRINTF("pmi hash save failed \n");
       goto exit;
    }
#if 0
    memset(blk_buf,0xff,blk_len );
    ret = nand_read_skip_bad(nand, (loff_t)(chip->PMIpos), &blk_len,blk_buf);
    if(ret <0)
    {
       BOOT_PRINTF("pmi hash double check failed \n");
       ret = -1 ;
       goto exit; 
    }
    BOOT_PRINTF("pmi hash after first write 0x%x :\n",blk_buf);
	BOOT_DUMP((blk_buf+PMI_HASH_OFFSET), HMAC_OUT_LENGTH +36);
    BOOT_DUMP((blk_buf), HMAC_OUT_LENGTH +36);    
#endif 
    
exit:
    free(tmp_buf1);
    return (ret=0) ;
}

 static INT32 nand_pmi_hash_verify()
{    
    UINT8*  blk_buf ,
           * tmp_buf1,
           * de_out_buf;
    INT32 ret = 0;
    
    blk_buf = g_pmi_blk;     

    ret = create_sha_ramdrv((UINT8*)((UINT32)blk_buf|0xa0000000), PMI_SIZE, hash_out_buf);    
    if(ret != RET_SUCCESS)
    {
       BOOT_PRINTF("pmi hash calculate failed \n");
       return -1;
    }        

    tmp_buf1 = (UINT8*)malloc(HASH_OUT_LENGTH+0x20); // decrypt output + R1 +R1
    if(!tmp_buf1)
    {
       BOOT_PRINTF("Error : Funcion[%s] malloc tmp_buf failed \n",__FUNCTION__);       
       return -1 ;
    }
    de_out_buf = (UINT8*)(((UINT32)tmp_buf1+0x1f) & 0xfffffff0);

    ret = nand_pmi_hash_crypt(blk_buf +PMI_HASH_OFFSET+4 ,de_out_buf, HASH_OUT_LENGTH,DSC_DECRYPT) ;
    if(ret <0 )
    {
       BOOT_PRINTF("pmi hash decrypt failed \n");
       goto exit;
    }

    ret = MEMCMP(de_out_buf, hash_out_buf, HASH_OUT_LENGTH);
    if( ret != 0 )
    {        
        BOOT_PRINTF("pmi hash cmp failed \n");
        BOOT_PRINTF("pmi hash calute result : \n");
	    BOOT_DUMP((UINT8*)(hash_out_buf), HASH_OUT_LENGTH ); 

        BOOT_PRINTF("pmi hash decrypt result : \n ");
	    BOOT_DUMP((UINT8*)(de_out_buf), HASH_OUT_LENGTH );
        
        ret = -1 ;
        goto exit;
    }            
    ret = 0;
exit:
      free(tmp_buf1);
      return ret ;

}

int api_nand_pmi_check()
{

    int ret = -1 ;
    UINT32 flg =0 ,tmp;

    ret = nand_pmi_hash_status();
    if( ret == 0 )  // not protect yet
    {
         tmp = nand_pmi_hash_create() ;
         if(tmp != 0){
             BOOT_PRINTF("pmi hash create failed\n");  
			 return -1 ;
         }
          tmp= nand_pmi_hash_verify() ;
          if(tmp != 0){
             BOOT_PRINTF("pmi hash first verify failed\n");   
			 return -1;
          }
		  
    }else if(ret == 1) // protected 
    {
        tmp= nand_pmi_hash_verify() ;
         if(0 != tmp )
         {
             BOOT_PRINTF("pmi hash verify failed\n");
             if( 0 != nand_pmi_hash_verify()) {
               // nand_pmi_erase(nand_dev)  ;   //if failed again erase pmi
                flg ++;
             }  
         }
    }else if(ret ==2 )  // empty pmi
         BOOT_PRINTF("pmi empty detect !\n");
    else
        return -1;
    ret = 0 ;     
    
    if(flg == 1)
        ret = -1 ;
    return ret ;
}

#endif 