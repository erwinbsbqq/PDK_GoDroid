#ifndef	__ADF_SOC_H_
#define	__ADF_SOC_H_



#ifdef __cplusplus
extern "C" {
#endif


/*! @struct soc_op_paras
@brief 寄存器访问操作参数。
*/
struct soc_op_paras {
    unsigned char * to;
    unsigned char *from;
    int len;
};                    

struct reboot_timer{
    unsigned long *time_exp;
    unsigned long *time_cur;    
};

struct boot_timer{
    unsigned long time_exp;
    unsigned long time_cur;  
};

/*! @struct soc_usb_port
@brief usb端口数信息。
*/
struct soc_usb_port {
    unsigned long usb_port;
};

/*! @struct soc_opt_paras8
@brief 寄存器访问操作参数，以8 bits为单位。
*/
struct soc_opt_paras8 {
    unsigned long addr;
    unsigned char data;    
};      

/*! @struct soc_opt_paras16
@brief 寄存器访问操作参数，以16 bits为单位。
*/
struct soc_opt_paras16 {
    unsigned long addr;
    unsigned short data;    
};      

/*! @struct soc_opt_paras32
@brief 寄存器访问操作参数，以32 bits为单位。
*/
struct soc_opt_paras32 {
    unsigned long addr;
    unsigned long data;    
};      

/*! @struct debug_level_paras
@brief 系统信息模块调试级别。
*/
struct debug_level_paras {
    unsigned long level;   
};      

/*! @struct soc_memory_map
@brief 系统各个模块内存映射表。
*/
struct soc_memory_map
{	
	unsigned long main_start;		
	unsigned long main_end;	
	unsigned long fb_start;
	unsigned long osd_bk;	
	unsigned long see_dmx_src_buf_start;
	unsigned long see_dmx_src_buf_end;
	unsigned long see_dmx_decrypto_buf_start;
	unsigned long see_dmx_decrypto_buf_end;
	unsigned long dmx_start;
	unsigned long dmx_top;
	unsigned long see_start;
	unsigned long see_top;
	unsigned long video_start;
	unsigned long video_top;
	unsigned long frame;
	unsigned long frame_size;
	unsigned long vcap_fb;
	unsigned long vcap_fb_size;
	unsigned long vdec_vbv_start;
	unsigned long vdec_vbv_len;
	unsigned long shared_start;
	unsigned long shared_top;	

	unsigned long reserved_mem_addr;
	unsigned long reserved_mem_size;

	unsigned long media_buf_addr;
	unsigned long media_buf_size;

	unsigned long mcapi_buf_addr;
	unsigned long mcapi_buf_size;
};

/*! @struct soc_opt_see_ver
@brief see cpu版本信息。
*/
struct soc_opt_see_ver {
    unsigned char *buf;
};  

/*! @enum boot_type
@brief 启动类型。
*/
enum boot_type {
    ALI_SOC_BOOT_TYPE_NOR,
    ALI_SOC_BOOT_TYPE_NAND,    
};


#ifdef __cplusplus
}
#endif
#endif

