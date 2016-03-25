#include <jffs2/load_kernel.h>
#include <common.h>
#include <asm/io.h>
#include <boot_common.h>
#include <asm/dma-mapping.h>
#include <linux/err.h>
#include <nand.h>
#include "ali_nand.h"
#include "block_sector.h"
#include <alidefinition/adf_boot.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

#define MULTI_COMP_KERNEL_INDEX		0
#define MULTI_COMP_RAMDISK_INDEX	1
#define MULTI_COMP_SEE_INDEX		2
#define MULTI_COMP_AE_INDEX			3

int get_part_by_name(const char *partName, loff_t *start, size_t *size)
{
    int ret = -1;
    
#if defined(CONFIG_NAND_ALI) 
    ret = get_part_by_name_nand(partName, start, size);
#elif defined(CONFIG_ALI_MMC)
    ret = get_part_by_name_emmc(partName, start, size);
#endif

    return ret;
}

int load_part(const char *partName, u_char *LoadAddr)
{
    int ret = -1;

#if defined(CONFIG_NAND_ALI) 
    ret = load_part_nand(partName, LoadAddr);
#elif defined(CONFIG_ALI_MMC)
    ret = load_part_emmc(partName, LoadAddr);
#endif

	return ret;
}

int load_part_ext(const char *partName, u_char *LoadAddr, loff_t offset, size_t len)
{
    int ret = -1;

#if defined(CONFIG_NAND_ALI)     
    ret = load_part_nand_ext(partName, LoadAddr, offset, len);
#elif defined(CONFIG_ALI_MMC)
    ret = load_part_emmc_ext(partName, LoadAddr, offset, len);
#endif

    return ret;
}

int load_part_sector(const char *partName, u_char *LoadAddr, int bufLen)
{
    int ret = -1;

#if defined(CONFIG_NAND_ALI)    
    ret = load_part_sector_nand(partName, LoadAddr, bufLen);
#elif defined(CONFIG_ALI_MMC)
    ret = load_part_sector_emmc(partName, LoadAddr, bufLen);
#endif

    return ret;
}  

int load_part_ubo(const char *partName, u_char *LoadAddr)
{
	size_t preReadSize;
	size_t realUBOLen;
	image_header_t *hdr = NULL;

	preReadSize = UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)LoadAddr;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	realUBOLen = image_get_data_size(hdr) + UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, realUBOLen) < 0)
	{
		printf("<%s>(%d): %s read data error\n", __FUNCTION__, __LINE__, partName);
		return -3;
	}

	return realUBOLen;
}

int load_part_ubo_with_sig(const char *partName, u_char *LoadAddr, u_char *sig)
{
	size_t preReadSize;
	size_t realUBOLen;
	image_header_t *hdr = NULL;

	preReadSize = UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)LoadAddr;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	realUBOLen = image_get_data_size(hdr) + UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, realUBOLen+0x100) < 0)
	{
		printf("<%s>(%d): %s read data error\n", __FUNCTION__, __LINE__, partName);
		return -3;
	}
	memcpy(sig, LoadAddr+realUBOLen, 0x100);

	return realUBOLen;
}

