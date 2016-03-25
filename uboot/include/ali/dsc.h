/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: dsc.h
*   
*    Description: this file is used to define some macros and structures 
*                 for descrambler and scrambler 
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _DSC_H_
#define  _DSC_H_

#include <ali/hld_dev.h>
#include <ali/basic_types.h>
#ifdef __cplusplus 
extern "C" { 
#endif 
//1 io cmd
#define IO_INIT_CMD  0
#define IO_CREAT_CRYPT_STREAM_CMD  1
#define IO_DELETE_CRYPT_STREAM_CMD  2
#define IO_KEY_INFO_UPDATE_CMD  3 

#define IO_PARSE_DMX_ID_SET_CMD   0
#define IO_PARSE_DMX_ID_GET_CMD   1

#define IO_DSC_GET_DES_HANDLE     2
#define IO_DSC_GET_AES_HANDLE     3
#define IO_DSC_GET_CSA_HANDLE     4
#define IO_DSC_GET_SHA_HANDLE     5
#define IO_DSC_SET_PVR_KEY_PARAM  6
#define IO_DSC_ENCRYTP_BL_UK      7
#define IO_DSC_SET_PVR_KEY_IDLE   8
#define IO_DSC_VER_CHECK          9
#define IO_DSC_SET_ENCRYPT_PRIORITY 10
#define IO_DSC_GET_DRIVER_VERSION 11
#define IO_DSC_SET_CLR_CMDQ_EN 12
#define IO_DSC_DELETE_HANDLE_CMD 13

#define IO_DSC_FIXED_DECRYPTION 14
#define IO_DSC_SYS_UK_FW 15


#define DSC_TIMEOUT_FORVER		 0xFFFFFFFF
#define RAM_MON_SET_FLAG         0x00000002
#define RAM_MON_CLEAR_FLAG       0x00000001

#define ALI_INVALID_CRYPTO_STREAM_HANDLE  0xffffffff
#define ALI_INVALID_CRYPTO_STREAM_ID        0xff
#define ALI_INVALID_DSC_SUB_DEV_ID          0xff

#define TS_MAX_SIZE  0x10000  //64k
#define PURE_DATA_MAX_SIZE  0x4000000  //64k
#define INVALID_DSC_SUB_DEV_ID            0xff
#define INVALID_DSC_STREAM_ID            0xff
#define INVALID_CE_KEY_POS 0xff

#define	TS_PID_MASK 0x1FFF
#define VIRTUAL_DEV_NUM 4

#define TS_PACKET_SIZE 188

/*just for version check caller*/
#define CHECK_HEAD 0x1000
#define CHECK_END  0x0001
#define CHUNKID_UBOOT        0x01FE0101
#define CHUNKID_KERNEL       0x06F90101
#define CHUNKID_FW           0x06F90101
#define CHUNKID_SEE          0x00FF0100
#define CHUNKID_BL           0x10000123

#define SHA1_HASH_SIZE 		20
#define SHA224_HASH_SIZE 	28
#define SHA256_HASH_SIZE 	32
#define SHA384_HASH_SIZE 	48
#define SHA512_HASH_SIZE 	64

#ifndef offsetof
#define offsetof(type, f) ((unsigned long) \
	((char *)&((type *)0)->f - (char *)(type *)0))
#endif


enum CSA_VERSION
{
    CSA1=1,
    CSA2=0,
    CSA3=2
};

enum SHA_MODE
{
SHA_SHA_1= 0,
SHA_SHA_224=(1<<29),
SHA_SHA_256=  (2<<29),
SHA_SHA_384 = (3<<29),
SHA_SHA_512  =(4<<29),
};

enum SHA_DATA_SOURCE
{
    SHA_DATA_SOURCE_FROM_DRAM =0,
    SHA_DATA_SOURCE_FROM_FLASH =1,
};

enum PARITY_MODE
{
    EVEN_PARITY_MODE  =0,
    ODD_PARITY_MODE =1,
    AUTO_PARITY_MODE0= 2,  /*for ts*/
    AUTO_PARITY_MODE1=3,
    OTP_KEY_FROM_68 = 4,
    OTP_KEY_FROM_6C = 5,
};

enum KEY_TYPE
{
    KEY_FROM_REG=0,
    KEY_FROM_SRAM=1,
    KEY_FROM_CRYPTO=2,
    KEY_FROM_OTP = 3,
};

