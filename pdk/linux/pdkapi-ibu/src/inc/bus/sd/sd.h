#ifndef __HLD_SD_H__
#define __HLD_SD_H__
#include <sys_config.h>
#include <bus/sd/sd_dev.h>

/*the command for ioctl*/
#define SD_GET_CAPACITY	1
#define SD_GET_BLOCK_SIZE	2
#define SD_GET_TRAN_SPEED	3
#define SD_SET_HOSE_MODE	4
#define SD_SET_BLOCK_SIZE	5
#define SD_GET_BUS_WIDTH	6
#define SD_SET_BUS_WIDTH	7
#define SD_CHECK_INITED     8
#define SD_GET_ID           9
#define SD_CHECK_CONNECTED  10

#define HOST_MODE_PIO	1
#define HOST_MODE_DMA	2

#define BUS_WIDTH_1BIT 0
#define BUS_WIDTH_4BIT 2

struct sd_device *g_sd_dev;

RET_CODE sd_m33_read(UINT32 sub_lun,UINT32 sd_block_addr,UINT32 *buffer, UINT32 block_num);
RET_CODE sd_m33_write(UINT32 sub_lun, UINT32 sd_block_addr,UINT32 *buffer, UINT32 block_num);
RET_CODE sd_m33_ioctl(UINT32 cmd, UINT32 param);
RET_CODE sd_m33_open(struct sd_device*dev);
RET_CODE sd_m33_close(struct sd_device*dev);
void sd_m33_set_gpio_id(UINT32 int_gpio_id,UINT32 protect_gpio_id);
void sd_m33_set_card_insert_polarity(UINT32 level);
void sd_m33_attach(void(*notify)(UINT32), void(*mount)(UINT32), void(*umount)(UINT32));
void sd_m33_detach(struct sd_device*dev);

#define sd_read(sub_lun,sd_block_addr,buffer,block_num)	sd_m33_read(sub_lun,sd_block_addr,buffer,block_num) 
#define sd_write(sub_lun,sd_block_addr,buffer,block_num)	sd_m33_write(sub_lun,sd_block_addr,buffer,block_num)
#define sd_ioctl(cmd,param)					sd_m33_ioctl(cmd,param)
#define sd_set_gpio_id(int_gpio_id,protect_gpio_id)		sd_m33_set_gpio_id(int_gpio_id,protect_gpio_id)
#define sd_open(dev)					sd_m33_open(dev)
#define sd_close(dev)					sd_m33_close(dev)
#define sd_detach(dev)				sd_m33_detach(dev) 
#define sd_close(dev)					sd_m33_close(dev)

#endif /*__HLD_SD_H__*/

