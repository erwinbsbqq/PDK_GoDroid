/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
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
#include <mmc.h>

#include "../disk/part_efi.h"


static int curr_device = -1;
#ifndef CONFIG_GENERIC_MMC
int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "init") == 0) {
		if (argc == 2) {
			if (curr_device < 0)
				dev = 1;
			else
				dev = curr_device;
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);
		} else {
			return CMD_RET_USAGE;
		}

		if (mmc_legacy_init(dev) != 0) {
			puts("No MMC card found\n");
			return 1;
		}

		curr_device = dev;
		printf("mmc%d is available\n", curr_device);
	} else if (strcmp(argv[1], "device") == 0) {
		if (argc == 2) {
			if (curr_device < 0) {
				puts("No MMC device available\n");
				return 1;
			}
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);

#ifdef CONFIG_SYS_MMC_SET_DEV
			if (mmc_set_dev(dev) != 0)
				return 1;
#endif
			curr_device = dev;
		} else {
			return CMD_RET_USAGE;
		}

		printf("mmc%d is current device\n", curr_device);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	mmc, 3, 1, do_mmc,
	"MMC sub-system",
	"init [dev] - init MMC sub system\n"
	"mmc device [dev] - show or set current device"
);
#else /* !CONFIG_GENERIC_MMC */

enum mmc_state {
	MMC_INVALID,
	MMC_READ,
	MMC_WRITE,
	MMC_ERASE,
};
static void print_mmcinfo(struct mmc *mmc)
{
	printf("Device: %s\n", mmc->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c \n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	printf("Tran Speed: %d\n", mmc->tran_speed);
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 4) & 0xf, mmc->version & 0xf);

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	puts("Capacity: ");
	print_size(mmc->capacity, "\n");

	printf("Bus Width: %d-bit\n", mmc->bus_width);
#if defined(CONFIG_ALI_MMC)
	alimmc_info();
#endif
}

int do_mmcinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mmc *mmc;

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}

	mmc = find_mmc_device(curr_device);

	if (mmc) {
		mmc_init(mmc);

		print_mmcinfo(mmc);
		return 0;
	} else {
		printf("no mmc device at slot %x\n", curr_device);
		return 1;
	}
}

U_BOOT_CMD(
	mmcinfo, 1, 0, do_mmcinfo,
	"display MMC info",
	"    - device number of the device to dislay info of\n"
	""
);

int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum mmc_state state;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}

	if (strcmp(argv[1], "rescan") == 0) {
		struct mmc *mmc = find_mmc_device(curr_device);

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		mmc->has_init = 0;

		if (mmc_init(mmc))
			return 1;
		else
			return 0;
	} else if (strncmp(argv[1], "part", 4) == 0) {
		block_dev_desc_t *mmc_dev;
		struct mmc *mmc = find_mmc_device(curr_device);

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}
		mmc_init(mmc);
		mmc_dev = mmc_get_dev(curr_device);
		if (mmc_dev != NULL &&
				mmc_dev->type != DEV_TYPE_UNKNOWN) {
			print_part(mmc_dev);
			return 0;
		}

		puts("get mmc type error!\n");
		return 1;
	} else if (strcmp(argv[1], "list") == 0) {
		print_mmc_devices('\n');
		return 0;
	} else if (strcmp(argv[1], "dev") == 0) {
		int dev, part = -1;
		struct mmc *mmc;

		if (argc == 2)
			dev = curr_device;
		else if (argc == 3)
			dev = simple_strtoul(argv[2], NULL, 10);
		else if (argc == 4) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);
			part = (int)simple_strtoul(argv[3], NULL, 10);
			if (part > PART_ACCESS_MASK) {
				printf("#part_num shouldn't be larger"
					" than %d\n", PART_ACCESS_MASK);
				return 1;
			}
		} else
			return CMD_RET_USAGE;

		mmc = find_mmc_device(dev);
		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return 1;
		}

		mmc_init(mmc);
		if (part != -1) {
			int ret;
			if (mmc->part_config == MMCPART_NOAVAILABLE) {
				printf("Card doesn't support part_switch\n");
				return 1;
			}

			if (part != mmc->part_num) {
				ret = mmc_switch_part(dev, part);
				if (!ret)
					mmc->part_num = part;

				printf("switch to partions #%d, %s\n",
						part, (!ret) ? "OK" : "ERROR");
			}
		}
		curr_device = dev;
		if (mmc->part_config == MMCPART_NOAVAILABLE)
			printf("mmc%d is current device\n", curr_device);
		else
			printf("mmc%d(part %d) is current device\n",
				curr_device, mmc->part_num);

		return 0;