enum KEY_MAP_MODE
{
   CSA_MODE=0,
   DES_MODE=0,
   CSA3_MODE=1,
   AES_128BITS_MODE=1,
   TDES_ABA_MODE=1,
   CRYPTO_128BITS_MODE=1,
   AES_192BITS_MODE=2,
   TDES_ABC_MODE=2,
   AES_256BITS_MODE=3
};

enum KEY_BIT_NUM
{
	BIT_NUMOF_KEY_64 = 64,
	BIT_NUMOF_KEY_128 = 128,
	BIT_NUMOF_KEY_192 = 192,
	BIT_NUMOF_KEY_256 = 256,
};

enum DMA_MODE
{
    PURE_DATA_MODE=0,
    TS_MODE=(1<<24),
};

enum RESIDUE_BLOCK
{
    RESIDUE_BLOCK_IS_NO_HANDLE = 0,
    RESIDUE_BLOCK_IS_AS_ATSC = (1 << 12),
    RESIDUE_BLOCK_IS_HW_CTS = (2 << 12),
    RESIDUE_BLOCK_IS_RESERVED = (3 << 12),
};


enum WORK_MODE
{
    WORK_MODE_IS_CBC=  0,
    WORK_MODE_IS_ECB =   (1<<4),
    WORK_MODE_IS_OFB=  (2<<4),
    WORK_MODE_IS_CFB  =(3<<4),   
    WORK_MODE_IS_CTR  =(4<<4),  /*only for aes*/
};

enum WORK_SUB_MODULE
{
    DES=0,
    TDES=3,
    AES=1,
    SHA=2,
    CSA=4,
};

enum CRYPT_SELECT
{//make the value same with keyladder define.
    DSC_DECRYPT = 1,
    DSC_ENCRYPT = 0
};

enum CE_OTP_DIR_MODE
{
	OTP_KEY_0_6_R = 0,
	OTP_KEY_0_6 = 1,
	OTP_KEY_0_7 = 2,
	OTP_KEY_0_7_R = 3
};

typedef enum dsc_ret_status
{
	ALI_DSC_SUCCESS = 0,
	ALI_DSC_ERROR,
	ALI_DSC_WARNING_DRIVER_ALREADY_INITIALIZED,

	ALI_DSC_ERROR_INVALID_PARAMETERS = 0x1F00,
	ALI_DSC_ERROR_OPERATION_NOT_ALLOWED,
	ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED,
	ALI_DSC_ERROR_INITIALIZATION_FAILED,
	ALI_DSC_ERROR_DRIVER_NOT_INITIALIZED,
	ALI_DSC_ERROR_INVALID_ADDR,
	ALI_DSC_ERROR_INVALID_DEV,
	ALI_DSC_ERROR_INVALID_HANDLE,
	ALI_DSC_ERROR_NO_IDLE_HANDLE,
	ALI_DSC_ERROR_NO_IDLE_PIDRAM,
	ALI_DSC_ERROR_NO_IDLE_KEYRAM,
}DSC_STATUS;
//1 DES INIT PARAM
typedef struct des_init_param 
{
    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    UINT32 scramble_control;
    enum KEY_MAP_MODE key_mode;
    UINT32 stream_id; /**which stream id is working*/
    enum DMA_MODE dma_mode;
    enum RESIDUE_BLOCK  residue_mode;
    enum WORK_MODE work_mode;
    enum WORK_SUB_MODULE sub_module;
	UINT32 cbc_cts_enable ;  /*for pure data*/
}DES_INIT_PARAM, *pDES_INIT_PARAM;


//1 AES INIT PARAM
typedef struct aes_init_param 
{
    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    UINT32 scramble_control;
    enum KEY_MAP_MODE key_mode;
    UINT32 stream_id; /**which stream id is working*/
    enum DMA_MODE dma_mode;
    enum RESIDUE_BLOCK  residue_mode;
    enum WORK_MODE work_mode;
    UINT32 cbc_cts_enable ;  /*for pure data*/
}AES_INIT_PARAM, *pAES_INIT_PARAM;


//1 CSA INIT PARAM
typedef struct csa_init_param 
{
    enum CSA_VERSION version;
    enum DMA_MODE dma_mode;/*pure_data, or ts*/	 
    UINT32 Dcw[4];  /*for csa only used Dcw[0]Dcw[1], for csa3 used all*/
    UINT32 pes_en;  

    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    UINT32 scramble_control;
    UINT32 stream_id; /**which stream id is working*/  

}CSA_INIT_PARAM, *pCSA_INIT_PARAM;


//1 SHA INIT PARAM
typedef struct sha_init_param
{
  enum SHA_MODE sha_work_mode; 
  enum SHA_DATA_SOURCE sha_data_source;
} SHA_INIT_PARAM, *pSHA_INIT_PARAM;

