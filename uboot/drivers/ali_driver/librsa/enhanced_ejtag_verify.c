//#include <api/libc/string.h>
//#include <bus/otp/otp.h>
#include <ali/enhanced_ejtag_verify.h>
#include <ali/types.h>
#include <ali/retcode.h>

#ifdef EJTAG_PRINTF
#undef EJTAG_PRINTF
#endif
#define EJTAG_PRINTF(...)     do{}while(0)
//#define EJTAG_PRINTF     libc_printf

static void ejtag_dump_data(UINT8 *data, UINT32 len)
{
	UINT32 i;
	for(i=0; i<len; i++)
	{
		EJTAG_PRINTF("0x%02x, ", *(data+i));
		if((i+1)%16==0)
			EJTAG_PRINTF("\n");
	}
	if(i%16!=0)
		EJTAG_PRINTF("\n");
}

int fetch_jtag_enhanced_pub_key(UINT8 *pubkey,UINT16 wlen)
{
	#define EJTAG_ENHANCED_PUBKEY_OFFSET	0x700
	UINT32 bl_offset=0;
	UINT16 i=0;
	UINT8 ret=FALSE;
	
	for(i=0;i<4;i++)
	{
		bl_offset+=*(UINT8*)(0xAFC00000+8+i);
		if(i<3)
			bl_offset<<=8;
	}	
	//bl_offset=*(UINT32*)(0xAFC00000+8);	
	EJTAG_PRINTF("bl_size: %0x\n", bl_offset); 
	if(bl_offset>EJTAG_ENHANCED_PUBKEY_OFFSET)
	{
		bl_offset-=EJTAG_ENHANCED_PUBKEY_OFFSET;
		EJTAG_PRINTF("bl_offset move to: %0x\n", bl_offset);
		for(i=0;i<wlen;i++)
			pubkey[i]=*(UINT8*)(0xAFC00000+bl_offset+i);	
		ejtag_dump_data(pubkey, wlen);		
		ret=TRUE;	
	}	
	return ret;
}

int decrypt_ejtag(UINT8* Dest,UINT32* Destlen,UINT8* Src,UINT32 Srclen)
{
	#define BUF_SIZE	256
	#define RSA_PUB_KEY_SIZE	516	// fetch from ABS file
	UINT8 ejtag_rsa_pubkey[RSA_PUB_KEY_SIZE];
	UINT8 temp_buf[4];	
	UINT8 chip_id_MSB[2];

	UINT8 EKDS_E[BUF_SIZE];
	UINT8 EKDS_E_ID_MAC[BUF_SIZE];
	UINT8 EKDS_E_ID[BUF_SIZE];	
	
	UINT32	buf_len=0;
	int ret=FALSE,val=0;
	
	EJTAG_PRINTF("decrypt_ejtag start\n"); 

	if(Srclen!=BUF_SIZE*2)
		EJTAG_PRINTF("Invalid input len\n"); 	
	*Destlen=BUF_SIZE;
	MEMSET(Dest,0xFF,BUF_SIZE);
	
// 1. read [EKDS_E],[EKDS_CHIPID_MAC] from smart card
	//EJTAG_PRINTF("Step1\n"); 
	MEMCPY(EKDS_E,Src,BUF_SIZE);
	MEMCPY(EKDS_E_ID_MAC,Src+BUF_SIZE,BUF_SIZE);
	EJTAG_PRINTF("EKDS_E from LibRsa:\n"); 
	ejtag_dump_data(EKDS_E,16);
	EJTAG_PRINTF("EKDS_E_ID_MAC from LibRsa:\n"); 
	ejtag_dump_data(EKDS_E_ID_MAC,16);

// 2. read RSA Public key from Code //move to lib
	//EJTAG_PRINTF("Step2\n"); 
	val=fetch_jtag_enhanced_pub_key(ejtag_rsa_pubkey,RSA_PUB_KEY_SIZE);	
	EJTAG_PRINTF("fetch_jtagkey_enhanced_pub_key() return:%x\n", val); 
	EJTAG_PRINTF("---ejtag_rsa_pubkey---\n");		
	ejtag_dump_data(ejtag_rsa_pubkey,16);
// 3. read chip ID MSB (2B) //move to lib
	//EJTAG_PRINTF("Step3\n"); 
	otp_read( 0x01*4, &temp_buf, 4);
	//EJTAG_PRINTF("OTP[0x01]:0x %x %x %x %x\n", temp_buf[0],temp_buf[1],temp_buf[2],temp_buf[3]); 
	chip_id_MSB[0]=temp_buf[1];
	chip_id_MSB[1]=temp_buf[0];
	EJTAG_PRINTF("ChipId_MSB:0x %x %x\n", chip_id_MSB[0],chip_id_MSB[1]); 

// 4. decrypt [EKDS_CHIPID_MAC] to get [EKDS_CHIPID]
	//EJTAG_PRINTF("Step4\n"); 
	val= rsapublicfunc(EKDS_E_ID, &buf_len, EKDS_E_ID_MAC, BUF_SIZE, ejtag_rsa_pubkey);
	if(val!=0)
		EJTAG_PRINTF("rsapublicfunc() failed!\n");		
	else
	{
		EJTAG_PRINTF("rsapublicfunc() success(out len:0x%x)!\n",buf_len);		
		EJTAG_PRINTF("---EKDS_E_ID---\n");			
		ejtag_dump_data(EKDS_E_ID, buf_len);		
	}
// 5. compare //move to lib
	val=MEMCMP(EKDS_E_ID+4,chip_id_MSB,2);
	#ifdef REDEMO_PATCH
	val=0; //force skip Chip ID, due to redemo.exe
	#endif
	if(val!=0)
	{
		EJTAG_PRINTF("Chip ID compare mismatch!\n");		
	}	
	else
	{
		EJTAG_PRINTF("Chip ID compare Pass!\n");	
		val=MEMCMP(EKDS_E_ID+6,EKDS_E+6,250);		
		#ifdef REDEMO_PATCH
		val=MEMCMP(EKDS_E_ID+12,EKDS_E+12,244);	//compare last 244bytes only, due to redemo.exe
		#endif
		if(val!=0)
		{
			EJTAG_PRINTF("Data compare mismatch!\n");		
		}	
		else
		{
			EJTAG_PRINTF("Data compare Pass!\n");	
			ret=TRUE;
		}		
	}
	if(TRUE==ret)
	{		
		MEMCPY(Dest,EKDS_E,BUF_SIZE);
	} 	
	return ret;
}
