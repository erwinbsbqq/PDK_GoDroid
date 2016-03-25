#ifndef _ALI_DSC_H_
#define _ALI_DSC_H_

#include <linux/types.h>
#include <ali_magic.h>

#define VIRTUAL_DEV_NUM 4

#define DSC_TIMEOUT_FORVER		 0xFFFFFFFF
#define RAM_MON_SET_FLAG         0x00000002
#define RAM_MON_CLEAR_FLAG       0x00000001

#ifndef RET_SUCCESS
#define RET_SUCCESS ((__s32)0)
#define RET_FAILURE	((__s32)1)
#endif

#define ALI_INVALID_CRYPTO_STREAM_HANDLE  0xffffffff
#define ALI_INVALID_CRYPTO_KEY_POS			0xff
#define ALI_INVALID_CRYPTO_STREAM_ID		0xff
#define ALI_INVALID_DSC_SUB_DEV_ID          0xff

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
};

enum KEY_TYPE
{
    KEY_FROM_REG=0,
    KEY_FROM_SRAM=1,
    KEY_FROM_CRYPTO=2,
};

enum KEY_MAP_MODE
{
   CSA_MODE=0,
   DES_MODE=0,
   CSA3_MODE=1,
   AES_128BITS_MODE=1,
   TDES_ABA_MODE=1,
   AES_192BITS_MODE=2,
   TDES_ABC_MODE=2,
   AES_256BITS_MODE=3
};

enum DMA_MODE
{
    PURE_DATA_MODE=0,
    TS_MODE=(1<<24),
};

