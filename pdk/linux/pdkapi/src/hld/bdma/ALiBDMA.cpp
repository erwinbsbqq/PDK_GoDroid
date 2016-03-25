#include <hld_cfg.h>

//#include "alihwdma-ioctl.h"
#include <linux/alihwdma-ioctl.h>

#include "ALiBDMAreg.h"
#include "ALiBDMA.h"
#include <stdlib.h>
#include <math.h>

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <sys/mman.h>
#ifdef ADR_ALIDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#ifdef ADR_ALIDROID
#include <linux/sem.h>
#else
#include <sys/sem.h>
#endif
bool   ALiOpen_Done = false;
volatile unsigned int* HWRegAddr; // Virtual addr for 0xB8044000
ALiBDMA_thread threads_bdma[THREAD_NUM];

#define BDMA_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(BDMA, fmt, ##args); \
			} while(0)


ALiBDMA_thread::ALiBDMA_thread()
{
	flag = 0;
	thread_id = -1;
	devfd = -1;

	ALiOpen_Done = false;

//	curr_PaintType = 0x1B00;
}

ALiBDMA_thread::~ALiBDMA_thread()
{

}

/***************************************
		local function declare
****************************************/
int ALiDMAOpen(void)    
{ 
	ALiDMAGetThreadID()
	if(id == THREAD_NUM) //not created
	{
		for(id = 0; id<THREAD_NUM; id++)
		{
			if(threads_bdma[id].flag == 0)
			{
				threads_bdma[id].flag = 1;
				threads_bdma[id].thread_id = getthread;
				threads_bdma[id].devfd = open("/dev/hwDMA",O_RDWR|O_SYNC | O_CLOEXEC);
				if (threads_bdma[id].devfd < 0)
				{
				    BDMA_DBG_PRINT("%s,%d, Open File[%d] fail! \n", __FUNCTION__, __LINE__, id);
				}				
				
				break;
			}
		}
	}	
	BDMA_DBG_PRINT("ALiDMAOpen() ID = %d!\n", id);	
	if(threads_bdma[id].devfd < 0)
	{
		BDMA_DBG_PRINT("device open failed\n");
		return DRV_ERROR;
	}
#ifdef MAP_BDMA_IO
       BDMA_DBG_PRINT("HWRegAddr map before! \n");
	HWRegAddr = (unsigned int *)mmap(0, 28, PROT_READ|PROT_WRITE, MAP_FILE | MAP_SHARED, threads_bdma[id].devfd, 0x18044000);//mmap 0xB800A000	
    BDMA_DBG_PRINT("HWRegAddr map %08x ! \n",HWRegAddr);
#endif
	threads_bdma[id].ALiOpen_Done = true;
#ifdef OVG_Enable_CHK
  	FILE *fd = popen("lsmod | grep ali_ovg", "r");
	char buf[16];

	if (fread (buf, 1, sizeof (buf), fd) > 0){ // if there is some result the module must be loaded
		BDMA_DBG_PRINT ("====> OVG module dev/ali_ovg is loaded\n");
		threads_bdma[id].Ovg_exist=1;
	}else{
		BDMA_DBG_PRINT ("====> OVG module dev/ali_ovg is not exist !!!\n");
		threads_bdma[id].Ovg_exist=0;
	}
#endif
        return DRV_NO_ERROR;
}

void ALiDMAReset(void)
{
	ALiDMAGetThreadID()
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_Reset);
	//BDMA_DBG_PRINT("ALiDMAReset() !\n");
}

void ALiDMAClose(void)
{
	BDMA_DBG_PRINT("ALiDMAClose...\n");
	ALiDMAGetThreadID()
#ifdef MAP_BDMA_IO
	munmap((void*)HWRegAddr, 28);
#endif
	//close(threads_bdma[id].devfd);
}

void ALiDMA_regDump(void)
{
#ifdef MAP_BDMA_IO
	int i;
	for(i=0;i<28;i++)
		BDMA_DBG_PRINT("BDMA reg 0x%08x : %08x\n",i*4,HWRegAddr[i]);
#endif	

}

