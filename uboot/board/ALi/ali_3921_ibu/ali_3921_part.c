#include <jffs2/load_kernel.h>
#include <common.h>
#include <asm/io.h>
#include <boot_common.h>
#include <asm/dma-mapping.h>
#include <linux/err.h>
#include <nand.h>
#include <ali_nand.h>
#include "block_sector.h"
#include <alidefinition/adf_boot.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t


int get_part_by_name(const char *partName,loff_t *start, size_t *size)
{
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = find_dev_and_part(partName, &dev, &pnum, &part);
	if (ret)
		return -1;

	*start = part->offset;
	*size = part->size;
	
	return pnum;
}

int load_part(const char *partName, u_char * LoadAddr)
{
	loff_t start;
	size_t size;
	u8 pnum;
	nand_info_t *nand = &nand_info[nand_curr_device];
	int r;
	
	pnum = get_part_by_name(partName,&start, &size);
	r = nand_read_skip_bad(nand, start, &size, LoadAddr);
	if (r) {
		printf("Read error happen on <nand_load_part_idx>\n");
		return -1;
	}

	printf("load_part %s, start:0x%x 0x%x, size:0x%x\n",
		partName, start, size);

	return size;
}

int load_part_ext(const char *partName, u_char * LoadAddr, u32 offset, u32 len)
{
	loff_t start;
	size_t size;
	u8 pnum;
	nand_info_t *nand = &nand_info[nand_curr_device];
	int r;
	
	pnum = get_part_by_name(partName,&start, &size);

	if (offset+len > size)
	{
		printf("%s(%d) error: exceeds part size(0x%x)\n",__FUNCTION__,__LINE__,size);
		return -1;
	}
	
	r = nand_read_skip_bad(nand, start+offset, &len, LoadAddr);
	if (r) {
		printf("Read error happen on <nand_load_part_idx>\n");
		return -1;
	}

	printf("load_part_ext %s, offset:0x%x, len:0x%x\n",
		partName, offset, len);

	return len;
}

int load_part_sector(const char *partName, u_char * LoadAddr, int bufLen)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	loff_t start;
	size_t size;
	loff_t offset;
	size_t len;
	u_char *datbuf, *datbuf_tmp;	
	int blockNum;
	int dataLen;
	int i;

	get_part_by_name(partName,&start, &size);

	blockNum = size/nand->erasesize;
	//adds for cache line size align
	datbuf_tmp = (u_char *)kmalloc(nand->erasesize+0x20, GFP_KERNEL);
	datbuf = (u8 *)(((u32)datbuf_tmp + (u32)0x1F) & (~(u32)0x1F));

	for (i=0;i<blockNum;i++)
	{
		offset = start+i*nand->erasesize;
		len = nand->erasesize;
		if (nand_block_isbad(nand, offset&~(nand->erasesize - 1)))
		{
			printf("block is bad,offset: 0x%x\n", offset);
			continue;
		}

		if (nand_read(nand, offset, &len, datbuf))
		{
			printf("Read error happen on <get_bootargs>\n");
			kfree(datbuf_tmp);
			return -1;
		}

		/* check block data */
		dataLen = _sector_crc_check(datbuf, nand->erasesize);
		if (dataLen>0)
			break;
	}

	if (i==blockNum)
	{
		printf("can not find sector in part %s\n",partName);
		kfree(datbuf_tmp);					
		return -1;
	}

        if (bufLen > dataLen-12)
        {
            printf("sector format is not matched, size:0x%x, datalen:0x%x\n",bufLen,dataLen-12);    
            kfree(datbuf_tmp);                   
            return -1;      
        }
    
        memcpy(LoadAddr, datbuf+12, bufLen);           
    
        kfree(datbuf_tmp);                       
        return bufLen;
}

int load_part_ubo(const char *partName, u_char * LoadAddr)
{
	size_t preReadSize;
	size_t realUBOLen;
	image_header_t *hdr = NULL;

	preReadSize = UBO_HEAD_LEN;		// UBO head length
	if (load_part_ext(partName, LoadAddr, 0, preReadSize)<0)
	{
		printf("%s(%d): %s read ubo head error\n",__FUNCTION__,__LINE__,partName);
		return -1;
	}

	hdr = (image_header_t *)LoadAddr;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC) {
		printf("<%s>: Bad Magic Number\n",	__FUNCTION__);
		return -3;
	}
	realUBOLen = image_get_data_size(hdr) + UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, realUBOLen)<0)
	{
		printf("%s(%d): %s read data error\n",__FUNCTION__,__LINE__,partName);
		return -4;
	}
	
	return realUBOLen;
}

