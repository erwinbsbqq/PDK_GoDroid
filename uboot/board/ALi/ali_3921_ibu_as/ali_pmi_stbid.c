#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <ali_nand.h>
#include "ali_pmi_stbid.h"
//#include <configs/ali-stb.h>
#include <ali/basic_types.h>


#ifdef CONFIG_NAND_ALI
#define ALI_DEBUG_ON
#ifdef ALI_DEBUG_ON
    #define ALI_PRINTF   printf
#else
    #define ALI_PRINTF(...)	do{}while(0)
#endif

#define SDBBP()		asm volatile(".word	0x7000003f; nop")

/*
******block struct*******

		block1					block2
	________________		________________
	|	sector11		|		|	sector21		|
	|---------------|		|---------------|
	|	sector12		|		|	sector22		|
	|---------------|		|---------------|
	|	...			|		|	...			|		
	|				|		|				|	
	|---------------| 		|---------------|
	|	sector1n		|		|	sector2n		|
	|_______________|		|_______________|

	1. a block has several sectors
	2. in a block, each sector size is the same, 
		sector11 size = sector12 size = ...= sector1n size
		sector21 size = sector22 size = ...= sector2m size		
	3. in normal case, two blocks have the same sector size
		sector11 size = sector21 size
	4. sector size must align to pagesize, because read/write unit is based on pagesize.
	5. data save serial: (b1: valid sector num in b1; b2: valid sector num in b2)
		 init state:block1 & block2 are free
		 	b1=b2=0;

		case1:save new sector(b1<n-1)
		(1) b1=b2=i
		(2) write block1 sector[i+1], b1=i+1, b2=i
		(3) write block2 sector[i+1], b1=i+1, b2=i+1

		case2:save new sector(b1==n-1)
		(1) b1=b2=n-1
		(2) write block1 sector[n], b1=n, b2=n-1
		(3) erase b2, b1=n, b2=0
		(4) write block2 sector[1], b1=n, b2=1
		(5) erase b1, b1=0, b2=1
		(6) write block1, b1=b2=1
		
		case3:if new sector size change
		(1) b1=b2=i
		(2) erase b2, b1=i, b2=0
		(3) write block2 sector[1], b1=i,b2=1
		(4) erase b1, b1=0,b2=1
		(5) write block1 sector[1], b1=b2=1
		

******sector struct******
	________________________
	|	data_len(4B)			|
	|-----------------------|
	|	data_len_invert(4B)		|
	|-----------------------|
	|	CRC(4B)				|
	|-----------------------| 
	|						|			
	|	payload_data			|
	|	(data_len-12 bytes)		|			
	|-----------------------| 
	|	reserve data			|				
	|_______________________|

	valid sector:
	1. data_len + data_len_invert = 0xFFFFFFFF
	2. CRC = payload data crc
*/

#define NS_BLOCK_MAX_NUM 10

#define LONG_LIT(addr)		(((addr)[0])|((addr)[1]<<8)|((addr)[2]<<16)|((addr)[3]<<24))
#define LONG_BIT(addr)		(((addr)[0]<<24)|((addr)[1]<<16)|((addr)[2]<<8)|((addr)[3]))

#define NS_BLOCK_FREE -1
#define NS_BLOCK_INVALID -2

#define NS_SECTOR_FREE -1
#define NS_SECTOR_INVALID -2

#define MG_CRC_32_ARITHMETIC_CCITT

static unsigned int MG_CRC_Table[256];
static u_char pmi_buf[PMI_HASH_OFFSET +1024+ NAND_BUFFER_ADD_SIZE];
static PMI_t *pPMI = NULL;
static STB_ID *p_stbid=NULL;
static u_char stbid_buf[MAX_STBID_LEN];
#ifndef HAVE_PART_TBL_PARTITION
static PART_TBL PartitionInfo;
#else
static PART_TBL *p_partTbl=NULL;
static PART_TBL *p_PMIPartInfo=NULL;
static u_char parttbl_buf[sizeof(PART_TBL)];
static unsigned int offset_parttbl,len_parttbl;
#endif

#ifdef PMI_PART_SCT
struct PMI         _gxPMI;
struct Partation   _gxPartations[MAX_STBPART_NUM];
UINT8* g_pmi_blk = NULL ; 
UINT32 eccsize ;

#endif