typedef struct crypto_128bit_key
{
	UINT8 even_key[16];
	UINT8 odd_key[16];
}CRYPTO_128BITS_KEY;

typedef struct crypto_128bit_iv
{
	UINT8 even_iv[16];
	UINT8 odd_iv[16];
}CRYPTO_128BITS_IV;
struct AES_128Bit_KEY 
{
UINT8 even_key[16];
UINT8 odd_key[16];
};

typedef struct AES_128Bit_KEY AES_128BITS_KEY;
struct AES_192Bit_KEY 
{
UINT8 even_key[24];
UINT8 odd_key[24];
};

typedef struct AES_192Bit_KEY AES_192BITS_KEY;
struct AES_256Bit_KEY 
{
UINT8 even_key[32];
UINT8 odd_key[32];
};

typedef struct AES_256Bit_KEY AES_256BITS_KEY;
typedef union aes_key_param
{
    struct AES_128Bit_KEY aes_128bit_key ;
    struct AES_192Bit_KEY aes_192bit_key ;
    struct AES_256Bit_KEY aes_256bit_key ;
   
}AES_KEY_PARAM;

typedef struct aes_iv_info
{
  UINT8 even_iv[16];
  UINT8 odd_iv[16];
}AES_IV_INFO;

struct  AES_KEY
{
   UINT8 OddKey[16];
   UINT8 EvenKey[16];
};

struct  CSA_KEY
{
   UINT8 OddKey[8];
   UINT8 EvenKey[8];
};

typedef struct CSA_KEY CSA_64BITS_KEY;
struct  CSA3_KEY
{
   UINT8 OddKey[16];
   UINT8 EvenKey[16];
};

typedef struct CSA3_KEY CSA3_128BITS_KEY;
typedef union csa_key_param
{
    struct  CSA_KEY csa_key ;
    struct  CSA3_KEY csa3_key ;
}CSA_KEY_PARAM;

struct  DES_64BITS_KEY_INFO
{
   UINT8 OddKey[8];
   UINT8 EvenKey[8];
};

typedef struct DES_64BITS_KEY_INFO DES_64BITS_KEY;
struct  DES_128BITS_KEY_INFO
{
   UINT8 OddKey[16];
   UINT8 EvenKey[16];
};

typedef struct DES_128BITS_KEY_INFO TDES_128BITS_KEY;
struct  DES_192BITS_KEY_INFO
{
   UINT8 OddKey[24];
   UINT8 EvenKey[24];
};

typedef struct DES_192BITS_KEY_INFO TDES_192BITS_KEY;
typedef union des_key_param
{
   struct  DES_64BITS_KEY_INFO  des_64bits_key ;
   struct  DES_128BITS_KEY_INFO des_128bits_key ;
   struct  DES_192BITS_KEY_INFO des_192bits_key ;
   
}DES_KEY_PARAM;

typedef struct des_iv_info
{
  UINT8 even_iv[8];
  UINT8 odd_iv[8];
}DES_IV_INFO;

//1 KEY INFO PARAM
typedef struct  
{
	UINT32 handle ;  /* out parameter*/
	UINT16 *pid_list;
	UINT16 pid_len;
	AES_KEY_PARAM *p_aes_key_info; /*for ts data mode*/
	CSA_KEY_PARAM *p_csa_key_info;
	DES_KEY_PARAM *p_des_key_info;
	UINT32 key_length;
	AES_IV_INFO *p_aes_iv_info;
	DES_IV_INFO *p_des_iv_info;
	UINT16 stream_id;
	UINT8 *init_vector; 
	UINT8 *ctr_counter;
	UINT8 force_mode; 
	UINT8 pos ;
	UINT8 no_even;
	UINT8 no_odd;
	UINT8 not_refresh_iv;
}KEY_PARAM, *pKEY_PARAM;


typedef struct crypt_init_nos
{
	enum PARITY_MODE parity_mode;    
	enum KEY_TYPE key_from;
	UINT32 scramble_control;
	enum KEY_MAP_MODE key_mode;
	UINT32 stream_id; /**which stream id is working*/
	enum DMA_MODE dma_mode;
	enum RESIDUE_BLOCK residue_mode;
	enum WORK_MODE work_mode;
	enum WORK_SUB_MODULE sub_module;
	UINT32 cbc_cts_enable ;  /*for pure data*/

	KEY_PARAM *pkey_param;
}INIT_PARAM_NOS;

