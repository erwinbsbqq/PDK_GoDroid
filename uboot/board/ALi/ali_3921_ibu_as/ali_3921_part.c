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

#include <ali/sys_parameters.h>
#include <ali/sec_boot.h>
#include <ali/flash_cipher.h>
#include <ali/otp.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

extern unsigned char g_uk_pos;
#define SIG_LEN   0x100


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

	*(volatile unsigned long *)(0x18000074) = (1<<23)|(1<<18);		// strap pin to nand
	
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
/*
        if (bufLen > dataLen-12)
        {
            printf("sector format is not matched, size:0x%x, datalen:0x%x\n",bufLen,dataLen-12);    
            kfree(datbuf_tmp);                   
            return -1;      
        }
   */ 
        memcpy(LoadAddr, datbuf, bufLen);           
    
        kfree(datbuf_tmp);                       
        return bufLen;
}

int load_part_ubo(const char *partName, u_char * LoadAddr)
{
	size_t preReadSize;
	size_t realUBOLen;
	image_header_t *hdr = NULL;

	*(volatile unsigned long *)(0x18000074) = (1<<23)|(1<<18);		// strap pin to nand
	
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
	realUBOLen = ((realUBOLen+0x20-1) & ~(0x20-1)) + SIG_LEN;
	if (load_part_ext(partName, LoadAddr, 0, realUBOLen)<0)
	{
		printf("%s(%d): %s read data error\n",__FUNCTION__,__LINE__,partName);
		return -4;
	}
	flush_dcache_range(LoadAddr, LoadAddr+realUBOLen);
	return realUBOLen;
}

int load_part_ubo_as(const char *partName, u_char * LoadAddr)
{
	size_t preReadSize;
	size_t realUBOLen;
	image_header_t *hdr = NULL;
	u_char * tmp_buff = (u_char *)(0x8a000000);
    //u_char * tmp_buff = NULL;
    
	*(volatile unsigned long *)(0x18000074) = (1<<23)|(1<<18);		// strap pin to nand

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
	realUBOLen = ((realUBOLen+0x20-1) & ~(0x20-1)) + SIG_LEN;
/*	
	tmp_buff = (struct boot_args*)memalign(0x20,realUBOLen);
    if(tmp_buff == NULL)
    {
        printf("%s: malloc error\n", __FUNCTION__);
        return -1;
    }
*/	 
	if (load_part_ext(partName, tmp_buff, 0, realUBOLen)<0)
	{
		printf("%s(%d): %s read data error\n",__FUNCTION__,__LINE__,partName);
		return -4;
	}
	aes_cbc_decrypt_ram_chunk(g_uk_pos,LoadAddr+UBO_HEAD_LEN, tmp_buff+UBO_HEAD_LEN, realUBOLen-UBO_HEAD_LEN);
	memcpy(LoadAddr, tmp_buff, UBO_HEAD_LEN);
	//free(tmp_buff);
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

	if(dcrc_chk(entry) == 0)
	{
           return 0;
	}

       return -1;
}
static int calculate_version(unsigned char *name)
{
	int i = 0;
	unsigned char flag[3] = "_V";
	unsigned int version = 0;
	
	for(i=0; i<IH_NMLEN-1; i++)
	{
		if(strncmp(name, flag, 2) == 0)
			break;
		name++;
	}
	if(i == IH_NMLEN-1)
	{
		printf("%s: Not find version\n", __FUNCTION__);
		return -1;
	}
	version = (unsigned int)*(name+2) - 0x30;
	printf("current version is %d\n", version);
	return version;
}
static int check_sw_version(u_char *entry)
{
	UINT32 current_version;
    INT32 ret = 0;
	image_header_t *hdr = NULL;
	unsigned char *name = NULL;

	hdr = (image_header_t *)entry;
	name = hdr->ih_name;
	current_version = calculate_version(name);
	if(current_version == -1)
		return -1;
	ret = version_verify(current_version, SW_VER);
	return ret;
}

