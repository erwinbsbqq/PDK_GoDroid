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

#ifndef __USB_H__
#define __USB_H__

//#ifdef USB_SUPPORT_HUB
//#include <bus/usb2/usb.h>
//#else

#include <basic_types.h>



//#define USB_DEBUG_PRINTF printf
#define USB_DEBUG_PRINTF(...)

struct  usb_dev_info
{
	UINT16 bcdUSB;
	UINT8  bDeviceClass;
	UINT8  bDeviceSubClass;
	UINT8  bDeviceProtocol;
	UINT16 idVendor;
	UINT16 idProduct;
	UINT16 bcdDevice;
};

struct  usb_disk_info
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
};


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
RET_CODE usbd_attach(void(*notify)(UINT32), void(*mount)(UINT32), void(*umount)(UINT32));


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
RET_CODE usbd_device_ready(UINT32 lun);

/**************************************************************************************
 usbd_device_get_disk_count -  To get a count of the device that connected on the port.

 Parms:
	struct usb_disk_info *disk_info: pointer to point usb disk info structure.
	
 Returns: 
     RET_SUCCESS	: successful;
    !RET_SUCCESS	: failure;
 ***************************************************************************************/
INT32  usbd_get_disk_info(struct usb_disk_info *disk_info);

/**************************************************************************************
 usbd_device_get_disk_count -  To get a count of the device that connected on the port.

 Parms:
	none:
	
 Returns: 
    unsigned long value.
 ***************************************************************************************/
UINT32 usbd_device_get_disk_count (void);

/**************************************************************************************
for usb if test
vid:0x1a0a
PID_test_se0 0x0101
PID_test_J 0x0102
PID_test_K 0x0103
PID_test_packet 0x0104
PID_test_suspend 0x0106
PID_test_get_desc 0x0107
PID_test_get_desc_data  0x0108
 ***************************************************************************************/
void usb_if_test_enter(void);
void usb_if_test_exit(void);
void usb_if_test_start(UINT32 vid,UINT32 pid);
void usb_if_test_stop(void);
void test_bus_enumeration(void);

/**************************************************************************************
 usbd_device_get_disk_image -  To return a bitmap of mounted device 
							(the index number of device is pointed by position of bitmap).
 Parms:
	none:
	
 Returns: 
    unsigned long value.
 ***************************************************************************************/
UINT32 usbd_device_get_disk_image(void);


/***************************************************************************************
 usbd_get_disk_size - To get the disk size of the usb device.

 Parms:
    lun:	The index number of the device.
    
 Returns: 
    	0:				The usb device not work or not exist.
	other value:		size of disk.
 ***************************************************************************************/
INT32 usbd_get_disk_size(UINT8 lun);


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
INT32 usbd_device_read(UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);


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
INT32 usbd_device_write(UINT8 sub_lun, UINT32 lbn, UINT8* buffer, UINT32 sector_count);
//#endif
//#endif

#endif/*__USB_H__*/

