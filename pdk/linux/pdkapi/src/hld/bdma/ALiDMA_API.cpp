#include <hld_cfg.h>
#include "ALiBDMA.h"
#include "ALiBDMAreg.h"
//#include "ALiDMA_API.h"
#include <hld/bdma/ALiDMA_API.h>
//#include "alihwdma-ioctl.h"
#include <linux/alihwdma-ioctl.h>
#include <sys/mman.h>
#ifdef ADR_ALIDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <sys/ioctl.h>
#include <stdlib.h>   //  c++ compiler needed
#include <unistd.h> //For usleep function
#include <cstdio>

extern ALiBDMA_thread threads_bdma[];

#define DMA_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DMA, fmt, ##args); \
			} while(0)


/*=============================================
Author           : Allen_chen 20120702
Function name    : ALiDMA_debug
description      :set memory value
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_debug(unsigned int* buf_data, unsigned int ctl)
{
#if 0
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_debugDump debug;
	debug.dump_ctl = ctl;
	debug.dst_address = (unsigned int)buf_data;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_DEBUG_DUMP, &debug);
	#endif
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Allen_chen 20110725
Function name    : ALiDMA_memset
description      :set memory value
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_memset(unsigned int* buf_data, unsigned char val, unsigned int length, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_job job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.bits_pixel = ALIBDMA_8_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	job.copy_length = length;
	job.type = ALIBDMA_SET;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_START, &job);
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Allen_chen 20110725
Function name    : ALiDMA_memcpy
description      :copy memory
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_memcpy(void* buf_src, void* buf_dest, unsigned int length, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_job job;
	job.source_data = (unsigned int)buf_src;
	job.destination_address = (unsigned int)buf_dest;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_data, job.destination_address);
	job.copy_length = length;
	job.type = ALIBDMA_COPY;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_START, &job);
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Allen_chen 20110725
Function name    : ALiOVG_blit
description      :copy memory
Change list      : 
===============================================*/
DMAPI_Result ALiOVG_blit(void* buf_src, void* buf_dest, unsigned int length, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiOVG_blit ID = %d\n", id);
	ali_hwdma_job job;
	job.source_data = (unsigned int)buf_src;
	job.destination_address = (unsigned int)buf_dest;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_data, job.destination_address);
	job.copy_length = length;
	job.type = ALIBDMA_OVG_BLIT;
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_START, &job);
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Allen_chen 20110725
Function name    : ALiDMA_memcpr
description      :memory compare
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_memcpr(char* buf_src, char* buf_dest, unsigned int length)
{
	
	unsigned int i = 0;
	for(i = 0;i < length ; i++)
	{
		if(*(buf_src+i) != *(buf_dest+i))
		{
			DMA_DBG_PRINT("ALiDMA_memcpr error src_adr = %x, dst_adr = %x ,src = %d , dst = %d , index = %d\n"
			,buf_src, buf_dest, *(buf_src+i), *(buf_dest+i) , i );
		/*	DMA_DBG_PRINT("compared error src = %d , dst = %d , index = %d\n"
			, *(buf_src+i+1), *(buf_dest+i+1) , i );
			DMA_DBG_PRINT("compared error src = %d , dst = %d , index = %d\n"
			, *(buf_src+i+2), *(buf_dest+i+2) , i );
			DMA_DBG_PRINT("compared error src = %d , dst = %d , index = %d\n"
			, *(buf_src+i+3), *(buf_dest+i+3) , i );*/
			return DMAPI_ERROR;
		}
	}
	return DMAPI_NO_ERROR;
}