//1 SHA DEVICE
typedef struct sha_device
{
    struct sha_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
    void *pRoot;
    void *priv ; 
    void (*open)(struct sha_device *);
    void (*close)(struct sha_device *);
    RET_CODE (*digest)(struct sha_device * , UINT8 *, UINT8 *,UINT32 );
    RET_CODE  (*Ioctl)( struct sha_device * ,UINT32 cmd , UINT32 param);
    UINT8 id_number;	
}SHA_DEV,*pSHA_DEV;

//1 AES DEVICE
typedef struct aes_device
{
    struct aes_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
	INIT_PARAM_NOS *p_info_nos; /* Only For NOS AS */
    void *priv ;
    void (*open)(struct aes_device *);
    void (*close)(struct aes_device *);
    RET_CODE (*Encrypt)(struct aes_device *p_aes_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*Decrypt)(struct aes_device *p_aes_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*Ioctl)(struct aes_device *p_aes_dev, UINT32 cmd, UINT32 param);
    UINT8 id_number;
	
}AES_DEV,*pAES_DEV;


//1 DES DEVICE
typedef struct des_device
{
    struct des_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
	INIT_PARAM_NOS *p_info_nos; /* Only For NOS AS */
    void *priv ;
    void (*open)(struct des_device *p_des_dev );
    void (*close)(struct des_device *p_des_dev );
    RET_CODE (*Encrypt)(struct des_device *p_des_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*Decrypt)(struct des_device *p_des_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*Ioctl)(struct des_device *p_des_dev, UINT32 cmd, UINT32 param);
    UINT8 id_number;
}DES_DEV,*pDES_DEV;


//1 CSA DEVICE
typedef struct csa_device
{
    struct csa_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
    void *p_root;        
    void *priv ;
    void (*open)(struct csa_device*);
    void (*close)(struct csa_device*);
    RET_CODE (*Decrypt)(struct csa_device*,UINT16, UINT8*, UINT8*, UINT32 );
    RET_CODE (*Ioctl)(struct csa_device *,UINT32 cmd,UINT32 param);
    UINT8 id_number;
}CSA_DEV,*pCSA_DEV;


//1 DSC DEVICE
typedef struct descrambler_device
{
	struct descrambler_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	void *priv;		/*only point to SHA */
	UINT32 base_addr;
	UINT32  interrupt_id;

	
    void      (*attach)(void);
    void      (*detach)( struct descrambler_device *);
    RET_CODE	(*open)( struct descrambler_device *);
    RET_CODE   	(*close)( struct descrambler_device *);
    RET_CODE   (*ioctl)(struct descrambler_device *, UINT32 , UINT32 );
   
}DSC_DEV, *pDSC_DEV;

typedef struct DSC_PVR_KEY_PARAM
{
	UINT32 input_addr;
    UINT32 valid_key_num;
    UINT32 current_key_num;
    UINT32 pvr_key_length;
	UINT8 pvr_user_key_pos;
    UINT32 total_quantum_number; 
    UINT32 current_quantum_number;
    UINT32 ts_packet_number;
    UINT8 pvr_key_change_enable;
    UINT16 stream_id;
	UINT8 pvr_first_key_pos;
}DSC_PVR_KEY_PARAM,*pDSC_PVR_KEY_PARAM;



typedef struct DSC_BL_UK_PARAM
{
	UINT8 *input_key;
    UINT8 *r_key;
    UINT8 *output_key;
    UINT32 crypt_type;
}DSC_BL_UK_PARAM,*pDSC_BL_UK_PARAM;

typedef struct HW_CHECK_USER_NODE
{
    UINT32 startaddr;
    UINT32 length;    
}HW_CHK_NODE;

typedef struct HW_CHECK_USER_CONFIG
{
    UINT32 nodecount; // for SC mode only, should <= 65536
    HW_CHK_NODE *pnodelist;  // nodes list 
}HW_CHK_USER_CFG;

typedef struct DSC_VER_CHK_PARAM
{
	UINT32 input_mem;
    UINT32 len;
    UINT32 chk_mode; 
    UINT32 chk_id ;
}DSC_VER_CHK_PARAM,*pDSC_VER_CHK_PARAM;

typedef struct DeEncrypt_config
{
	UINT32 do_encrypt ;
	void *dec_dev;                /*Decrypt device for stream*/
    UINT8 Decrypt_Mode;
	UINT16 dec_dmx_id;
    UINT32 do_decrypt ;
	void *enc_dev;                /*Encrypt device for stream*/
    UINT8 Encrypt_Mode;
	UINT16 enc_dmx_id;
}DEEN_CONFIG,*pDEEN_CONFIG;