#ifdef LOAD_KERNEL_SEE
#define SEE_ACK_BIT     (1<<8) /*see verify pass =1 */
extern UINT32 daemon_time;
extern UINT32 main_hw_ack_sync(UINT32 type,UINT32 tmo);

int load_kernel(struct UKEY_INFO *ukey)
{
	unsigned long len, kernel_len;
	u_char *entry, *kernel_entry;
	//unsigned int part_idx = -1;
	volatile ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	otp_init(NULL);
    UINT8  *see_tmp_buf = 0x8a000000-UBO_HEAD_LEN;
 	entry = (void *)(info->memmap_info.ae_start |0x80000000) - UBO_HEAD_LEN;//64byte ubo header size
	//entry = (void *)ADDR_LOAD_AUD_CODE;
	printf("\nStart to load partition '%s' to 0x%x....\n", PART_NAME_AUD_CODE,entry);
	len = load_part_ubo(PART_NAME_AUD_CODE, see_tmp_buf);
	if (len<0)
	{
		printf("Load audio CPU code Fail...\n");
		return -1;
	}	
	printf("Load audio CPU code Done...\n");
	set_see_code_info(see_tmp_buf,len,entry+UBO_HEAD_LEN);
	set_see_sig_info((UINT32)see_tmp_buf+len-256, 256);
	set_see_key_info(ukey,1);
	if(RET_SUCCESS != bl_verify_SW_see(CASE2_SEE)){
	    printf( "audio code verify failed\n" );        
		return -1;
	}
	kernel_entry = (void *)(info->memmap_info.kernel_start |0x80000000) - 64;//64byte ubo header size
	printf("\n new Start to load partition '%s' to 0x%x....\n", PART_NAME_KERNEL,entry);
	kernel_len = load_part_ubo_as(PART_NAME_KERNEL, kernel_entry);
    printf("Load kernel code Done...\n");


	if(RET_SUCCESS != main_hw_ack_sync(SEE_ACK_BIT,daemon_time))
	{
	    printf( "audio code verify failed\n" );        
		return -1;
	}
	//step1:first load see and boot, to speed up show logo time;
	entry = (void *)(info->memmap_info.see_start |0x80000000)  - UBO_HEAD_LEN;//64byte ubo header size
    
    // see_tmp_buf -= 64;
	printf("\nnew Start to load partition '%s' to 0x%x....\n", PART_NAME_SEE,entry);
	
	len = load_part_ubo(PART_NAME_SEE, see_tmp_buf);
	if (len<0)
	{
		printf("Load see code Fail...\n");
		return -1;
	}	
	printf("Load see code Done...\n");

	set_see_code_info(see_tmp_buf,len,entry+UBO_HEAD_LEN);
	set_see_sig_info((UINT32)see_tmp_buf+len-256, 256);
	set_see_key_info(ukey,1);
	if(RET_SUCCESS != bl_verify_SW_see(CASE2_SEE)){
	    printf( "uboot see software verify failed\n" );        
		return -1;
	}
	if(test_rsa_ram((UINT32)kernel_entry, kernel_len) != 0)
	{
		printf("kernel code rsa verify error\n");
		return -1;
	}
	if(check_sw_version(kernel_entry) != RET_SUCCESS)
	{
		printf("Error : %s, kernel version check\n", __FUNCTION__);
		return -1;
	}
	if(RET_SUCCESS != bl_run_SW_see()){
	    printf( "uboot see software run failed\n" );        
		return -1;
	}
	printf("SEE software run success \n");
	set_cmdline(0);
	return 0;
}
#endif