DMAPI_Result ALiDMA_memset_RGB(unsigned int* buf_data, unsigned short val, unsigned int length, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_job job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.bits_pixel = ALIBDMA_16_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	job.copy_length = length;
	job.type = ALIBDMA_SET;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_START, &job);
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_memset_RGBA(unsigned int* buf_data, unsigned int val, unsigned int length, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_job job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.bits_pixel = ALIBDMA_32_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	job.copy_length = length;
	job.type = ALIBDMA_SET;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_START, &job);
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_blit(void* buf_src, void* buf_dest
													, unsigned int src_x, unsigned int src_y
													, unsigned int dst_x, unsigned int dst_y
													, unsigned int src_stride, unsigned int dst_stride
													, unsigned int width, unsigned int height
													, unsigned int bits_pixel, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_2Djob job;
	job.source_data = (unsigned int)buf_src;
	job.destination_address = (unsigned int)buf_dest;
	job.src_x = src_x;
	job.src_y = src_y;
	job.dst_x = dst_x;
	job.dst_y = dst_y;
	job.src_stride = src_stride;
	job.dst_stride = dst_stride;
	job.height = height;
	job.width = width;
	job.bits_pixel = (bits_pixel == 8) ? ALIBDMA_8_BIT :
									 (bits_pixel == 16) ? ALIBDMA_16_BIT :
									 (bits_pixel == 24) ? ALIBDMA_24_BIT : ALIBDMA_32_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_data, job.destination_address);
	//job.copy_length = length;
	job.type = ALIBDMA_COPY;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_2D_START, &job);
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_memset_RGB(unsigned int* buf_data, unsigned short val
															, unsigned int dst_x, unsigned int dst_y
															, unsigned int dst_stride
															, unsigned int width, unsigned int height
															, unsigned int bits_pixel, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_2Djob job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.src_x = 0;
	job.src_y = 0;
	job.dst_x = dst_x;
	job.dst_y = dst_y;
	job.src_stride = 0;
	job.dst_stride = dst_stride;
	job.height = height;
	job.width = width;
	job.bits_pixel = ALIBDMA_16_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	
	job.type = ALIBDMA_SET;
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_2D_START, &job);
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_memset_RGBA(unsigned int* buf_data, unsigned int val
															 , unsigned int dst_x, unsigned int dst_y
															 , unsigned int dst_stride
															 , unsigned int width, unsigned int height
															 , unsigned int bits_pixel, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_2Djob job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.src_x = 0;
	job.src_y = 0;
	job.dst_x = dst_x;
	job.dst_y = dst_y;
	job.src_stride = 0;
	job.dst_stride = dst_stride;
	job.height = height;
	job.width = width;
	
	job.bits_pixel = ALIBDMA_32_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	
	job.type = ALIBDMA_SET;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_2D_START, &job);
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_memset_A8(unsigned int* buf_data, unsigned char val
															 , unsigned int dst_x, unsigned int dst_y
															 , unsigned int dst_stride
															 , unsigned int width, unsigned int height
															 , unsigned int bits_pixel, int mmu_mode)
{
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_memcpy ID = %d\n", id);
	ali_hwdma_2Djob job;
	job.source_data = (unsigned int)val;
	job.destination_address = (unsigned int)buf_data;
	job.src_x = 0;
	job.src_y = 0;
	job.dst_x = dst_x;
	job.dst_y = dst_y;
	job.src_stride = 0;
	job.dst_stride = dst_stride;
	job.height = height;
	job.width = width;
	
	job.bits_pixel = ALIBDMA_8_BIT;
	//DMA_DBG_PRINT("buf_src = %08x, buf_dest = %08x\n",job.source_address, job.destination_address);
	
	job.type = ALIBDMA_SET;
#ifdef OVG_Enable_CHK	
	job.ovg_sync = threads_bdma[id].Ovg_sync;
	threads_bdma[id].Ovg_sync = 0;	
#endif	
	job.mmu_mode = (mmu_mode == 1) ? MMU_SRC_ON_DST_ON : MMU_SRC_OFF_DST_OFF;
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_2D_START, &job);
	return DMAPI_NO_ERROR;
}

unsigned int get_pixel_val(char* buf, unsigned int x, unsigned int y
													, unsigned int stride, unsigned int byte_pixel)
{
	unsigned int* buffer = (unsigned int*)buf;
	unsigned int address = (unsigned int)buffer + (y*stride) + (x*byte_pixel);
	//DMA_DBG_PRINT("address = %x, buf =%x, stride %d, x %d,y %d , pixel %d \n"
	//		, address, buffer, stride, x,y,byte_pixel);
	return *((unsigned int*)address);
	
	
}

