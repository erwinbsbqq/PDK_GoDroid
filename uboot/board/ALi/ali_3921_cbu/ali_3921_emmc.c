#include <jffs2/load_kernel.h>
#include <common.h>
#include <asm/io.h>
#include <boot_common.h>
#include <asm/dma-mapping.h>
#include <linux/err.h>
#include "block_sector.h"
#include <alidefinition/adf_boot.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

#define SECTOR_DATA_BUF_LEN   0x200000

static disk_partition_t *gpt=NULL;
static int efi_total_num = -1;

disk_partition_t* get_gpt_info(int *count)
{
#ifdef CONFIG_EFI_PARTITION     
    int curr_device= -1;
	block_dev_desc_t *mmc_dev = NULL;
	struct mmc *mmc = NULL;
	int k = 0;
	disk_partition_t *info;

    if (gpt)
    {
        *count = efi_total_num;
        return gpt;
    }

	if (get_mmc_num() > 0)
		curr_device = 0;
	else
	{
		puts("No MMC device available\n");
		return NULL;
	}

	mmc = find_mmc_device(curr_device);
	if (!mmc) 
	{
		printf("No MMC device at slot %x\n", curr_device);
		return NULL;
	}

	if (mmc_init(mmc))
	{
	    printf("mmc_init error %x\n", curr_device);
		return;
	}
		
	mmc_dev = mmc_get_dev(curr_device);
	if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) 
	{
	    //print gpt partition info
	    printf("GPT Partition Map As Follow:\n");
	    print_part_efi(mmc_dev); 

    	//get gpt max partitions number
        efi_total_num = get_efi_partition_total_num(mmc_dev); 

        gpt = (disk_partition_t*)malloc(sizeof(disk_partition_t)*efi_total_num);
        if (gpt == NULL)
        {
            printf("%s(%d): malloc fail\n",__FUNCTION__,__LINE__);
            return NULL;  
        }
        memset(gpt, 0, sizeof(disk_partition_t)*efi_total_num);

        info = gpt;
        
    	//search for start block of kerne, see, ae
    	for(k=1;k<=efi_total_num;k++)
    	{
    	   //get current partition info
    	   get_partition_info_efi(mmc_dev,k,info);
           info++;
    	}
        
        *count = efi_total_num;
	}
	else
	{
	   printf("%s(%d):Get GPT Partition Info Error!\n",__FUNCTION__,__LINE__);
       *count = 0;
	   return NULL;
	}

    return gpt;
#else
    *count = 0;
    return NULL;
#endif
}

int get_gpt_part_by_name(const char *PartName, disk_partition_t* info)
{
#ifdef CONFIG_EFI_PARTITION 
    int curr_device= -1;
	block_dev_desc_t *mmc_dev = NULL;
	struct mmc *mmc = NULL;
	int k = 0;
    int gptCount;
	disk_partition_t *gptInfo = get_gpt_info(&gptCount);

    if (info == NULL)
    {
		printf("%s(%d):info is NULL\n",__FUNCTION__,__LINE__);
		return -1;    
    }

    if (gptInfo == NULL)
    {
        printf("%s(%d):Get GPT Partition Info Error!\n",__FUNCTION__,__LINE__);
        return -1;    
    }

	//search for start block of kerne, see, ae
	for(k=1; k<=gptCount; k++)
	{
	   if(!strcmp((char*)(gptInfo->name),(char*)PartName))
	   {
	       printf("Now, Get GPT Part Index:[%d] By Name:[%s]!\n",k, PartName);
           memcpy(info, gptInfo, sizeof(disk_partition_t));
           return k;

	   }
       gptInfo++;
	}
    return -1;

#else
    return -1;
#endif
}

int get_part_by_name_emmc(const char *partName, loff_t *start, size_t *size)
{
    int ret;
    disk_partition_t info;
    
    ret = get_gpt_part_by_name(partName, &info);
    *start = info.start*info.blksz;
    *size = info.size*info.blksz;
    return ret;
}

int load_part_emmc(const char *partName, u_char *LoadAddr)
{
    disk_partition_t partInfo;
    char s[100];

    if (get_gpt_part_by_name(partName, &partInfo) < 0)
    {
        printf("%s(%d):Get GPT Partition(%s) Info Error!\n",__FUNCTION__,__LINE__,partName);
        return -1;    
    }

    sprintf(s,"mmc read %08x %x %x", LoadAddr, partInfo.start, partInfo.size);
    run_command(s, 0); 

    return 0;
}

int load_part_emmc_ext(const char *partName, u_char *LoadAddr, loff_t offset, size_t len)
{
    disk_partition_t partInfo;
    char s[100];
    ulong blk_start;
    ulong blk_count;
    ulong remain;
    unsigned char datbuf[SECTOR_DATA_BUF_LEN];    
    ulong blk_off;
    u_char *p;

    if (get_gpt_part_by_name(partName, &partInfo) < 0)
    {
        printf("Get GPT Partition(%s) Info Error!\n",partName);
        return -1;    
    }

    if (offset+len > partInfo.size*partInfo.blksz)
    {
        printf("load_part_emmc_ext len(0x%x) is over!\n",len);
        return -1;    
    }

    remain = len;
    blk_off = offset%partInfo.blksz;
    
    blk_start = partInfo.start + offset/partInfo.blksz;
    p = LoadAddr;
    if (blk_off)
    {
        blk_count = 1;
        sprintf(s,"mmc read %08x %x %x", datbuf, blk_start, blk_count);
        run_command(s, 0); 

        memcpy(p, &datbuf[blk_off], partInfo.blksz-blk_off);

        p += partInfo.blksz-blk_off;
        blk_start++;
        remain -= partInfo.blksz-blk_off;        
    }

    blk_count = remain/partInfo.blksz;
    if (blk_count > 0)
    {
        sprintf(s,"mmc read %08x %x %x", p, blk_start, blk_count);
        run_command(s, 0); 

        p += blk_count*partInfo.blksz;
        blk_start += blk_count;
        remain -= blk_count*partInfo.blksz;        
    }
    
    if (remain)
    {    
        blk_count = 1;
        sprintf(s,"mmc read %08x %x %x", datbuf, blk_start, blk_count);
        run_command(s, 0);            
        memcpy(p, datbuf, remain);
    }

    return 0;
}

int load_part_sector_emmc(const char *partName, u_char * LoadAddr, int bufLen)
{
    int ret;
    disk_partition_t partInfo;
    unsigned char datbuf[SECTOR_DATA_BUF_LEN];
    char s[100];
    ulong start;
    ulong size;
	int dataLen;
    
    if (get_gpt_part_by_name(partName, &partInfo) < 0)
    {
        printf("Get GPT Partition(%s) Info Error!\n",partName);
        return -1;    
    }

    start = partInfo.start;
    size = partInfo.size;
    if (size * partInfo.blksz > SECTOR_DATA_BUF_LEN)
    {
        size = SECTOR_DATA_BUF_LEN/partInfo.blksz; 
    }
    
	/* read partition data from emmc to datbuf */
	sprintf(s,"mmc read %08x %x %x",datbuf,start,size);
	run_command (s, 0);

	/* check block data */
	dataLen = _sector_crc_check(datbuf, size*partInfo.blksz);
	if (dataLen < 0)
	{
	   printf("dataLen err!\n");
	   return -1;
	}

    ret = bufLen;
	if (bufLen > dataLen-12)
	{
		printf("sector format is not matched, size:0x%x, datalen:0x%x\n",bufLen,dataLen-12);	
		ret = dataLen - 12;		
	}
   
	memcpy(LoadAddr, datbuf+12, ret);
						
	return ret;
}

