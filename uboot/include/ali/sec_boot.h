#ifndef _SECURE_BOOT_H_
#define _SECURE_BOOT_H_


/*API for bootloader*/
void set_see_code_info( UINT32 code_addr, UINT32 len, UINT32 run_addr);
void set_see_sig_info(UINT32 sig_location, UINT32 sig_len);
RET_CODE set_see_key_info( UINT32 key_pos,UINT32 flag); /*flag = 1 ,using uboot pk ,
														  flag = 0, using system pk	*/


/* API
*  Bootloader trig see_bl to verify see_sw
*/
RET_CODE bl_verify_SW_see(UINT32 param);

/* API
*  Bootloader trig see_bl to jump to run see_sw
*/
RET_CODE bl_run_SW_see();

#define SEE_ENTRY  0xa6000200

#define _DECRYPT_SEE	0x1
#define _UZIP_SEE		0x2	
#define _VERSION_SEE	0x3	
#define _SIG_SEE		0x4

/*For TDS SW*/
#define CASE1_SEE	((_DECRYPT_SEE <<28) | (_UZIP_SEE << 24)| \
					 (_VERSION_SEE <<20) | (_SIG_SEE <<16 ))	

/*For Linux SW*/				
#define CASE2_SEE	((_DECRYPT_SEE <<28) | (0xf<< 24)| \
					 (_VERSION_SEE <<20) | (_SIG_SEE <<16 ))	
/*For USEE*/
#define CASE3_SEE	((0xf <<28) | (0xf<< 24)| \
					 (0xf <<20) | (_SIG_SEE <<16 ))				
			
#endif 
