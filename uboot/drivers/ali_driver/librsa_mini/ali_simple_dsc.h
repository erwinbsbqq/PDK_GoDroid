#ifndef _ALI_SIMPLE_DSC_
#define _ALI_SIMPLE_DSC_
#include "basic_types.h"
//#include <sys_define.h>

#define DSC_READ_WORD(offset)  (*(volatile UINT32 *)(offset))  
#define DSC_WRITE_WORD(offset,val)   (*(volatile UINT32 *)(offset) = (val))
//#define UNCHACE(x) (((UINT32)x) | (0xa << 28))
//#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))   
//#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))   
//#define BYTESWAP(x) ((ROTR((x), 8) & 0xff00ff00L) | (ROTL((x), 8) & 0x00ff00ffL)) 
RET_CODE create_sha_ramdrv(UINT8 *input, UINT32 length, UINT32 *output);

#endif
