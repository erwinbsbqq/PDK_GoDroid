#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>

#include <linux/mm.h>

#include <ali_soc_common.h>
#include "dmx_see_interface.h"
#include "dmx_stack.h"


static int g_dmx_internal_init = 0;

void dmx_internal_init(void)
{
        __u32 chip_id;

        
        if (0 == g_dmx_internal_init)
        {
            dmx_see_init();

            /* Init data engine for dev ID 0, this engine will fetch data from hw_interface 0.
	        */
            dmx_data_engine_module_init_kern(ALI_HWDMX0_OUTPUT_HWIF_ID, ALI_HWDMX0_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);

            /* Init data engine for hw_interface 1, this engine will fetch data from hw_interface 1.
        	*/
            dmx_data_engine_module_init_kern(ALI_HWDMX1_OUTPUT_HWIF_ID, ALI_HWDMX1_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
    
	chip_id = ali_sys_ic_get_chip_id();

        if (ALI_C3921 == chip_id)
        {    	
            /* Init data engine for hw_interface 2, this engine will fetch data from hw_interface 2.
        	*/
            dmx_data_engine_module_init_kern(ALI_HWDMX2_OUTPUT_HWIF_ID, ALI_HWDMX2_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
        	
            /* Init data engine for hw_interface 3, this engine will fetch data from hw_interface 3.
        	*/
            dmx_data_engine_module_init_kern(ALI_HWDMX3_OUTPUT_HWIF_ID, ALI_HWDMX3_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
    	}
   
    	dmx_data_engine_module_init_usr(ALI_SWDMX0_OUTPUT_HWIF_ID, ALI_SWDMX0_ENGINE_NAME, DMX_DATA_ENGINE_SRC_VIRTUAL_HW);

	    /* Enable internal engine, which will fetch data from SEE to main.
		*/				
		dmx_data_engine_module_init_kern(ALI_SEETOMAIN_BUF_HWIF_ID, ALI_DMX_SEE2MAIN0_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);

        /* Support linux legacy interface.
         * legacy input interface and output interface are independent to 
         * main line input interface and output interface, hence their interface_id
         * are stand by there own.
         */
        dmx_channel_module_legacy_init();

        dmx_statistic_show_init();


            g_dmx_internal_init = 1;
        }        
}

static int __init dmx_init(void)
{ 
    __u32 chip_id;
	
    DMX_INIT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    /* Step 1: Init SEE part.
     * Move to xxx_open() for standby test. Root cause still unkonwn.
     * Date:2014.11.13
	*/
	//dmx_see_init();


	/* Step 2: Init HW independent part.
	*/
    dmx_pid_flt_module_init();

    dmx_ts_flt_module_init();

    dmx_sec_flt_module_init();

    dmx_data_buf_module_init();

    dmx_stream_module_init();

    dmx_mutex_module_init();

	
	/* Step 3: Init HW dependent part. 
	 */
	/*
	 * We abstract dmx data source buffer and it's control mechanism as a "hw_interface", which contains a
	 * chunk of physically continuous SRAM buffer and a bounch of fuctions to manupulate it.
	 * Each of this "hw_interface" is assigned an ID for identification.
	 * A "hw_interface" could be bounded to a "data_engine" to retrive data from it;
	 * A "hw_interface" could also be bounded to a "linux_interface" to communicate with userspace.
	 */
	 
    /* Init HW DMX 0, assign hw_interface ID 0 to it. 
	 */
    dmx_hw_interface_init(ALI_HWDMX0_OUTPUT_HWIF_ID, DMX_HW_INTERFACE_TYPE_HW, 0);

    #if 0 //
    /* Init data engine for dev ID 0, this engine will fetch data from hw_interface 0.
	*/
    dmx_data_engine_module_init_kern(ALI_HWDMX0_OUTPUT_HWIF_ID, ALI_HWDMX0_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
    #endif

	/* Init linux interface for dev ID 0, user space dev name defined by ALI_HWDMX0_OUTPUT_NAME.
	*/
    dmx_linux_output_interface_init(ALI_HWDMX0_OUTPUT_HWIF_ID, ALI_HWDMX0_OUTPUT_NAME);

    dmx_subt_if_init(ALI_HWDMX0_OUTPUT_HWIF_ID, "ali_hwdmx0_subt");


    /* Init HW DMX 1, assign hw_interface ID 1 to it. 
	 */
    dmx_hw_interface_init(ALI_HWDMX1_OUTPUT_HWIF_ID, DMX_HW_INTERFACE_TYPE_HW, 1);

    #if 0 //
    /* Init data engine for hw_interface 1, this engine will fetch data from hw_interface 1.
	*/
    dmx_data_engine_module_init_kern(ALI_HWDMX1_OUTPUT_HWIF_ID, ALI_HWDMX1_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
    #endif
    
	chip_id = ali_sys_ic_get_chip_id();

    if (ALI_C3921 == chip_id)
    {
    	/* Init linux interface for hw_interface 1, user space could access serivce provided by 
    	 * this interface by dev name defined by ALI_HWDMX1_OUTPUT_NAME.
    	 */	
        dmx_linux_output_interface_init(ALI_HWDMX1_OUTPUT_HWIF_ID, ALI_HWDMX1_OUTPUT_NAME);
    	
        dmx_subt_if_init(ALI_HWDMX1_OUTPUT_HWIF_ID, "ali_hwdmx1_subt");
    
        /* Init HW DMX 2, assign hw_interface ID 2 to it. 
    	 */
        dmx_hw_interface_init(ALI_HWDMX2_OUTPUT_HWIF_ID, DMX_HW_INTERFACE_TYPE_HW, 2);

         #if 0 //
        /* Init data engine for hw_interface 2, this engine will fetch data from hw_interface 2.
    	*/
        dmx_data_engine_module_init_kern(ALI_HWDMX2_OUTPUT_HWIF_ID, ALI_HWDMX2_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
         #endif
    
    	/* Init linux interface for hw_interface 2, user space could access serivce provided by 
    	 * this interface by dev name defined by ALI_HWDMX2_OUTPUT_NAME.
    	 */	
        dmx_linux_output_interface_init(ALI_HWDMX2_OUTPUT_HWIF_ID, ALI_HWDMX2_OUTPUT_NAME);
    
        dmx_subt_if_init(ALI_HWDMX2_OUTPUT_HWIF_ID, "ali_hwdmx2_subt");
    	
    
        /* Init HW DMX 3, assign hw_interface ID 3 to it. 
    	 */
        dmx_hw_interface_init(ALI_HWDMX3_OUTPUT_HWIF_ID, DMX_HW_INTERFACE_TYPE_HW, 3);

         #if 0 //
        /* Init data engine for hw_interface 3, this engine will fetch data from hw_interface 3.
    	*/
        dmx_data_engine_module_init_kern(ALI_HWDMX3_OUTPUT_HWIF_ID, ALI_HWDMX3_ENGINE_NAME, DMX_DATA_ENGINE_SRC_REAL_HW);
         #endif
    
    	/* Init linux interface for hw_interface 3, user space could access serivce provided by 
    	 * this interface by dev name defined by ALI_HWDMX3_OUTPUT_NAME.
    	 */	
        dmx_linux_output_interface_init(ALI_HWDMX3_OUTPUT_HWIF_ID, ALI_HWDMX3_OUTPUT_NAME);
    
        dmx_subt_if_init(ALI_HWDMX3_OUTPUT_HWIF_ID, "ali_hwdmx3_subt");
	}

    /* Init SW DMX 0, assign ALI_SWDMX0_OUTPUT_HWIF_ID to it. 
	 */
    dmx_hw_interface_init(ALI_SWDMX0_OUTPUT_HWIF_ID, DMX_HW_INTERFACE_TYPE_USR, 0);

    #if 0 //
	dmx_data_engine_module_init_usr(ALI_SWDMX0_OUTPUT_HWIF_ID, ALI_SWDMX0_ENGINE_NAME, DMX_DATA_ENGINE_SRC_VIRTUAL_HW);
    #endif
    
    dmx_linux_output_interface_init(ALI_SWDMX0_OUTPUT_HWIF_ID, ALI_SWDMX0_OUTPUT_NAME);
	
    dmx_linux_input_interface_init(ALI_SWDMX0_OUTPUT_HWIF_ID, ALI_SWDMX0_INPUT_NAME);

    dmx_subt_if_init(ALI_SWDMX0_OUTPUT_HWIF_ID, "ali_swdmx0_subt");
	

#if 0
    dmx_hw_interface_module_init(4, DMX_HW_INTERFACE_TYPE_VIRTUAL, 1);
    
    dmx_data_engine_module_init(4, "ali_dmx_3_output", DMX_DATA_ENGINE_SRC_VIRTUAL_HW);
    
    dmx_linux_interface_module_init(4, "ali_dmx_3_output", DMX_LINUX_INTERFACE_TYPE_OUTPUT, 4);
    
    dmx_linux_interface_module_init(4, "ali_dmx_3_input", DMX_LINUX_INTERFACE_TYPE_INPUT, 1);
#endif

    /* Enable internal engine, which will fetch data from SEE to main.
	*/
    dmx_hw_interface_init(ALI_SEETOMAIN_BUF_HWIF_ID, DMX_HW_INTERFACE_TYPE_SEE, 0);

#if 1
    /* Support linux legacy interface.
     * legacy input interface and output interface are independent to 
     * main line input interface and output interface, hence their interface_id
     * are stand by there own.
     */
      #if 0 //
    dmx_channel_module_legacy_init();
      #endif
    
    dmx_linux_interface_module_legacy_init(ALI_HWDMX0_OUTPUT_HWIF_ID, "ali_m36_dmx_0", DMX_LINUX_INTERFACE_TYPE_OUTPUT, 0);

    dmx_linux_interface_module_legacy_init(ALI_HWDMX1_OUTPUT_HWIF_ID, "ali_m36_dmx_1", DMX_LINUX_INTERFACE_TYPE_OUTPUT, 1);

    dmx_linux_interface_module_legacy_init(ALI_SWDMX0_OUTPUT_HWIF_ID, "ali_dmx_pb_0_out", DMX_LINUX_INTERFACE_TYPE_OUTPUT, 2);

    dmx_linux_interface_module_legacy_init(ALI_SWDMX0_OUTPUT_HWIF_ID, "ali_dmx_pb_0_in", DMX_LINUX_INTERFACE_TYPE_INPUT, 0);

    //dmx_self_test_init();
#endif

    #if 0 //
    dmx_statistic_show_init();
    #endif

    DMX_INIT_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    return(0);
}


static void __exit dmx_exit(void)
{
    DMX_INIT_DEBUG(KERN_ALERT "Goodbye, cruel world\n");

    return;
}


module_init(dmx_init);
module_exit(dmx_exit);
MODULE_LICENSE("GPL");