int get_part_ubo_comp_cnt(const char *partName)
{
	size_t preReadSize;
	u_char preReadBuf[2 * UBO_HEAD_LEN + 0x20], *preReadBuffer;
	image_header_t *hdr = NULL;
	ulong count;

	preReadSize = 2 * UBO_HEAD_LEN;
	preReadBuffer = (u8 *)(((u32)preReadBuf + (u32)0x1F) & (~(u32)0x1F));
	if (load_part_ext(partName, preReadBuffer, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)preReadBuffer;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	if (hdr->ih_type != IH_TYPE_MULTI)
	{
		printf("<%s>(%d): %s is not a multi part\n", __FUNCTION__, __LINE__, partName);
		count = 1;
	}
	else
	{
		count = image_multi_count(hdr);
	}

    return (int)count;
}

int get_part_ubo_comp_info(const char *partName, const int idx, loff_t *poffset, uint32_t *plength)
{
	size_t preReadSize;
	u_char preReadBuf[2 * UBO_HEAD_LEN + 0x20], *preReadBuffer;
	image_header_t *hdr = NULL;
	uint32_t *size;
	size_t len;
	ulong count;
	loff_t offset;
	int i;

	preReadSize = 2 * UBO_HEAD_LEN;
	preReadBuffer = (u8 *)(((u32)preReadBuf + (u32)0x1F) & (~(u32)0x1F));
	if (load_part_ext(partName, preReadBuffer, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)preReadBuffer;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	if (hdr->ih_type != IH_TYPE_MULTI)
	{
		printf("<%s>(%d): %s is not a multi part\n", __FUNCTION__, __LINE__, partName);
		return -3;
	}

	count = image_multi_count(hdr);

	size = (uint32_t *)image_get_data(hdr);

	if (idx < count)
	{
		len = uimage_to_cpu(size[idx]);
		offset = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++)
		{
			/* add up i-th component size, rounding up to 4 bytes */
			offset += (uimage_to_cpu(size[i]) + 3) & ~3 ;
		}

		offset += UBO_HEAD_LEN + (count + 1) * 4;

		*poffset = offset;
		*plength = len;

		return 0;
	}
	else
	{
		printf("<%s>(%d): Part %s comp %d index exceeds comp_cnt(%d)\n", __FUNCTION__, __LINE__, partName, idx, count);
		return -4;
	}

	return 0;
}

int load_part_ubo_comp(const char *partName, const int idx, u_char *LoadAddr)
{
	size_t preReadSize;
	image_header_t *hdr = NULL;
	uint32_t *size;
	size_t len;
	ulong count;
	loff_t offset;
	int i;

	preReadSize = 2 * UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)LoadAddr;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	if (hdr->ih_type != IH_TYPE_MULTI)
	{
		printf("<%s>(%d): %s is not a multi part\n", __FUNCTION__, __LINE__, partName);
		return -3;
	}

	count = image_multi_count(hdr);

	size = (uint32_t *)image_get_data(hdr);

	if (idx < count)
	{
		len = uimage_to_cpu(size[idx]);
		offset = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++)
		{
			/* add up i-th component size, rounding up to 4 bytes */
			offset += (uimage_to_cpu(size[i]) + 3) & ~3 ;
		}

		offset += UBO_HEAD_LEN + (count + 1) * 4;

		if (load_part_ext(partName, LoadAddr, offset, len) < 0)
		{
			printf("<%s>(%d): Part %s read comp %d data error @0x%llx\n", __FUNCTION__, __LINE__, partName, idx, offset);
			return -4;
		}
	}
	else
	{
		printf("<%s>(%d): Part %s comp %d index exceeds comp_cnt(%d)\n", __FUNCTION__, __LINE__, partName, idx, count);
		return -5;
	}

	return len;
}

int load_part_ubo_comp_with_sig(const char *partName, const int idx, u_char *LoadAddr, u_char *sig)
{
	size_t preReadSize;
	image_header_t *hdr = NULL;
	uint32_t *size;
	size_t len;
	ulong count;
	loff_t offset;
	int i;

	preReadSize = 2 * UBO_HEAD_LEN;
	if (load_part_ext(partName, LoadAddr, 0, preReadSize) < 0)
	{
		printf("<%s>(%d): %s read ubo head error\n", __FUNCTION__, __LINE__, partName);
		return -1;
	}

	hdr = (image_header_t *)LoadAddr;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -2;
	}

	if (hdr->ih_type != IH_TYPE_MULTI)
	{
		printf("<%s>(%d): %s is not a multi part\n", __FUNCTION__, __LINE__, partName);
		return -3;
	}

	count = image_multi_count(hdr);

	size = (uint32_t *)image_get_data(hdr);

	if (idx < count)
	{
		len = uimage_to_cpu(size[idx]);
		offset = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++)
		{
			/* add up i-th component size, rounding up to 4 bytes */
			offset += (uimage_to_cpu(size[i]) + 3) & ~3 ;
		}

		offset += UBO_HEAD_LEN + (count + 1) * 4;
		
		//printf("offset=0x%llx, len=0x%x,partName=%s, idx=%d\n", offset, len, partName, idx);
		if (load_part_ext(partName, LoadAddr, offset, len) < 0)
		{
			printf("<%s>(%d): Part %s read comp %d data error @0x%llx\n", __FUNCTION__, __LINE__, partName, idx, offset);
			return -4;
		}
		len -= 0x100 ;
		memcpy(sig, LoadAddr+len, 0x100);
	}
	else
	{
		printf("<%s>(%d): Part %s comp %d index exceeds comp_cnt(%d)\n", __FUNCTION__, __LINE__, partName, idx, count);
		return -5;
	}

	return len;
}

