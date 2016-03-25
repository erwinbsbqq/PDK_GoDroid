/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    usb.h
*
*    Description:    This file contains interface functions declare
*		             of USB driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jul.10.2007      Victor chen      Ver 0.1    Create file.
*****************************************************************************/

#ifndef __USB_NEW_H__
#define __USB_NEW_H__

#include <basic_types.h>
//#include <bus/usb2/usb_dev.h>

//#define USB_DEBUG_PRINTF printf
//#define USB_DEBUG_PRINTF soc_printf
#define USB_DEBUG_PRINTF(...)	do{}while(0)

#define USB_COMMAND_TIMEOUT  3
struct  usb_dev_info_ex
{
	UINT16 bcdUSB;
	UINT8  bDeviceClass;
	UINT8  bDeviceSubClass;
	UINT8  bDeviceProtocol;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT16 bcdDevice;
};

struct  usb_disk_info_ex
{
	UINT16 bcdUSB;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT8  iSerialNumber;
	UINT8 subclass;
	UINT8 protocol;
	UINT8 max_lun;
	UINT32 phy_lbn0[16]; 
	UINT32 disk_size[16];
	UINT32 blk_size[16];
	UINT32 speed;	//real speed
	UINT32 which_port;
    UINT32 host_id;
};

struct usb_debug_info
{
    struct usb_disk_info_ex disk;
	UINT32 read_retry_counter;
	UINT32 write_retry_counter;
	UINT32 stall_err;
	UINT32 rsp_err;
	UINT32 tog_err;
	UINT32 pid_err;
	UINT32 utmi_err;
	UINT32 crc_err;
	UINT32 unexpct_err;
    UINT32 usb_speed;
};

typedef void* USBD_NODE_ID, * pUSBD_NODE_ID;

#if 1
/**************************************************************************************
 usbd_attach -  To attach host and mass-storage device driver.

 Parms:
    void(*notify)(UINT32): A callback for the upper layer. It will inform upper layer 
    					   the usb device is connected to or removed from the port. 
    void(*mount)(UINT32): A callback for the upper layer to mount device.
    void(*umount)(UINT32): A callback for the upper layer to umount device.
    
 Returns: 
    RET_SUCCESS = 0	: successful.
    !RET_SUCCESS	: failure.
 ***************************************************************************************/
RET_CODE usbd_attach_ex(void(*notify)(UINT32), void(*mount)(UINT32), void(*umount)(UINT32));


/***************************************************************************************
 usbd_device_ready - To check whether the device is ready for accessing. 
 					 That is means the device already have mounted by OS, 
 					 and can accessing to the device.

 Parms:
    lun:	The index number of the device.
    
 Returns: 
    RET_SUCCESS		: Device is ready for accessing.
    !RET_SUCCESS	: Device is not ready for accessing.
 ***************************************************************************************/
RET_CODE usbd_device_ready_ex(USBD_NODE_ID NodeId,UINT32 lun);

/**************************************************************************************
 usbd_device_get_disk_count -  To get a count of the device that connected on the port.

 Parms:
	struct usb_disk_info *disk_info: pointer to point usb disk info structure.
	
 Returns: 
     RET_SUCCESS	: successful;
    !RET_SUCCESS	: failure;
 ***************************************************************************************/
INT32  usbd_get_disk_info_ex(USBD_NODE_ID NodeId,struct usb_disk_info_ex *disk_info);

/**************************************************************************************
 usbd_device_get_disk_count -  To get a count of the device that connected on the port.

 Parms:
	none:
	
 Returns: 
    unsigned long value.
 ***************************************************************************************/
UINT32 usbd_device_get_disk_count_ex(USBD_NODE_ID NodeId);


/**************************************************************************************
 usbd_device_get_disk_image -  To return a bitmap of mounted device 
							(the index number of device is pointed by position of bitmap).
 Parms:
	none:
	
 Returns: 
    unsigned long value.
 ***************************************************************************************/
UINT32 usbd_device_get_disk_image_ex(USBD_NODE_ID NodeId);


/***************************************************************************************
 usbd_get_disk_size - To get the disk size of the usb device.

 Parms:
    lun:	The index number of the device.
    
 Returns: 
    	0:				The usb device not work or not exist.
	other value:		size of disk.
 ***************************************************************************************/
INT32 usbd_get_disk_size_ex(USBD_NODE_ID NodeId,UINT8 lun);


/**************************************************************************************
 usbd_device_read -  To read data form usb mass-storage device.
 
 Parms:
	sub_lun:			Index number of device.
	lbn:				local block number (Sector number), point to the beginning.
	buffer:				A pointer to point a buffer where will save the data that read from 
						usb mass-storage device.
	sector_count:		Total sector count that will be read.
	
 Returns: 
    RET_SUCCESS = 0	: successful.
    !RET_SUCCESS	: failure.
 ***************************************************************************************/
INT32 usbd_device_read_ex(USBD_NODE_ID NodeId,UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);


/**************************************************************************************
 usbd_device_write -  To write data to usb mass-storage device.
 
 Parms:
	sub_lun:			Index number of device.
	lbn:				local block number (Sector number), point to the beginning.
	buffer:				A pointer to point a buffer where store the source data that will be 
						write to usb mass-storage device.
	sector_count:		Total sector count that will be write.
	
 Returns: 
    RET_SUCCESS = 0	: successful.
    !RET_SUCCESS	: failure.
 ***************************************************************************************/
INT32 usbd_device_write_ex(USBD_NODE_ID NodeId, UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);
#endif


#endif/*__USB_H__*/