enum RESIDUE_BLOCK
{
    RESIDUE_BLOCK_IS_NO_HANDLE=0,
	RESIDUE_BLOCK_IS_AS_ATSC=(1<<12),
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
//1 DES INIT PARAM
typedef struct des_init_param 
{
    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    __u32 scramble_control;
    enum KEY_MAP_MODE key_mode;
    __u32 stream_id; /**which stream id is working*/
    enum DMA_MODE dma_mode;
    enum RESIDUE_BLOCK  residue_mode;
    enum WORK_MODE work_mode;
    enum WORK_SUB_MODULE sub_module;
	__u32 cbc_cts_enable ;  /*for pure data*/
}DES_INIT_PARAM, *pDES_INIT_PARAM;


//1 AES INIT PARAM
typedef struct aes_init_param 
{
    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    __u32 scramble_control;
    enum KEY_MAP_MODE key_mode;
    __u32 stream_id; /**which stream id is working*/
    enum DMA_MODE dma_mode;
    enum RESIDUE_BLOCK  residue_mode;
    enum WORK_MODE work_mode;
    __u32 cbc_cts_enable ;  /*for pure data*/
}AES_INIT_PARAM, *pAES_INIT_PARAM;


//1 CSA INIT PARAM
typedef struct csa_init_param 
{
    enum CSA_VERSION version;
    enum DMA_MODE dma_mode;/*pure_data, or ts*/	 
    __u32 Dcw[4];  /*for csa only used Dcw[0]Dcw[1], for csa3 used all*/
    __u32 pes_en;  

    enum PARITY_MODE  parity_mode;    
    enum KEY_TYPE key_from;
    __u32 scramble_control;
    __u32 stream_id; /**which stream id is working*/  

}CSA_INIT_PARAM, *pCSA_INIT_PARAM;


//1 SHA INIT PARAM
typedef struct sha_init_param
{
  enum SHA_MODE sha_work_mode; 
  enum SHA_DATA_SOURCE sha_data_source;
} SHA_INIT_PARAM, *pSHA_INIT_PARAM;


struct AES_128Bit_KEY 
{
__u8 even_key[16];
__u8 odd_key[16];
};

struct AES_192Bit_KEY 
{
__u8 even_key[24];
__u8 odd_key[24];
};

struct AES_256Bit_KEY 
{
__u8 even_key[32];
__u8 odd_key[32];
};

typedef union aes_key_param
{
    struct AES_128Bit_KEY aes_128bit_key ;
    struct AES_192Bit_KEY aes_192bit_key ;
    struct AES_256Bit_KEY aes_256bit_key ;
   
}AES_KEY_PARAM;

typedef struct aes_iv_info
{
  __u8 even_iv[16];
  __u8 odd_iv[16];
}AES_IV_INFO;

struct  AES_KEY
{
   __u8 OddKey[16];
   __u8 EvenKey[16];
};

struct  CSA_KEY
{
   __u8 OddKey[8];
   __u8 EvenKey[8];
};

struct  CSA3_KEY
{
   __u8 OddKey[16];
   __u8 EvenKey[16];
};

typedef union csa_key_param
{
    struct  CSA_KEY csa_key ;
    struct  CSA3_KEY csa3_key ;
}CSA_KEY_PARAM;

struct  DES_64BITS_KEY_INFO
{
   __u8 OddKey[8];
   __u8 EvenKey[8];
};

struct  DES_128BITS_KEY_INFO
{
   __u8 OddKey[16];
   __u8 EvenKey[16];
};

struct  DES_192BITS_KEY_INFO
{
   __u8 OddKey[24];
   __u8 EvenKey[24];
};

typedef union des_key_param
{
   struct  DES_64BITS_KEY_INFO  des_64bits_key ;
   struct  DES_128BITS_KEY_INFO des_128bits_key ;
   struct  DES_192BITS_KEY_INFO des_192bits_key ;
   
}DES_KEY_PARAM;

typedef struct des_iv_info
{
  __u8 even_iv[8];
  __u8 odd_iv[8];
}DES_IV_INFO;

//1 KEY INFO PARAM
typedef struct  
{
    __u32 handle ;  /* out parameter*/
    __u16 *pid_list;
    __u16 pid_len;  
    AES_KEY_PARAM *p_aes_key_info; /*for ts data mode*/
    CSA_KEY_PARAM *p_csa_key_info;
    DES_KEY_PARAM *p_des_key_info;
    __u32 key_length;
    AES_IV_INFO *p_aes_iv_info;
    DES_IV_INFO *p_des_iv_info;
    __u16 stream_id;
    
    __u8 *init_vector; /*for pure data mode*/
    __u8 *ctr_counter;
    __u8 force_mode; /*user point the fix position*/
    __u8 pos ;
}KEY_PARAM, *pKEY_PARAM;
//1 PID INFO PARAM
typedef struct pid_param
{  
   __u32 dmx_id ;
   __u16 pid;
   __u16 pos;
}PID_POS_PARAM,*pPID_POS_PARAM ;

typedef struct ram_monitor_param
{
	__u32 start_addr;
	__u32 len;
	__u32 interval;
	enum SHA_MODE sha_mode;
	__s32   flag;
	__u32 ram_flg_id;
}RAM_MONITOR_PARAM,*pRAM_MONITOR_PARAM;

//1 SHA DEVICE
typedef struct sha_device
{
    struct sha_device  *next;  /*next device */
	/*struct module *owner;*/
	__s32 type;
	__s8 name[16];	//HLD_MAX_NAME_SIZE
    void *pRoot;
    void *priv ; 
    void (*open)(struct sha_device *);
    void (*close)(struct sha_device *);
    __s32 (*digest)(struct sha_device * , __u8 *, __u8 *,__u32 );
    __s32  (*Ioctl)( struct sha_device * ,__u32 cmd , __u32 param);
    __u8 id_number;	
}SHA_DEV,*pSHA_DEV;

//1 AES DEVICE
typedef struct aes_device
{
    struct aes_device  *next;  /*next device */
	/*struct module *owner;*/
	__s32 type;
	__s8 name[16]; //HLD_MAX_NAME_SIZE
    void *pRoot;		/* Used to be 'private' but that upsets C++ */
    void *priv ;
    void (*open)(struct aes_device *);
    void (*close)(struct aes_device *);
    __s32 (*Encrypt)(struct aes_device * ,__u16, __u8 *, __u8 *, __u32 );
    __s32 (*Decrypt_cts)(struct aes_device * ,__u16, __u8 *, __u8 *, __u32, __u8 * );
	__s32 (*Decrypt)(struct aes_device * ,__u16, __u8 *, __u8 *, __u32);
    __s32 (*Ioctl)(struct aes_device *,__u32 cmd,__u32 param);
    __u8 id_number;
	
}AES_DEV,*pAES_DEV;


//1 DES DEVICE
typedef struct des_device
{
    struct des_device  *next;  /*next device */
	/*struct module *owner;*/
	__s32 type;
	__s8 name[16]; //HLD_MAX_NAME_SIZE
    void *pRoot;		/* Used to be 'private' but that upsets C++ */
	void *priv ;
    void (*open)(struct des_device * );
    void (*close)(struct des_device *);
    __s32 (*Encrypt)(struct des_device *, __u16,__u8 *, __u8 *, __u32 );
	__s32 (*Decrypt_cts)(struct des_device *, __u16,__u8 *, __u8 *, __u32, __u8 *);
	__s32 (*Decrypt)(struct des_device *, __u16,__u8 *, __u8 *, __u32 );
    __s32 (*Ioctl)(struct des_device *,__u32 cmd,__u32 param);
    __u8 id_number;                 	
}DES_DEV,*pDES_DEV;


//1 CSA DEVICE
typedef struct csa_device
{
    struct csa_device  *next;  /*next device */
	/*struct module *owner;*/
	__s32 type;
	__s8 name[16]; //HLD_MAX_NAME_SIZE
    void *pRoot;		/* Used to be 'private' but that upsets C++ */
    void *priv ;
    void (*open)(struct csa_device*);
    void (*close)(struct csa_device*);
    __s32 (*Decrypt)(struct csa_device*,__u16, __u8*, __u8*, __u32 );
    __s32 (*Ioctl)(struct csa_device *,__u32 cmd,__u32 param);
    __u8 id_number;
}CSA_DEV,*pCSA_DEV;


//1 DSC DEVICE
typedef struct descrambler_device
{
	struct descrambler_device  *next;  /*next device */
	/*struct module *owner;*/
	__s32 type;
	__s8 name[16]; //HLD_MAX_NAME_SIZE
	void *priv;		/*only point to SHA */
	__u32 base_addr;
	__u32  interrupt_id;

	
    void      (*attach)(void);
    void      (*detach)( struct descrambler_device *);
    __s32	(*open)( struct descrambler_device *);
    __s32   	(*close)( struct descrambler_device *);
    __s32   (*ioctl)(struct descrambler_device *, __u32 , __u32 );
   
}DSC_DEV, *pDSC_DEV;

typedef struct DSC_PVR_KEY_PARAM
{
	__u32 input_addr;
    __u32 valid_key_num;
    __u32 current_key_num;
    __u32 pvr_key_length;
	__u8 pvr_user_key_pos;
    __u32 total_quantum_number; 
    __u32 current_quantum_number;
    __u32 ts_packet_number;
    __u8 pvr_key_change_enable;
    __u16 stream_id;
}DSC_PVR_KEY_PARAM,*pDSC_PVR_KEY_PARAM;

typedef struct DSC_BL_UK_PARAM
{
	__u8 *input_key;
    __u8 *r_key;
    __u8 *output_key;
}DSC_BL_UK_PARAM,*pDSC_BL_UK_PARAM;

typedef struct DeEncrypt_config
{
	__u32 do_encrypt ;
	void *dec_dev;                /*Decrypt device for stream*/
    __u8 Decrypt_Mode;
	__u16 dec_dmx_id;
    __u32 do_decrypt ;
	void *enc_dev;                /*Encrypt device for stream*/
    __u8 Encrypt_Mode;
	__u16 enc_dmx_id;
}DEEN_CONFIG,*pDEEN_CONFIG;


enum ALI_CSA_PROTOC_VERSION
{
    CSA_1 = 1,
    CSA_2 = 0,
    CSA_3 = 2
};



enum ALI_CSA_DATA_FORMAT
{
    DATA_FORMAT_PURE = 0,
    DATA_FORMAT_TS = (1 << 24)
};


enum ALI_CSA_PARITY_MODE
{
    PARITY_MODE_EVEN = 0,
    PARITY_MODE_ODD = 1,
    PARITY_MODE_AUTO_0 = 2,  /*for ts*/
    PARITY_MODE_AUTO_1 = 3
};


enum ALI_CSA_CW_TYPE
{
    CW_TYPE_ODD = 0,
    CW_TYPE_EVEN = 1
};

enum ALI_CSA_CW_SRC
{
    CW_SRC_REG = 0,
    CW_SRC_SRAM = 1,
    CW_SRC_CRYPTO = 2,
};

/* record the device handle in SEE */
struct dsc_see_dev_hld 
{
    __u32 dsc_dev_hld;  /*The device handle address in SEE*/
    __u32 dsc_dev_id;   /*The id number of the dsc_dev_id*/ 
};



/* Same as "typedef struct csa_init_param{}" in SEE. */
struct ali_csa_work_mode_param 
{
    enum ALI_CSA_PROTOC_VERSION protoc_ver;

