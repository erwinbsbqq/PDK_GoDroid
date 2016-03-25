
#ifndef _ALI_CSA_H_
#define _ALI_CSA_H_

#include <linux/types.h>



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


struct ali_csa_cw_param
{
    __u16 pid_list[32];
    __u16 pid_cnt;  

    /* [MAX_ITEM][MAX_ITEM / 4] in TDS. (MAX_ITEM == 32 in TDS).
     * which means: ((32 * 32) / 4) == 256
     */
    /* Also acts as holder for odd/even cw pair need to passes to SEE. */
    __u8 cw_pair[256]; 

    __u16 src_dmx_id;   
};





#define ALI_CSA_IOC_MAGIC  0xA4

#define ALI_CSA_WORK_MODE_SET _IOW(ALI_CSA_IOC_MAGIC, 41, struct ali_csa_work_mode_param)

#define ALI_CSA_DESCRAM_CW_UPDATE _IOW(ALI_CSA_IOC_MAGIC, 42, struct ali_csa_cw_param)



#endif