#if defined(CONFIG_ALI_MMC)
	}else if (strcmp(argv[1], "bootpart") == 0) {
		int dev, part = -1;
		struct mmc *mmc;

		dev = curr_device;
		if (argc == 3)
			part = simple_strtoul(argv[2], NULL, 10);
		else
			return CMD_RET_USAGE;

		mmc = find_mmc_device(dev);
		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return 1;
		}

		mmc_init(mmc);
		if (part != -1) {
			int ret;
			if (mmc->part_config == MMCPART_NOAVAILABLE) {
				printf("Card doesn't support part_switch\n");
				return 1;
			}

			if (7 == part || (part <= 2 && part >= 0)) {
				ret = mmc_switch_part_bootflag(dev, part);

				printf("set boot flag to partion #%d, %s\n",
						part, (!ret) ? "OK" : "ERROR");
			}
		}

		return 0;
#endif
	}

	if (strcmp(argv[1], "read") == 0)
		state = MMC_READ;
	else if (strcmp(argv[1], "write") == 0)
		state = MMC_WRITE;
	else if (strcmp(argv[1], "erase") == 0)
		state = MMC_ERASE;
	else
		state = MMC_INVALID;

	if (state != MMC_INVALID) {
		struct mmc *mmc = find_mmc_device(curr_device);
		int idx = 2;
		u32 blk, cnt, n;
		void *addr;

		if (state != MMC_ERASE) {
			addr = (void *)simple_strtoul(argv[idx], NULL, 16);
			++idx;
		} else
			addr = 0;
		blk = simple_strtoul(argv[idx], NULL, 16);
		cnt = simple_strtoul(argv[idx + 1], NULL, 16);

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		printf("\nMMC %s: dev # %d, block # %d, count %d ... ",
				argv[1], curr_device, blk, cnt);

		mmc_init(mmc);

		switch (state) {
		case MMC_READ:
			n = mmc->block_dev.block_read(curr_device, blk,
						      cnt, addr);
			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */
			break;
		case MMC_WRITE:
			n = mmc->block_dev.block_write(curr_device, blk,
						      cnt, addr);
			break;
		case MMC_ERASE:
			n = mmc->block_dev.block_erase(curr_device, blk, cnt);
			break;
		default:
			BUG();
		}

		printf("%d blocks %s: %s\n",
				n, argv[1], (n == cnt) ? "OK" : "ERROR");
		return (n == cnt) ? 0 : 1;
	}

	return CMD_RET_USAGE;
}


U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"MMC sub system",
	"read addr blk# cnt\n"
	"mmc write addr blk# cnt\n"
	"mmc erase blk# cnt\n"
	"mmc rescan\n"
	"mmc part - lists available partition on current mmc device\n"
	"mmc dev [dev] [part] - show or set current mmc device [partition]\n"
#if defined(CONFIG_ALI_MMC)
	"mmc bootpart part - set boot partition (0,1,2,7)\n"
#endif
	"mmc list - lists available devices");



//add by kinson.zhou for GPT Write
#ifdef CONFIG_EFI_PARTITION 

typedef struct efi_partition_info{
	disk_partition_t PartInfo[128];
	unsigned int PartNum;
} efi_disk_partition_t;

#define PRIMARY_GPT_TABLE_LEN   	 0x22
#define SECONDARY_GPT_TABLE_LEN   	 0x21

