
/*Include performance debug dump function*/

#include "ali_ovg.h"
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/module.h>

#ifdef ALI_OVG_DUMP
struct ovg_time_record{
	unsigned int count;
	unsigned int dump;
	unsigned int max;
	unsigned int min;
	unsigned int deviation;
};

unsigned int num_ID = 19;
struct ovg_time_record time_dump[ ]
    = {{0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
 	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},
	 {0, 0, 0, 65536, 0},};

unsigned int time_temp[ ] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 0, 0 ,0, 0, 0 };

void ovg_getString(unsigned int index);

/*start timer*/
void ovg_record(unsigned int index)
{
	struct timeval now;
	do_gettimeofday(&now);
	time_temp[index] = (now.tv_sec * 1000000 + now.tv_usec);
}

/*End timer*/
void ovg_stop(unsigned int index)
{
	struct timeval now;
	unsigned int interval;
	do_gettimeofday(&now);
	interval = (now.tv_sec * 1000000 + now.tv_usec) - time_temp[index]; 
	
	time_dump[index].dump += interval;
	time_dump[index].count++;
	(interval > time_dump[index].max) ? time_dump[index].max = interval : 1;
	(interval < time_dump[index].min) ? time_dump[index].min = interval : 1;
	time_dump[index].deviation += interval * interval;
}

void ovg_dump(void)
{
// 1. total time, count and average time and standard deviation
	unsigned int index;
	unsigned int stddeviation;
	unsigned int average;

	for(index = 0 ; index < num_ID ; index++)
	{
		average = (time_dump[index].count > 0) ? 
			time_dump[index].dump / time_dump[index].count : 0;
		stddeviation =   (time_dump[index].count > 0) ?
			(time_dump[index].deviation / time_dump[index].count) : 0;
		
		ovg_getString(index+1);
		printk("total= %d ,", time_dump[index].dump);
		printk("count= %d ,", time_dump[index].count);		
		printk("average= %d ,", average);		
		printk("max= %d ,", time_dump[index].max);		
		printk("min= %d ,", time_dump[index].min);
		
		stddeviation = int_sqrt((unsigned int)( stddeviation - average*average) );
		printk("standard deviation= %d ", stddeviation);
		printk("\n");

		time_dump[index].dump = 0;
		time_dump[index].count = 0;
		time_dump[index].max = 0;
		time_dump[index].min = 65536;
		time_dump[index].deviation = 0;		
	}

	
}

void ovg_getString(unsigned int index)
{
	switch(index){
		case 1:
			printk("ALI_OPENVG_EnableCMDQ : ");
			break;
		case 2:
			printk("ALI_OPENVG_TessWaitTessFinish : ");
			break;
		case 3:
			printk("ALI_OPENVG_RastWaitRastFinish : ");
			break;
		case 4:
			printk("ALI_OPENVG_ImageWaitImageFinish : ");
			break;
		case 5:
			printk("ALI_OPENVG_MemoryAlloc : ");
			break;
		case 6:
			printk("ALI_OPENVG_MemoryFree : ");
			break;
		case 7:
			printk("ALI_OPENVG_SetData : ");			
			break;
		case 8:	
			printk("ALI_OPENVG_GetData : ");			
			break;
		case 9:
			printk("ALI_OPENVG_Reset : ");			
			break;
		case 10:
			printk("ALI_OPENVG_GetHWRegister : ");			
			break;
		case 11:
			printk("ALI_OPENVG_Virt2Phy : ");			
			break;
		case 12:
			printk("ALI_OPENVG_HWWaitHWFinish : ");			
			break;			
		case 13:
			printk("ALI_OPENVG_SetHWRegister : ");			
			break;	
		case 14:
			printk("ALI_OPENVG_PFNMapTable : ");			
			break;	
		case 15:
			printk("ALI_OPENVG_MMUMalloc : ");			
			break;
		case 16:
			printk("ALI_OPENVG_MMUFree : ");			
			break;
		case 17:
			printk("ALI_OPENVG_PGDTable : ");			
			break;
		case 18:
			printk("ALI_OPENVG_CheckAddress: ");			
			break;
		case 19:
			printk("mmap : ");			
			break;

	}
}

#endif

