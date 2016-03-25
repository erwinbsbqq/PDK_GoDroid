
#include <hld/pan/adr_pan.h>


INT32 pan_ch455_attach(struct pan_configuration *config)
{
	return pan_attach();
}


#ifdef HBBTV_MODULE_SUPPORT
INT32  pan_get_pan_ircode_by_linuxkey(unsigned short  linux_key,unsigned short * p_ir_code)
{
	int  i = 0;

	*p_ir_code =  linux_key;
	return SUCCESS;
}

#endif