int load_bootmedia(void)
{
	unsigned long len;
	u_char *entry;
	
	ADF_BOOT_BOARD_INFO *info = (ADF_BOOT_BOARD_INFO *)PRE_DEFINED_ADF_BOOT_START;

	*(volatile unsigned long *)(0x18000074) = (1<<23)|(1<<18);		// strap pin to nand
	
	memset(&info->media_info,0,sizeof(info->media_info));
	
	info->media_info.play_enable = 0;

	entry = (void *)(info->memmap_info.boot_media_start|0x80000000);
	printf("\n new Start to load partition '%s' to 0x%x....\n", PART_NAME_BM,entry);
	len = load_part_ubo_as(PART_NAME_BM, entry);
	if (len<0)
	{
		printf("Load %s Fail...\n",PART_NAME_BM);
		return -1;
	}		
	printf("Load %s Done...\n",PART_NAME_BM);
	
	if( version_check(CHUNKID_SEECODE,(UINT32)entry,len) != 0){
		printf("%s  version check failed\n",PART_NAME_BM);
		return -1;
	} 

	if(test_rsa_ram((UINT32)entry, len) != 0)
	{
		printf("%s  rsa verify error\n",PART_NAME_BM);
		return -1;
	}
	
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

void output_char(unsigned char c)
{
	while(((*(volatile unsigned char *)0x18018305)&0x40) == 0x00);	//wait empty
	*(volatile unsigned char *)0x18018300 = c;
}

char ascii[] = "0123456789ABCDEF";
void dump_reg(unsigned long addr,unsigned long len)
{
	unsigned long i,j;
	//char* ascii;
	unsigned char index;
	
//	char ascii[] = "0123456789ABCDEF";
	for(i=0;i<len;i++)
	{
		if(i%16 == 0)
		{
			output_char(0x0d);
			output_char(0x0a);
			for(j=0;j<8;j++)
			{
				output_char(ascii[((addr+i)>>(4*(7-j)))&0xF]);
			}
			output_char(':');
		}
		index = *(unsigned char *)(addr+i);
		output_char(ascii[(index>>4)&0xF]);
		output_char(ascii[index&0xF]);
		output_char(' ');
	}
	output_char(0x0d);
	output_char(0x0a);
	return;
}
/*define register for pass the parameter from CPU to SEE*/
#define SYS_SEE_NB_BASE_ADDR 0x18040000
#define SEEROM_SEE_REAL_RUN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b0)
#define SEEROM_SEE_CODE_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2b4)
#define SEEROM_SEE_CODE_LEN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b8)
#define SEEROM_SEE_SIGN_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2bC)
#define SEEROM_DEBUG_KEY_ADDR           (SYS_SEE_NB_BASE_ADDR+0x2C0)
#define ALIASIX_RSA_COMPARE_LEN               256
#define GET_FLASH_DATA(p)  ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3])
int load_uboot_see(UINT32 len)
{
	
    UINT32 see_code_addr = 0x85000200;
    UINT32 see_code_len = 0;
    UINT32 see_code_sign_addr = 0;
    UINT32 see_real_run_addr = 0;

    see_code_sign_addr = see_code_addr + len - 2*ALIASIX_RSA_COMPARE_LEN; 
    see_code_len = len-2*ALIASIX_RSA_COMPARE_LEN;
    install_memory();
   	printf("install_memory in uboot\n");
    see_code_addr |= 0xa0000000;
    see_code_sign_addr |= 0xa0000000;
    see_real_run_addr = see_code_addr;
    *((UINT32 *)SEEROM_SEE_CODE_LOCATION_ADDR) = see_code_addr;
    *((UINT32 *)SEEROM_SEE_CODE_LEN_ADDR) = see_code_len;
    *((UINT32 *)SEEROM_SEE_SIGN_LOCATION_ADDR) = see_code_sign_addr;
    (*(UINT32 *)SEEROM_SEE_REAL_RUN_ADDR) = see_real_run_addr;
	g_see_boot();
	//ali_see_boot(0x85000200);
	printf("g_see_boot in uboot\n");	
	return 0;
}

