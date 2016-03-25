#ifndef _SEC_BOOT_H_
#define _SEC_BOOT_H_

#define BOOT_ERR            libc_printf
#if 0
#define BOOT_PRINTF           	libc_printf
#else
#define BOOT_PRINTF(...)			do{} while(0)
#endif

#if 1
#define BOOT_DEBUG                libc_printf
#define BOOT_DUMP(data, len) { const int l = (len); int i;\
		                         for (i = 0; i < l; i++) {BOOT_DEBUG(" 0x%02x", *((data) + i)); \
		                         if((i+1)%16==0) BOOT_DEBUG("\n");}\
                                 BOOT_DEBUG("\n");}
#else
#define BOOT_DUMP(data, len)		do{} while(0)
#define BOOT_DEBUG(...)           do{} while(0)
#endif

#ifdef ALI_ARM_STB
#define SYS_SEE_CTRL_ADDR   0x18000220
#define SYS_SEE_ENTER_ADDR  0x18000200
/*define register for pass the parameter from CPU to SEE*/
#define SYS_SEE_NB_BASE_ADDR 0x18040000
//#define SYS_SEE_NB_BASE_ADDR 0xa8040000 // for s3503 debug***************************
#define SYS_NB_BASE_ADDR    0x18000000
#else
#define SYS_SEE_CTRL_ADDR   0xb8000220
#define SYS_SEE_ENTER_ADDR  0xb8000200
/*define register for pass the parameter from CPU to SEE*/
#define SYS_SEE_NB_BASE_ADDR 0xb8040000
//#define SYS_SEE_NB_BASE_ADDR 0xa8040000 // for s3503 debug***************************
#define SYS_NB_BASE_ADDR    0xb8000000
#endif

#define SEEROM_SEE_REAL_RUN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b0)
#define SEEROM_SEE_CODE_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2b4)
#define SEEROM_SEE_CODE_LEN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b8)
#define SEEROM_SEE_SIGN_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2bC)
#define SEEROM_DEBUG_KEY_ADDR           (SYS_SEE_NB_BASE_ADDR+0x2C0)


/*SEE status register*/
#define SEEROM_SEE_RUN_STATUS_ADDR      (SYS_SEE_NB_BASE_ADDR+0x2C4)
#define SEE_FAIL_BIT    (1<<9) /* see fail log measn if SEE_FAIL_BIT =1 & SEE_DECRYTP_BIT =1 , decrypt fail!!!*/
#define SEE_ACK_BIT     (1<<8) /*see verify pass =1 */
#define SEE_DECRYTP_BIT (1<<7) /*see decrypt pass =1*/
#define SEE_UZIP_BIT    (1<<6) /*see uzip pass =1*/
#define SEE_VERSION_BIT (1<<5) /*see version pass =1*/
#define SEE_COPY_BIT    (1)    /*see code copy to pirvate done =1*/
#define SEE_SYNC_REBOOT_BIT    (1<<16) /*see sync reboot info to main*/
#define SEE_ROM_RUN_BIT         (0xbea<<20)

/*Main status register*/
#define SEEROM_MAIN_RUN_STATUS_ADDR      (SYS_SEE_NB_BASE_ADDR+0x2C8)
#define CPU_GET_FAIL_BIT        (1<<9)
#define CPU_GET_ACK_BIT         (1<<8)  /*CPU get see ack bit status*/
#define CPU_GET_DECRYPT_BIT     (1<<7)  /*CPU get decrypt bit status*/
#define CPU_GET_UZIP_BIT        (1<<6)  /*CPU get uzip bit status*/
#define CPU_GET_VERSIO_BIT      (1<<5)  /*CPU get version check bit status*/
#define CPU_RUN_SW_BIT          (1)     /*CPU run system software*/


#define SEE_RETOOB_FLG                     (16)  

/*For M3515 secure boot*/
#define RSASSA_PKCS1_V1_5 0
#define RSASSA_PSS        1
#define SIG_LEN           256


typedef enum{
    COPY_DONE   = 0x1 ,
    VERIFY_DONE  ,
    VERIFY_FAIL  ,
    DECRYPT_FAIL ,
    UNZIP_FAIL,
    VERSIION_FAIL,
    OTHER_FAIL,
}ack_type;


typedef struct SIGN_FORMAT
{
    UINT32 length:   16;
    UINT32 format:   2;
    UINT32 reserved: 14; 
}SIGN_FORMAT;

struct SEEROM_PARAM        
{
    UINT32 see_run_addr;            //Reserved for branch instruction
    UINT32 see_code_addr;
    UINT32 see_code_len;              //Should be "HEAD" 0x44414548
    UINT32 see_sign_location;            //Change ip clock if no zero, now skip it
    SIGN_FORMAT see_sign_format;        //Boot device configuration, such as nand pmi,
    UINT32 debug_key_addr;
};

typedef struct SEE_KEY
{
    UINT32  see_sig_key;
    UINT32 uk_pos ;
}SEE_KEY;

typedef enum{
   SEEROM_BOOT =  0x1,
   SEE_SW_VERIFY =  0x2,
   SEE_SW_RUN = 0x3,
   SEE_STATUS_RESET = 0x4,
}seeboot_type;

typedef struct SEE_INFO {
    UINT32 see_addr;
    UINT32 see_len;
    UINT32 sig_addr;
    UINT32 sig_len;        
}SEE_INFO;

static inline void clean_cpu_ack_register()
{
     *(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR)  = 0;
}
static inline UINT32 is_hw_copy_done()
{
    return  (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) & SEE_COPY_BIT ; 
}



static inline UINT32 set_main_get_ack_flag(UINT32 type)
{
     *(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR) |= (type) ;
}

inline UINT32 is_hw_ack_fail_flag()
{
	return (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) &(SEE_FAIL_BIT);
}
inline UINT32 is_hw_ack_flag_true(UINT32 type)
{
    return  (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) & (type) ;
}
inline UINT32 get_hw_ack_flag()
{
    return  (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) ;
}

#define SEE_RETOOB_FLG                     (16)
static inline UINT32 is_hw_reboot_true()
{
    return  (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) & (0x1<<SEE_RETOOB_FLG);
}

#endif
