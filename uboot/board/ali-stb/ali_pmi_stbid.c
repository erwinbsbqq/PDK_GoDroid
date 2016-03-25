#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include "ali_pmi_stbid.h"
//#include "basic_types.h"
#include "ali_nand.h"

//===================================================
#define GET_DWORD(addr)                  (*(volatile UINT32 *)(addr))
#define SET_DWORD(addr, d)              (*(volatile UINT32 *)(addr)) = (d)
#define SETB_DWORD(addr, bit)          *(volatile UINT32 *)(addr) |= (bit)
#define SETB_BYTE(addr, bit)              *(volatile UINT8 *)(addr) |= (bit)
#define GET_BYTE(addr)                     (*(volatile UINT8 *)(addr))
#define SET_BYTE(addr, bit)                (*(volatile UINT8 *)(addr)) = (bit)

#define DRAM_SPLIT_CTRL_BASE 0xb8041000
#define PVT_S_ADDR 0x10
#define PVT_E_ADDR 0x14
#define SHR_S_ADDR 0x18
#define SHR_E_ADDR 0x1c

#define __MM_SHARED_MEM_LEN  	256
#define __MM_PRIVATE_SHARE_LEN	0x02000000	//32M
#define __MM_HIGHEST_ADDR  		0xa8000000		//128M
#define __MM_PRIVATE_LEN		(__MM_PRIVATE_SHARE_LEN-__MM_SHARED_MEM_LEN)
#define __MM_SHARE_BASE_ADDR 	(__MM_HIGHEST_ADDR - __MM_SHARED_MEM_LEN)
#define __MM_PRIVATE_TOP_ADDR 	(__MM_SHARE_BASE_ADDR)
#define __MM_PRIVATE_ADDR		(__MM_PRIVATE_TOP_ADDR-__MM_PRIVATE_LEN)//
#define __MM_HEAP_TOP_ADDR		__MM_PRIVATE_ADDR	//

UINT32 priv_mem_base_addr;
UINT32 priv_mem_len;
UINT32 share_mem_base_addr; 	// SHARED_MEM_BASE, __MM_SHARE_BASE_ADDR
UINT32 share_mem_len;			// SHARED_MEM_LEN, __MM_SHARED_MEM_LEN
//===================================================

#ifdef CONFIG_NAND_ALI

//#define ALI_DEBUG_ON
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
static u_char pmi_buf[MAX_PMI_SIZE + NAND_BUFFER_ADD_SIZE];
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

/********************************** Function define *********************************/
/* 0xbba1b5 = 1'1011'1011'1010'0001'1011'0101b
 <= x24+x23+x21+x20+x19+x17+x16+x15+x13+x8+x7+x5+x4+x2+1 */
#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_CCITT		0x00ddd0da
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24			0x00ad85dd
#endif

/* 0x04c11db7 = 1'0000'0100'1100'0001'0001'1101'1011'0111b
 <= x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 */
#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_CCITT		0x04c11db7
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32			0xedb88320
#endif

/*From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
static unsigned int MG_Compute_CRC(register unsigned int crc,
						register unsigned char *bufptr,
						register int len)
{
	register int i;

#ifdef MG_CRC_24_ARITHMETIC_CCITT
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)(*bufptr) << 16);
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x00800000)	/*Highest bit procedure*/
				crc = (crc << 1) ^ MG_CRC_24_CCITT;
			else
				crc <<= 1;
		}
	}
	return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_24_ARITHMETIC
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)*bufptr;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 1)				/*Lowest bit procedure*/
				crc = (crc >> 1) ^ MG_CRC_24;
			else
				crc >>= 1;
		}
	}
	return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)(*bufptr) << 24;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x80000000)	/*Highest bit procedure*/
				crc = (crc << 1) ^ MG_CRC_32_CCITT;
			else
				crc <<= 1;
		}
	}
	return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)*bufptr;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 1)				/*Lowest bit procedure*/
				crc = (crc >> 1) ^ MG_CRC_32;
			else
				crc >>= 1;
		}
	}
	return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif
}

