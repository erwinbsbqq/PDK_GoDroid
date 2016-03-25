/*******************************************************

 File Name: ali_m36_ip.h

 Description: Header File for bandwidth test

 Copyright: 

*******************************************************/

#ifndef __ALI_M36_IP_H__
#define __ALI_M36_IP_H__

/* Includes ----------------------------------------- */

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
**** Variables Definition ****
***************************************************************/
#if defined(CONFIG_ALI_CHIP_M3921)
uint8_t IP_NAME[34][15] = 
{
	{"MAIN_CPU"},
    {"ALI_CPU"},
    {"DE_FXDE0"},
    {"DE_FXDE1"},
    {"AUDIO_CPU0"},
    {"AUDIO_CPU1"},
    {"AUDIO"},
    {"PPV"},
    {"DEMUX1"},
    {"TSGEN"},
    {"Nand Flash"},
    {"VDEC Long"},
    {"USB0"},
    {"VENC Long"},
    {"BDMA"},
    {"VDEC_CTRL"},
    {"VDEC SHORT"},
    {"MALI400"},
    {"MAC"},
	{"DESCRAMBLE"},
	{"USB1"},
	{"VENC_Short"},
    {"DVBC PHY1"},
    {"MAC2"},
    {"DEMUX2"},
    {"DE_FXDE3"},
	{"VENC_CTRL"},
	{"SGDMA1"},
    {"SGDMA2"},
    {"DE_FXDE2"},
    {"GE_GC"},
    {"DEMUX3"},
    {"USB2"},   
    {"SDIO"}   
};
#else
__u8 IP_NAME[24][15] = 
{
	{"Main CPU"},
    {"ALI CPU"},
    {"DE Video"},
    {"DE Graphic"},
    {"VE Long"},
    {"VE Short"},
    {"Audio"},
    {"SFlash"},
    {"Demux1"},
    {"TsGen"},
    {"Nand Flash"},
    {"VIN Deo1 Sut"},
    {"USB2"},
    {"Descramble"},
    {"BDMA"},
    {"SGDMA1"},
    {"GE"},
    {"Demux2"},
    {"TOE"},
    {"DVBC PHY1"},
    {"USB1"},
    {"Deo0 Vin1"},
    {"SGDMA2"},
    {"DVBC PHY2"}   
};
#endif

enum ALI_M36_IP_BW_STATUS
{
    ALI_M36_IP_BW_STATUS_IDLE,
    ALI_M36_IP_BW_STATUS_RUN
};


enum IP_BW_MON_STATUS
{
	IP_BW_MON_IDLE = 0,		//IP Monitor is idle
    IP_BW_MON_START,		//Start IP BW Monitor
    IP_BW_MON_STOP,			//Stop IP BW Monitor
    IP_BW_MON_PAUSE,		//Pause IP BW Monitor
    IP_BW_MON_RESUME,		//Resume IP BW Monitor
};

struct ali_m36_ip_bw_data_node
{
    struct list_head link;
    /* Counted in 8-bytes unit, remainder bytes will be dropped. */
    uint32_t payload_len;
    int8_t *payload;
    uint32_t payload_phy_addr;
};


struct ali_m36_ip_bw_dev
{
    int8_t name[16];

    dev_t dev_id;

    enum ALI_M36_IP_BW_STATUS status;

    /* Linux char device for DM Ctrl. */
    struct cdev cdev;

    /* Protect againest concurrent device access by user space.
     * (Race condition for mulitple userland process accessing same file).
     */
    struct mutex io_mutex; 

    /* Protect againest concurrent data list access.
     * (data producer from userland and data cosumer from kernel task).
     */
    struct mutex data_wr_mutex; 

	/* Used to create the task */
    struct workqueue_struct *xfer_workq;
    struct work_struct xfer_work;

    /*1, start to calculate the IP BW;
    * 0, end to calculate the IP BW  */
    uint8_t  ip_bw_total_flag;

    uint8_t  ip_bw_single_flag;

    /*bit0~bit33 respond to IP0~IP33 */
    uint32_t  mon_ip_idx;
    uint32_t  mon_ip_idx_add;

	/*the work status of IP BW task;Refer enum variable of IP_BW_MON_STATUS*/
    uint8_t  ip_test_flag; 

};

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ALI_M36_IP_H__ */




