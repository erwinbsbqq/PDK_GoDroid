#include <common.h>
#include <nand.h>
#include "fastCRC.h"

/*
******sector struct******
	________________________
	|	data_len(4B)			|
	|-----------------------|
	|	data_len_invert(4B)	|
	|-----------------------|
	|	CRC(4B)				|
	|-----------------------| 
	|						|			
	|	payload_data			|
	|	(data_len-12 bytes)	|			
	|-----------------------| 
	|	reserve data			|				
	|_______________________|

	valid sector:
	1. data_len + data_len_invert = 0xFFFFFFFF
	2. CRC = payload data crc
*/
#define NS_ERROR	printf	
#define NS_NOTICE	printf
#define NS_DEBUG	printf
#define NS_DUMP

#define LONG_LIT(addr)		(((addr)[0])|((addr)[1]<<8)|((addr)[2]<<16)|((addr)[3]<<24))
#define LONG_BIT(addr)		(((addr)[0]<<24)|((addr)[1]<<16)|((addr)[2]<<8)|((addr)[3]))

#define NS_BLOCK_FREE -1
#define NS_BLOCK_INVALID -2

#define NS_SECTOR_FREE -1
#define NS_SECTOR_INVALID -2

#define NS_FIND_FREE_SECTOR_SUCCESS 0
#define NS_ERASE_CUR_BLOCK -1
#define NS_ERASE_NEW_BLOCK -2


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
static int _sector_crc_pack(unsigned char *sector, unsigned int payload_len)
{
	unsigned int crc;
	unsigned int data_len,data_len_invert;
	
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
	crc = MG_Table_Driven_CRC(0xFFFFFFFF, (sector+12), payload_len);	
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
int _sector_crc_check(unsigned char *sector, unsigned int sector_size)
{
	unsigned int crc, crc_check;
	unsigned int data_len, data_len_invert;
	unsigned int i;

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
		NS_NOTICE("sector_len:0x%x, sector_len_invert: 0x%x, not match\n",data_len,data_len_invert);
		return NS_SECTOR_INVALID;
	}

	if (data_len > sector_size)
	{
		NS_NOTICE("sector_len(0x%x) > sector_align_size(0x%x)\n",data_len,sector_size);	
		return NS_SECTOR_INVALID;
	}

	crc =  LONG_LIT(sector+8);

	crc_check = (unsigned int)MG_Table_Driven_CRC(0xFFFFFFFF, sector+12, data_len-12);
	if (crc_check == crc)
	{
		//NS_DEBUG("find valid sector data\n");
		return data_len;
	}
	else
	{
		NS_NOTICE("sector crc check fail, crc_check [0x%08x], crc [0x%08x]\n", crc_check, crc);
		return NS_SECTOR_INVALID;		
	}					
	
}
