#include <common.h>
#include <nand.h>
#include "block_sector.h"
#include "bootargs.h"
#include <alidefinition/adf_boot.h>


static struct boot_args *bootargs=NULL;

struct boot_args * get_bootargs()
{
	loff_t args_off;
	loff_t offset;    
	size_t len;
	nand_info_t *nand = &nand_info[nand_curr_device];
	unsigned char adatbuf[0x200000+0x020];
	unsigned char *datbuf=NULL;
//	unsigned char datbuf[0x200000];
	unsigned int i;

     const char *partName = "bootargs";
	int ret = -1;
	
	if (bootargs)
		return bootargs;
		
	//adds for cache line size align
	datbuf = (u8 *)(((u32)adatbuf + (u32)0x1F) & (~(u32)0x1F));
	
	MG_Setup_CRC_Table();

	bootargs = (struct boot_args*)malloc(sizeof(struct boot_args));

#if defined(CONFIG_NAND_ALI) 
	args_off =  nand->writesize*2048;	//-->16M(8K*2048)  /  8M(4K*2048) 
	//datbuf = (unsigned char *)malloc(nand->erasesize);
	
		//profile_dbg("nand->writesize:%x, nand->erasesize:%x  \n", nand->writesize,nand->erasesize);
	
	for (i=0;i<4;i++)
	{
		offset = args_off+i*nand->erasesize;
		len = nand->erasesize;
		profile_dbg("nand->writesize:%x, nand->erasesize:%x  \n", nand->writesize,nand->erasesize);
		profile_dbg("get_bootargs, offset:%llx, len:%x  \n", offset,len);
		if (nand_block_isbad(nand, offset&~(nand->erasesize - 1))) 
		{
			printf("block is bad,offset:0x%x 0x%x\n",offset);
			continue;
		}
	
		if (nand_read(nand, offset, &len, datbuf))		
		{
			printf("Read error happen on <get_bootargs>\n");
			//free(datbuf);
			return NULL;		
		}
		
		//profile_dbg("get_bootargs, offset:%llx, len:%llx  \n", offset,len);
		//dump_reg(datbuf,sizeof(struct boot_args));

		/* check block data */
		if (_sector_crc_check(datbuf, nand->erasesize)>0)
		{
			break;
		}
	}

	if (i==4)
	{
		printf("can not find bootargs\n");
		//free(datbuf);
		return NULL;
	}

	memcpy(bootargs, datbuf+12, sizeof(struct boot_args));
#endif

 #if defined(CONFIG_ALI_MMC)      //emmc

	ret = load_part_sector_emmc(partName, (u_char *)bootargs, sizeof(struct boot_args));
	if (ret < 0)
	{
		printf("get deviceinfo fail.\n");	
		free(bootargs);
		return NULL;
	}
#endif

	//free(datbuf);
	return bootargs;	
}

int set_mtdparts(void)
{
	struct boot_args *bootargs = get_bootargs();
	char *cmdline = bootargs->cmdline_rcv;
	char *mtdparts = strstr(cmdline,"mtdparts=");

	setenv("mtdparts", mtdparts);

#if defined(CONFIG_NAND_ALI) 
	char mtdids[128];
	char *p, *s;
	int i;
	p = mtdparts+strlen("mtdparts=");
	strcpy(mtdids, "nand0=");
	s = mtdids+strlen(mtdids);
	while (*p!=':')
	{
		*s++ = *p++;
	}
	*s = 0;
		
	setenv("mtdids", mtdids);

	do_mtdparts(0, 0, 1, 0);
#endif
	return 0;
}

int set_gptparts_emmc(void)
{	
	struct boot_args *bootargs = get_bootargs();
	
	char *cmdline = bootargs->cmdline_rcv;
	
	char *mtdparts = strstr(cmdline,"mtdparts=");
	
	setenv("mtdparts", mtdparts);

#if 0
	char mtdids[128];
	char *p, *s;
	int i;
	p = mtdparts+strlen("mtdparts=");
	strcpy(mtdids, "nand0=");
	s = mtdids+strlen(mtdids);
	while (*p!=':')
	{
		*s++ = *p++;
	}
	*s = 0;

	printf("\n[%s]--%d\n",__FUNCTION__,__LINE__);
		
	setenv("mtdids", mtdids);
#endif
	//do_mtdparts(0, 0, 1, 0);

	return 0;
}


