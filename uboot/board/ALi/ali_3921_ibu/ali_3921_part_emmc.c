

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
static unsigned char datbuf[SECTOR_DATA_BUF_LEN];

int get_gpt_partition_index(const char *PartName)
{
	/*******Begin:Added by kinson.zhou for Get GPT Info*******/
#ifdef CONFIG_EFI_PARTITION 
    int curr_device= -1;
	block_dev_desc_t *mmc_dev = NULL;
	struct mmc *mmc = NULL;
	int efi_total_num = 0;
	int k = 0;
	disk_partition_t current_partition_info;

	if(curr_device < 0) 
	{
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return -1;
		}
	}

	mmc = find_mmc_device(curr_device);
	if (!mmc) 
	{
		printf("No MMC device at slot %x\n", curr_device);
		return -1;
	}

	//mmc->has_init = 0;

	if (mmc_init(mmc))
	{
	    printf("mmc_init error %x\n", curr_device);
		return -1;
	}
		
	mmc_dev = mmc_get_dev(curr_device);
	if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) 
	{
		//get gpt max partitions number
	    efi_total_num = get_efi_partition_total_num(mmc_dev); 

		//search for start block of kerne, see, ae
		for(k=1;k<=efi_total_num;k++)
		{
		   //get current partition info
		   get_partition_info_efi(mmc_dev,k,&current_partition_info);

		   //compare partition name
		   if(!strcmp((char*)(current_partition_info.name),(char*)PartName))
		   {
		       printf("Now, Get GPT Part Index:[%d] By Name:[%s]!\n",k, PartName);
		       return k;
		   }
		}
	}
	else
	{
	   printf("Can not Get GPT Part Index By Name:[%s]!\n",PartName);
	   return -1;
	}
#else
    return -1;
#endif
	/*******End:Added by kinson.zhou for Get GPT Info*******/
}

disk_partition_t* get_gpt_partition_info_by_index(int gpt_part_index)
{
	
#ifdef CONFIG_EFI_PARTITION 
		int curr_device= -1;
		block_dev_desc_t *mmc_dev = NULL;
		struct mmc *mmc = NULL;
		disk_partition_t partition_info;
		disk_partition_t* p_current_partition_info = &partition_info;
	
		if(curr_device < 0) 
		{
			if (get_mmc_num() > 0)
				curr_device = 0;
			else {
				puts("No MMC device available\n");
				return;
			}
		}
	
		mmc = find_mmc_device(curr_device);
		if (!mmc) 
		{
			printf("No MMC device at slot %x\n", curr_device);
			return;
		}
	
		//mmc->has_init = 0;
	
		if (mmc_init(mmc))
		{
			printf("mmc_init error %x\n", curr_device);
			return;
		}
			
		mmc_dev = mmc_get_dev(curr_device);
		if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) 
		{
			//get current partition info
			get_partition_info_efi(mmc_dev,gpt_part_index,p_current_partition_info);
			return p_current_partition_info;
		}
		else
		{
		   printf("Get GPT Partition Info Error!\n");
		   return NULL;
		}
#endif
}

int load_part_sector_emmc(const char *partName, u_char * LoadAddr, int bufLen)
{
    int dataLen;
	int part_index = -1;
	disk_partition_t* cur_partition_info = NULL;
	char s[100];
	
    //get the gpt partition index by partition name
	part_index = get_gpt_partition_index(partName);
	if(part_index <1)
	{
	   printf("partition[%s] not exit!\n",partName);
	   return -1;
	}

	//get the gpt partition info
	cur_partition_info = (disk_partition_t*)get_gpt_partition_info_by_index(part_index);
	if(!cur_partition_info)
	{
	   printf("partition[%s] info err!\n",partName);
	   return -1;
	}

	//read partition data from emmc to datbuf
	sprintf(s,"mmc read %08x %x %x",datbuf,cur_partition_info->start,cur_partition_info->size);
	run_command (s, 0);

	/* check block data */
	dataLen = _sector_crc_check(datbuf, SECTOR_DATA_BUF_LEN);
	if (dataLen<0)
	{
	   printf("dataLen err!\n");
	   return -1;
	}

	if (bufLen != dataLen-12)
	{
		printf("sector format is not matched, size:0x%x, datalen:0x%x\n",bufLen,dataLen-12);	
		return -1;		
	}
   
	memcpy(LoadAddr, datbuf+12, dataLen-12);
						
	return dataLen-12;


}


#ifdef LOAD_KERNEL_SEE

#if defined(CONFIG_CMD_NAND) 
#error "It seems you define CONFIG_CMD_NAND,CONFIG_ALI_MMC and LOAD_KERNEL_SEE at the same time, please double check it."
#endif

int eMMC_PartitionDataChk(unsigned char *entry)
{
	int i,j;
	unsigned int nz_exist =0;

       for(i=0;i<UBO_HEAD_LEN;i++)
       {
            if (entry[i] !=0)
            {
                nz_exist = 1;
                break;
            }
       }

       if (0 == nz_exist)
       {
            printf("---------The UBO header are ZERO----------\n");
            return -1;
       }

	if(dcrc_chk(entry) == 0)
	{
#if 0
           printf("---------UBO file header----------\n");
           for(i=0;i<4;i++)
           {
                for(j=0;j<16;j++)
                {
                    printf("0x%02x,",entry[i*16+j]);
                }
                printf("\n");
           }

           printf("---------first 64 bytes data----------\n");
           for(i=4;i<8;i++)
           {
                for(j=0;j<16;j++)
                {
                    printf("0x%02x,",entry[i*16+j]);
                }
                printf("\n");
           }
           printf("\n");
#endif
           return 0;
	}

       return -1;
}




