#include <common.h>
#include <nand.h>
#include "block_sector.h"
#include "bootargs.h"

#include <boot_common.h>
#include <ali/sys_parameters.h>


#define SIG_LEN  0x100
#define BASEARGS_HEADER 12
extern unsigned char g_uk_pos;

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
	printf("%s: enter get_bootargs\n", __FUNCTION__);	
	//adds for cache line size align
	datbuf = (u8 *)(((u32)adatbuf + (u32)0x1F) & (~(u32)0x1F));
	MG_Setup_CRC_Table();
	bootargs = (struct boot_args*)malloc(sizeof(struct boot_args));

#if defined(CONFIG_NAND_ALI) 
	args_off =  nand->writesize*2048;	//-->16M(8K*2048)  /  8M(4K*2048) 
	//datbuf = (unsigned char *)malloc(nand->erasesize);
	
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
			//free(datbuf);
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
		//free(datbuf);
		return NULL;
	}
#if 1
	if(test_rsa_ram((UINT32)(datbuf), sizeof(struct boot_args)+BASEARGS_HEADER+SIG_LEN) != 0)
	{
		printf("bootargs  rsa verify error\n");
		return NULL;
	}
#endif
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
	return bootargs;	
}

int set_mtdparts(void)
{
	struct boot_args *bootargs = NULL;
	char *cmdline = NULL;
	char *mtdparts = NULL;

    bootargs = get_bootargs();
    if(bootargs == NULL)
    {
        printf("%s: bootargs is NULL\n", __FUNCTION__);
        return -1;
    }
    cmdline = bootargs->cmdline_rcv;
    mtdparts = strstr(cmdline,"mtdparts=");
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


