#ifndef __ALI_MINIRPC_SERVICE_H
#define __ALI_MINIRPC_SERVICE_H

#include "ali_rpc_type.h"
#include "ali_rpc_cfg.h"
#include "ali_rpc_osdep.h"
#include "ali_rpc_debug.h"

typedef struct _Minirpc_TestStruct{
        Int32  ii;
        Uint32 uii;
        signed char   cc;
        Uchar  ucc;
        Long   ll;
        Ulong  ull;
        Bool   bb;
}Minirpc_TestStruct;

typedef struct _Param_Minirpc_Test
{
    Uint32 array[400];
    Uint32 x0;
	Uint32 x1;
    Uint32 x2;
    Uint32 x3;
}Param_Minirpc_Test;


typedef struct hld_device_rpc
{
    Uint32              HLD_DEV;
	Uint32				type;						/* Device type */
	Char				name[16];	/* Device name */
}Hld_device_rpc;


typedef enum _ParamId_MiniRpc {
    /****common define****/
	PARAM_MINIRPC_VOID,
	PARAM_MINIRPC_INT32,
	PARAM_MINIRPC_UINT32,
	PARAM_MINIRPC_LONG,
	PARAM_MINIRPC_ULONG,
	PARAM_MINIRPC_INT16,
	PARAM_MINIRPC_UINT16,
	PARAM_MINIRPC_BOOL,
	PARAM_MINIRPC_ENUM,
	PARAM_MINIRPC_ARRAY,
	PARAM_MINIRPC_BYTES,
	PARAM_MINIRPC_OPAQUE,
	PARAM_MINIRPC_STRING,
	PARAM_MINIRPC_UNION,
	PARAM_MINIRPC_CHAR,
	PARAM_MINIRPC_UCHAR,
	PARAM_MINIRPC_VECTOR,
	PARAM_MINIRPC_FLOAT,
	PARAM_MINIRPC_DOUBLE,
	PARAM_MINIRPC_REFERENCE,
	PARAM_MINIRPC_POINTER,
	PARAM_MINIRPC_WRAPSTRING,
	PARAM_MINIRPC_STRARRAY,
	PARAM_MINIRPC_RPCDBGINFO,
	/****common define*****/

	/**************** ce *******************/
	PARAM_MINIRPC_Ce_data_info_rpc,
	PARAM_MINIRPC_Otp_param_rpc,
	PARAM_MINIRPC_Data_param_rpc,
	PARAM_MINIRPC_Des_param_rpc,
	PARAM_MINIRPC_Ce_key_param_rpc,
	PARAM_MINIRPC_Ce_debug_key_info_rpc,
	PARAM_MINIRPC_Ce_pos_status_param_rpc,
	PARAM_MINIRPC_Ce_found_free_pos_param_rpc,
	PARAM_MINIRPC_Ce_pvr_key_param_rpc,

	/**************** dsc ******************/    
	PARAM_MINIRPC_DeEncrypt_config_rpc,
	PARAM_MINIRPC_Sha_init_param_rpc,
	PARAM_MINIRPC_Aes_init_param_rpc,
	PARAM_MINIRPC_Des_init_param_rpc,
	PARAM_MINIRPC_Pid_param_rpc,
	PARAM_MINIRPC_Csa_init_param_rpc,
	PARAM_MINIRPC_Dsc_pvr_key_param_rpc,
	PARAM_MINIRPC_Key_param_rpc,
	PARAM_MINIRPC_Sha_hash_rpc,
	PARAM_MINIRPC_Trng_data_rpc,

	/**************** hld ******************/  
	PARAM_MINIRPC_Hld_device_rpc,
    /*user struct define start*/
	PARAM_MINIRPC_TESTSTRUCT,
	
	PARAM_MINIRPC_TEST,
	
	PARAM_MINIRPC_ID_MAX
}ParamId_minirpc;

typedef Bool (*XdrOpFunc_minirpc)();

typedef struct _XdrOpTable_minirpc XdrOpTable_minirpc;
struct _XdrOpTable_minirpc {
	ParamId_minirpc id;
	XdrOpFunc_minirpc op;
};


/*Create Param usage*/
#define MINIRPC_PARAM_CREATE(param_name, param_type, param_id, param_len, data_ptr) \
			Param param_name; \
			param_name.type = param_type; \
			param_name.paramId = param_id;\
			param_name.len = param_len; \
			param_name.pData = data_ptr;


#define MINIRPC_PARAM_UPDATE(param_name, param_type, param_id, param_len, data_ptr) \
                        param_name.type = param_type; \
                        param_name.paramId = param_id;\
                        param_name.len = param_len; \
                        param_name.pData = data_ptr;