int part_ubo_check(UINT8 *entry)
{
	image_header_t *hdr = NULL;

	hdr = (image_header_t *)entry;
	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC)
	{
		printf("<%s>(%d): Bad Magic Number\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (dcrc_chk(entry) == 0)
	{
		return 0;
	}

	return -1;
}

int part_check(UINT8 *name, UINT8 *entry, UINT32 len, UINT8 *sig)
{

    	if (part_ubo_check(entry) < 0)
    	{
    		printf("Check %s Failed...\n", name);
    		return -1;
    	}

    return RET_SUCCESS;
}

static int part_see_running_check(ADF_BOOT_BOARD_INFO *info)
{
	static int pre_tick = 0;
	int i=0;
	int valid_tick_cnt =0;

	for(i=0;i<10;i++)
	{
		if(info->heart_beat.live_flag != BOOT_MAGIC_NUM)
		{
			mdelay(30);
		}
		else{
				return 0;
				#if 0
				//check live tick change
				if(info->heart_beat.live_tick != pre_tick)
				{
					pre_tick = info->heart_beat.live_tick;
					valid_tick_cnt++;

					if(valid_tick_cnt>3)
					{
						return 0;
					}		
				}
				#endif
			}
		
	}
	return -1;
}


int correct_entry(u_char *bin_entry, uint32_t load_entry, uint32_t execute_entry)
{
	image_header_t *hdr, new_hdr;
	ulong hcrc, len;
	uint32_t le, ee;

	hdr = (image_header_t *)bin_entry;

	if (!image_check_hcrc(hdr))
	{
		printf("%s() Check ubo header crc fail\n", __FUNCTION__);
		return -1;
	}

	le = image_get_load(hdr);
	ee = image_get_ep(hdr);

	if (le == load_entry && ee == execute_entry)
	{
		//printf("Entrys match, need not correct!\n");
		return 0;
	}

	printf("Entrys mismatch, load_entry=0x%08x(0x%08x) execute_entry=0x%08x(0x%08x)\n", le, load_entry, ee, execute_entry);

	image_set_load(hdr, load_entry);
	image_set_ep(hdr, execute_entry);

	len = image_get_header_size();

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&new_hdr, (char *)hdr, len);
	image_set_hcrc(&new_hdr, 0);

	hcrc = crc32(0, (unsigned char *)&new_hdr, len);
	image_set_hcrc(hdr, hcrc);

	return 0;
}

int load_bootmedia(void)
{
	u_char *entry;
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)PRE_DEFINED_ADF_BOOT_START;

	memset(&info->media_info, 0, sizeof(info->media_info));

	info->media_info.play_enable = 0;

	entry = (u_char *)(info->memmap_info.boot_media_start | 0x80000000);
	if (load_part_ubo(PART_NAME_BM, entry) < 0)
	{
		printf("Load '%s' Fail...\n", PART_NAME_BM);
		return -1;
	}
	printf("Load '%s' Done...\n", PART_NAME_BM);

	if (part_ubo_check(entry) < 0)
	{
		printf("Check '%s' Fail...\n", PART_NAME_BM);
		return -1;
	}

	info->media_info.play_enable = 1;

	return 0;
}

int load_kernel(void)
{
	loff_t offset, size;

	init_cmdline_ext();

	if (-1 != get_part_by_name(PART_NAME_SEE, &offset, &size))
	{
#ifdef LOAD_KERNEL_SEE
        load_kernel_see();
#endif
	}
	else
	{
#ifdef LOAD_MAIN_BIN
        load_main_bin();
#endif
	}
}

