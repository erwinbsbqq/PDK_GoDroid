/*
 * (C) Copyright 2003
 * Thomas.Lange@corelatus.se
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/io.h>
#include <sys_define.h>

#include "include/sys_config.h"



#define ALI_DEBUG_ON
#ifdef ALI_DEBUG_ON
    #define ALI_PRINTF   printf
#else
    #define ALI_PRINTF(...)	do{}while(0)
#endif

#define ALI_SOC_BASE			              (0xB8000000)

phys_size_t initdram(int board_type)
{
	/* Sdram is setup by assembler code */
	/* If memory could be changed, we should return the true value here */
	return MEM_SIZE*1024*1024;	
}

/* In arch/mips/cpu/cpu.c */
void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 );


int checkboard (void)
{

	u32 proc_id, chip_id;
	u32 cpu_feq, SEE_feq;
    u32 ChipGen = sys_ic_get_chip_id();

	cpu_feq = sys_ic_get_cpu_clock();
	SEE_feq = sys_ic_get_SEE_clock();
	
	proc_id = read_c0_prid();
	chip_id = *((volatile u32 *)(ALI_SOC_BASE));	
    printf ("CPU: MIPS32 , id: 0x%02x, rev: 0x%02x\n",
			   (proc_id >> 8) & 0xFF, proc_id & 0xFF);
	
	printf("Board :");
       if (ALI_S3901 == ChipGen)
       {
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x39010000:	printf("M3911 256Pin NMP 32bits DDR2\n");			break;
        		case 0x39010100:	printf("M3901 216Pin NMP 16bits DDR2\n");			break;			
        		case 0x39010200:	printf("M3701G 256Pin STB with QAM 32bits DDR2\n");	break;
        		case 0x39010300:	printf("M39xx 256Pin STB with CI 16bits DDR2\n");   break;								
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }
       else if (ALI_C3701 == ChipGen){
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x37010300:	printf("M3701C 144Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;								
        		case 0x37010200:	printf("M3701H 256Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
				case 0x37010100:	printf("M3701C 292Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }else if (ALI_S3503 == ChipGen){
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x35030300:	printf("M3503A 144Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;								
        		case 0x35030200:	printf("M3503A 128Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;
				case 0x35030100:	printf("M3503A 256Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;
				case 0x35030000:	printf("M3503A 292Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
			
       }
       else{
        	switch( chip_id & 0xFFFF0C0F )
        	{
        		case 0x36020002:	printf("M3602B\n");			break;
        		case 0x36030001:	printf("M3601E\n");			break;			
        		case 0x36030401:	printf("M3701E/M3701F\n");	break;
        		case 0x36030801:	printf("M3606/M3606C\n");		break;
        		case 0x36030C01:	printf("M3603\n");			break;									
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }

	//printf(" proc_id=0x%x\n", proc_id);

	return 0;
}