DMAPI_Result ALiDMA_2D_memcpr(char* buf_src, char* buf_dest
													, unsigned int src_x, unsigned int src_y
													, unsigned int dst_x, unsigned int dst_y
													, unsigned int src_stride, unsigned int dst_stride
													, unsigned int width, unsigned int height
													, unsigned int byte_pixel)
{
	
	unsigned int i = 0, j = 0;
	unsigned int k = 0, l = 0;
	//DMA_DBG_PRINT("2d compare start! \n");
	for(i = dst_y,k=src_y ;i < height + dst_y/**byte_pixel*/ ; i++)
	{
		for(j = dst_x,l=src_x ;j < width + dst_x/**byte_pixel*/ ; j++)
		{
			unsigned int src_data, dst_data;
			src_data = get_pixel_val(buf_src, l, k, src_stride, byte_pixel );
			dst_data = get_pixel_val(buf_dest, j, i, dst_stride, byte_pixel );
			if(src_data != dst_data)
			{
				DMA_DBG_PRINT("ALiDMA_2D_memcpr error src[%x] = %x, dst[%x] = %x ,x = %d , y = %d \n"
				,buf_src,src_data, buf_dest, dst_data, i ,j );
		
				return DMAPI_ERROR;
			}
			
		}
	}
	//DMA_DBG_PRINT("2d compare OK! \n");
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_mem_val_cpr(char* buf_dest
													, unsigned int value
													, unsigned int dst_x, unsigned int dst_y
													, unsigned int dst_stride
													, unsigned int width, unsigned int height
													, unsigned int byte_pixel)
{
	
	unsigned int i = 0, j = 0;
//	DMA_DBG_PRINT("2d RGB compare start! \n");
	for(i = dst_y ;i < width*byte_pixel ; i++)
	{
		for(j = dst_x ;j < width*byte_pixel ; j++)
		{
			unsigned int src_data, dst_data;
			
			dst_data = get_pixel_val(buf_dest, dst_x, dst_y, dst_stride, byte_pixel );
			if(value != dst_data)
			{
				DMA_DBG_PRINT("ALiDMA_2D_mem_val_cpr error src = %x, dst[%x] = %x ,x = %d , y = %d \n"
				,src_data, buf_dest, dst_data, i ,j );
		
				return DMAPI_ERROR;
			}
			
		}
	}
	//DMA_DBG_PRINT("2d compare OK! \n");
	return DMAPI_NO_ERROR;
}

unsigned short get_pixel_val_short(char* buf, unsigned int x, unsigned int y
													, unsigned int stride, unsigned int byte_pixel)
{
	unsigned short* buffer = (unsigned short*)buf;
	return *(buffer + y*stride + x*byte_pixel);
	
	
}

unsigned char get_pixel_val_char(char* buf, unsigned int x, unsigned int y
													, unsigned int stride, unsigned int byte_pixel)
{
	unsigned char* buffer = (unsigned char*)buf;
	return *(buffer + y*stride + x*byte_pixel);
	
	
}

DMAPI_Result ALiDMA_2D_mem_val_cpr_rgb(char* buf_dest
													, unsigned short value
													, unsigned int dst_x, unsigned int dst_y
													, unsigned int dst_stride
													, unsigned int width, unsigned int height
													, unsigned int byte_pixel)
{
	
	unsigned short i = 0, j = 0;
	//DMA_DBG_PRINT("2d compare start! \n");
	for(i = dst_y ;i < width*byte_pixel ; i++)
	{
		for(j = dst_x ;j < width*byte_pixel ; j++)
		{
			unsigned short src_data, dst_data;
			
			dst_data = get_pixel_val_short(buf_dest, dst_x, dst_y, dst_stride, byte_pixel );
			if(value != dst_data)
			{
				DMA_DBG_PRINT("ALiDMA_2D_mem_val_cpr_rgb error src = %x, dst[%x] = %x ,x = %d , y = %d \n"
				,src_data, buf_dest, dst_data, i ,j );
		
				return DMAPI_ERROR;
			}
			
		}
	}
	//DMA_DBG_PRINT("2d compare OK! \n");
	return DMAPI_NO_ERROR;
}

DMAPI_Result ALiDMA_2D_mem_val_cpr_a8(char* buf_dest
													, unsigned char value
													, unsigned int dst_x, unsigned int dst_y
													, unsigned int dst_stride
													, unsigned int width, unsigned int height
													, unsigned int byte_pixel)
{
	
	unsigned char i = 0, j = 0;
	//DMA_DBG_PRINT("2d compare start! \n");
	for(i = dst_y ;i < width*byte_pixel ; i++)
	{
		for(j = dst_x ;j < width*byte_pixel ; j++)
		{
			unsigned char src_data, dst_data;
			
			dst_data = get_pixel_val_char(buf_dest, dst_x, dst_y, dst_stride, byte_pixel );
			if(value != dst_data)
			{
				DMA_DBG_PRINT("ALiDMA_2D_mem_val_cpr_a8 error value = %x, dst[%x] = %x ,x = %d , y = %d \n"
				,value, (int)buf_dest, dst_data, i ,j );
		
				return DMAPI_ERROR;
			}
			
		}
	}
	//DMA_DBG_PRINT("2d compare OK! \n");
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Lucas_Lai 20121102
Function name    : ALiDMA_set_block_mode
description      : Set BDMA Module to block mode/non block mode.
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_set_block_mode(/*bool*/unsigned int enable)
{
	ALiDMAGetThreadID();
//	DMA_DBG_PRINT("ALiDMA_set_block_mode Set to %s mode\n", (enable) ? "block":"async");
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_SET_BLOCK_MODE, enable);
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Lucas_Lai 20121102
Function name    : ALiDMA_wait_finish
description      : Wait BDMA Module HW Finish in non block mode.
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_wait_finish(unsigned int timeout)
{	
	ALiDMAGetThreadID();
	//DMA_DBG_PRINT("ALiDMA_wait_finish start\n");
	ioctl(threads_bdma[id].devfd, ALI_HWDMA_WAIT_FINISH_TIMEOUT, timeout);
	return DMAPI_NO_ERROR;
}

/*=============================================
Author           : Allen_Chen 20121102
Function name    : ALiDMA_wait_finish
description      : make BDMA wait OVG finish
Change list      : 
===============================================*/
DMAPI_Result ALiDMA_syncOVG(void)
{	
	ALiDMAGetThreadID();
	if (threads_bdma[id].Ovg_exist)
		threads_bdma[id].Ovg_sync = 1;
	else
		threads_bdma[id].Ovg_sync = 0;
	return DMAPI_NO_ERROR;
}

