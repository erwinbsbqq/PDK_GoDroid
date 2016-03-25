/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */
#ifndef __INCLUDE_ASM_TICKS_H_
#define __INCLUDE_ASM_TICKS_H_


//#define PERF_DEBUG

#ifndef PERF_DEBUG

#define PERF_TRACE(fmt,args...) do{}while(0)

#define INIT_TICKS() do {}while(0)
#define START_TICKS() do{}while(0)
#define END_TICKS() do {}while(0)
#define MEMCPY_TICKS(size) do {}while(0)

#define ALI_MEMCPY_TICKS(size) do{  }while(0)

#define NAND_COPY_TICKS(size) do { }while(0)


#else
#define PERF_TRACE(fmt,args...) printf("UBOOT PERF:"fmt,##args)

#define INIT_TICKS() 		init_ticks()
#define START_TICKS() 		start_ticks()
#define END_TICKS(size) 	end_ticks(size)
#define CACUL_S(size)		caculate_s(size)	
#define MEMCPY_TICKS(size) do { \
        END_TICKS() ;\
        debug("memcpy size = 0x%x bytes , duration = %d ms , speed = %d KB/s \n" ,\ 
                size,ticks, CACUL_S(size)  );\
        }while(0)

#define ALI_MEMCPY_TICKS(size) do { \
        END_TICKS() ;\
        debug("ali memcpy size = 0x%x bytes , duration = %d ms, speed = %d KB/s \n" ,\
                    size,ticks, CACUL_S(size) );\
        }while(0)

#define NAND_COPY_TICKS(size) do { \
        END_TICKS() ;\
        debug("ali_nand dma copy size = 0x%x bytes , duration = %d ms , speed = %d KB/s \n" , \
                size,ticks, CACUL_S(size)  );\
        }while(0)


#endif


#define MEM_PRINTF(fmt, arg...)	printf(fmt, ##arg)
#define MEM_DUMP(data, len) \
	do { \
		int i, l = (len); \
		for (i = 0; i < l; i++) \
			MEM_PRINTF(" 0x%02x", *((data) + i)); \
		MEM_PRINTF("\n"); \
	} while (0)

#endif