    enum ALI_CSA_DATA_FORMAT data_format; /*pure_data, or TS. */     

    __u32 dcw[4];  /*Default CW. csa only uses Dcw[0]Dcw[1], csa3 uses all. */

    __u32 pes_en; /* Note used, just keep compartible with SEE struct csa_init_param{}*/ 

    enum ALI_CSA_PARITY_MODE  parity_mode;  
  
    enum ALI_CSA_CW_SRC cw_src;

    __u32 scram_ctrl;/* Note used, just keep compartible with SEE struct csa_init_param{}*/ 

    __u32 data_src; /**which stream id is working*/  
};

//Define this struct in DSC instead of DMX
struct dec_parse_param
{  
    void *dec_dev;   
    __u32 type;
};


#define ALI_DSC_HLD_PARAM_MAX_SIZE 8
struct ali_dsc_hld_param
{
    __u32 p[ALI_DSC_HLD_PARAM_MAX_SIZE];    
};

struct ali_dsc_krec_mem
{
    __u32 size;
    void *pa_mem;
    void *va_mem;
};

#define PURE_DATA_MAX_SIZE  0x100000  //1M

//for AES/DES/CSA/SHA
#define IO_DSC_BASE_KERNEL						(ALI_DSC_MAGIC<<8)
#define IO_INIT_CMD                          		(IO_DSC_BASE_KERNEL + 0)
#define IO_CREAT_CRYPT_STREAM_CMD       (IO_DSC_BASE_KERNEL + 1)
#define IO_DELETE_CRYPT_STREAM_CMD     (IO_DSC_BASE_KERNEL + 2)
#define IO_KEY_INFO_UPDATE_CMD             (IO_DSC_BASE_KERNEL + 3)
#define IO_GET_PID_POS_CMD                      (IO_DSC_BASE_KERNEL + 4)
#define IO_DECRYPT                           		(IO_DSC_BASE_KERNEL + 5)
#define IO_ENCRYPT                           		(IO_DSC_BASE_KERNEL + 6)


//for DSC
#define IO_PARSE_DMX_ID_SET_CMD            (IO_DSC_BASE_KERNEL + 7) 
#define IO_PARSE_DMX_ID_GET_CMD            (IO_DSC_BASE_KERNEL + 8)                                              
#define IO_DSC_GET_DES_HANDLE              	 (IO_DSC_BASE_KERNEL + 9)  
#define IO_DSC_GET_AES_HANDLE                (IO_DSC_BASE_KERNEL + 10)  
#define IO_DSC_GET_CSA_HANDLE                (IO_DSC_BASE_KERNEL + 11)  
#define IO_DSC_GET_SHA_HANDLE                (IO_DSC_BASE_KERNEL + 12)  
#define IO_DSC_SET_PVR_KEY_PARAM          (IO_DSC_BASE_KERNEL + 13)  
#define IO_DSC_ENCRYTP_BL_UK                   (IO_DSC_BASE_KERNEL + 14) 
#define IO_DSC_SET_PVR_KEY_IDLE              (IO_DSC_BASE_KERNEL + 15) 
#define IO_TRIG_RAM_MON                    	  (IO_DSC_BASE_KERNEL + 16)  
#define IO_DEENCRYPT                       		  (IO_DSC_BASE_KERNEL + 17) 
#define IO_GET_FREE_STREAM_ID                 (IO_DSC_BASE_KERNEL + 18)  
#define IO_GET_FREE_SUB_DEVICE_ID          (IO_DSC_BASE_KERNEL + 19) 
#define IO_SET_SUB_DEVICE_ID_IDLE           (IO_DSC_BASE_KERNEL + 20) 
#define IO_SET_STREAM_ID_IDLE                  (IO_DSC_BASE_KERNEL + 21) 
#define IO_SET_STREAM_ID_USED                 (IO_DSC_BASE_KERNEL + 22) 
#define IO_SET_SUB_DEVICE_ID_USED          (IO_DSC_BASE_KERNEL + 23) 
#define IO_GET_DEV_HLD                          	  (IO_DSC_BASE_KERNEL + 24) 
#define IO_REQUEST_KREC_SPACE                  (IO_DSC_BASE_KERNEL + 25) 
#define IO_RELEASE_KREC_SPACE                  (IO_DSC_BASE_KERNEL + 26) 

/* DSC function */
__u32 ali_dsc_get_free_sub_device_id(__u8 sub_mode);
int ali_dsc_set_sub_device_id_idle(__u8 sub_mode,__u32 device_id);
void ali_sha_open(__u32 pShaDev);
void ali_sha_close(__u32 pShaDev);
int ali_sha_ioctl( __u32 pShaDev ,__u32 cmd , __u32 param);
int ali_sha_digest_ex(pSHA_DEV pShaDev, __u8 *input, __u8 *output,__u32 data_length);
int ali_trig_ram_mon(__u32 start_addr,__u32 end_addr, __u32 interval, __u32 sha_mode,__s32 DisableOrEnable);
#endif 
