#ifndef  _CRYPTO_H_
#define  _CRYPTO_H_

#include <linux/types.h>
#include <ali_magic.h>

#define OTP_ADDESS_1 0x4d
#define OTP_ADDESS_2 0x51
#define OTP_ADDESS_3 0x55
#define OTP_ADDESS_4 0x59
enum CE_OTP_KEY_SEL
{
    OTP_KEY_0_0=0,
    OTP_KEY_0_1=1,
    OTP_KEY_0_2=2,
    OTP_KEY_0_3=3

};

enum  HDCP_DECRYPT_MODE
{
   NOT_FOR_HDCP=0,
   TARGET_IS_HDCP_KEY_SRAM=(1<<14)

};

enum CE_CRYPT_TARGET
{
    CRYPT_KEY_1_0=0x4,
    CRYPT_KEY_1_1=0x5,
    CRYPT_KEY_1_2=0x6,
    CRYPT_KEY_1_3=0x7,

    CRYPT_KEY_2_0=0x8,
    CRYPT_KEY_2_1=0x9,
    CRYPT_KEY_2_2=0xa,
    CRYPT_KEY_2_3=0xb,
    CRYPT_KEY_2_4=0xc,
    CRYPT_KEY_2_5=0xd,
    CRYPT_KEY_2_6=0xe,
    CRYPT_KEY_2_7=0xf,

    CRYPT_KEY_3_0=0x10,
    CRYPT_KEY_3_1=0x11,
    CRYPT_KEY_3_2=0x12,
    CRYPT_KEY_3_3=0x13,
    CRYPT_KEY_3_4=0x14,
    CRYPT_KEY_3_5=0x15,
    CRYPT_KEY_3_6=0x16,
    CRYPT_KEY_3_7=0x17,
    CRYPT_KEY_3_8=0x18,
    CRYPT_KEY_3_9=0x19,
    CRYPT_KEY_3_10=0x1a,
    CRYPT_KEY_3_11=0x1b,
    CRYPT_KEY_3_12=0x1c,
    CRYPT_KEY_3_13=0x1d,
    CRYPT_KEY_3_14=0x1e,
    CRYPT_KEY_3_15=0x1f,

    HDCP_KEY_0=0x10,
    HDCP_KEY_1=0x11,
    HDCP_KEY_2=0x12,
    HDCP_KEY_3=0x13,
    HDCP_KEY_4=0x14,
    HDCP_KEY_5=0x15,
    HDCP_KEY_6=0x16,
    HDCP_KEY_7=0x17,
    HDCP_KEY_8=0x18,
    HDCP_KEY_9=0x19,
    HDCP_KEY_10=0x1a,
    HDCP_KEY_11=0x1b,
    HDCP_KEY_12=0x1c,
    HDCP_KEY_13=0x1d,
    HDCP_KEY_14=0x1e,
    HDCP_KEY_15=0x1f,
    HDCP_KEY_16=0x0f,
};

enum CE_KEY
{
KEY_0_0=0,
KEY_0_1=1,
KEY_0_2=2,
KEY_0_3=3,
KEY_1_0=4,
KEY_1_1=5,
KEY_1_2=6,
KEY_1_3=7,
KEY_2_0=8,
KEY_2_1=9,
KEY_2_2=0xa,
KEY_2_3=0xb,
KEY_2_4=0xc,
KEY_2_5=0xd,
KEY_2_6=0xe,
KEY_2_7=0xf

};

enum CE_IV_SELECT
{  
   IV_AS_ZERO = 0,
    IV_FROM_REGISTER=1
};

enum CE_CRYPT_SELECT
{  
    CE_IS_DECRYPT = 1,
    CE_IS_ENCRYPT=0
};

enum CE_MODULE_SELECT
{  
    CE_SELECT_AES = 1,
    CE_SELECT_DES=0
};


enum CE2DESC_KEY_SELECT
{
    DESC_KEY_2_0=0x0,
    DESC_KEY_2_1=0x1,
    DESC_KEY_2_2=0x2,
    DESC_KEY_2_3=0x3,
    DESC_KEY_2_4=0x4,
    DESC_KEY_2_5=0x5,
    DESC_KEY_2_6=0x6,
    DESC_KEY_2_7=0x7,
    DESC_KEY_2_8=0x8,
    DESC_KEY_2_9=0x9,
    DESC_KEY_2_10=0xa,
    DESC_KEY_2_11=0xb,
    DESC_KEY_2_12=0xc,
    DESC_KEY_2_13=0xd,
    DESC_KEY_2_14=0xe,
    DESC_KEY_2_15=0xf
};

enum HDCP_KEY_SELECT
{
   CE_KEY_READ=0,
   HDCP_KEY_READ=1
};

enum ce_key_level
{
    SKIP_LEVEL = 0 ,
    ONE_LEVEL,
    TWO_LEVEL,
    THREE_LEVEL
};

typedef struct ce_pvr_key_param
{
    UINT8 *input_addr;
    UINT32 second_pos;  
    UINT32 first_pos;   
}CE_PVR_KEY_PARAM, *pCE_PVR_KEY_PARAM;