#define KERNEL_START_BLK	16	
#define KERNEL_BLK_CNT		13025
#define SEE_START_BLK		97728
#define SEE_BLK_CNT			6084
#define AUDIO_START_BLK	    195456
#define AUDIO_BLK_CNT		1043
#define DATA_CRC_ON

void Load_Kernel_See(void)
{
	unsigned char *entry;
	//int part_idx = -1;
	 char s[100];
	 int start_blk,blk_cnt;
	 int kernel_start_blk,kernel_blk_cnt;
	 int see_start_blk,see_blk_cnt;
	 int ae_start_blk,ae_blk_cnt;
	 volatile ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	//printf ("Please switch STRAPIN 18, then hit any key to continue ... ");
	//while (!tstc())
		//;
	/* consume input */
	//(void) getc();

	/*******Begin:Added by kinson.zhou for Get GPT Info*******/
#ifdef CONFIG_EFI_PARTITION 
    int curr_device= -1;
	block_dev_desc_t *mmc_dev = NULL;
	struct mmc *mmc = NULL;
	int efi_total_num = 0;
	int k = 0;
	disk_partition_t current_partition_info;

	if(curr_device < 0) 
	{
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return;
		}
	}

	mmc = find_mmc_device(curr_device);
	if (!mmc) 
	{
		printf("No MMC device at slot %x\n", curr_device);
		return;
	}

	//mmc->has_init = 0;

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

		//search for start block of kerne, see, ae
		for(k=1;k<=efi_total_num;k++)
		{
		   //get current partition info
		   get_partition_info_efi(mmc_dev,k,&current_partition_info);

		   //compare partition name
		   if(!strcmp((char*)(current_partition_info.name),(char*)"kernel"))
		   {
		   	   kernel_start_blk = current_partition_info.start;
			   kernel_blk_cnt = current_partition_info.size;
			   printf("kernel:start_blk=0x%08x  blk_cnt=%d\n",kernel_start_blk,kernel_blk_cnt);
		   }
		   else if(!strcmp((char*)(current_partition_info.name),(char*)"see"))
		   {
		   	   see_start_blk = current_partition_info.start;
			   see_blk_cnt = current_partition_info.size;
			   printf("see   :start_blk=0x%08x  blk_cnt=%d\n",see_start_blk,see_blk_cnt);
		   }
		   else if(!strcmp((char*)(current_partition_info.name),(char*)"ae"))
		   {
		   	   ae_start_blk = current_partition_info.start;
			   ae_blk_cnt = current_partition_info.size;
			   printf("ae    :start_blk=0x%08x  blk_cnt=%d\n",ae_start_blk,ae_blk_cnt);
		   }
		}
	}
	else
	{
	   printf("Get GPT Partition Info Error!\n");
	   return;
	}
	/*******End:Added by kinson.zhou for Get GPT Info*******/
#else
	kernel_start_blk = KERNEL_START_BLK;
	kernel_blk_cnt = KERNEL_BLK_CNT;

    see_start_blk = SEE_START_BLK;
	see_blk_cnt = SEE_BLK_CNT;

	ae_start_blk = AUDIO_START_BLK;
	ae_blk_cnt = AUDIO_BLK_CNT;
#endif

    //load audio
	entry = (void *)(info->memmap_info.ae_start |0x80000000) - 64;//entry = (void *)ADDR_LOAD_AUD_CODE;
	printf("\nStart to load partition '%s' to 0x%x....\n", PART_NAME_AUD_CODE,entry);
	//start_blk = AUDIO_START_BLK;
	//ae_blk_cnt = AUDIO_BLK_CNT;
	sprintf(s,"mmc read %08x %x %x",entry,ae_start_blk,ae_blk_cnt);
	run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
	printf("Load audio CPU code from Emmc Done...\n");
#if defined(DATA_CRC_ON)
	eMMC_PartitionDataChk(entry);
#endif

    //load see
	entry = (void *)(info->memmap_info.see_start |0x80000000) - 64; //entry = (void *)ADDR_LOAD_SEE;
	printf("\nStart to load partition '%s' to 0x%x....\n", PART_NAME_SEE,entry);
	//start_blk = SEE_START_BLK;
	//see_blk_cnt = SEE_BLK_CNT;
	sprintf(s,"mmc read %08x %x %x",entry,see_start_blk,see_blk_cnt);
	run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
	printf("Load see code from Emmc Done...\n");
#if defined(DATA_CRC_ON)
	eMMC_PartitionDataChk(entry);
#endif

	//boot see
	install_memory();
	printf("install_memory in uboot\n");
	g_see_boot();
	printf("g_see_boot in uboot\n");

    //load kernel
	entry = (void *)(info->memmap_info.kernel_start |0x80000000) - 64;//entry = = (void *)ADDR_LOAD_KERNEL;
	printf("\nStart to load eMMC '%s' to 0x%x....\n", PART_NAME_KERNEL,entry);
	//kernel_start_blk = KERNEL_START_BLK;
	//kernel_blk_cnt = KERNEL_BLK_CNT;
	sprintf(s,"mmc read %08x %x %x",entry,kernel_start_blk,kernel_blk_cnt);
	run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
	printf("Load kernel code from Emmc Done...\n");
#if defined(DATA_CRC_ON)
	eMMC_PartitionDataChk(entry);
#endif

	//set_cmdline(0);
}
#endif