/**************************************************************
	pack sector data, calculate crc for sector[12]~sector[12+data_len]
	sector[0]~sector[3] : data_len
	sector[4]~sector[7] : data_len_invert
	sector[8]~sector[11] : payload data crc

	para:
		sector : point to sector data
		data_len : sector data len
	return:
		0 : success
***************************************************************/
static int _sector_crc_pack(UINT8 *sector, UINT32 payload_len)
{
	UINT32 crc;
	UINT32 data_len,data_len_invert;
	
	/* sector[0-3]: data len */
	data_len = payload_len+12;	
	sector[0] = data_len&0xff;
	sector[1] = (data_len>>8)&0xff;
	sector[2] = (data_len>>16)&0xff;
	sector[3] = (data_len>>24)&0xff;

	/* sector[4-7]: data len invert */
	data_len_invert = 0xFFFFFFFF-data_len;
	sector[4] = data_len_invert&0xff;
	sector[5] = (data_len_invert>>8)&0xff;
	sector[6] = (data_len_invert>>16)&0xff;
	sector[7] = (data_len_invert>>24)&0xff;

	/* sector[8-11]: crc for sector data */
	crc = (UINT32)MG_Table_Driven_CRC(0xFFFFFFFF, sector+12, payload_len);	
	sector[8] = crc&0xff;
	sector[9] = (crc>>8)&0xff;
	sector[10] = (crc>>16)&0xff;
	sector[11] = (crc>>24)&0xff;

	return 0;
}

/**************************************************************
	check sector data crc
	para: 
		sector -- point to sector data;
		sector_size -- sector struct len;
	return:
		data_len -- sector data len
		NS_SECTOR_FREE -- sector each byte is 0xFF
		NS_SECTOR_INVALID -- data_len!=~data_len_invert 
							or data_len>data_size 
							or crc error
***************************************************************/
static int _sector_crc_check(UINT8 *sector, UINT32 sector_size)
{
	UINT32 crc, crc_check;
	UINT32 data_len, data_len_invert;
	UINT32 i;

	/* sector is free */
	for (i=0; i<sector_size; i++)
	{
		if (sector[i]!=0xFF)
			break;
	}
	if (i == sector_size)
	{
		//NS_DEBUG("sector is free\n");
		return NS_SECTOR_FREE;
	}
	
	data_len = LONG_LIT(sector);
	data_len_invert = LONG_LIT(sector+4);

	if (data_len + data_len_invert != 0xFFFFFFFF)
	{
		ALI_PRINTF("sector_len:0x%x, sector_len_invert: 0x%x, not match\n",data_len,data_len_invert);
		return NS_SECTOR_INVALID;
	}

	if (data_len > sector_size)
	{
		ALI_PRINTF("sector_len(0x%x) > sector_align_size(0x%x)\n",data_len,sector_size);	
		return NS_SECTOR_INVALID;
	}

	crc =  LONG_LIT(sector+8);

	crc_check = (UINT32)MG_Table_Driven_CRC(0xFFFFFFFF, sector+12, data_len-12);
	if (crc_check == crc)
	{
		//ALI_PRINTF("find valid sector data\n");
		return data_len;
	}
	else
	{
		ALI_PRINTF("sector crc check fail, crc_check [0x%08x], crc [0x%08x]\n", crc_check, crc);
		return NS_SECTOR_INVALID;		
	}					
	
}

int show_nand_parts ()
{
	int i;
#ifndef HAVE_PART_TBL_PARTITION
	PART_TBL *info = &PartitionInfo;
#else
	PART_TBL *info = p_partTbl;
#endif
	if (info->part_num > MAX_STBPART_NUM)
	{
              printf ("\nWrong partitionNum = 0x%lx.\n", info->part_num);
              return 1;
	}
	
	printf ("\n partitionNum = %ld\n", info->part_num);
	
	for (i=0; i<info->part_num; i++)
	{
		printf ("    part[%d]:  start=0x%08lx    size=0x%08lx    name=%s\n", 
			i, info->parts[i].start, info->parts[i].size, info->parts[i].name);		
	}
	printf ("\n");

	return 0;
}

extern int nand_read_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		       u_char *buffer);

static int NandPageRead(nand_info_t *nand, u16 Page, u_char *buf, size_t Len)
{
	//nand_info_t *nand;
	//nand = &nand_info[nand_curr_device];
	//struct nand_chip *this = mtd->priv;
	//int page = (int) (from >> this->page_shift);
	//int chip = (int) (from >> this->chip_shift);
	int r;
	struct nand_chip *this = nand->priv;
	//ulong off = nand->oobblock * Page;
	loff_t off = Page << this->page_shift;
#if 0
	if (nand_block_isbad(nand, off))
	{
		printf("Bad block (page %d).\n", Page);
		return -2;	//bad block
	}
#endif
	//printf("<NandPageRead>: call nand_read_skip_bad(): nand=0x%x, offset=0x%x, length=0x%x, buffer=0x%x\n", nand, off, (size_t )Len, buf);
	r = nand_read_skip_bad(nand, off, &Len, buf);
	if (r < 0) {
		printf("Error (%d) reading page %08x\n", r, Page);
		return -1;
	}

       return 0;
}

