/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: crypto.h
*   
*    Description: this file is used to define some macros and structures 
*                 for secure key ladder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _CRYPTO_H_
#define  _CRYPTO_H_

#ifdef __cplusplus 
extern "C" { 
#endif 

#define  IO_OTP_ROOT_KEY_GET           0
#define  IO_SECOND_KEY_GENERATE        1
#define  IO_CRYPT_DATA_INPUT           2
#define  IO_CRYPT_PARAM_SET            3
#define  IO_CRYPT_SECOND_KEY_CONFIG    4
#define  IO_CRYPT_DEBUG_GET_KEY        5  /*only for debug*/
#define  IO_CRYPT_POS_IS_OCCUPY        6
#define  IO_CRYPT_POS_SET_USED         7
#define  IO_CRYPT_POS_SET_IDLE         8
#define  IO_CRYPT_FOUND_FREE_POS       9
#define  IO_DECRYPT_PVR_USER_KEY       10

#define OTP_ADDESS_1 0x4d
#define OTP_ADDESS_2 0x51
#define OTP_ADDESS_3 0x55
#define OTP_ADDESS_4 0x59
#define OTP_ADDESS_5 0x60
#define OTP_ADDESS_6 0x64
#define OTP_ADDESS_7 0x68
#define OTP_ADDESS_8 0x6c

#define AES_CE_KEY_LEN  16
#define XOR_CE_KEY_LEN  16
#define TDES_CE_KEY_LEN  8

#define INVALID_ALI_CE_KEY_POS 0xff
#define ALI_INVALID_CRYPTO_KEY_POS   INVALID_ALI_CE_KEY_POS

typedef enum ce_crypto_status
{
	ALI_CRYPTO_SUCCESS = RET_SUCCESS,
	ALI_CRYPTO_ERROR,
	ALI_CRYPTO_WARNING_DRIVER_ALREADY_INITIALIZED,

	ALI_CRYPTO_ERROR_INVALID_PARAMETERS = 0x1f00,
	ALI_CRYPTO_ERROR_OPERATION_NOT_ALLOWED,
	ALI_CRYPTO_ERROR_OPERATION_NOT_SUPPORTED,
	ALI_CRYPTO_ERROR_INITIALIZATION_FAILED,
	ALI_CRYPTO_ERROR_DRIVER_NOT_INITIALIZED,
	ALI_CRYPTO_ERROR_INVALID_ADDR,
	ALI_CRYPTO_ERROR_INVALID_DEV,
}CRYPTO_STATUS;


enum CE_OTP_KEY_SEL
{
	OTP_KEY_0_0 = 0,
	OTP_KEY_0_1 = 1,
	OTP_KEY_0_2 = 2,
	OTP_KEY_0_3 = 3,
	OTP_KEY_0_4 = 4,
	OTP_KEY_0_5 = 5,
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

	CRYPT_KEY_3_16=0x20,
	CRYPT_KEY_3_17=0x21,
 	CRYPT_KEY_3_18=0x22,
	CRYPT_KEY_3_19=0x23,
	CRYPT_KEY_3_20=0x24,
	CRYPT_KEY_3_21=0x25,
	CRYPT_KEY_3_22=0x26,
	CRYPT_KEY_3_23=0x27,
	CRYPT_KEY_3_24=0x28,
	CRYPT_KEY_3_25=0x29,
 	CRYPT_KEY_3_26=0x2a,
	CRYPT_KEY_3_27=0x2b,
	CRYPT_KEY_3_28=0x2c,
	CRYPT_KEY_3_29=0x2d,
	CRYPT_KEY_3_30=0x2e,
	CRYPT_KEY_3_31=0x2f,
	
	CRYPT_KEY_3_32=0x30,
	CRYPT_KEY_3_33=0x31,
 	CRYPT_KEY_3_34=0x32,
	CRYPT_KEY_3_35=0x33,
	CRYPT_KEY_3_36=0x34,
	CRYPT_KEY_3_37=0x35,
	CRYPT_KEY_3_38=0x36,
	CRYPT_KEY_3_39=0x37,
	
	CRYPT_KEY_3_40=0x38,
	CRYPT_KEY_3_41=0x39,
 	CRYPT_KEY_3_42=0x3a,
	CRYPT_KEY_3_43=0x3b,
	CRYPT_KEY_3_44=0x3c,
	CRYPT_KEY_3_45=0x3d,
	CRYPT_KEY_3_46=0x3e,
	CRYPT_KEY_3_47=0x3f,	
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
	KEY_2_7=0xf,
	KEY_3_0=0x10,
	KEY_3_1=0x11,
	KEY_3_2=0x12,
	KEY_3_3=0x13,
	KEY_3_4=0x14,
	KEY_3_5=0x15,
	KEY_3_6=0x16,
	KEY_3_7=0x17,
	KEY_3_8=0x18,
	KEY_3_9=0x19,
	KEY_3_10=0x1a,
	KEY_3_11=0x1b,
	KEY_3_12=0x1c,
	KEY_3_13=0x1d,
	KEY_3_14=0x1e,
	KEY_3_15=0x1f,
};

enum CE_CRYPT_SELECT
{  
    CE_IS_DECRYPT = 1,
    CE_IS_ENCRYPT=0
};

enum CE_MODULE_SELECT
{  
    CE_SELECT_DES= 0,
    CE_SELECT_AES = 1,    
    CE_SELECT_XOR = 2,
    CE_SELECT_AES64BIT = 3,    
    CE_SELECT_XOR64BIT = 4,
};

enum DATA_MODULE_MODE
{
	BIT128_DATA_MODULE = 0,
	BIT64_DATA_MODULE =  1
};

enum DATA_HILO_MODE
{
	LOW_64BITS_DATA = 0,
	HIGH_64BITS_DATA= 1
};

enum KEY_MODULE_FROM
{  
    CE_KEY_FROM_SRAM = 0,
    CE_KEY_FROM_CPU  = 1,    
};

enum DATA_MODULE_FROM
{
	CE_DATA_IN_FROM_CPU = 0,
	CE_DATA_IN_FROM_SRAM = 1,
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
	enum CE_OTP_KEY_SEL root;
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
	ID semaphore_id;
    ID semaphore_id2;
	
	void      (*attach)(void);
	void      (*detach)( struct ce_device *);
	RET_CODE	(*open)( struct ce_device *);
	RET_CODE   	(*close)( struct ce_device *);
	RET_CODE  (*ioctl)(struct ce_device *,UINT32 ,UINT32 );
	RET_CODE (*key_generate)(struct ce_device *,pCE_DATA_INFO );
	RET_CODE (*key_load)(struct ce_device *,pOTP_PARAM);	
	
}CE_DEVICE, *pCE_DEVICE;


typedef struct aes_key_ladder_param { 
    UINT32 key_ladder;      // 1,2,3 
    UINT32 root_key_pos;
    UINT8 r[256];           //16 groups key
}AES_KEY_LADDER_BUF_PARAM, *pAES_KEY_LADDER_BUF_PARAM; 

/*****************************************************************************
 * Function: hdmi_set_aksv
 * Description: 
 *	This function is used to patch the HDMI swith crypto key issue
 * Input: 
 *		None
 * Output: 
 *		None
 * Returns:
 *		None
 * 
*****************************************************************************/
void hdmi_set_aksv(void);

/*****************************************************************************
 * Function: patch_write_bksv2hdmi
 * Description: 
 *	This function is used to patch the HDMI swith crypto key issue
 * Input: 
 *		None
 * Output: 
 *		None
 * Returns:
 *		None
 * 
*****************************************************************************/
void patch_write_bksv2hdmi(UINT8 *bksv_data);

/*****************************************************************************
 * Function: ce_api_attach
 * Description: 
 *    This function initializes the key ladder hardware and software structures.
 *	  This function has to be called once before any other function call of key ladder.
 * Input: 
 *      None
 * Output: 
 *      None
 * 
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE ce_api_attach(void);

/*****************************************************************************
 * Function: ce_api_detach
 * Description: 
 *    This function is used to terminal key ladder low level driver and release the key ladder occupied resources.
 *    If ce_api_detach is performed, key ladder functions should not be called.
 * Input: 
 *      None
 * Output: 
 *      None
 * 
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
 * 
*****************************************************************************/
RET_CODE ce_api_detach(void);

/*****************************************************************************
 * Function: ce_ioctl
 * Description: 
 *    This function is used to implement Key ladder IO control.
 * Input: 
 *      p_ce_dev	key ladder device pointer.
 *		cmd			IO control commands defined in above. 
 *      param		Parameters pointer.
 * Output: 
 *      param
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_ioctl(CE_DEVICE *p_ce_dev, UINT32 cmd, UINT32 param);

/*****************************************************************************
 * Function: ce_key_generate
 * Description: 
 *    This function is used to generate the first level key from OTP level.
 *	  It will load the OTP key from OTP into Key ladder firstly, 
 *	  then it will use the OTP key to generate next level key.
 * Input: 
 *      p_ce_dev:          	key ladder device pointer.
 *		p_ce_data:    key data information pointer
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_key_generate(CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data);

/*****************************************************************************
 * Function: ce_key_load
 * Description: 
 *    This function is used to load OTP key from OTP into Key Ladder SRAM.
 * Input: 
 *      p_ce_dev:          	key ladder device pointer.
 *		p_ce_opt_info:	    key data information pointer
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_key_load(CE_DEVICE *p_ce_dev, OTP_PARAM *p_ce_otp_info);

/*****************************************************************************
 * Function: ce_generate_cw_key
 * Description: 
 *    This function is used to generate key from the in_cw_data to second pos using the key in first key pos.
 * Input: 
 *      UINT8 *in_cw_data:          	key ladder device pointer.
 *		UINT8 mode:	                    AES mode or TDES mode
 *		UINT8 first_pos:				the first key position 
 *		UINT8 second_pos:				the second key position
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_cw_key(const UINT8 *in_cw_data, UINT8 mode,
                             UINT8 first_pos, UINT8 second_pos);


/*****************************************************************************
 * Function: ce_generate_single_level_key
 * Description: 
 *    This function is used to generate single level key to any key level.
 * Input: 
 *      p_ce_dev:          	key ladder device pointer.
 *		p_ce_data_info:    key data information pointer
 * Output: 
 *      None
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_single_level_key(CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data_info);

/*****************************************************************************
 * Function: aes_generate_key_with_multi_keyladder
 * Description: 
 *    This function is used to generate multi level key using AES mode.
 * Input: 
 *      AES_KEY_LADDER_BUF_PARAM *p_ce_aesparam:   key ladder information pointer.
 * Output: 
 *      UINT8 *key_pos:  output key position
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_generate_key_with_multi_keyladder( \
        AES_KEY_LADDER_BUF_PARAM *p_ce_aesparam, \
        UINT8 *key_pos);

/*****************************************************************************
 * Function: ce_key_load
 * Description: 
 *    This function is used to load OTP key from OTP into Key Ladder SRAM by indicated key pos.
 * Input: 
 *      UINT32 key_pos:   first level OTP key position.
 * Output: 
 *      The output key position will be fixed in key_pos + 4.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
int ce_load_otp_key(UINT32 key_pos);

/*****************************************************************************
 * Function: ce_generate_cw_key
 * Description: 
 *    This function is used to generate key from the key to second pos using the key in first key pos with AES.
 * Input: 
 *      UINT8 *key:          	        key ladder device pointer.
 *		UINT32 ce_crypt_select:	        Encrypt mode or Decrypt mode
 *		UINT8 first_pos:				the first key position 
 *		UINT8 second_pos:				the second key position
 * Output: 
 *      output key in second key position
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
int ce_generate_key_by_aes(const UINT8 *key, UINT32 first_key_pos,
                           UINT32 second_key_pos, UINT32 ce_crypt_select);



/*****************************************************************************
 * Function: tdes_decrypt_key_to_ce_64bit
 * Description: 
 *    This function is used to generate 64bits key by using TDES mode.
 *	  If first_key is zero level key, the target must equal to first_key +4. 
 *	  This function will generate 64 bits key, so the input eck should be 64 bits data
 * Input: 
 *      UINT8 *eck:   					 input data pointer which need to be crypt, the data length should be 128 bits.
 		UINT8 first_key:				 first key position	
 		enum CE_CRYPT_TARGET target:  second key position
 		UINT8 hilo_addr:				decrypt eck to the target position high 64bits or low 64 bit 
 * Output: 
 *      output key will be stored in Key Ladder SRAM by key position "target"
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_key_to_ce_64bit(UINT8 *eck, UINT8 first_key, enum CE_CRYPT_TARGET target, UINT8 hilo_addr);


/*****************************************************************************
 * Function: tdes_decrypt_key_to_ce_one_level
 * Description: 
 *    This function is used to generate single level key using TDES mode.
 *	  If level_one is zero level key, the level_gen must equal to level_one +4. 
 *	  This function will generate 128 bits key, so the input eck should be 128 bits data
 * Input: 
 *      UINT8 *eck:   					 input data pointer which need to be crypt, the data length should be 128 bits.
 		UINT8 level_one:				 first key position	
 		enum CE_CRYPT_TARGET level_gen:  second key position
 * Output: 
 *      output key will be stored in Key Ladder SRAM by key position level_gen
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_key_to_ce_one_level(UINT8 *eck, UINT8 level_one,
        enum CE_CRYPT_TARGET level_gen);


/*
	
input -> data to be decrypt
level_root -> root pos in CE(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
key_pos -> level_two key_pos in CE, will return to caller for other use
after used the key_pos, need to set idle: ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
*/

/*****************************************************************************
 * Function: tdes_decrypt_to_ce_two_level
 * Description: 
 *    This function is used to generate two level key using TDES mode.
 *	  TDES 128bit decrypt 16 + 16 byte data to CE
 *	  If level_root must be zero level key or first level key position. 
 *	  This function will generate 128 bits key using two level key, so the input eck should be 128*2 bits data
 * Input: 
 *      UINT8 *input:   				 input data pointer which need crypt, the data length should be 128*2 bits.
 		UINT8 level_root:				 first key position 		
 * Output: 
 *		UINT8 *key_pos:		The output key position in Key Ladder SRAM
 *      output key will be stored in Key Ladder SRAM by key position key_pos.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_to_ce_two_level(UINT8 *input, UINT8 level_root, UINT8 *key_pos);


/*****************************************************************************
 * Function: tdes_decrypt_setup_kl_three_level
 * Description: 
 * TDES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
 *
 * Input: 
 *      UINT8 *input:  input data pointer which need crypt, the data length should be 128*3 bits.
 *      UINT8 root_pos:	 root key position 		
 * Output: 
 *	UINT8 *key_pos:	 Output an idle key position in Key Ladder SRAM
 *      output key will be stored in Key Ladder SRAM by key position key_pos. 
 *	If this key_pos won't be used anymore, need to set it idle: 
 *	   ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos);


/*****************************************************************************
 * Function: aes_decrypt_setup_kl_three_level
 * Description: 
 * AES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
 *
 * Input: 
 *      UINT8 *input:   				 input data pointer which need crypt, the data length should be 128*3 bits.
 *      UINT8 root_pos:				 root key position 		
 * Output: 
 *		UINT8 *key_pos:		Output an idle key position in Key Ladder SRAM
 *       output key will be stored in Key Ladder SRAM by key position key_pos. If this key_pos won't be used anymore, 
 *	need to set it idle: 
 *	   				ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT8 *key_pos);


/*****************************************************************************
 * Just define below functions for compiling, user no need to use these symbols.
 * 
*****************************************************************************/
void hld_crypto_callee(UINT8 *msg);
void lld_crypto_m36f_callee( UINT8 *msg );

#ifdef __cplusplus 
} 
#endif 

#endif  /*_CRYPTO_H_*/