#define GPT_FIRST_DATA_OFFSET   	 0x22

#define  GPT_HEAD_SIZE               0x0000005C
#define  GPT_HEAD_LBA                0x0000000000000001
#define  GPT_FIRST_USABLE_LBA        0x0000000000000022
#define  PARTITION_ENTRY_LBA         0x0000000000000002
#define  NUM_PARTITION_ENTRIES       0x00000080
#define  SIZE_OF_PARTITION_ENTRIES   0x00000080

unsigned char g_partition_type_guid[16]={0XA2,0XA0,0XD0,0XEB,0XE5,0XE9,0X33,0X44,0X87,0XC0,0X68,0XB6,0XB7,0X26,0X99,0XC7};

unsigned char g_unique_partition_guid[16]={0X17,0X81,0Xc5,0X11,0X95,0XE6,0X58,0Xff,0Xdf,0Xda,0Xb3,0X29,0X34,0X28,0Xcd,0X00};

unsigned char g_disk_guid[16]={0xa4,0x3d,0x33,0x9b,0x73,0x54,0x46,0x43,0x85,0x26,0x88,0x94,0x09,0x23,0x9a,0x64};

/* Convert char[4] in little endian format to the host format integer
 */
static inline unsigned long le32_to_int(unsigned char *le32)
{
	return ((le32[3] << 24) + (le32[2] << 16) + (le32[1] << 8) + le32[0]);
}

/**
 * efi_crc32() - EFI version of crc32 function
 * @buf: buffer to calculate crc32 of
 * @len - length of buf
 *
 * Description: Returns EFI-style CRC32 value for @buf
 */
static inline unsigned long efi_crc32(const void *buf, unsigned long len)
{
	return crc32(0, buf, len);
}
/*
typedef struct _gpt_header {
	unsigned char signature[8];
	unsigned char revision[4];
	unsigned char header_size[4];
	unsigned char header_crc32[4];
	unsigned char reserved1[4];
	unsigned char my_lba[8];
	unsigned char alternate_lba[8];
	unsigned char first_usable_lba[8];
	unsigned char last_usable_lba[8];
	efi_guid_t disk_guid;
	unsigned char partition_entry_lba[8];
	unsigned char num_partition_entries[4];
	unsigned char sizeof_partition_entry[4];
	unsigned char partition_entry_array_crc32[4];
	unsigned char reserved2[GPT_BLOCK_SIZE - 92];
} __attribute__ ((packed)) gpt_header;

*/


