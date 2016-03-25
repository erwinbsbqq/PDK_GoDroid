#ifndef __HLD_STONAND_H__
#define __HLD_STONAND_H__

#include <sys_config.h>
#include <types.h>

//#define MEMGETPMI         1
#define GETMTDBASICINFO   2


#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_PARTITION_NUM 31
int nand_open(UINT8 index, int flags);
int nand_len(UINT8 index);
int nand_check(UINT8 index,UINT32 start, UINT32 length);
int nand_erase(UINT8 index,UINT32 start, UINT32 length);
int nand_erase_block(UINT8 index, INT32 block_no);
int nand_read(UINT8 index, void* buf, UINT32 size);
int nand_lseek(UINT8 index, int offset, int whenence);
int nand_write(UINT8 index, void* buf, UINT32 size);
int nand_physical_read(UINT8 index, void* buf, UINT32 start,UINT32 size);
int nand_ioctl(UINT8 index, INT32 cmd, UINT32 param);
int nand_close(UINT8 index);

#ifdef __cplusplus
}
#endif
#endif/* __HLD_STONAND_H__ */
