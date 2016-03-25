#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/string.h>
#include <linux/ali_rpc.h>
#include <rpc_hld/ali_rpc_hld.h>
#include <rpc_hld/ali_rpc_hld_dsc.h>

#include <linux/dvb/ali_dsc.h>

#include "ali_dsc_lld.h"

#define INVALID_DSC_SUB_DEV_ID 0xFF
#define INVALID_DSC_STREAM_ID 0xFF

/* 
   crypt_mode -> 0 encrypt
   crypt_mode -> 1 decrypt 
*/
__s32 NAND_Crypto(__u8 crypt_mode, __u8 *input, __u8 *output, __u32 length, __u8 *key)
{
    __s32 ret = RET_FAILURE;
    __u32 stream_id = INVALID_DSC_STREAM_ID;
    pAES_DEV pAesDev = NULL;
    struct aes_init_param aes_param;
    KEY_PARAM key_param;
    AES_KEY_PARAM key_info[1];
    __u16 pid[1] = {0x1234};
   
    __u32 device_id = ali_dsc_get_free_sub_device_id(AES);
    if(device_id == INVALID_DSC_SUB_DEV_ID)
        return ret;
    pAesDev = (pAES_DEV)hld_dev_get_by_id(HLD_DEV_TYPE_AES, device_id);
    
    if ((stream_id = ali_dsc_get_free_stream_id(PURE_DATA_MODE)) == INVALID_DSC_STREAM_ID)
	{
        goto DONE1;
	}

    memset( &aes_param, 0, sizeof ( struct aes_init_param ) );
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_SRAM;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = EVEN_PARITY_MODE;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0 ;
    aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_ECB ;
    aes_param.cbc_cts_enable = 0;
    ret = ali_aes_ioctl ( pAesDev , IO_INIT_CMD , ( __u32 ) &aes_param );
    if(ret != RET_SUCCESS){
        goto DONE2;
    }

	memset(&key_param, 0, sizeof(KEY_PARAM));
    memcpy(key_info[0].aes_128bit_key.even_key, key, 16);
	key_param.handle = 0xFF ;
	key_param.ctr_counter = NULL; 
	key_param.init_vector = NULL;
	key_param.key_length = 128;  
	key_param.pid_len = 1; 				
	key_param.pid_list = pid; 			
	key_param.p_aes_key_info = key_info;
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
	ret = ali_aes_ioctl(pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (__u32)&key_param);
    if(ret != RET_SUCCESS){
        goto DONE2;
    }
    
    if(crypt_mode == 0)
        ret = ali_aes_encrypt( pAesDev, stream_id, input, output, length );
    else
        ret = ali_aes_decrypt( pAesDev, stream_id, input, output, length );
    
DONE3:
    ali_aes_ioctl( pAesDev, IO_DELETE_CRYPT_STREAM_CMD , key_param.handle);
DONE2:
    ali_dsc_set_stream_id_idle(stream_id);
DONE1:
    ali_dsc_set_sub_device_id_idle(AES, device_id);
    return ret;
}
EXPORT_SYMBOL(NAND_Crypto);