int set_cmdline(int isRecovery)
{
	struct boot_args *bootargs = get_bootargs();
	char *cmdline;

	if (isRecovery)
		cmdline = bootargs->cmdline_rcv;
	else
		cmdline = bootargs->cmdline;
	
	setenv("bootargs", cmdline);
		
	return 0;
}

unsigned int get_memmap(unsigned char **memmap, int isRecovery)
{
	struct boot_args *bootargs = get_bootargs();
	unsigned int size;
    unsigned int i;
	unsigned int Addr;
    volatile ADF_BOOT_BOARD_INFO *memory_map_info = NULL;

	if (bootargs == NULL)
	{
		printf("get memmap fail.\n");			
		*memmap = NULL;
		return 0;
	}
	
	if (isRecovery)
	{
		*memmap = &bootargs->meminfo_rcv;
		size = bootargs->meminfo_size_rcv;
	}
	else
	{
		*memmap = &bootargs->meminfo;
		size = bootargs->meminfo_size;
	}

    /*Check memory map address*/
    memory_map_info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

    if(memory_map_info->memmap_info.memory_size == 512)
    {
        /*Check vcap buffer start address, default value:0x3c83000*/ 
        Addr = memory_map_info->memmap_info.vcap_buffer_start_512m;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.vcap_buffer_start_512m = 0x3c83000;
        }
        
        /*Check vbv start address, default value:0x547d000*/ 
        Addr = memory_map_info->memmap_info.vbv_start_512m;
        if(Addr == 0 || (Addr && 0xff != 0))
        {
            memory_map_info->memmap_info.vbv_start_512m = 0x547d000;
        }
        
        /*Check video frame buffer start address, default value:0x6000000*/ 
        Addr = memory_map_info->memmap_info.video_fb_start_512m;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.video_fb_start_512m = 0x6000000;
        }
        
        /*Check video frame buffer end address, default value:0x8000000*/ 
        Addr = memory_map_info->memmap_info.video_fb_end_512m;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.video_fb_end_512m = 0x8000000;
        }
        
        /*Check cmd queue buffer start address, default value:0x3fbf000*/ 
        Addr = memory_map_info->memmap_info.cmd_queue_buffer_start_512m;
        if(Addr == 0 || (Addr && 0x7ff != 0))
        {
            memory_map_info->memmap_info.cmd_queue_buffer_start_512m = 0x3fbf000;
        }
    }
    else
    {
        /*Check vcap buffer start address, default value:0x3c83000*/ 
        Addr = memory_map_info->memmap_info.vcap_buffer_start_1g;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.vcap_buffer_start_1g = 0x3c83000;
        }
        
        /*Check vbv start address, default value:0x05A00000*/ 
        Addr = memory_map_info->memmap_info.vbv_start_1g;
        if(Addr == 0 || (Addr && 0xff != 0))
        {
            memory_map_info->memmap_info.vbv_start_1g = 0x05A00000;
        }
        
        /*Check video frame buffer start address, default value:0x08100000*/ 
        Addr = memory_map_info->memmap_info.video_fb_start_1g;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.video_fb_start_1g = 0x08100000;
        }
        
        /*Check video frame buffer end address, default value:0x0DB00000*/ 
        Addr = memory_map_info->memmap_info.video_fb_end_1g;
        if(Addr == 0 || (Addr && 0xfff != 0))
        {
            memory_map_info->memmap_info.video_fb_end_1g = 0x0DB00000;
        }
        
        /*Check cmd queue buffer start address, default value:0x3fbf000*/ 
        Addr = memory_map_info->memmap_info.cmd_queue_buffer_start_1g;
        if(Addr == 0 || (Addr && 0x7ff != 0))
        {
            memory_map_info->memmap_info.cmd_queue_buffer_start_1g = 0x3fbf000;
        }    
    }

	return size;
}

unsigned int get_tveinfo(unsigned char **tveinfo)
{
	struct boot_args *bootargs = get_bootargs();

	if (bootargs == NULL)
	{
		printf("get tveinfo fail.\n");		
		*tveinfo = NULL;
		return 0;
	}

	*tveinfo = &bootargs->tveinfo;
	return bootargs->tveinfo_size;
}

unsigned int get_registerinfo(unsigned char **registerinfo)
{
	struct boot_args *bootargs = get_bootargs();

	if (bootargs == NULL)
	{
		printf("get registerinfo fail.\n");			
		*registerinfo = NULL;
		return 0;
	}

	*registerinfo = &bootargs->registerinfo;
	return bootargs->registerinfo_size;
}