//generate correct primary gpt table according to actual partitiong info based ont the template
unsigned int GeneratePrimaryGptTable(unsigned char *template_primary_gpt_table,unsigned int len_primary_gpt_table,unsigned int emmc_max_lba,efi_disk_partition_t * info)
{
	unsigned int header_crc32_value=0;
	unsigned int all_paritions_crc32_value=0;
	unsigned int i=0, j=0;
	legacy_mbr *pmbr;

	if(len_primary_gpt_table!=PRIMARY_GPT_TABLE_LEN*512) //primary_gpt_table has total 34 blocks 
	{
	   return -1; //error
	}

    //PMBR
	pmbr = (legacy_mbr *)(template_primary_gpt_table); //block 0 is PMBR

	//must be this, do not change
	for(i=0;i<2;i++)
	{
	   pmbr->signature[i] = ((unsigned long)MSDOS_MBR_SIGNATURE>>(8*i))&0xff;
	}
	
    //must be this,  do not change
	pmbr->partition_record[0].sys_ind = EFI_PMBR_OSTYPE_EFI_GPT;

    //must be this, do not change
	pmbr->partition_record[0].start_sect[0] = 0x01;
	pmbr->partition_record[0].start_sect[1] = 0x00;
	pmbr->partition_record[0].start_sect[2] = 0x00;
	pmbr->partition_record[0].start_sect[3] = 0x00;
	
    //gpt header
	gpt_header* gpt_head = (gpt_header*)(template_primary_gpt_table+1*512); //block 1 is GPT HEADER

	//gpt header signature
	for(i=0;i<8;i++)
	{
	   gpt_head->signature[i] = ((unsigned long long)GPT_HEADER_SIGNATURE>>(8*i))&0xff;
	}

	//revision
	for(i=0;i<4;i++)
	{
	   gpt_head->revision[i] = ((unsigned long long)GPT_HEADER_REVISION_V1>>(8*i))&0xff;
	}

	//header_size
	for(i=0;i<4;i++)
	{
	   gpt_head->header_size[i] = ((unsigned long long)GPT_HEAD_SIZE>>(8*i))&0xff;
	}

	//reserved1 must be all 0
	memset(gpt_head->reserved1,0,4);

	//my lba, always 0x01
	for(i=0;i<8;i++)
	{
	   gpt_head->my_lba[i] = ((unsigned long long)GPT_HEAD_LBA>>(8*i))&0xff;
	}

	//alternate lba 备份的
	gpt_head->alternate_lba[0] = (emmc_max_lba-1)&0xff;
	gpt_head->alternate_lba[1] = ((emmc_max_lba-1)>>8)&0xff;
	gpt_head->alternate_lba[2] = ((emmc_max_lba-1)>>16)&0xff;
	gpt_head->alternate_lba[3] = ((emmc_max_lba-1)>>24)&0xff;

	//first usable lba,always 0x22
	for(i=0;i<8;i++)
	{
	   gpt_head->first_usable_lba[i] = ((unsigned long long)GPT_FIRST_USABLE_LBA>>(8*i))&0xff;
	}

	//last usable lba     
	gpt_head->last_usable_lba[0] = (emmc_max_lba-0x22)&0xff;
	gpt_head->last_usable_lba[1] = ((emmc_max_lba-0x22)>>8)&0xff;
	gpt_head->last_usable_lba[2] = ((emmc_max_lba-0x22)>>16)&0xff;
	gpt_head->last_usable_lba[3] = ((emmc_max_lba-0x22)>>24)&0xff;

	memcpy(gpt_head->disk_guid.b,g_disk_guid,16);
		
	for(i=0;i<8;i++)
	{
	   gpt_head->partition_entry_lba[i] = ((unsigned long long)PARTITION_ENTRY_LBA>>(8*i))&0xff;
	}

	//num_partition_entries
	for(i=0;i<4;i++)
	{
	   gpt_head->num_partition_entries[i] = ((unsigned long long)NUM_PARTITION_ENTRIES>>(8*i))&0xff;
	}

	//sizeof_partition_entry
	for(i=0;i<4;i++)
	{
	   gpt_head->sizeof_partition_entry[i] = ((unsigned long long)SIZE_OF_PARTITION_ENTRIES>>(8*i))&0xff;
	}

	//reserved2 must be all 0
	memset(gpt_head->reserved2,0,GPT_BLOCK_SIZE - 92);

	//from block 2 to block 33 is 128 partition entries
	gpt_entry *pgpt_pte = (gpt_entry *)(template_primary_gpt_table+2*512);

	//128 partition entries clears to 0
	memset((unsigned char *)pgpt_pte,0,32*512);

	g_unique_partition_guid[15] = 0; //for index 0 start

	//set partition entries item
	for(i=0;i<info->PartNum;i++)
	{
		//partition type guid
		memcpy((unsigned char*)(pgpt_pte+i)->partition_type_guid.b, g_partition_type_guid, 16);

		//unique partition guid
		g_unique_partition_guid[15]++;
		memcpy((unsigned char*)(pgpt_pte+i)->unique_partition_guid.b, g_unique_partition_guid, 16);

		//partition name
		for(j=0;j<sizeof(info->PartInfo[i].name);j++)
		{
		   (pgpt_pte+i)->partition_name[j] = info->PartInfo[i].name[j];
		}

		//partition starting lba
		(pgpt_pte+i)->starting_lba[0] = (info->PartInfo[i].start)&0xff;		
		(pgpt_pte+i)->starting_lba[1] = (info->PartInfo[i].start>>8)&0xff;
		(pgpt_pte+i)->starting_lba[2] = (info->PartInfo[i].start>>16)&0xff;		
		(pgpt_pte+i)->starting_lba[3] = (info->PartInfo[i].start>>24)&0xff;

		//partition ending lba		
		(pgpt_pte+i)->ending_lba[0] = (info->PartInfo[i].start+info->PartInfo[i].size-1)&0xff;	
		(pgpt_pte+i)->ending_lba[1] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>8)&0xff;	
		(pgpt_pte+i)->ending_lba[2] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>16)&0xff;	
		(pgpt_pte+i)->ending_lba[3] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>24)&0xff; 
	}	

	/* calculate Partition Table Entry Array CRC for gpt header */
	all_paritions_crc32_value = efi_crc32((const unsigned char *)pgpt_pte,le32_to_int(gpt_head->num_partition_entries) *le32_to_int(gpt_head->sizeof_partition_entry));
	gpt_head->partition_entry_array_crc32[0] = all_paritions_crc32_value&0xff;
	gpt_head->partition_entry_array_crc32[1] = (all_paritions_crc32_value>>8)&0xff;
	gpt_head->partition_entry_array_crc32[2] = (all_paritions_crc32_value>>16)&0xff;
	gpt_head->partition_entry_array_crc32[3] = (all_paritions_crc32_value>>24)&0xff;

	//calculate gpt header crc32
	//Header CRC32 value set to 0 when calculate crc32
	gpt_head->header_crc32[0] = 0;
	gpt_head->header_crc32[1] = 0;
	gpt_head->header_crc32[2] = 0;
	gpt_head->header_crc32[3] = 0;

	header_crc32_value = efi_crc32((const unsigned char*)gpt_head,le32_to_int(gpt_head->header_size));
	gpt_head->header_crc32[0] = header_crc32_value&0xff;
	gpt_head->header_crc32[1] = (header_crc32_value>>8)&0xff;
	gpt_head->header_crc32[2] = (header_crc32_value>>16)&0xff;
	gpt_head->header_crc32[3] = (header_crc32_value>>24)&0xff;

	return 0;

}



