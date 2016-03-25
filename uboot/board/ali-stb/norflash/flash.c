/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifdef CONFIG_NORFLASH_ALI
#include "sto_dev.h"
#include "flash_data.h"
#include "flash.h"

//flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips */
static struct sto_device *flash_dev = NULL;

#include "sys_define.h"
int sys_ic_get_chip_id()   
{  
      unsigned int id = (*(volatile unsigned int *)0xb8000000)  >> 16;
    if( 0x3701 == id )
        return ALI_C3701;
    else
        return ALI_S3602F ;
}

int sys_ic_get_rev_id()   
{
    return IC_REV_4 ;
}


int sys_ic_is_M3202 () 
{ 
    return (sys_ic_get_chip_id() == ALI_M3202C) ;
}

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
//void bl_init_flash()
unsigned long flash_init (void)
{
        *((unsigned long *)0xb8000080) |=  0x00004000;   //reset flash
        osal_task_sleep(10);
        *((unsigned long *)0xb8000080) =  0x00000000;
        *((unsigned long *)0xb802e098) |=  0xc6000000;  //for flash can read 4M

        //flash_info_sl_init();

        sto_local_sflash_attach(NULL);

        flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
        if (flash_dev == NULL)
        {
                printf("Can't find FLASH device!\n");
                return 0;
        }

        sto_open(flash_dev);
        sto_chunk_init(0, flash_dev->totol_size);
        //printf("flash init OK,flash size is:0x%x!\n",flash_dev->totol_size);

        return (flash_dev->totol_size);
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	printf ("write_buff not implemented\n");
	return (-1);
}

//Load Main/Kernel and See code from Nor flash.
void StartAppInFlash(u8 LoadOnly)
{
        u_char *entry;
        char s[100];;
        UINT32 chid,offset;
        UINT32 code_len;
        UINT8 *codestart;

        codestart = (void *)ADDR_LOAD_SEE;
        chid = CHUNKID_SEECODE;
        offset = sto_chunk_goto(&chid, CHUNKID_SEECODE_MASK, 1);
        code_len = sto_fetch_long((UINT32)offset + CHUNK_LENGTH);
        printf("Load see Code...  offset=%08x, len=0x%x, TO Ram: 0x%x\n", offset,code_len,codestart);
        sto_get_data(flash_dev, (void *)codestart, offset + CHUNK_HEADER_SIZE, code_len);

        entry = codestart = (void *)ADDR_LOAD_MAIN;
        chid = CHUNKID_MAINCODE;
        offset = sto_chunk_goto(&chid, CHUNKID_MAINCODE_MASK, 1);
        code_len = sto_fetch_long((UINT32)offset + CHUNK_LENGTH);
        printf("Load Main Code...  offset=%08x, len=0x%x, TO Ram: 0x%x\n", offset,code_len,codestart);
        sto_get_data(flash_dev, (void *)codestart, offset + CHUNK_HEADER_SIZE, code_len);


       if (1 != LoadOnly)
       {
	     printf("leave u-boot, Starting APP...\n");
            sprintf(s,"bootm 0x%08x",ADDR_LOAD_MAIN);
            run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
        }
	//exec(entry);
}	
#endif