typedef struct dsc_en_priority
{
	enum WORK_SUB_MODULE sub_module;
	enum PARITY_MODE priority;
    UINT32 dev_ptr; 
}DSC_EN_PRIORITY,*pDSC_EN_PRIORITY;

typedef struct dsc_fixed_cryption_param
{
	UINT8 *input;
	UINT32 length; 
	UINT32 pos;
}DSC_FIXED_CRYPTION,*p_dsc_fixed_cryption;

typedef struct dsc_deal_sys_fw
{
	UINT8 *input;
	UINT8 *output;
	UINT32 length; 
	UINT8 ck[16];
	UINT8 pos;
	enum CRYPT_SELECT mode;
}DSC_SYS_UK_FW;

/*****************************************************************************
 * Function: dsc_api_attach
 * Description: 
 *    This function initializes the DSC hardware and software structures.
 *	  This function has to be called once before any other function call of DSC.
 * Input: 
 *      None
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_api_attach(void);

/*****************************************************************************
 * Function: dsc_api_detach
 * Description: 
 *    This function is used to terminal DSC low level driver and release the DSC occupied resources.
 *    If dsc_api_detach is performed, DSC functions should not be called.
 * Input: 
 *      None
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_api_detach(void);

/*****************************************************************************
 * Function: des_decrypt
 * Description: 
 *    This function is used to implement DES/TDES decryption.
 * Input: 
 *      DES_DEV *p_des_dev		DES device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input	Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number;
 *			For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE des_decrypt(DES_DEV *p_des_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_decrypt_rpc
 * Description: 
 *    This function is just used to specify the DES/TDES decryption poniter for RPC function list. 
 * Input: 
 *      DES_DEV 	*p_des_dev 	DES device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input	Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number;
 *			For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE des_decrypt_rpc(DES_DEV *p_des_dev,UINT16 stream_id,  UINT8 *input, 
			UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_encrypt
 * Description: 
 *    This function is used to implement DES/TDES encryption.
 * Input: 
 *      DES_DEV *p_des_dev 	DES device pointer.
 *		UINT16 stream_id	Stream ID for current encryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE des_encrypt(DES_DEV *p_des_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_encrypt_rpc
 * Description: 
 *    This function is just used to specify the DES/TDES encryption poniter for RPC function list.
 * Input: 
 *      DES_DEV *p_des_dev 	DES device pointer.
 *		UINT16 stream_id	Stream ID for current encryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE des_encrypt_rpc ( DES_DEV *p_des_dev, UINT16 stream_id, UINT8 *input,
			UINT8 *output, UINT32 total_length );

/*****************************************************************************
 * Function: des_ioctl
 * Description: 
 *    This function is used to implement some DES IO controls.
 * Input: 
 *      DES_DEV *p_des_dev 	DES device pointer.
 *		UINT32 cmd			IO control commands defined in above. 
 *      UINT32 param		Parameters defined in des_init_param etc.
 * Output: 
 *      None
 * 
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE des_ioctl( DES_DEV *p_des_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: aes_decrypt
 * Description: 
 *    This function is used to implement AES decryption.
 * Input: 
 *      AES_DEV *p_aes_dev 	AES device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt(AES_DEV *p_aes_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_decrypt_rpc
 * Description: 
 *    This function is just used to specify the AES decryption poniter for RPC function list.
 * Input: 
 *      AES_DEV *p_aes_dev 	AES device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt_rpc(AES_DEV *p_aes_dev,UINT16 stream_id,  UINT8 *input, 
			UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_encrypt
 * AEScription: 
 *    This function is used to implement AES/TAES encryption.
 * Input: 
 *      AES_DEV *p_aes_dev 	AES device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_encrypt(AES_DEV *p_aes_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_encrypt_rpc
 * Description: 
 *    This function is just used to specify the AES encryption poniter for RPC function list.
 * Input: 
 *      p_aes_dev p_aes_dev 	AES device pointer.
 *		UINT16 stream_id	Stream ID for current encryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_encrypt_rpc(AES_DEV *p_aes_dev,UINT16 stream_id,  UINT8 *input, 
			UINT8 *output, UINT32 total_length);


/*****************************************************************************
 * Function: aes_ioctl
 * Description: 
 *    This function is used to implement some AES IO controls.
 * Input: 
 *      AES_DEV *p_aes_dev 	AES device pointer.
 *		UINT32 cmd			IO control commands defined in above. 
 *      UINT32 param		Parameters defined in aes_init_param etc.
 * Output: 
 *      None
 * 
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_ioctl( AES_DEV *p_aes_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: csa_decrypt
 * Description: 
 *    This function is used to implement CSA decryption.
 * Input: 
 *      CSA_DEV *p_csa_dev 	CSA device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE csa_decrypt(CSA_DEV *p_csa_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: csa_decrypt_rpc
 * Description: 
 *    This function is just used to specify the CSA decryption poniter for RPC function list.
 * Input: 
 *      CSA_DEV *p_csa_dev 	CSA device pointer.
 *		UINT16 stream_id	Stream ID for current decryption.
 *		UINT8 *input		Input data address. *		
 *		UINT32 total_length	Data length. For TS data, it should be the total packets number; 
 *		For raw data, it should be the total bytes number.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/

RET_CODE csa_decrypt_rpc ( CSA_DEV *p_csa_dev, UINT16 stream_id, UINT8 *input, 
			UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: CSA_ioctl
 * Description: 
 *    This function is used to implement some CSA IO controls.
 * Input: 
 *      CSA_DEV *p_csa_dev 	CSA device pointer.
 *		UINT32 cmd			IO control commands defined in above. 
 *      UINT32 param		Parameters defined in CSA_init_param etc.
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE csa_ioctl( CSA_DEV *p_csa_dev , UINT32 cmd , UINT32 param);


/*****************************************************************************
 * Function: SHA_ioctl
 * Description: 
 *    This function is used to implement some SHA IO controls.
 * Input: 
 *      SHA_DEV *p_sha_dev 	SHA device pointer.
 *		UINT32 cmd			IO control commands defined in above. 
 *      UINT32 param		Parameters defined in SHA_init_param etc.
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE sha_ioctl( SHA_DEV *p_sha_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: sha_digest
 * Description: 
 *      This function is used to generate a digest for the input data by using SHA module.
 * Input: 
 *      SHA_DEV *p_sha_dev	 SHA device pointer.
 *		UINT8 *input		Input data address.
 *		UINT32 data_length	Specify the data size in byte.
 * Output: 
 *      UINT8 *output	Output data address.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE sha_digest(SHA_DEV *p_sha_dev, UINT8 *input,
                    UINT8 *output, UINT32 data_length);

/*****************************************************************************
 * Function: dsc_ioctl
 * Description: 
 *    This function is used to implement DSC IO control.
 * Input: 
 *      DSC_DEV *p_dsc_dev 	DSC device pointer.
 *		UINT32 cmd			IO control commands defined in above. 
 *      UINT32 param		Parameters pointer.
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE dsc_ioctl( DSC_DEV *p_dsc_dev, UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: trig_ram_mon
 * Description: 
 *    This function used to enable the memory monitor feature
 * Input: 
 *      UINT32 start_addr: the monitor start address.
 *      UINT32 end_addr: the monitor end address.
 *      UINT32 interval: the interval time of each monitor checking.
 *      enum SHA_MODE sha_mode: Memory monitor SHA mode
 *		BOOL DisableOrEnable: reserved for further using
 * Output: 
 *      None
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE trig_ram_mon(UINT32 start_addr, UINT32 end_addr, UINT32 interval,
                      enum SHA_MODE sha_mode, BOOL disable_or_enable);

/*****************************************************************************
 * Function: DeEncrypt
 * Description: 
 *    This function is used to re-encrypt the transport stream packets.
 * Input: 
 *      DEEN_CONFIG *p_de_en	Re-encryption configuration.
 *		UINT8 *input	Input TS data address.
 *		UINT32 total_length	Data length in TS packets.
 * Output: 
 *		UINT8 *output	Output TS data address. 
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE de_encrypt(DEEN_CONFIG *p_de_en, UINT8 *input,
                   UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: dsc_get_free_stream_id
 * Description: 
 *    This function is used to get an idle stream ID with specified data mode. 
 *	  If the function is performed without error, it will return 4~7 for raw data mode;
 *	  it will return 0~3 or 8~15 (not available on the M3603/M3281) for TS data mode.
 * Input: 
 *      enum DMA_MODE dma_mode	Specify the data mode, for raw data or TS data.
 * Output: 
 *      UINT16 return value
 * 
 * Returns: 
 *		Success: return a free stream id number which can be used
 * 		Fail:	 return ALI_INVALID_CRYPTO_STREAM_ID
*****************************************************************************/
UINT16 dsc_get_free_stream_id(enum DMA_MODE dma_mode);