static int NandBlockRead(nand_info_t *nand,unsigned long PartStartOff, int BlockIdx, u_char *buf, size_t Len)
{
	int r;
	//struct nand_chip *this = nand->priv;
	loff_t off;
        PMI_t * GetPMI(void);

       if (NULL == GetPMI())
       {
		printf("Error in reading PMI on <%s>Line:%d.\n",__FUNCTION__,__LINE__);
		return -1;
       }

        off = PartStartOff + BlockIdx * nand->erasesize;
	//printf("<NandBlockRead>: nand=0x%x, PartStartOff=0x%lx, BlockIdx=0x%x, buf=0x%x,Len=%d,BlockSize=0x%x,off=0x%lx\n", nand, PartStartOff, BlockIdx, buf,Len,nand->erasesize,off);
	//printf("<NandBlockRead>: call nand_read_skip_bad(): nand=0x%x, offset=0x%x, buffer=0x%x, Len=0x%x\n", nand, off, buf, Len);
	r = nand_read_skip_bad(nand, off, &Len, buf);
	if (r < 0) {
		printf("Error (%d) reading BlockIdx %08x\n", r, BlockIdx);
		return -2;
	}

       return 0;
}

// nand_update_block will erase and re-write a block.
// BlockIdx start from 0, not 1.
static int nand_update_block(nand_info_t *nand,int PartIdx, int BlockIdx, u_char *buf)
{
    unsigned long part_offset;    
    unsigned long part_len;
    nand_erase_options_t nand_erase_options;
    UINT32 BlockSize = nand->erasesize;

    if (0 != Get_NandPartParmIdx(PartIdx,&part_offset,&part_len))
    {
        return -1;
    }

	memset(&nand_erase_options, 0, sizeof(nand_erase_options));
	nand_erase_options.length = BlockSize;
	nand_erase_options.offset = part_offset + BlockIdx*BlockSize;
	nand_erase_options.quiet = 1;
#if 0
	struct nand_chip *this = nand->priv;
	printf("<nand_update_block>: page_shift=0x%x, pagemask=0x%x, phys_erase_shift=0x%x, chip_shift=0x%x,chipsize=0x%x,subpagesize=0x%x\n", this->page_shift, this->pagemask,this->phys_erase_shift,this->chip_shift,this->chipsize,this->subpagesize);
#endif

	if (nand_erase_opts(nand, &nand_erase_options))
		return -2;

	if (nand_write_skip_bad(nand, nand_erase_options.offset, &BlockSize, buf, 0))
		return -3;

        putc('.');
        return 0;
}