/************************************************************
*
*              User self define struct area
*
************************************************************/

/***************** CE ********************/
typedef enum CE_OTP_KEY_SEL_rpc
{
    OTP_KEY_0_0_r=0,
    OTP_KEY_0_1_r=1,
    OTP_KEY_0_2_r=2,
    OTP_KEY_0_3_r=3

}Ce_otp_key_sel_rpc;

typedef struct otp_param_rpc
{
    Uchar otp_addr;
    Ce_otp_key_sel_rpc  otp_key_pos;   
}Otp_param_rpc;

typedef struct data_param_rpc
{
	Uint32 crypt_data[4] ; 
	Uint32 data_len ;
}Data_param_rpc;

typedef enum CE_CRYPT_SELECT_rpc
{  
    CE_IS_DECRYPT_r = 1,
    CE_IS_ENCRYPT_r=0
}Ce_crypt_select_rpc;

typedef enum CE_MODULE_SELECT_rpc
{  
    CE_SELECT_AES_r = 1,
    CE_SELECT_DES_r=0
}Ce_module_select_rpc;

typedef struct  des_param_rpc
{
	Ce_crypt_select_rpc  crypt_mode;
	Ce_module_select_rpc aes_or_des;
	Uchar des_low_or_high;
}Des_param_rpc;

typedef enum CE_KEY_rpc
{
    KEY_0_0_r=0,
    KEY_0_1_r=1,
    KEY_0_2_r=2,
    KEY_0_3_r=3,
    KEY_1_0_r=4,
    KEY_1_1_r=5,
    KEY_1_2_r=6,
    KEY_1_3_r=7,
    KEY_2_0_r=8,
    KEY_2_1_r=9,
    KEY_2_2_r=0xa,
    KEY_2_3_r=0xb,
    KEY_2_4_r=0xc,
    KEY_2_5_r=0xd,
    KEY_2_6_r=0xe,
    KEY_2_7_r=0xf

}Ce_key_rpc;

typedef enum CE_CRYPT_TARGET_rpc
{
    CRYPT_KEY_1_0_r=0x4,
    CRYPT_KEY_1_1_r=0x5,
    CRYPT_KEY_1_2_r=0x6,
    CRYPT_KEY_1_3_r=0x7,

    CRYPT_KEY_2_0_r=0x8,
    CRYPT_KEY_2_1_r=0x9,
    CRYPT_KEY_2_2_r=0xa,
    CRYPT_KEY_2_3_r=0xb,
    CRYPT_KEY_2_4_r=0xc,
    CRYPT_KEY_2_5_r=0xd,
    CRYPT_KEY_2_6_r=0xe,
    CRYPT_KEY_2_7_r=0xf,

    CRYPT_KEY_3_0_r=0x10,
    CRYPT_KEY_3_1_r=0x11,
    CRYPT_KEY_3_2_r=0x12,
    CRYPT_KEY_3_3_r=0x13,
    CRYPT_KEY_3_4_r=0x14,
    CRYPT_KEY_3_5_r=0x15,
    CRYPT_KEY_3_6_r=0x16,
    CRYPT_KEY_3_7_r=0x17,
    CRYPT_KEY_3_8_r=0x18,
    CRYPT_KEY_3_9_r=0x19,
    CRYPT_KEY_3_10_r=0x1a,
    CRYPT_KEY_3_11_r=0x1b,
    CRYPT_KEY_3_12_r=0x1c,
    CRYPT_KEY_3_13_r=0x1d,
    CRYPT_KEY_3_14_r=0x1e,
    CRYPT_KEY_3_15_r=0x1f,

    HDCP_KEY_0_r=0x10,
    HDCP_KEY_1_r=0x11,
    HDCP_KEY_2_r=0x12,
    HDCP_KEY_3_r=0x13,
    HDCP_KEY_4_r=0x14,
    HDCP_KEY_5_r=0x15,
    HDCP_KEY_6_r=0x16,
    HDCP_KEY_7_r=0x17,
    HDCP_KEY_8_r=0x18,
    HDCP_KEY_9_r=0x19,
    HDCP_KEY_10_r=0x1a,
    HDCP_KEY_11_r=0x1b,
    HDCP_KEY_12_r=0x1c,
    HDCP_KEY_13_r=0x1d,
    HDCP_KEY_14_r=0x1e,
    HDCP_KEY_15_r=0x1f,
    HDCP_KEY_16_r=0x0f,
}Ce_crypt_target_rpc;