/*****************************************************************************
 * Function: dsc_get_free_sub_device_id
 * Description: 
 *    This function is used to get an idle device ID of the specified sub-module. 
 * Input: 
 *    enum WORK_SUB_MODULE sub_mode	Specify the sub-module.
 * Output: 
 *      Equal to the return value UINT32
 * 
 * Returns: 
 *		Success: return a free sub device id which can be used
 *      Fail:    return ALI_INVALID_DSC_SUB_DEV_ID
*****************************************************************************/
UINT32 dsc_get_free_sub_device_id(enum WORK_SUB_MODULE sub_mode);

/*****************************************************************************
 * Function: dsc_set_sub_device_id_idle
 * Description: 
 *    This function is used to free the specified device ID of the specified sub-module.
 * Input: 
 *      enum WORK_SUB_MODULE sub_mode	Specify the sub-module.
 *		UINT32 device_id	Specify the device ID to be freed.
 * Output: 
 *      None
 * 
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_set_sub_device_id_idle(enum WORK_SUB_MODULE sub_mode,
                                    UINT32 device_id);

/*****************************************************************************
 * Function: dsc_set_sub_device_id_used
 * Description: 
 *    This function is used to occupy the specified device id of the specified sub-module.
 * Input: 
 *      enum WORK_SUB_MODULE sub_mode	Specify the sub-module.
 *		UINT32 device_id	Specify the device ID to be used.
 * Output: 
 *      None
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 ****************************************************************************/
RET_CODE dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode,
                                    UINT32 device_id);