int ChkPMI(UINT8* PMI_buf)
{
#if 0
        int e;

	/*compare PMI tag */
	/* 1.check ECC type  all 0:16 bit  all 1:24bit*/					
	if (ECC_16==chip->ecctype)
	{
		for (e=0;e<12;e++){
			if (PMI_buf[e]) {
				printf("ecc type error,expect Ecc16,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
				ret=1;
				break;
			}	
		}		
	}	
	else if (ECC_24==chip->ecctype)
	{
		for (e=0;e<12;e++){
			if (0xFF!=PMI_buf[e]) {
				printf("ecc type error,expect Ecc24,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
				ret=1;						
				break;
			}
		}	
	}
	else if(ECC_1==chip->ecctype)
	{
		if ((0 != PMI_buf[0]) || (0 != PMI_buf[1]) || (0 != PMI_buf[2]) || (0 != PMI_buf[3]) ||
				(0xFF != PMI_buf[4]) ||(0xFF != PMI_buf[5]) || (0xFF != PMI_buf[6]) || (0xFF != PMI_buf[7])){
			printf("ecc type error,expect Ecc1,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
			ret = 1;						
			break;								
		}
	}

	/*2.compare signature 0x3901-55AA*/
	if (((0x00==ret) &&(0x39==PMI_buf[12]) && (0x01==PMI_buf[13]) && (0x55==PMI_buf[14]) && (0xAA==PMI_buf[15]))||
		((0x00==ret) &&(0x36==PMI_buf[12]) && (0x03==PMI_buf[13]) && (0x55==PMI_buf[14]) && (0xAA==PMI_buf[15]))){

		//chip->PMIpos = PMIpos[i]*chip->bytes_perpage;

		//ALI_NAND_PRINTF("Get PMI at page #%d\n", PMIpos[i]);
		return 0;
	}
	else
	{
		if (!ret)
			printf("signature compare error,Ecc=%x,Page=%x, pos=%x\n",chip->ecctype, chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);							
	}
#endif
}

#ifdef HAVE_PART_TBL_PARTITION
static PART_TBL * GetPartTab(void);
#endif

#ifdef PMI_PART_SCT
PMI_t * GetPMI()
{
	int i;
	u16 pmi_index;
	nand_info_t *nand;
   size_t Len = MAX_PMI_SIZE;
   struct PMI * gPMI = &_gxPMI;

	
    memset(pmi_buf,0x0,MAX_PMI_SIZE);
	nand = &nand_info[nand_curr_device];

	for (pmi_index = 0; pmi_index < 4; pmi_index++)
	{
		if (0 == NandPageRead(nand,256 * pmi_index,(u_char *)pmi_buf,Len))
		{
			ALI_PRINTF("PMI read success on page %d\n", 256 * pmi_index);
			memcpy(gPMI, pmi_buf, sizeof(struct PMI));
			
		    eccsize = gPMI->Config.wBytesPerECCSec;
		    for(i =0 ;i <MAX_STBPART_NUM; i++)
		    {         
		         _gxPartations[i].offset = (*(gPMI->Partation_Buf+i*2))*eccsize;
		         _gxPartations[i].part_len = (*(gPMI->Partation_Buf+(i+1)*2))*eccsize;
		         _gxPartations[i].img_len = *(gPMI->Partation_Buf+i+MAX_STBPART_NUM*2);
		    }    
	    
	    	g_pmi_blk = pmi_buf ;
			return g_pmi_blk;
		}
	}
	return NULL;
}


#else

enum{
	ECC_16=0,
	ECC_24=1,	
	ECC_1=2,
	CRC_MODE=3,
	EF_MODE=4,
	BA_MODE=5,
	ECC_24_SCRB=6,
	LBA_PNP=0,
};

PMI_t * GetPMI(void)
{
	int i;
	PART_INFO *part;
	u16 pmi_index;
	nand_info_t *nand;
       size_t Len = MAX_PMI_SIZE;

       if (NULL != pPMI)
            return pPMI;
    
       memset(pmi_buf,0x0,MAX_PMI_SIZE);
	nand = &nand_info[nand_curr_device];

	for (pmi_index = 0; pmi_index < 4; pmi_index++)
	{
		if (0 == NandPageRead(nand,256 * pmi_index,(u_char *)pmi_buf,Len))
		{
			ALI_PRINTF("PMI read success on page %d\n", 256 * pmi_index);

			pPMI = (PMI_t *) pmi_buf;

#ifdef HAVE_PART_TBL_PARTITION
			p_PMIPartInfo = parttbl_buf;
			p_PMIPartInfo->part_num = *(UINT32 *) &pmi_buf[PartCntOff];
			//printf("part_num: pmi_buf[PartCntOff]=0x%08x,&pmi_buf[PartCntOff]=0x%08x\n", pmi_buf[PartCntOff],&pmi_buf[PartCntOff]);

			for (i=0; i<p_PMIPartInfo->part_num; i++)
			{
                		part = &(p_PMIPartInfo->parts[i]);
#else
			PartitionInfo.part_num = *(UINT32 *) &pmi_buf[PartCntOff];
			//printf("part_num: pmi_buf[PartCntOff]=0x%08x,&pmi_buf[PartCntOff]=0x%08x\n", pmi_buf[PartCntOff],&pmi_buf[PartCntOff]);

                	for (i=0; i<PartitionInfo.part_num; i++)
			{
                		part = &(PartitionInfo.parts[i]);
#endif
                		part->start = *(UINT32 *) &pmi_buf[PartOff + i * 8];	
                		part->size = *(UINT32 *) &pmi_buf[PartOff + i * 8 + 4];	
                		part->size &= 0x3FFFFFFF;			
                		if (ECC_1 == pPMI->Config.bECCType)
				{
                		    part->start = part->start <<9;
                		    part->size = part->size <<9;
				}else{
                		    part->start = part->start <<10;
                		    part->size = part->size <<10;
				}

				if ((pmi_buf[PartNameOff+i*16] >= 0x21) && (pmi_buf[PartNameOff+i*16] <= 0x7D))
				{
                		        memcpy(part->name, &pmi_buf[PartNameOff+i*16], 16);
                		        part->name[15] = 0;
				}
#ifdef HAVE_PART_TBL_PARTITION
				//Get the start offset and size of parttbl partition from PMI firstly. (Later partition(s) may be modified,so PMI's part info is not believable )
				if (strcmp((char *) part->name, PART_NAME_PARTTBL) == 0) 
				{
                                    offset_parttbl = part->start;
                                    len_parttbl = part->size;
				}
			}
                    //clear parttbl_buf to get part info from parttbl partition.
			memset(parttbl_buf,0x0,sizeof(parttbl_buf));
			if (NULL == GetPartTab())
			{
        		    ALI_PRINTF("Read Part Table partition failure\n");
#endif
			}

			return pPMI;
		}
	}

	return NULL;
}

#endif

int GetPartIdxByName(const char *PartName)
{
	UINT32 i;
       PART_TBL *info = NULL;

       if (NULL == GetPMI())
       {
		printf("Error in reading PMI on <%s>Line:%d.\n",__FUNCTION__,__LINE__);
		return 1;
       }
       
#ifdef HAVE_PART_TBL_PARTITION
       info = p_partTbl;
#else
       info = &PartitionInfo;
#endif
	
	for (i=0; i<info->part_num; i++)
	{
	        if (strcmp((char *) info->parts[i].name, PartName) == 0) 
	        {
                    return i;
	        }
	}

	return -1;
}

int Get_NandPartParmIdx (int PartIdx, unsigned long *start, unsigned long *size)
{
       PART_TBL *info = NULL;

       if (NULL == GetPMI())
       {
		printf("Error in reading PMI on <%s>Line:%d.\n",__FUNCTION__,__LINE__);
		return 1;
       }
       
#ifdef HAVE_PART_TBL_PARTITION
       info = p_partTbl;
#else
       info = &PartitionInfo;
#endif
	
        if (PartIdx >= 0 && PartIdx < info->part_num) 
        {
                *start = info->parts[PartIdx].start;
                *size = info->parts[PartIdx].size;
                return 0;
        }
        else {
                *start = 0;
                *size = 0;
        }

	return 2;
}

int nand_load_part_idx(const int PartIdx, u_char * LoadAddr)
{
        int r;
        unsigned long part_offset;    
        unsigned long part_len;
        nand_info_t *nand = &nand_info[nand_curr_device];
    
        if (0 != Get_NandPartParmIdx(PartIdx,&part_offset,&part_len))
        {
            return -1;
        }
	//memset(LoadAddr,0xFF,part_len);
	r = nand_read_skip_bad(nand, part_offset,(size_t *) &part_len,  LoadAddr);
	if (r) {
		ALI_PRINTF("** Read error\n");
		return -2;
	}
       return 0;
}

int nand_save_part_idx(const int PartIdx, u_char * DataAddr,unsigned long DataLen)
{
        int i;
        unsigned long part_offset;    
        unsigned long part_len;
        nand_info_t *nand = &nand_info[nand_curr_device];
        int blockCnt;
    
        if (0 != Get_NandPartParmIdx(PartIdx,&part_offset,&part_len))
        {
            return -1;
        }

        if (DataLen > part_len)
        {
    		ALI_PRINTF("Data length exceed the partition size (DataLen=0x%x,PartSize=0x%x) on <%s>Line:%d.\n",DataLen,part_len,__FUNCTION__,__LINE__);
              return -2;
        }
        blockCnt = DataLen/nand->erasesize;
        if (DataLen % nand->erasesize)
            blockCnt++;

	ALI_PRINTF("<%s>Line:%d  part_len:0x%x, DataLen=0x%x, blockCnt:%d,BlockSize=0x%x,writesize=0x%x,oobsize=0x%x\n", __FUNCTION__,__LINE__,part_len,DataLen,blockCnt,nand->erasesize,nand->writesize,nand->oobsize);
	for (i=0; i<blockCnt; i++)
	{
	      if (nand_update_block(nand,PartIdx,i,DataAddr+i*nand->erasesize))
            {
        		ALI_PRINTF("Error in update partion %d block :%x,BlockSize=0x%x,writesize=0x%x on <%s>Line:%d.\n",PartIdx,i,nand->erasesize,nand->writesize,__FUNCTION__,__LINE__);
                     return -3;
            }
	}

       return 0;
}

void StartAppInFlash(u8 LoadOnly)
{
        UINT8 *entry;
	 char s[100];;
        int part_idx = -1;

        part_idx = GetPartIdxByName(PART_NAME_SEE);
        if (-1 == part_idx)
        {
            printf("Partition '%s' seems not exist,please double check.\n",PART_NAME_SEE);
            return ;
        }

	printf("Load see Code...\n");
	entry = (void *)ADDR_LOAD_SEE;
	if (nand_load_part_idx(part_idx, entry))
		printf("Load see Code Fail...\n");

        part_idx = GetPartIdxByName(PART_NAME_MAIN);
        if (-1 == part_idx)
        {
            printf("Partition '%s' seems not exist,please double check.\n",PART_NAME_MAIN);
            return ;
        }

	printf("Load main Code...\n");
	entry = (void *)KERNEL_START_ADDR;
	//entry = (void *)info.startAddr;
	if (nand_load_part_idx(part_idx, entry))
		printf("Load main Code Fail...\n");

       if (1 != LoadOnly)
       {
	     printf("leave u-boot, Starting APP...\n");
            sprintf(s,"bootm 0x%08x",KERNEL_START_ADDR);
            run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
        }
	//exec(entry);
}	

static int load_valid_block(unsigned long part_offset,unsigned long part_len, UINT8 *buf, UINT32 *buf_len)
{
	//struct mtd_info_user info;
	UINT8 *block_buf;
	int i;
	int data_len, payload_len;
 
        nand_info_t *nand = &nand_info[nand_curr_device];

	/* malloc temp buf for block */
	block_buf = (UINT8 *)malloc(nand->erasesize + NAND_BUFFER_ADD_SIZE);
	if (!block_buf)
	{
		printf("malloc temp block(0x%x) fail\n",nand->erasesize);
		return -1;
	}

	//ALI_PRINTF("<%s>Line:%d  len:0x%x, blocknum:%d\n", __FUNCTION__,__LINE__,part_len,part_len/nand->erasesize);
	
	for (i=0; i<part_len/nand->erasesize; i++)
	{
		ALI_PRINTF("read block[%d]\n", i);
		
		/* read block[i] of part[part_idx] */
		if(NandBlockRead(nand,part_offset,i,block_buf,nand->erasesize) < 0)
		{
			printf("load block[%d] of part[stbid] fail\n",i);
			free(block_buf);
			return -1;		
		}

		/* check block data */
		data_len = _sector_crc_check(block_buf, nand->erasesize);		
		if(data_len > 0)
		{
			payload_len = data_len - 12;

			if (*buf_len < (UINT32)payload_len)		
			{
				payload_len = *buf_len;
			}
			*buf_len =  data_len - 12;			

			ALI_PRINTF("find valid block[%d], data_len:%d, payload_len:%d\n", i, data_len, payload_len);
			
			memcpy(buf, block_buf+12, payload_len);
			free(block_buf);
			return i;
		}
	}

	free(block_buf);
	
	return -1;
}

static int load_valid_stbid(UINT8 *buf, UINT32 *buf_len)
{
        int part_idx = -1;
        unsigned long part_offset;    
        unsigned long part_len;
    
        part_idx = GetPartIdxByName(PART_NAME_STBID);
        if (-1 == part_idx)
        {
            printf("Partition '%s' seems not exist,please double check.\n",PART_NAME_STBID);
            return -1;
        }
        if (0 != Get_NandPartParmIdx(part_idx,&part_offset,&part_len))
        {
            return -1;
        }

	return load_valid_block(part_offset,part_len,buf,buf_len);
}

STB_ID * GetStbID(void)
{
	UINT32 i;
       UINT32 len = sizeof(STB_ID);

       if (NULL != p_stbid)
            return p_stbid;
    

       i = load_valid_stbid(stbid_buf,&len);
       if (i>=0)
        {
                p_stbid = (STB_ID *) stbid_buf;
        }

       return p_stbid;
}

static int Ali_SetDefaultMacAddr()
{
#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

        setenv("ethaddr", MK_STR(CONFIG_ETHADDR));
        return 0;
}

int Ali_SetMacAddr()
{
#ifdef GET_ETHADDR_STBID
        u_char MacStr[20];
        u_char *pMac;

       //apply MAC address from stbid partition to enviroment variable "ethaddr"
        //if (p_stbid !=NULL && is_valid_ether_addr(p_stbid->mac))
        if (p_stbid !=NULL)
        {
            pMac = p_stbid->mac;
            sprintf(MacStr,"%02X:%02X:%02X:%02X:%02X:%02X", *pMac,*(pMac+1),*(pMac+2),*(pMac+3),*(pMac+4),*(pMac+5));
            setenv("ethaddr", MacStr);
        }
        else
#endif
        {
            Ali_SetDefaultMacAddr();
        }

        return 0;
}

int Ali_PMI_Init(void)
{
	MG_Setup_CRC_Table();
    if (NULL == GetPMI())
    {
        printf("Error in reading PMI on <%s>Line:%d.\n",__FUNCTION__,__LINE__);
        return 1;
    }
#ifndef PMI_PART_SCT
	GetStbID();
	Ali_SetMacAddr();
#endif	
	return 0;
}

static int update_stbid_part(UINT8 *buf, UINT32 buf_len)
{
	//struct mtd_info_user info;
	UINT8 *block_buf;
	int i;
        unsigned long part_offset;    
        unsigned long part_len;
        nand_info_t *nand = &nand_info[nand_curr_device];
        //int part_idx = PART_IDX_STBID;
        int part_idx = -1;
        int BlockCnt,ErrCnt = 0;

        part_idx = GetPartIdxByName(PART_NAME_STBID);
        if (-1 == part_idx)
        {
            printf("Partition '%s' seems not exist,please double check.\n",PART_NAME_STBID);
            return -5;
        }

        if (0 != Get_NandPartParmIdx(part_idx,&part_offset,&part_len))
        {
            return -1;
        }
 
       BlockCnt = part_len/nand->erasesize;
	/* malloc temp buf for block */
	block_buf = (UINT8 *)malloc(nand->erasesize + NAND_BUFFER_ADD_SIZE);
	if (!block_buf)
	{
		printf("malloc temp block(0x%x) fail\n",nand->erasesize);
		return -2;
	}

	ALI_PRINTF("<%s>Line:%d  len:0x%x, blocknum:%d\n", __FUNCTION__,__LINE__,part_len,BlockCnt);
	
	/* make new block */
	memset(block_buf,0xFF,nand->erasesize);

	/* sector[8~]: buf data */
	memcpy(block_buf+12, buf, buf_len);

	/* pack block data */
	if (_sector_crc_pack(block_buf, buf_len) < 0)
	{
		printf("_sector_crc_pack fail\n");
		free(block_buf);
		return -3;	
	}

	for (i=0; i<BlockCnt; i++)
	{
	      if (nand_update_block(nand,part_idx,i,block_buf))
            {
        		printf("Error in update partion %d block :%x on <%s>Line:%d.\n",part_idx,i,__FUNCTION__,__LINE__);
                     ErrCnt ++;
            }
	}

	free(block_buf);

       if (ErrCnt == BlockCnt)
          return -4;
       
	return 0;
}

int SetStbID(stbid_set_t type, const void *NewValue)
{
       UINT32 len = sizeof(STB_ID);
       u_char MacAddr[6];
       int ret = 0;

       switch (type)
        {
            case STBID_SET_SN:
                if (strlen((char *)NewValue) >0 && strlen((char *)NewValue) <= STB_HWINFO_SN_LEN)
                {
                        printf("Updating ");
                        memset(p_stbid->sn,0,STB_HWINFO_SN_LEN);
                        strncpy((char *)p_stbid->sn,(char *)NewValue,STB_HWINFO_SN_LEN);
                        ret = update_stbid_part(stbid_buf,len);
                        if (0 == ret)
                            printf("Done.\n");
                        else
                            printf("Error: 0x%x.\n",ret);
                }
                else
                {
                    printf("Error: new serial number(%s) is too long.\n",(char*) NewValue);
                }
                break;

            case STBID_SET_MAC:
                {
                        eth_parse_enetaddr(NewValue, MacAddr);
                        if (is_valid_ether_addr(MacAddr))
                        {
                            printf("Updating ");
                            memcpy(p_stbid->mac,MacAddr,STB_HWINFO_MAC_LEN);
                            ret = update_stbid_part(stbid_buf,len);
                            if (0 == ret)
                                printf("Done.\n");
                            else
                                printf("Error: 0x%x.\n",ret);
                        }
                        else
                        {
                            printf("Error: Invalid mac address (%s) .\n",(char*) NewValue);
                        }
                }
                break;

            default:
                break;
        }

        return ret;
}

#ifdef HAVE_PART_TBL_PARTITION
static int load_valid_part_table(UINT8 *buf, UINT32 *buf_len)
{
	return load_valid_block(offset_parttbl,len_parttbl,buf,buf_len);
}


static PART_TBL * GetPartTab(void)
{
	UINT32 i;
       UINT32 len = sizeof(PART_TBL);

       if (NULL != p_partTbl)
            return p_partTbl;
    

       i = load_valid_part_table(parttbl_buf,&len);
       if (i>=0)
       {
                p_partTbl = (PART_TBL *) parttbl_buf;
       }
       else
       {
                return NULL;
       }

       return p_partTbl;
}

/*
root format:
root=/dev/mtdblock8 ro rootfstype=cramfs
*/
int get_root(UINT8 **root)
{
	int ret=0;
	PART_TBL *part_table;
	
	part_table = GetPartTab();

	*root = part_table->root;

	return ret;
}

/*
mtdparts format:
mtdparts=ali_nand:2048k(miniboot),1024k(aliboot),1024k(parttbl),2048k(stbinfo),
10240k(loader1),10240k(loader2),5120k(kernel),4096k(see),4096k(rootfs),
49152K(app),10240K(userdata),1024k(stbid),1024k(bootlogo),4096k(cadata),
1024k(hdcpkey),10240k(rsv)		
*/
int get_mtdparts(UINT8 **mtdparts)
{
	int ret=0;
	PART_TBL *part_table;
	
	part_table = GetPartTab();

	*mtdparts = part_table->mtdparts;
	
	return ret;
}
#endif


#ifdef PMI_PART_SCT
/*
*@ret_real_buf	: The malloc buffer return value from Malloc . 
*@ret_buf		: Align buffer for DMA read buffer.ret_buf is the Align buffer from ret_real_buf.
*@retlen		: The actual read length by bytes.
*@part_id		: The partation id defined by ALi nand flash mapping
*@flag			: The flag for deciding with buffer output
			      if the flag = MEM_TO_DEC, the ret_buf shall be the malloc in the funciton
				  otherwise the ret_buf shall be the appointment buffer by the callee and the 
				  ret_real_buf is useless in this case.
*				  
*/
int nand_load_data(UINT8 *ret_real_buf, UINT8 **ret_buf, UINT32 * retlen, const UINT32 part_id, const UINT32 flag)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
    struct Partation * part = NULL;
	UINT32 bblen, bbchecklen;
	loff_t block_start;  
    int ret = -1;
    int p = 0;

    if (retlen == NULL)
    {
        printf("Error[%s, %d]: retlen is null\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (part_id > ALI_NAND_PART_MAX)
    {
        printf("Error[%s, %d]: part_id out of arrage, part_id is 0x%x, the max part is 0x%x \n", \
            __FUNCTION__, __LINE__, part_id, ALI_NAND_PART_MAX);
        goto exit;
    }

    part = (struct Partation * )malloc(sizeof(struct Partation));
    if (part == NULL)
    {
        printf("Error[%s, %d]: out of memory 0x%x \n", __FUNCTION__, __LINE__, sizeof(struct Partation));
        goto exit;
    }
    memset(part, 0x0, sizeof(struct Partation));

    switch(part_id)
    {
        case ALI_NAND_MAC:
            part->offset =  _gxPMI.Mac_Addr_Start; // MAC logic address
            part->img_len = MAC_ADDR_LEN;
            printf("[%s, %d]:Load MAC\n", __FUNCTION__, __LINE__);
          break;
        case ALI_NAND_LOGO:
            part->offset =  _gxPMI.Logo_Start;
            part->img_len = _gxPMI.Logo_Len;
            printf("[%s, %d]:Load Logo\n", __FUNCTION__, __LINE__);
          break;
        case ALI_NAND_STMACH:
            // (State Machine physic address) = (State Machine logic address) + (Bad block before State Machine)
            part->offset =  _gxPMI.stmach_addr; //State Machine logic address
            part->img_len = sizeof(struct state_machine_t);

            bbchecklen = part->offset - _gxPMI.Mac_Addr_Start;
            check_skip_len(nand, (loff_t)_gxPMI.Mac_Addr_Start, bbchecklen, &bblen);
            part->offset += bblen; //State Machine physic address
            printf("[%s, %d]:Load state machine, part->offset = 0x%x\n", __FUNCTION__, __LINE__, part->offset);
          break;
        case ALI_NAND_PART_0:
        case ALI_NAND_PART_1:
        case ALI_NAND_PART_2:
        case ALI_NAND_PART_3:
        case ALI_NAND_PART_4:
        case ALI_NAND_PART_5:
        case ALI_NAND_PART_6:
        case ALI_NAND_PART_7:
        case ALI_NAND_PART_8:
        case ALI_NAND_PART_9:
        case ALI_NAND_PART_10:
        case ALI_NAND_PART_11:
        case ALI_NAND_PART_12:
        case ALI_NAND_PART_13:
            p = min(part_id, MAX_STBPART_NUM);
            memcpy((UINT8 *)part, ((UINT8 *)&(_gxPartations[p])), sizeof(struct Partation));
          //  printf("[%s, %d]:Load Partition %d\n", __FUNCTION__, __LINE__, part_id);
          break;
        default:
            printf("[%s, %d]:don't support part_id(part_id = 0x%x)\n", __FUNCTION__, __LINE__, part_id);
            goto exit;
    }

    if (flag & MEM_TO_DEC)
    {
        ret_real_buf = (UINT8*)malloc(part->img_len + 0x100);
        if (!ret_real_buf)
        {        
            printf("Error[%s, %d]: out of memory 0x%x \n", __FUNCTION__, __LINE__, part->img_len);
            goto exit;
        }

        *ret_buf = (UINT8*)(((UINT32)ret_real_buf + 0x1f) & 0xfffffff0);
    }		

	ret = nand_read_skip_bad(nand, (loff_t)(part->offset), &(part->img_len), *ret_buf);
	if (ret < 0) {
		printf("Error (%d) reading offset %d\n", ret,part->offset);
		return -1;
	}

    *retlen = part->img_len;

exit:
    if (part != NULL)
    {
        free(part);
    }
    return ret;
}

int nand_save_data(const UINT8 *buf, const UINT32 len, UINT32 *retlen, const UINT32 part_id)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
    unsigned long offs;
    UINT32 bblen, bbchecklen;
    INT32 ret = -1;

    if (buf == NULL)
    {
        printf("Error[%s, %d]: buf is null\n", __FUNCTION__, __LINE__);
        return ret;
    }

    if (retlen == NULL)
    {
        printf("Error[%s, %d]: retlen is null\n", __FUNCTION__, __LINE__);
        return ret;
    }

    if (part_id > ALI_NAND_PART_MAX)
    {
        printf("Error[%s, %d]: part_id out of arrage, part_id is 0x%x, the max part is 0x%x \n", \
            __FUNCTION__, __LINE__, part_id, ALI_NAND_PART_MAX);
        return ret;
    }

    switch(part_id)
    {
        case ALI_NAND_STMACH:
            // (State Machine physic address) = (State Machine logic address) + (Bad block before State Machine)
            offs =  _gxPMI.stmach_addr;
            *retlen = len;           

            bbchecklen = offs - _gxPMI.Mac_Addr_Start;
            check_skip_len(nand, (loff_t)_gxPMI.Mac_Addr_Start, bbchecklen, &bblen);
            offs += bblen; // State Machine physic address
			printf("[%s, %d]:Save state machine, offs = 0x%x\n", __FUNCTION__, __LINE__, offs);
          break;
        default:
            printf("Error[%s, %d]:don't support part_id(part_id = 0x%x)\n", __FUNCTION__, __LINE__, part_id);
            return ret;
    }
    ret = nand_write_skip_bad(nand, (loff_t)offs, retlen, buf, 0);
    return ret;
}
#endif

#endif