typedef struct otp_param
{
    UINT8 otp_addr;
    enum CE_OTP_KEY_SEL  otp_key_pos;   
}OTP_PARAM, *pOTP_PARAM;

typedef struct data_param
{
	UINT32 crypt_data[4] ; /*for des 2 words, for aes 4 words*/
	UINT32 data_len ;
}DATA_PARAM, *pDATA_PARAM;

typedef struct  des_param
{
	enum CE_CRYPT_SELECT  crypt_mode;
	enum CE_MODULE_SELECT aes_or_des;
	UINT8 des_low_or_high;
}DES_PARAM, *pDES_PARAM;

typedef struct ce_key_param
{
	enum CE_KEY first_key_pos;
	enum CE_CRYPT_TARGET second_key_pos;
	enum  HDCP_DECRYPT_MODE hdcp_mode ;
}CE_KEY_PARAM, *pCE_KEY_PARAM;

typedef struct ce_debug_key_info
{
    enum HDCP_KEY_SELECT sel; /*CE_KEY_READ,HDC_KEY_READ*/
    UINT32 buffer[4];  /*debug result*/
    UINT32 len;      /* buffer length,UINT is 32bits, DES len shoud be 2; AES should be 4, HDCP mode must used AES submodule*/
}CE_DEBUG_KEY_INFO, *pCE_DEBUG_KEY_INFO;

typedef struct ce_data_info
{
	OTP_PARAM otp_info;
	DATA_PARAM data_info;
	DES_PARAM des_aes_info;
	CE_KEY_PARAM key_info; 
}CE_DATA_INFO, *pCE_DATA_INFO;

typedef struct ce_pos_status_param
{
    UINT32 pos;
    UINT32 status; 
}CE_POS_STS_PARAM, *pCE_POS_STS_PARAM;

typedef struct ce_found_free_pos_param
{
    UINT32 pos;
    enum ce_key_level ce_key_level; 
    UINT8 number;
}CE_FOUND_FREE_POS_PARAM, *pCE_FOUND_FREE_POS_PARAM;


typedef struct ce_device
{
	struct ce_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];

	void *pCePriv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	UINT32   interrupt_id;
	
	//ID event_flg_id;
	__s32 semaphore_id;
    __s32 semaphore_id2;
	
	void      (*attach)(void);
	void      (*detach)( struct ce_device *);
	RET_CODE	(*open)( struct ce_device *);
	RET_CODE   	(*close)( struct ce_device *);
	RET_CODE  (*ioctl)(struct ce_device *,UINT32 ,UINT32 );
	RET_CODE (*key_generate)(struct ce_device *,pCE_DATA_INFO );
	RET_CODE (*key_load)(struct ce_device *,pOTP_PARAM);	
	
}CE_DEVICE, *pCE_DEVICE;


#define ALI_CE_HLD_PARAM_MAX_SIZE 8
struct ali_ce_hld_param
{
    __u32 p[ALI_CE_HLD_PARAM_MAX_SIZE];    
};

#define IO_CE_BASE_KERNEL					(ALI_CE_MAGIC<<8)
#define  IO_OTP_ROOT_KEY_GET           						(IO_CE_BASE_KERNEL + 0)
#define  IO_SECOND_KEY_GENERATE        					(IO_CE_BASE_KERNEL + 1)
#define  IO_CRYPT_DATA_INPUT           						(IO_CE_BASE_KERNEL + 2)
#define  IO_CRYPT_PARAM_SET            						(IO_CE_BASE_KERNEL + 3)
#define  IO_CRYPT_SECOND_KEY_CONFIG    					(IO_CE_BASE_KERNEL + 4)
#define  IO_CRYPT_DEBUG_GET_KEY        					(IO_CE_BASE_KERNEL + 5)  /*only for debug*/
#define  IO_CRYPT_POS_IS_OCCUPY        					(IO_CE_BASE_KERNEL + 6)
#define  IO_CRYPT_POS_SET_USED         					(IO_CE_BASE_KERNEL + 7)
#define  IO_CRYPT_POS_SET_IDLE         					(IO_CE_BASE_KERNEL + 8)
#define  IO_CRYPT_FOUND_FREE_POS       					(IO_CE_BASE_KERNEL + 9)
#define  IO_DECRYPT_PVR_USER_KEY       					(IO_CE_BASE_KERNEL + 10)
#define  IO_CE_KEY_GENERATE            						(IO_CE_BASE_KERNEL + 11)     
#define  IO_CE_KEY_LOAD                                     			(IO_CE_BASE_KERNEL + 12)
#define  IO_CE_DES_KEY_GENERATE_TWO_LEVEL                    (IO_CE_BASE_KERNEL + 13)
#define  IO_CE_AES_KEY_GENERATE_TWO_LEVEL                    (IO_CE_BASE_KERNEL + 14)
#define  IO_CE_GENERATE_CW_KEY                              		(IO_CE_BASE_KERNEL + 15)
#define  IO_CE_GENERATE_SINGLE_LEVEL_KEY                    	(IO_CE_BASE_KERNEL + 16)
#define  IO_CE_GET_DEV_HLD                                  			(IO_CE_BASE_KERNEL + 17)

#endif  /*_CRYPTO_H_*/