/*****************************************************************************
 * Function: dsc_set_stream_id_idle
 * Description: 
 *    This function is used to free the specified stream ID.
 * Input: 
 *      UINT32 pos	Specify the value.
 * Output: 
 *      None
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE dsc_set_stream_id_idle(UINT32 pos);

/*****************************************************************************
 * Function: dsc_set_stream_id_used
 * Description: 
 *    This function is used to occupy the specified stream ID.
 * Input: 
 *      UINT32 pos	Specify the value.
 * Output: 
 *      None
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
void dsc_set_stream_id_used(UINT32 pos);

/*****************************************************************************
 * Function: dsc_deal_quantum_for_mixed_ts
 * Description: 
 *    This function is used to change key for PVR by quantum
 * Input: 
 *      DEEN_CONFIG *p_de_en: Re-encryption configuration.
 *      UINT32 temp_length: the ts packet number of each quantum.
 * Output: 
 *      None
 * Returns: 
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_deal_quantum_for_mixed_ts(DEEN_CONFIG *p_de_en,
                                       UINT32 temp_length);

/*****************************************************************************
 * Function: aes_crypt_puredata_with_ce_key
 * Description: 
 *    This function is used to decrypt or encrypt the puredata use key from crypto engine
 * Input: 
 *			u8 *input,                the input data
 *			u32 length,               the data length
 *			u32 key_pos,              the key pos in crypto engine
 *			enum CRYPT_SELECT sel     decrypt or encrypt select
 * Output: 
 *			u8 *output,               the output data 
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE aes_crypt_puredata_with_ce_key(UINT8 *input, UINT8 *output,
                                        UINT32 length, UINT32 key_pos,
                                        enum CRYPT_SELECT sel);


/*****************************************************************************
 * Function: aes_pure_ecb_crypt
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with ECB mode using key from DSC sram (host key)
 * Input: 
 *		UINT8 *key:		the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_ecb_crypt( UINT8 *key, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_cbc_crypt
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with CBC mode using key from DSC sram (host key)
 * Input: 
 *		UINT8 *key:		the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_cbc_crypt( UINT8 *key, UINT8 *iv, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_ctr_crypt
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with CTR mode using key from DSC sram (host key)
 * Input: 
 *		UINT8 *key:		the crypt key pointer
 *		UINT8 *ctr:		the ctr buffer pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_ctr_crypt( UINT8 *key, UINT8 *ctr, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);


/*****************************************************************************
 * Function: tdes_pure_ecb_crypt
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with ECB mode 
 *	  using key from DSC sram (host key)
 * Input: 
 *		UINT8 *key:		the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE tdes_pure_ecb_crypt( UINT8 *key, UINT8 *input, UINT8 *output,
                              UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: tdes_pure_ecb_crypt_with_ce_key
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with ECB mode 
 *	  using key from crypto engine sram
 * Input: 
 *		UINT8 key_pos:	the crypt key position in ecrypto engine
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE tdes_pure_ecb_crypt_with_ce_key( UINT8 key_pos, UINT8 *input,
        UINT8 *output, UINT32 length,
        UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_ctr_crypt_with_ce_key
 * Description: 
 *    This function is used to decrypt or encrypt the puredata with CTR mode 
 *	  using key from crypto engine sram
 * Input: 
 *		UINT8 key_pos:	the crypt key position in ecrypto engine
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *		UINT8 crypt_mode: the crypt mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE aes_pure_ctr_crypt_with_ce_key( UINT8 key_pos, UINT8 *ctr,
        UINT8 *input, UINT8 *output,
        UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: ali_dsc_encrypt_bl_uk
 * Description: 
 *    This function is used to encrypt the Bootloader universal key
 * Input: 
 *      UINT8 *input:   the input buffer pointer
 *      UINT8 *r_key:   the random number buffer pointer
 *		UINT32 encrypt_type: the crypt mode using which key and which mode
 * Output: 
 *		UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE ali_dsc_encrypt_bl_uk(UINT8 *input, UINT8 *r_key,
        UINT8 *output, UINT32 encrypt_type);


/*****************************************************************************
 * Function: see_version_check
 * Description: 
 *	This function is used to do the version checking of software
 *	This function only is available in SEE software
 * Input: 
 *			UINT32 block_id,      the check block id number
 *			UINT32 block_addr,    the check block address in dram
 *			UINT32 block_len,     the check block length
 * Output: 
 *			None
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE see_version_check(UINT32 block_id, UINT32 block_addr, UINT32 block_len);

/*****************************************************************************
 * Function: ali_sha_digest
 * Description: 
 *	This function is used to caculate the digest of input data with specified SHA mode
 * Input: 
 *			UINT8 *input,               the input buffer pointer
 *			UINT32 input_len,           the crypt data length
 *			enum SHA_MODE sha_mode,     SHA mode 
 * Output: 
 *			UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE ali_sha_digest(UINT8 *input, UINT32 input_len, enum SHA_MODE sha_mode, UINT8 *output);

/*****************************************************************************
 * Function: see_ali_sha_digest
 * Description: 
 *	This function is used to caculate the digest of input data with specified SHA mode
 * Input: 
 *			UINT8 *input,               the input buffer pointer
 *			UINT32 input_len,           the crypt data length
 *			enum SHA_MODE sha_mode,     SHA mode 
 * Output: 
 *			UINT8 *output:  the output buffer pointer
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE see_ali_sha_digest(UINT8 *input, UINT32 input_len, enum SHA_MODE sha_mode, UINT8 *output);

/*****************************************************************************
 * Function: dsc_set_fb_region
 * Description: 
 *	This function is to set the DSC special region.
 * Input: 
 *			UINT32 addr         the region addr
 *			UINT32 size          the region size
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_set_fb_region(UINT32 addr, UINT32 size);

/*****************************************************************************
 * Function: dsc_fixed_cryption
 * Description: 
 *	This function is to used to one DSC fixed cryption.
 * Input: 
 *			UINT32 addr         the region addr
 *			UINT32 size          the region size
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_fixed_cryption(UINT8 *input, UINT32 length, UINT32 pos);

/*****************************************************************************
 * Function: dsc_deal_sys_uk_fw
 * Description: 
 *	This function is to used encrypt/decrypt the system UK or FW.
 * Input: 
 *			UINT8 *input,                          input data addr
 *			UINT8 *output                         output data addr
 *			UINT8 *key		                   cipher key
 *			UINT32 length                          data size (in byte)
 *			UINT8 pos                               key index in keyladder
 *			enum CRYPT_SELECT mode        DSC_ENCRYPT or DSC_DECRYPT
 *
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_deal_sys_uk_fw(UINT8 *input, UINT8 *output, UINT8 *key, UINT32 length,
								UINT8 pos, enum CRYPT_SELECT mode);

/*****************************************************************************
 * Function: dsc_enable_disable_cmdq
 * Description: 
 *	This function is to used enable or disable cmdq mode in DSC(available from M3823).
 * Input: 
 *			UINT32 en_or_dis                        en is 1, dis is 0.
 *
 * Returns:
 *		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE dsc_enable_disable_cmdq(UINT32 en_or_dis);


/*****************************************************************************
 * Just define below functions for compiling, user no need to use these symbols.
 * 
*****************************************************************************/
void hld_dsc_callee(UINT8 *msg);
void lld_dsc_m36f_callee( UINT8 *msg );


#ifdef __cplusplus 
} 
#endif 

#endif  /*_DSC_H_*/