//generate correct secondary gpt table according to actual partitiong info based ont the template
unsigned int GenerateSecondaryGptTable(unsigned char *template_secondary_gpt_table,unsigned int len_secondary_gpt_table,unsigned int emmc_max_lba,efi_disk_partition_t * info)
{
    unsigned int header_crc32_value=0;
	unsigned int all_paritions_crc32_value=0;
	unsigned int i=0, j=0;
	
    if(len_secondary_gpt_table!=SECONDARY_GPT_TABLE_LEN*512) //primary_gpt_table has total 34 blocks 
    {
       return -1; //error
    }
	
    gpt_header* gpt_head = (gpt_header*)(template_secondary_gpt_table+32*512); //block 32 is GPT HEADER

	//gpt header signature
	for(i=0;i<8;i++)
	{
	   gpt_head->signature[i] = ((unsigned long long)GPT_HEADER_SIGNATURE>>(8*i))&0xff;
	}

	//revision
	for(i=0;i<4;i++)
	{
	   gpt_head->revision[i] = ((unsigned long long)GPT_HEADER_REVISION_V1>>(8*i))&0xff;
	}

	//header_size
	for(i=0;i<4;i++)
	{
	   gpt_head->header_size[i] = ((unsigned long long)GPT_HEAD_SIZE>>(8*i))&0xff;
	}

	//reserved1 must be all 0
	memset(gpt_head->reserved1,0,4);

    //current lba
	gpt_head->my_lba[0] = (emmc_max_lba-1)&0xff;
	gpt_head->my_lba[1] = ((emmc_max_lba-1)>>8)&0xff;
	gpt_head->my_lba[2] = ((emmc_max_lba-1)>>16)&0xff;
	gpt_head->my_lba[3] = ((emmc_max_lba-1)>>24)&0xff;

	
	//alternate lba 备份的
	for(i=0;i<8;i++)
	{
	   gpt_head->alternate_lba[i] = ((unsigned long long)GPT_HEAD_LBA>>(8*i))&0xff;
	}

	//first usable lba,always 0x22
	for(i=0;i<8;i++)
	{
	   gpt_head->first_usable_lba[i] = ((unsigned long long)GPT_FIRST_USABLE_LBA>>(8*i))&0xff;
	}

	//last usable lba     
	gpt_head->last_usable_lba[0] = (emmc_max_lba-0x22)&0xff;
	gpt_head->last_usable_lba[1] = ((emmc_max_lba-0x22)>>8)&0xff;
	gpt_head->last_usable_lba[2] = ((emmc_max_lba-0x22)>>16)&0xff;
	gpt_head->last_usable_lba[3] = ((emmc_max_lba-0x22)>>24)&0xff;

	memcpy(gpt_head->disk_guid.b,g_disk_guid,16);
		
	//pte start block
	gpt_head->partition_entry_lba[0] = (emmc_max_lba-0x21)&0xff;
    gpt_head->partition_entry_lba[1] = ((emmc_max_lba-0x21)>>8)&0xff;
	gpt_head->partition_entry_lba[2] = ((emmc_max_lba-0x21)>>16)&0xff;
    gpt_head->partition_entry_lba[3] = ((emmc_max_lba-0x21)>>24)&0xff;

	//num_partition_entries
	for(i=0;i<4;i++)
	{
	   gpt_head->num_partition_entries[i] = ((unsigned long long)NUM_PARTITION_ENTRIES>>(8*i))&0xff;
	}

	//sizeof_partition_entry
	for(i=0;i<4;i++)
	{
	   gpt_head->sizeof_partition_entry[i] = ((unsigned long long)SIZE_OF_PARTITION_ENTRIES>>(8*i))&0xff;
	}

	//reserved2 must be all 0
	memset(gpt_head->reserved2,0,GPT_BLOCK_SIZE - 92);

	//from block 2 to block 33 is 128 partition entries
	gpt_entry *pgpt_pte = (gpt_entry *)(template_secondary_gpt_table);

	//128 partition entries clears to 0
	memset((unsigned char *)pgpt_pte,0,32*512);

	g_unique_partition_guid[15] = 0; //for index 0 start

	//set partition entries item
	for(i=0;i<info->PartNum;i++)
	{
		//partition type guid
		memcpy((unsigned char*)(pgpt_pte+i)->partition_type_guid.b, g_partition_type_guid, 16);

		//unique partition guid
		g_unique_partition_guid[15]++;
		memcpy((unsigned char*)(pgpt_pte+i)->unique_partition_guid.b, g_unique_partition_guid, 16);

		//partition name
		for(j=0;j<sizeof(info->PartInfo[i].name);j++)
		{
		   (pgpt_pte+i)->partition_name[j] = info->PartInfo[i].name[j];
		}

		//partition starting lba
		(pgpt_pte+i)->starting_lba[0] = (info->PartInfo[i].start)&0xff; 	
		(pgpt_pte+i)->starting_lba[1] = (info->PartInfo[i].start>>8)&0xff;
		(pgpt_pte+i)->starting_lba[2] = (info->PartInfo[i].start>>16)&0xff; 	
		(pgpt_pte+i)->starting_lba[3] = (info->PartInfo[i].start>>24)&0xff;

		//partition ending lba		
		(pgpt_pte+i)->ending_lba[0] = (info->PartInfo[i].start+info->PartInfo[i].size-1)&0xff;	
		(pgpt_pte+i)->ending_lba[1] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>8)&0xff;	
		(pgpt_pte+i)->ending_lba[2] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>16)&0xff;	
		(pgpt_pte+i)->ending_lba[3] = ((info->PartInfo[i].start+info->PartInfo[i].size-1)>>24)&0xff; 
	}	

	/* calculate Partition Table Entry Array CRC for gpt header */
	all_paritions_crc32_value = efi_crc32((const unsigned char *)pgpt_pte,le32_to_int(gpt_head->num_partition_entries) *le32_to_int(gpt_head->sizeof_partition_entry));
	gpt_head->partition_entry_array_crc32[0] = all_paritions_crc32_value&0xff;
	gpt_head->partition_entry_array_crc32[1] = (all_paritions_crc32_value>>8)&0xff;
	gpt_head->partition_entry_array_crc32[2] = (all_paritions_crc32_value>>16)&0xff;
	gpt_head->partition_entry_array_crc32[3] = (all_paritions_crc32_value>>24)&0xff;

	//Header CRC32 value set to 0 when calculate crc32
	gpt_head->header_crc32[0] = 0;
	gpt_head->header_crc32[1] = 0;
	gpt_head->header_crc32[2] = 0;
	gpt_head->header_crc32[3] = 0;

    //calculate gpt header crc32
	header_crc32_value = efi_crc32((const unsigned char*)gpt_head,le32_to_int(gpt_head->header_size));
    gpt_head->header_crc32[0] = header_crc32_value&0xff;
	gpt_head->header_crc32[1] = (header_crc32_value>>8)&0xff;
	gpt_head->header_crc32[2] = (header_crc32_value>>16)&0xff;
	gpt_head->header_crc32[3] = (header_crc32_value>>24)&0xff;

	return 0;

}