typedef enum  HDCP_DECRYPT_MODE_rpc
{
   NOT_FOR_HDCP_r=0,
   TARGET_IS_HDCP_KEY_SRAM_r=(1<<14)

}Hdcp_decrypt_mode_rpc;

typedef struct ce_key_param_rpc
{
	Ce_key_rpc first_key_pos;
	Ce_crypt_target_rpc second_key_pos;
	Hdcp_decrypt_mode_rpc hdcp_mode ;
}Ce_key_param_rpc;

typedef struct ce_data_info_rpc
{
	Otp_param_rpc otp_info;
	Data_param_rpc data_info;
	Des_param_rpc des_aes_info;
	Ce_key_param_rpc key_info; 
}Ce_data_info_rpc;

typedef enum HDCP_KEY_SELECT_rpc
{
   CE_KEY_READ_r=0,
   HDCP_KEY_READ_r=1
}Hdcp_key_select_rpc;

typedef struct ce_debug_key_info_rpc
{
    Hdcp_key_select_rpc sel; 
    Uint32 buffer[4];  
    Uint32 len;      
}Ce_debug_key_info_rpc;

typedef struct ce_pos_status_param_rpc
{
    Uint32 pos;
    Uint32 status; 
}Ce_pos_status_param_rpc;

typedef enum ce_key_level_rpc
{
    SKIP_LEVEL_r = 0 ,
    ONE_LEVEL_r,
    TWO_LEVEL_r,
    THREE_LEVEL_r
}Ce_key_level_rpc;

typedef struct ce_found_free_pos_param_rpc
{
    Uint32 pos;
    Ce_key_level_rpc ce_key_level_r; 
    Uchar number;
    Ce_otp_key_sel_rpc root;  
}Ce_found_free_pos_param_rpc;

typedef struct ce_pvr_key_param_rpc
{
    Uchar input_addr;
    Uint32 second_pos;  
    Uint32 first_pos;   
}Ce_pvr_key_param_rpc;

/******************* dsc ********************/
typedef struct deEncrypt_config_rpc
{
	Uint32 do_encrypt;
	Uint32 dec_dev;                /*Decrypt device for stream*/
    Uchar Decrypt_Mode;
	Uint16 dec_dmx_id;
    Uint32 do_decrypt ;
	Uint32 enc_dev;                /*Encrypt device for stream*/
    Uchar Encrypt_Mode;
	Uint16 enc_dmx_id;
}DeEncrypt_config_rpc;

typedef enum SHA_MODE_rpc
{
SHA_SHA_1_r= 0,
SHA_SHA_224_r=(1<<29),
SHA_SHA_256_r=  (2<<29),
SHA_SHA_384_r = (3<<29),
SHA_SHA_512_r  =(4<<29),
}Sha_mode_rpc_r;

typedef enum SHA_DATA_SOURCE_rpc
{
    SHA_DATA_SOURCE_FROM_DRAM_r =0,
    SHA_DATA_SOURCE_FROM_FLASH_r =1,
}Sha_data_source_rpc;

typedef struct sha_init_param_rpc
{
    Sha_mode_rpc_r sha_work_mode; 
    Sha_data_source_rpc sha_data_source;
    Uint32 sha_buf;
}Sha_init_param_rpc;

typedef enum PARITY_MODE_rpc
{
    EVEN_PARITY_MODE_r =0,
    ODD_PARITY_MODE_r =1,
    AUTO_PARITY_MODE0_r = 2,  /*for ts*/
    AUTO_PARITY_MODE1_r =3,
    OTP_KEY_FROM_68_r = 4,
    OTP_KEY_FROM_6C_r = 5,
}Parity_mode_rpc;

typedef enum KEY_TYPE_rpc
{
    KEY_FROM_REG_r=0,
    KEY_FROM_SRAM_r=1,
    KEY_FROM_CRYPTO_r=2,
    KEY_FROM_OTP_r= 3,
}Key_type_rpc;

typedef enum KEY_MAP_MODE_rpc
{
   CSA_MODE_r=0,
   DES_MODE_r=0,
   CSA3_MODE_r=1,
   AES_128BITS_MODE_r=1,
   TDES_ABA_MODE_r=1,
   AES_192BITS_MODE_r=2,
   TDES_ABC_MODE_r=2,
   AES_256BITS_MODE_r=3
}Key_map_mode_rpc;

typedef enum DMA_MODE_rpc
{
    PURE_DATA_MODE_r=0,
    TS_MODE_r=(1<<24),
}Dma_mode_rpc;