int part_ubo_check(UINT8 *entry)
{
	image_header_t *hdr = NULL;
	hdr = (image_header_t *)entry;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC) {
		printf("<%s>: Bad Magic Number\n",	__FUNCTION__);
		return -3;
	}

	//if(dcrc_chk(entry) == 0)  //remove CRC
	{
		return 0;
	}

	return -1;
}

#ifdef LOAD_KERNEL_SEE
int load_kernel(void)
{
	u_char *entry;
	u_char *entry_ramdisk;
	//unsigned int part_idx = -1;
	
	volatile ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	entry = (void *)(info->memmap_info.ae_start |0x80000000) - 64;//64byte ubo header size
	//entry = (void *)ADDR_LOAD_AUD_CODE;
	printf("\nStart to load partition '%s' to 0x%x....\n", PART_NAME_AUD_CODE,entry);

	if (load_part_ubo(PART_NAME_AUD_CODE, entry)<0)
	{
		printf("Load audio CPU code Fail...\n");
		return -1;
	}	
	printf("Load audio CPU code Done...\n");

#ifndef PROFILE_ON
	if (part_ubo_check(entry)<0)
	{
		printf("Check audio code Fail...\n");
		return -1;
	}		
#endif

	
	//step1:first load see and boot, to speed up show logo time;
	entry = (void *)(info->memmap_info.see_start |0x80000000)  - 64;//64byte ubo header size
	//entry = (void *)ADDR_LOAD_SEE;
	printf("\nnew Start to load partition '%s' to 0x%x....\n", PART_NAME_SEE,entry);

	if (load_part_ubo(PART_NAME_SEE, entry)<0)
	{
		printf("Load see code Fail...\n");
		return -1;
	}	
	printf("Load see code Done...\n");
	
#ifndef PROFILE_ON
	if (part_ubo_check(entry)<0)
	{
		printf("Check see code Fail...\n");
		return -1;
	}		
#endif

	install_memory();
	printf("install_memory in uboot\n");
	g_see_boot();
	printf("g_see_boot in uboot\n");	

	//while(1);
	entry = (void *)(info->memmap_info.kernel_start |0x80000000) - 64;//64byte ubo header size
	//entry = (void *)ADDR_LOAD_KERNEL;
	printf("\n new Start to load partition '%s' to 0x%x....\n", PART_NAME_KERNEL,entry);

	if (load_part_ubo(PART_NAME_KERNEL, entry)<0)
	{
		printf("Load kernel code Fail...\n");
		return -1;
	}		
	printf("Load kernel code Done...\n");
	
#ifndef PROFILE_ON
	if (part_ubo_check(entry)<0)
	{
		printf("Check kernel code Fail...\n");
		return -1;
	}		
#endif		

#ifdef LOAD_RAM_DISK
	entry_ramdisk = (void *)(info->memmap_info.ramdisk_start |0x80000000);//(void *)ADDR_LOAD_RAMDISK;
	printf("\nStart to load partition '%s' to (0x%x....)\n", PART_NAME_RAMDISK, entry_ramdisk);
	int ret_read_len = load_part_ubo(PART_NAME_RAMDISK, entry_ramdisk);
	printf("\nStart to load partition '%s' return len 0x%x....\n", PART_NAME_RAMDISK, ret_read_len);
	if (ret_read_len < 0)
	{
		printf("Load ramdisk Fail...\n");
		return;
	}
	printf("Load ramdisk Done...\n");
	
	
	/* set bootm cmd env */
	char bootm_cmd_str[32];
	memset(bootm_cmd_str, 0, sizeof(bootm_cmd_str));
	sprintf(bootm_cmd_str, "bootm 0x%08x 0x%08x", entry,entry_ramdisk);
	printf("bootcmd :%s \n",bootm_cmd_str);
	
	setenv("bootcmd", bootm_cmd_str);
#endif


	set_cmdline(0);
	return 0;
}
#endif

int load_bootmedia(void)
{
	u_char *entry;
	
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)PRE_DEFINED_ADF_BOOT_START;

	memset(&info->media_info,0,sizeof(info->media_info));
	
	info->media_info.play_enable = 0;

	entry = (void *)(info->memmap_info.boot_media_start|0x80000000);
	printf("\n new Start to load partition '%s' to 0x%x....\n", PART_NAME_BM,entry);
	if (load_part_ubo(PART_NAME_BM, entry)<0)
	{
		printf("Load %s Fail...\n",PART_NAME_BM);
		return -1;
	}		
	printf("Load %s Done...\n",PART_NAME_BM);
	
#ifndef PROFILE_ON
	if (part_ubo_check(entry)<0)
	{
		printf("Check %s Fail...\n",PART_NAME_BM);
		return -1;
	}		
#endif

	info->media_info.play_enable = 1;

	return 0;
}