//gpt operation
int do_gptops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	block_dev_desc_t *mmc_dev = NULL;
	struct mmc *mmc = NULL;
	int efi_total_num = 0;
	int k = 0;
	int m=0;
	disk_partition_t current_partition_info;

	if (argc < 2)
	return CMD_RET_USAGE;
	
	if(curr_device < 0) 
	{
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return;
		}
	}
	if (strcmp(argv[1], "mktable") == 0) 
    {   
		unsigned char pbuf1[34*512]={0}; //gpt table
        block_dev_desc_t *mmc_dev;
		efi_disk_partition_t EFI_Disk_Partition_Info;
		struct mmc* mmc = find_mmc_device(curr_device);
		if (!mmc) 
		{
			printf("No MMC device at slot %x\n", curr_device);
			return 0;
		}
		mmc->has_init = 0;
		if (mmc_init(mmc))
		{
			printf("mmc_init error %x\n", curr_device);
			return 0;
		}	
		mmc_dev = mmc_get_dev(curr_device);
		
		memset((void*)&EFI_Disk_Partition_Info, 0 ,sizeof(efi_disk_partition_t));
		EFI_Disk_Partition_Info.PartNum = 0;

        memset(pbuf1,0,34*512);
		GeneratePrimaryGptTable(pbuf1, PRIMARY_GPT_TABLE_LEN*512, mmc_dev->lba, &EFI_Disk_Partition_Info);
		mmc->block_dev.block_write(curr_device, 0,PRIMARY_GPT_TABLE_LEN,pbuf1);

		memset(pbuf1,0,34*512);
		GenerateSecondaryGptTable(pbuf1, SECONDARY_GPT_TABLE_LEN*512, mmc_dev->lba, &EFI_Disk_Partition_Info);
		mmc->block_dev.block_write(curr_device, mmc_dev->lba-0x21,SECONDARY_GPT_TABLE_LEN,pbuf1);

		printf("ok!\n");
       
	}
	else if (strcmp(argv[1], "mkpart") == 0) 
	{
		unsigned char pbuf1[34*512]={0}; //gpt table
		efi_disk_partition_t EFI_Disk_Partition_Info;
        int efi_total_num;
		block_dev_desc_t *mmc_dev;

		if (argc < 5)
	     return CMD_RET_USAGE;
		
		struct mmc* mmc = find_mmc_device(curr_device);
		if (!mmc) 
		{
			printf("No MMC device at slot %x\n", curr_device);
			return 0;
		}
		mmc->has_init = 0;
		if (mmc_init(mmc))
		{
			printf("mmc_init error %x\n", curr_device);
			return 0;
		}	
		mmc_dev = mmc_get_dev(curr_device);
		
		memset((void*)&EFI_Disk_Partition_Info, 0 ,sizeof(efi_disk_partition_t));

		efi_total_num = get_efi_partition_total_num(mmc_dev); 
		
		EFI_Disk_Partition_Info.PartNum = efi_total_num;

		//get current  all  partitions info
		for(k=0;k<efi_total_num;k++)
		{ 
		   //get current all partitions info
		   get_partition_info_efi(mmc_dev,k+1,&current_partition_info);

		   strcpy(EFI_Disk_Partition_Info.PartInfo[k].name, current_partition_info.name);
           EFI_Disk_Partition_Info.PartInfo[k].blksz = 512;
		   EFI_Disk_Partition_Info.PartInfo[k].start = current_partition_info.start;
		   EFI_Disk_Partition_Info.PartInfo[k].size = current_partition_info.size;

		}

		//new partition info
		strcpy(EFI_Disk_Partition_Info.PartInfo[k].name,argv[2]);
		EFI_Disk_Partition_Info.PartInfo[k].blksz = 512;
		EFI_Disk_Partition_Info.PartInfo[k].start = simple_strtoul(argv[3], NULL, 16); //start block
		EFI_Disk_Partition_Info.PartInfo[k].size = simple_strtoul(argv[4], NULL, 16);  //block cnt
		
        //start block little than last size, error
        if( (k>0) && (EFI_Disk_Partition_Info.PartInfo[k].start < EFI_Disk_Partition_Info.PartInfo[k-1].start+EFI_Disk_Partition_Info.PartInfo[k-1].size))
    	{
    	   printf("Error:Current partition start block:0x%08x is little than 0x%08x!\n",EFI_Disk_Partition_Info.PartInfo[k].start,EFI_Disk_Partition_Info.PartInfo[k-1].start+EFI_Disk_Partition_Info.PartInfo[k-1].size);
		   return 0;
    	}
		
		EFI_Disk_Partition_Info.PartNum++;

        //generate and write primary gpt table
        memset(pbuf1,0,PRIMARY_GPT_TABLE_LEN*512);
		GeneratePrimaryGptTable(pbuf1, PRIMARY_GPT_TABLE_LEN*512, mmc_dev->lba, &EFI_Disk_Partition_Info);
		mmc->block_dev.block_write(curr_device, 0,PRIMARY_GPT_TABLE_LEN,pbuf1);

        //generate and write secondary gpt table 
        memset(pbuf1,0,SECONDARY_GPT_TABLE_LEN*512);
		GenerateSecondaryGptTable(pbuf1, SECONDARY_GPT_TABLE_LEN*512, mmc_dev->lba, &EFI_Disk_Partition_Info);
		mmc->block_dev.block_write(curr_device, mmc_dev->lba-SECONDARY_GPT_TABLE_LEN, SECONDARY_GPT_TABLE_LEN,pbuf1);

		printf("ok!\n");
 	}
	else if (strcmp(argv[1], "print") == 0) 
    {
	    block_dev_desc_t *mmc_dev;
		struct mmc* mmc = find_mmc_device(curr_device);
		if (!mmc) 
		{
			printf("No MMC device at slot %x\n", curr_device);
			return 0;
		}
		mmc->has_init = 0;
		if (mmc_init(mmc))
		{
			printf("mmc_init error %x\n", curr_device);
			return 0;
		}	
		mmc_dev = mmc_get_dev(curr_device);
		if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) 
		{
			//print gpt partition info
			printf("GPT Partition Map As Follow:\n");
			print_part_efi(mmc_dev); 
		}
   	}
	else
	{
	   printf("unknown command!\n");
	}

	return 0;
}
	
//GPT Parition Cmd
U_BOOT_CMD(
	gpt, 6, 1, do_gptops,
	"gpt mktable, gpt mkpart partname offset size, gpt print\n",
	);

#endif


#endif