typedef enum RESIDUE_BLOCK_rpc
{
    RESIDUE_BLOCK_IS_NO_HANDLE_r = 0,
    RESIDUE_BLOCK_IS_AS_ATSC_r = (1 << 12),
    RESIDUE_BLOCK_IS_HW_CTS_r = (2 << 12),
    RESIDUE_BLOCK_IS_RESERVED_r = (3 << 12),
}Residue_block_rpc;

typedef enum WORK_MODE_rpc
{
    WORK_MODE_IS_CBC_r=  0,
    WORK_MODE_IS_ECB_r =   (1<<4),
    WORK_MODE_IS_OFB_r=  (2<<4),
    WORK_MODE_IS_CFB_r  =(3<<4),   
    WORK_MODE_IS_CTR_r  =(4<<4),  
}Work_mode_rpc;

typedef struct aes_init_param_rpc 
{
    Parity_mode_rpc parity_mode;    
    Key_type_rpc key_from;
    Uint32 scramble_control;
    Key_map_mode_rpc key_mode;
    Uint32 stream_id; 
    Dma_mode_rpc dma_mode;
    Residue_block_rpc  residue_mode;
    Work_mode_rpc work_mode;
    Uint32 cbc_cts_enable ;  
}Aes_init_param_rpc;

typedef struct des_init_param_rpc 
{
    Parity_mode_rpc parity_mode;    
    Key_type_rpc key_from;
    Uint32 scramble_control;
    Key_map_mode_rpc key_mode;
    Uint32 stream_id; 
    Dma_mode_rpc dma_mode;
    Residue_block_rpc  residue_mode;
    Work_mode_rpc work_mode;
    Uint32 cbc_cts_enable ;  
}Des_init_param_rpc;

typedef struct pid_param_rpc
{  
   Uint32 dmx_id ;
   Uint16 pid;
   Uint16 pos;
   Uchar key_addr;
}Pid_param_rpc;

typedef enum CSA_VERSION_rpc
{
    CSA1_r=1,
    CSA2_r=0,
    CSA3_r=2
}Csa_version_rpc;

typedef struct csa_init_param_rpc 
{
    Csa_version_rpc version;
    Dma_mode_rpc dma_mode;
    Uint32 Dcw[4];
    Uint32 pes_en;  

    Parity_mode_rpc  parity_mode;    
    Key_type_rpc key_from;
    Uint32 scramble_control;
    Uint32 stream_id;
    
}Csa_init_param_rpc;

typedef struct Dsc_pvr_key_param_rpc
{
	Uint32 input_addr;
    Uint32 valid_key_num;
    Uint32 current_key_num;
    Uint32 pvr_key_length;
	Uchar pvr_user_key_pos;
    Uint32 total_quantum_number; 
    Uint32 current_quantum_number;
    Uint32 ts_packet_number;
    Uchar pvr_key_change_enable;
    Uint16 stream_id;
}Dsc_pvr_key_param_rpc;

typedef struct AES_256Bit_KEY_rpc 
{
    Uchar even_key[32];
    Uchar odd_key[32];
}Aes_256bit_key_rpc;

typedef struct CSA3_KEY_rpc
{
   Uchar OddKey[16];
   Uchar EvenKey[16];
}Csa3_key_rpc;

typedef struct  DES_192BITS_KEY_INFO_rpc
{
   Uchar oddKey[24];
   Uchar EvenKey[24];
}Des_192bits_key_info_rpc;

typedef struct aes_iv_info_rpc
{
  Uchar even_iv[16];
  Uchar odd_iv[16];
}Aes_iv_info_rpc;

typedef struct des_iv_info_rpc
{
  Uchar even_iv[8];
  Uchar odd_iv[8];
}Des_iv_info_rpc;

typedef struct key_param_rpc
{
    Uint32 handle;
    Uint16 *pid_list;
    Uint16 pid_len;  
    void *p_aes_key_info;
    void *p_csa_key_info;
    void *p_des_key_info;
    Uint32 key_length;
    void *p_aes_iv_info;
    void *p_des_iv_info;
    Uint16 stream_id;
    
    Uchar *init_vector;
    Uchar *ctr_counter;
    Uchar force_mode;
    Uchar pos ;
    Uchar no_even;
    Uchar no_odd;
    Uchar not_refresh_iv;    
}Key_param_rpc;

typedef struct trng_data_rpc
{
    Uchar data[128];
}Trng_data_rpc;

typedef struct sha_hash_rpc
{
    Uchar hash[64];
}Sha_hash_rpc;




#endif
