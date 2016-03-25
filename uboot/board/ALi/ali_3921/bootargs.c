#include <common.h>
#include <nand.h>
#include "block_sector.h"
#include "bootargs.h"

static struct boot_args *bootargs=NULL;

struct boot_args * get_bootargs()
{
	loff_t args_off;
	loff_t offset;    
	size_t len;
	nand_info_t *nand = &nand_info[nand_curr_device];
	//unsigned char datbuf[0x200000];
	unsigned int i;

    const char *partName = "bootargs";
	int ret = -1;
	
	if (bootargs)
		return bootargs;


	MG_Setup_CRC_Table();

	bootargs = (struct boot_args*)malloc(sizeof(struct boot_args));
	memset(bootargs, 0, sizeof(struct boot_args));

#if defined(CONFIG_NAND_ALI) 
	args_off =  nand->writesize*2048;	//-->16M(8K*2048)  /  8M(4K*2048) 
	unsigned char *datbuf_tmp = (unsigned char *)malloc(nand->erasesize + 0x20);
	unsigned char *datbuf = (u8 *)(((u32)datbuf_tmp + (u32)0x1F) & (~(u32)0x1F));

	for (i=0;i<4;i++)
	{
		offset = args_off+i*nand->erasesize;
		len = nand->erasesize;
		if (nand_block_isbad(nand, offset&~(nand->erasesize - 1))) 
		{
			printf("block is bad,offset:0x%x 0x%x\n",offset);
			continue;
		}
	
		if (nand_read(nand, offset, &len, datbuf))		
		{
			printf("Read error happen on <get_bootargs>\n");
			free(datbuf_tmp);
            free(bootargs);
            bootargs = NULL;
			return NULL;		
		}

		/* check block data */
		if (_sector_crc_check(datbuf, nand->erasesize)>0)
		{
			break;
		}
	}

	if (i==4)
	{
		printf("can not find bootargs\n");
		free(datbuf_tmp);
        free(bootargs);
        bootargs = NULL;      
		return NULL;
	}

	memcpy(bootargs, datbuf+12, sizeof(struct boot_args));
    free(datbuf_tmp);

#elif defined(CONFIG_ALI_MMC)

	ret = load_part_sector_emmc(partName, (u_char *)bootargs, sizeof(struct boot_args));
	if (ret < 0)
	{
		printf("get bootargs fail.\n");	
		free(bootargs);
        bootargs = NULL;     
		return NULL;
	}
#endif

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