/*Setup fast CRC compute table*/
static void MG_Setup_CRC_Table()
{
	register int count;
	unsigned char zero=0;

	for(count = 0; count <= 255; count++) /*Comput input data's CRC, from 0 to 255*/
#ifdef MG_CRC_24_ARITHMETIC_CCITT
		MG_CRC_Table[count] = MG_Compute_CRC(count << 16,&zero,1);
#endif

#ifdef MG_CRC_24_ARITHMETIC
		MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
		MG_CRC_Table[count] = MG_Compute_CRC(count << 24,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC
		MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif

}

/*Fast CRC compute*/
unsigned int MG_Table_Driven_CRC(register unsigned int crc,
			    register unsigned char *bufptr,
			    register int len)
{
	register int i;

	for(i = 0; i < len; i++)
#ifdef MG_CRC_24_ARITHMETIC_CCITT
		crc=(MG_CRC_Table[((crc >> 16) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
		crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
		crc=(MG_CRC_Table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0xffffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
		crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0xffffffff;
#endif
	return(crc);
}

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
static INT32 _sector_crc_pack(UINT8 *sector, UINT32 payload_len)
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
static INT32 _sector_crc_check(UINT8 *sector, UINT32 sector_size)
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
	if (info->part_num > MAX_PARTITION_NUM)
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
       u_int32_t BlockSize = nand->erasesize;

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

	if (nand_write_skip_bad(nand, nand_erase_options.offset, &BlockSize,buf,0))
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
				ALI_NAND_PRINTF("ecc type error,expect Ecc16,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
				ret=1;
				break;
			}	
		}		
	}	
	else if (ECC_24==chip->ecctype)
	{
		for (e=0;e<12;e++){
			if (0xFF!=PMI_buf[e]) {
				ALI_NAND_PRINTF("ecc type error,expect Ecc24,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
				ret=1;						
				break;
			}
		}	
	}
	else if(ECC_1==chip->ecctype)
	{
		if ((0 != PMI_buf[0]) || (0 != PMI_buf[1]) || (0 != PMI_buf[2]) || (0 != PMI_buf[3]) ||
				(0xFF != PMI_buf[4]) ||(0xFF != PMI_buf[5]) || (0xFF != PMI_buf[6]) || (0xFF != PMI_buf[7])){
			ALI_NAND_PRINTF("ecc type error,expect Ecc1,Page=%x, pos=%x\n",chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);
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
			ALI_NAND_PRINTF("signature compare error,Ecc=%x,Page=%x, pos=%x\n",chip->ecctype, chip->bytes_perpage,PMIpos[i]*chip->bytes_perpage);							
	}
#endif
}

#ifdef HAVE_PART_TBL_PARTITION
static PART_TBL * GetPartTab(void);
#endif

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
	entry = (void *)ADDR_LOAD_MAIN;
	//entry = (void *)info.startAddr;
	if (nand_load_part_idx(part_idx, entry))
		printf("Load main Code Fail...\n");

       if (1 != LoadOnly)
       {
	     printf("leave u-boot, Starting APP...\n");
            sprintf(s,"bootm 0x%08x",ADDR_LOAD_MAIN);
            run_command (s, 0); //call do_bootm_linux() to pass some enviroment variable from u-boot to Linux.
        }
	//exec(entry);
}	

#ifdef UBOOT_BOOT_MEDIA
void StartupBMediaSee()
{
	/* start see */
	printf("\n/********Start see....********/\n");
	mdelay(10);
	*((volatile UINT8 *)0xb8000220) &= 0xfd;
	*((volatile UINT8 *)0xb8000220) |= 0x2;
	*((unsigned long *)0xb8000200) = ADDR_LOAD_BMEDIA_SEE;//bl_priv_base; 

}

void Load_BootMedia()
{
	UINT8 *entry;
	int part_idx = -1;
#if 0
	priv_mem_base_addr = (UINT32)__MM_PRIVATE_ADDR;//=0xa5ffff00
	priv_mem_len = (UINT32)__MM_PRIVATE_LEN;
	share_mem_base_addr = (UINT32)__MM_SHARE_BASE_ADDR;//0xa7ffff00
	share_mem_len = (UINT32)__MM_SHARED_MEM_LEN;

	SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_S_ADDR, priv_mem_base_addr&0xfffffff);
	SET_DWORD(DRAM_SPLIT_CTRL_BASE+PVT_E_ADDR, (priv_mem_base_addr&0xfffffff)+priv_mem_len);    
	SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_S_ADDR, share_mem_base_addr&0xfffffff);    
	SET_DWORD(DRAM_SPLIT_CTRL_BASE+SHR_E_ADDR, (share_mem_base_addr&0xfffffff)+share_mem_len);

	printf("\n/******** *Private mem base= 0x%x, len = 0x%x********/\n", priv_mem_base_addr,priv_mem_len);
	printf("\n/******** *Share mem base= 0x%x, len = 0x%x********/\n\n", share_mem_base_addr,share_mem_len);
#endif

	part_idx = GetPartIdxByName(PART_NAME_BM);

	entry = (void *)ADDR_LOAD_MEDIA;
	printf("Load BootMedia to 0x%x from partition:%d...\n",entry,part_idx);
	if (nand_load_part_idx(part_idx, entry))
	{
		printf("Load BootMedia File Fail...\n");
	}

	/*Load see code*/
	part_idx = GetPartIdxByName(PART_NAME_SEE_BM);
	entry = (void *)ADDR_LOAD_BMEDIA_SEE;
	printf("Load see_bmedia.abs to 0x%x from partition:%d...\n",entry,part_idx);
	if (nand_load_part_idx(part_idx, entry))
		printf("Load see Code Fail...\n");


}
#endif

static INT32 load_valid_block(unsigned long part_offset,unsigned long part_len, UINT8 *buf, UINT32 *buf_len)
{
	//struct mtd_info_user info;
	UINT8 *block_buf;
	int i;
	INT32 data_len, payload_len;
 
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
       GetStbID();
       Ali_SetMacAddr();
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
INT32 get_root(UINT8 **root)
{
	INT32 ret=0;
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
INT32 get_mtdparts(UINT8 **mtdparts)
{
	INT32 ret=0;
	PART_TBL *part_table;
	
	part_table = GetPartTab();

	*mtdparts = part_table->mtdparts;
	
	return ret;
}
#endif

#endif