#ifdef LOAD_KERNEL_SEE
int load_kernel_see(void)
{
	u_char *entry, *rd_entry;
	unsigned char sig[260];
	int len = 0;
	volatile ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	/* Load see/ae first to show boot logo/media earlier */
	entry = (u_char *)(info->memmap_info.ae_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Start to load partition '%s' to 0x%x....\n", PART_NAME_AUD_CODE,entry);
	len = load_part_ubo_with_sig(PART_NAME_AUD_CODE, entry, sig);
	if (len < 0)
	{
		printf("Load audio CPU code Fail...\n");
		return -1;
	}
	if (part_check(PART_NAME_AUD_CODE, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	entry = (u_char *)(info->memmap_info.see_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Start to load partition '%s' to 0x%x....\n", PART_NAME_SEE, entry);
	len = load_part_ubo_with_sig(PART_NAME_SEE, entry, sig);
	if (len < 0)
	{
		printf("Load see code Fail...\n");
		return -1;
	}
	if (part_check(PART_NAME_SEE, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	install_memory();
	printf("install_memory in uboot\n");
	g_see_boot();
	printf("g_see_boot in uboot\n");	

	entry = (u_char *)(info->memmap_info.kernel_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Start to load partition '%s' to 0x%x....\n", PART_NAME_KERNEL, entry);
	len = load_part_ubo_with_sig(PART_NAME_KERNEL, entry, sig);
	if (len < 0)
	{
		printf("Load kernel code Fail...\n");
		return -1;
	}
	if (part_check(PART_NAME_KERNEL, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}
	
	if (correct_entry(entry, (uint32_t)entry, (uint32_t)entry + UBO_HEAD_LEN) < 0)
	{
		printf("Correct '%s' Entry Fail\n", PART_NAME_KERNEL);
		return -1;
	}

#ifdef CHECK_SEE_RUNNING
	if(part_see_running_check(info) <0)
	{
		printf("check see run code Fail...############################\n");
		return -1;
	}
#endif	

	/* Load rootfs */
	rd_entry = (void *)(info->memmap_info.ramdisk_start |0x80000000);
	printf("\nStart to load partition '%s' to (0x%x....)\n", PART_NAME_ROOTFS, rd_entry);
	len = load_part_ubo_with_sig(PART_NAME_ROOTFS, rd_entry, sig);	
	if (len < 0)
	{
		printf("Load rootfs Fail...\n");
		return -1;
	}
	if (part_check(PART_NAME_ROOTFS, rd_entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}
	        
	/* set bootm cmd env */
	char bootm_cmd_str[64];
	memset(bootm_cmd_str, 0, sizeof(bootm_cmd_str));
	sprintf(bootm_cmd_str, "bootm 0x%08x 0x%08x", entry,rd_entry);        
	setenv("bootcmd", bootm_cmd_str);

	set_cmdline(0);
	set_androidboot_cmdline();

	return 0;
}
#endif

#ifdef LOAD_MAIN_BIN
int load_main_bin(void)
{
	u_char *entry, *rd_entry;
	int comp_cnt;
	unsigned char sig[260];
	int len = 0;
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)PRE_DEFINED_ADF_BOOT_START;

	comp_cnt = get_part_ubo_comp_cnt(PART_NAME_MAIN_BIN);
	if (comp_cnt <= 1)
	{
		printf("Get part '%s' component count Fail, comp_cnt=%d\n", PART_NAME_MAIN_BIN, comp_cnt);
		return -1;
	}

	/* Load see/ae first to show boot logo/media earlier */
	entry = (u_char *)(info->memmap_info.ae_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Loading comp '%s' to 0x%x...\n", PART_NAME_AUD_CODE, entry);
	len = load_part_ubo_comp_with_sig(PART_NAME_MAIN_BIN, MULTI_COMP_AE_INDEX, entry, sig);
	if (len < 0)
	{
		printf("Load comp '%s' Fail\n", PART_NAME_AUD_CODE);
		return -1;
	}
	if (part_check(PART_NAME_AUD_CODE, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	entry = (u_char *)(info->memmap_info.see_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Loading comp '%s' to 0x%x...\n", PART_NAME_SEE, entry);
	len = load_part_ubo_comp_with_sig(PART_NAME_MAIN_BIN, MULTI_COMP_SEE_INDEX, entry, sig);
	if (len < 0)
	{
		printf("Load comp '%s' Fail\n", PART_NAME_SEE);
		return -1;
	}
	if (part_check(PART_NAME_SEE, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	install_memory();
	printf("install_memory in uboot\n");
	g_see_boot();
	printf("g_see_boot in uboot\n");

	/* Load kernel */
	entry = (u_char *)(info->memmap_info.kernel_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Loading comp '%s' to 0x%x...\n", PART_NAME_KERNEL, entry);
	len = load_part_ubo_comp_with_sig(PART_NAME_MAIN_BIN, MULTI_COMP_KERNEL_INDEX, entry, sig);
	if (len < 0)
	{
		printf("Load comp '%s' Fail\n", PART_NAME_KERNEL);
		return -1;
	}
	if (part_check(PART_NAME_KERNEL, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}
	
	if (correct_entry(entry, (uint32_t)entry, (uint32_t)entry + UBO_HEAD_LEN) < 0)
	{
		printf("Correct '%s' Entry Fail\n", PART_NAME_KERNEL);
		return -1;
	}

	/* Load ramdisk */
	rd_entry = (u_char *)(info->memmap_info.ramdisk_start | 0x80000000);
	printf("Loading comp '%s' to 0x%x...\n", PART_NAME_RAMDISK, rd_entry);
	len = load_part_ubo_comp_with_sig(PART_NAME_MAIN_BIN, MULTI_COMP_RAMDISK_INDEX, rd_entry, sig);
	if (len < 0)
	{
		printf("Load comp '%s' Fail\n", PART_NAME_RAMDISK);
		return -1;
	}
	if (part_check(PART_NAME_RAMDISK, rd_entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	/* set bootm cmd env */
	char bootm_cmd_str[64];
	memset(bootm_cmd_str, 0, sizeof(bootm_cmd_str));
	sprintf(bootm_cmd_str, "bootm 0x%08x 0x%08x", entry, rd_entry);
	setenv("bootcmd", bootm_cmd_str);

	set_cmdline(0);
	set_androidboot_cmdline();

	return 0;
}

#endif

#ifdef LOAD_RECOVERY
static int _load_recovery(const char *part_name)
{
	u_char *entry, *rd_entry;
	int comp_cnt;
	unsigned char sig[260];
	int len = 0;
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)PRE_DEFINED_ADF_BOOT_START;

	set_avinfo_default(&info->avinfo);

	comp_cnt = get_part_ubo_comp_cnt(part_name);
	if (comp_cnt <= 1)
	{
		printf("Get part '%s' component count Fail, comp_cnt=%d\n", part_name, comp_cnt);
		return -1;
	}

	/* Stop SEE CPU first in order to load Recovery SEE code */
	g_see_stop();

	/* Load see first to avoid see boot fail */
	entry = (u_char *)(info->memmap_info.see_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Loading part '%s' comp '%s' to 0x%x...\n", part_name, PART_NAME_SEE, entry);
	len = load_part_ubo_comp_with_sig(part_name, MULTI_COMP_SEE_INDEX, entry, sig);
	if (len < 0)
	{
		printf("Load part '%s' comp '%s' Fail\n", part_name, PART_NAME_SEE);
		return -1;
	}
	if (part_check(PART_NAME_SEE, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}
	
	install_memory();
	printf("install_memory in %s\n", part_name);
	g_see_boot();
	printf("g_see_boot in %s\n", part_name);

	/* Load kernel */
	entry = (u_char *)(info->memmap_info.kernel_start | 0x80000000) - UBO_HEAD_LEN;
	printf("Loading part '%s' comp '%s' to 0x%x...\n", part_name, PART_NAME_KERNEL, entry);
	len = load_part_ubo_comp_with_sig(part_name, MULTI_COMP_KERNEL_INDEX, entry, sig);
	if (len < 0)
	{
		printf("Load part '%s' comp '%s' Fail\n", part_name, PART_NAME_KERNEL);
		return -1;
	}
	if (part_check(PART_NAME_KERNEL, entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}
	if (correct_entry(entry, (uint32_t)entry, (uint32_t)entry + UBO_HEAD_LEN) < 0)
	{
		printf("Correct part '%s' comp '%s' Entry Fail\n", part_name, PART_NAME_KERNEL);
		return -1;
	}

	/* Load ramdisk */
	rd_entry = (u_char *)(info->memmap_info.ramdisk_start | 0x80000000);
	printf("Loading part '%s' comp '%s' to 0x%x...\n", part_name, PART_NAME_RAMDISK, rd_entry);
	len = load_part_ubo_comp_with_sig(part_name, MULTI_COMP_RAMDISK_INDEX, rd_entry, sig);
	if (len < 0)
	{
		printf("Load part '%s' comp '%s' Fail\n", part_name, PART_NAME_RAMDISK);
		return -1;
	}
	if (part_check(PART_NAME_RAMDISK, rd_entry, len, sig)!= RET_SUCCESS)
	{
		return -1;
	}

	/* set bootm cmd env */
	char bootm_cmd_str[64];
	memset(bootm_cmd_str, 0, sizeof(bootm_cmd_str));
	sprintf(bootm_cmd_str, "bootm 0x%08x 0x%08x", entry, rd_entry);
	setenv("bootcmd", bootm_cmd_str);

	set_cmdline(1);
	set_recoveryboot_cmdline(part_name);

	return 0;
}

int load_recovery(void)
{
	init_cmdline_ext();

    if (_load_recovery(PART_NAME_RECOVERY) != 0)
    {
        if (_load_recovery(PART_NAME_RECOVERYBAK) != 0)
            return -1;
    }
	printf("cmdline_ext:%s\n", get_cmdline_ext());
	return 0;
}
#endif

