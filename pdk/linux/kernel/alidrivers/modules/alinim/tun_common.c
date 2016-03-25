#include "tun_common.h"
#include "porting_linux_header.h"

#if 0
#define PRINTK_INFO(x...) printk(KERN_INFO x)
#else
#define PRINTK_INFO(x...)
#endif






//Please add callback function to this arrray when new tuner is coming.
//kent.2014.1.13

static TUNER_IO_FUNC g_tuner_array[]=
{
#ifdef  CONFIG_AV2012   // CONFIG_AV2012		
	{
	   	NIM_DVBS, 
	   	AV_2012,	
	   	(tuner_init_callback)ali_nim_av2011_init,     
	   	(tuner_control_callback)ali_nim_av2011_control,   
	   	(tuner_status_callback)ali_nim_av2011_status,  
	   	(tuner_close_callback)ali_nim_av2011_close
	 },
#endif

#ifdef CONFIG_MXL603
	{
	   	NIM_DVBC,
	   	MXL603,
	   	(tuner_init_callback)tun_mxl603_init_DVBC,     
	   	(tuner_control_callback)tun_mxl603_control_DVBC_X,   
	   	(tuner_status_callback)tun_mxl603_status,  
	   	(tuner_close_callback)NULL
	 },
#endif
#ifdef CONFIG_SHARP_VZ7306
	{
	   	NIM_DVBS,
		SHARP_VZ7306,
	   	(tuner_init_callback)ali_nim_vz7306_init,     
	   	(tuner_control_callback)ali_nim_vz7306_control,   
	   	(tuner_status_callback)ali_nim_vz7306_status,  
	   	(tuner_close_callback)ali_nim_vz7306_close
	 },
#endif		
#ifdef CONFIG_TDA18250
	{
	    NIM_DVBC,
	   	TDA18250,
	   	(tuner_init_callback)tun_tda18250_init,     
	   	(tuner_control_callback)tun_tda18250_control_X,   
	   	(tuner_status_callback)tun_tda18250_status,  
	   	(tuner_close_callback)tun_tda18250_close
	 },
#endif	

#ifdef CONFIG_TDA18250_AB
	{
	    NIM_DVBC,
	   	TDA18250AB,
	   	(tuner_init_callback)tun_tda18250ab_init,     
	   	(tuner_control_callback)tun_tda18250ab_control,   
	   	(tuner_status_callback)tun_tda18250ab_status,  
	   	(tuner_close_callback)NULL
	 },
#endif	


#ifdef CONFIG_OTHER_TUNER
	{
	    NIM_DVBC,
	   	DCT70701,
	   	(tuner_init_callback)tun_dct70701_init,     
	   	(tuner_control_callback)tun_dct70701_control,   
	   	(tuner_status_callback)tun_dct70701_status,  
	   	(tuner_close_callback)tun_dct70701_release
	 },
#endif	
#ifdef CONFIG_CXD2837
	{
	    NIM_DVBT,
	   	CXD2872,
	    (tuner_init_callback)tun_cxd_ascot3_Init,
        (tuner_control_callback)tun_cxd_ascot3_control,
        (tuner_status_callback)tun_cxd_ascot3_status,
        (tuner_close_callback)tun_cxd_ascot3_release
	 },
#endif	
#ifdef CONFIG_CXD2838
	{
	    NIM_DVBT,
	   	CXD2872,
	    (tuner_init_callback)tun_cxd_ascot3_Init,
        (tuner_control_callback)tun_cxd_ascot3_control,
        (tuner_status_callback)tun_cxd_ascot3_status,
        (tuner_close_callback)tun_cxd_ascot3_release
	 },
#endif	
};


TUNER_IO_FUNC *tuner_setup(UINT32 type,UINT32 tuner_id)
{

	UINT32 i = 0;
    UINT32 tuner_count =0 ;
	tuner_count = sizeof(g_tuner_array)/sizeof(TUNER_IO_FUNC);
    
	for(i = 0;i<tuner_count;i++)
	{
		if((g_tuner_array[i].tuner_id == tuner_id) && (g_tuner_array[i].nim_type==type))
		{
			PRINTK_INFO("[%s]line=%d,found tuner,tunerid=0x%x,i=%d!\n",__FUNCTION__,__LINE__,(unsigned int)tuner_id,(int)i);
			
			
			return &g_tuner_array[i];
		}
	}

    PRINTK_INFO("[%s]line=%d,no found tuner,tuner_id=0x%x,return null!\n",__FUNCTION__,__LINE__,(unsigned int)tuner_id);
	return NULL;
	
}


