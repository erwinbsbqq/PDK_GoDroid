/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.h
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
* 	2.  Dec.19.2003		 Justin Wu		 Ver 0.2    Add ESC CMD macros.
*	3.  Sep.23.2005		 Justin Wu		 Ver 0.3    Add pan information.
*****************************************************************************/

#ifndef __ADR_HLD_PAN_H__
#define __ADR_HLD_PAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! @addtogroup pan
 *  @{
 */
#define PAN_KEY_INVALID				0xFFFFFFFF

/* Key value of Front keypad. */
#define PAN_KEY_01				0xFFFF0001
#define PAN_KEY_02				0xFFFF0002
#define PAN_KEY_03				0xFFFF0004
#define PAN_KEY_04				0xFFFF0008
#define PAN_KEY_05				0xFFFF0010
#define PAN_KEY_06				0xFFFF0020
#define PAN_KEY_07				0xFFFF0040
#define PAN_KEY_08				0xFFFF0080

//multi-panel key pressed
#define PAN_COMPOUND_KEY			0x1A2B3C4D

/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define PAN_ESC_CMD_LBD			'L'		//!<LBD operate command
#define PAN_ESC_CMD_LBD_FUNCA	0		//!<Extend function LBD A 
#define PAN_ESC_CMD_LBD_FUNCB	1		//!<Extend function LBD B 
#define PAN_ESC_CMD_LBD_FUNCC	2		//!<Extend function LBD C 
#define PAN_ESC_CMD_LBD_FUNCD	3		//!<Extend function LBD D 
#define PAN_ESC_CMD_LBD_LEVEL	5			//!<Level status LBD, no used

#define PAN_ESC_CMD_LED		'E'		//!<LED operate command
#define PAN_ESC_CMD_LED_LEVEL	0		//!<Level status LED

#define	PAN_ESC_CMD_LBD_ON	1		//!<Set LBD to turn on status 
#define	PAN_ESC_CMD_LBD_OFF	0		//!<Set LBD to turn off status 

#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
#define PAN_ESC_CMD_STANDBY_LED			'P'		//!<LED operate command 
#endif

#ifndef __ASSEMBLER__

#include <hld/pan/adr_pan_dev.h>

/*!@struct pan_gpio_info
   @brief ��������ǰ��尴��GPIO����Ϣ��
*/
struct pan_gpio_info			/* Total 2 byte */
{
	UINT16	polar	: 1;		//!<Polarity of GPIO, 0 or 1 active(light) 
	UINT16	io		: 1;	//!<HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h 
	UINT16	position: 14;		//!<GPIO index, upto over 64 GPIO 
} __attribute__((packed));

/*!@struct pan_hw_info
   @brief ��������ǰ���Ӳ����Ϣ��
*/
#if 0
struct pan_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panal device type */
	UINT8					type_kb	: 2;	//!<Key board (array) type 
	UINT8					type_scan:1;	///!<0: slot scan, 1: shadow scan 
	UINT8					type_key: 1;	//!<Key exit or not 
	UINT8					type_irp: 3;	//!<0: not IRP, 1: NEC, 2: LAB 
	UINT8					type_mcu: 1;	//!<MCU exit or not 
	/* Offset 1: GPIO number */
	UINT8					num_com	: 4;	//!<Number of COM PIN, 0: no com; <= 8 
	UINT8					pos_colon:4;	//!<Position of colon flag, 0 to 7, 8 no colon 
    	/* Offset 2: */
	UINT8					num_scan: 2;	//!<Number of scan PIN, 0: no scan; <= 2 
    	UINT8					rsvd_bits:6;	//!<in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"
	/* Offset 3: Panel shift control */
	struct pan_gpio_info	flatch;			//!<Shifter latch PIN 
	struct pan_gpio_info	fclock;			//!<Shifter clock PIN 
	struct pan_gpio_info	fdata;			//!<Shifter data PIN 
	/* Offset 6: Panel scan control */
	struct pan_gpio_info	scan[2];		//!<Panel scan PIN 
	/* Offset 8: Panel com PIN */
	struct pan_gpio_info	com[8];			//!<COM PIN 
	/* Offset 16: Panel LBD control */
	struct pan_gpio_info	lbd[4];			//!<LBD GPIO PIN 
	/* Offset 20: Panel input attribute */
	UINT32	intv_repeat_first;				//!<Repeat interval first in mS 
    	/* 24 */
	UINT32	intv_repeat;					//!<Repeat interval in mS 
    	/* 28 */
	UINT32	intv_release;					//!<Release interval in mS 
    	/* 32 */
	UINT32	(*hook_scan)(struct pan_device *dev, UINT32 key);
    	/* 36 */
	UINT32	(*hook_show)(struct pan_device *dev, char *data, UINT32 len);
    	/* 40 */
} __attribute__((packed));
#else
struct pan_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panel device type */
	UINT8					type_kb	: 2;	//!<Key board (array) type 
	UINT8					type_scan:1;	//!<0: slot scan, 1: shadow scan 
	UINT8					type_key: 1;	//!<Key exit or not 
	UINT8					type_irp: 3;	//!<0: not IRP, 1: NEC, 2: LAB 
	UINT8					type_mcu: 1;	//!<MCU exit or not 
	/* Offset 1: GPIO number */
	UINT8					num_com : 4;	//!<Number of COM PIN, 0: no com; <= 8 
	UINT8					pos_colon:4;	//!<Position of colon flag, 0 to 7, 8 no colon 
	/* Offset 2: */
	UINT8					num_scan: 2;	//!<Number of scan PIN, 0: no scan; <= 2 
	UINT8					rsvd_bits:6;	//!<in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"
	/* Offset 3: */
	UINT8 					rsvd_byte;	//!<Reserved for alignment 
	/* Offset 4: Panel shift latch */
	struct pan_gpio_info	flatch;			//!<Shifter latch PIN 
	struct pan_gpio_info	fclock;			//!<Shifter clock PIN 
	struct pan_gpio_info	fdata;			//!<Shifter data PIN 
	/* Offset 10: Panel scan control */
	struct pan_gpio_info	scan[2];		//!<Panel scan PIN 
	/* Offset 14: Panel com PIN */
	struct pan_gpio_info	com[8];			//!<COM PIN 
	/* Offset 30: Panel LBD control */
	struct pan_gpio_info	lbd[4];			//!<LBD GPIO PIN 
	struct pan_gpio_info	rsvd_hw;		//!<Reserved for alignment 
	
	/* Offset 40: Panel input attribute */
	UINT32	intv_repeat_first;			//!<Repeat interval first in mS 
    	/* 44 */
	UINT32	intv_repeat;				//!<Repeat interval in mS 
    	/* 48 */
	UINT32	intv_release;				//!<Release interval in mS 
    	/* 52 */
	//UINT32	(*hook_scan)(struct pan_device *dev, UINT32 key);
    	/* 56 */
	//UINT32	(*hook_show)(struct pan_device *dev, char *data, UINT32 len);
    	/* 60 */
} __attribute__((packed));
#endif

/*!@struct led_bitmap
   @brief LED bitmap��
*/
#ifdef PAN_HT_16315_COMPATIBLE
struct led_bitmap
{
	UINT8 character;
	UINT16 bitmap;
};
#else
struct led_bitmap
{
	UINT8 character;
	UINT8 bitmap;
};
#endif

/*!@struct led_bitmap32
   @brief LED bitmap��
*/
struct led_bitmap32
{
	UINT8 character;
	UINT32 bitmap;
};

/*!@struct pan_configuration
   @brief Panel driver configuration��
*/
struct pan_configuration
{
	struct pan_hw_info *hw_info;			//!<Panel GPIO information 
	int    bitmap_len;				//!<Bitmap list length 
	struct led_bitmap  *bitmap_list;		//!<Bitmap list 
};

/*!@struct pan_configuration32
   @brief Panel driver configuration��
*/
struct pan_configuration32
{
	struct pan_hw_info *hw_info;			//!<Panel GPIO information 
	int    bitmap_len;				//!<Bitmap list length 
	struct led_bitmap32  *bitmap_list;		//!<Bitmap list 
};


/*!
@brief �ҽ�����豸��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_attach(void);

/*!
@brief ж������豸��
@param[in] dev ��ж�ص�����豸�ڵ㡣
*/
void pan_detach(struct pan_device *dev);

INT32 pan_ch455_attach(struct pan_configuration *config);

/*!
@brief ��ǰ����豸��
@param[in] dev ��򿪵�ǰ����豸�ڵ㡣
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_open(struct pan_device *dev);

/*!
@brief �ر�ǰ����豸��
@param[in] dev ��رյ�ǰ����豸�ڵ㡣
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_close(struct pan_device *dev);

/*!
@brief ��ʾ����豸��
@param[in] dev ����ʾ������豸�ڵ㡣
@param[in] data ����ʾ��Ϣ�ĵ�ַ��
@��data[0] == 27ʱ����ESC command��
@data[1] PAN_ESC_CMD_LBD��
@data[2] PAN_ESC_CMD_LBD_POWER������power LED״̬; PAN_ESC_CMD_LBD_LOCK������LOCK LED״̬��
@data[3] = PAN_ESC_CMD_LBD_ON������LED��PAN_ESC_CMD_LBD_OFF���ر�LED��
@param[in] len ����ʾ��Ϣ�ĳ��ȡ�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_display(struct pan_device *dev, char *data, UINT32 len);

/*!
@brief �������豸��ǰ��ʾ����Ϣ��
@param[in] dev ������Ϣ������豸�ڵ㡣
@param[out] data ������ʾ��Ϣ�ĵ�ַ��
@param[out] len ������ʾ��Ϣ�ĳ��ȡ�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_get_display(struct pan_device *dev, char *data, UINT32 * len);


/*!
@brief ��� ģ���io contorl ������
@param[in] dev ָ�����ģ�� ��ָ�롣
@param[in] cmd ���� ���������͡��ο�pan_device_ioctrl_command���塣
@param[in,out] param �����Ĳ��������ݲ�ͬ��������в�ͬ�Ĳ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  IO�������:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>

  <tr align="center">
    <td>PAN_DRIVER_ATTACH</td>    
    <td>struct pan_hw_info *</td>    
    <td>����</td>
    <td>Front panel driver attach</td>
  </tr>
*/
INT32 pan_io_control(struct pan_device *dev, INT32 cmd, UINT32 param);

/*!
@brief ���ǰ����豸�İ�����Ϣ��
@param[in] dev ���ð�����Ϣ��ǰ����豸�ڵ㡣
@param[in] timeout ���հ�����Ϣ�ĳ�ʱ������
@return  struct pan_key *��
@retval  struct pan_key *      �ɹ���
@retval  NULL  ʧ�ܡ�
*/
struct pan_key * pan_get_key(struct pan_device *dev, UINT32 timeout);

/*!
@brief ���ǰ����豸�İ�����Ϣ��
@param[in] dev ���ð�����Ϣ��ǰ����豸�ڵ㡣
@param[in] timeout ���հ�����Ϣ�ĳ�ʱ������
@return  struct pan_key_info *��
@retval  struct pan_key_info *      �ɹ���
@retval  NULL  ʧ�ܡ�
*/
struct pan_key_info *  pan_get_key_info(struct pan_device *dev, UINT32 timeout);

/*!
@brief ������豸�������ݡ�
@param[in] dev �跢�����ݵ�ǰ����豸�ڵ㡣
@param[in] data �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȡ�
@param[in] timeout �������ݵĳ�ʱ������
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_send_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);

/*!
@brief ������豸�������ݡ�
@param[in] dev ��������ݵ�����豸�ڵ㡣
@param[out] data �������ݵĵ�ַ��
@param[in] len �������ݵĳ��ȡ�
@param[in] timeout �������ݵĳ�ʱ������
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_receive_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);

/*!
@brief ���ǰ����豸����������Ϣ��
@param[in] dev ���������������Ϣ��ǰ����豸�ڵ㡣
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_buff_clear(struct pan_device *dev);

/*!
@brief ����ǰ����豸����repeat���ԡ�
@param[in] dev �����ð���repeat���Ե�ǰ����豸�ڵ㡣
@param[in] enable_repeat 0����ֹrepeat��1��ʹ��repeat��
*/
void pan_buff_set_repeat(struct pan_device *dev, UINT8 enable_repeat);

/*!
@brief ���ǰ����豸����repeat���ԡ�
@param[in] dev ���ð���repeat���Ե�ǰ����豸�ڵ㡣
@return UINT8  ����repeat���ԡ�
@retval  0     ��ֹrepeat��
@retval  1     ʹ��repeat��
*/
UINT8 pan_buff_get_repeat(struct pan_device *dev);

/*!δʵ��
*/
INT32 pan_buff_queue_tail(struct pan_key *key);

/*!
@brief ���ú����豸������Ϣ���͵�ʱ������
@param[in] dev �����ð�����Ϣʱ������ǰ����豸�ڵ㡣
@param[in] delay ���û���ס�����豸ĳ����������ʱ��Linux inputϵͳ���͵ڶ��ΰ�����Ϣ��ʱ������
@param[in] interval ���û���ס�����豸ĳ����������ʱ��Linux inputϵͳ���͵����μ����Ժ󰴼���Ϣ��ʱ������
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_config_ir_rep_interval(struct pan_device *dev, UINT32 delay, UINT32 interval);

/*!
@brief ��������豸������Ϣ���͵�ʱ������
@param[in] dev �����ð�����Ϣʱ������ǰ����豸�ڵ㡣
@param[in] delay ���û���ס���ĳ����������ʱ��Linux inputϵͳ���͵ڶ��ΰ�����Ϣ��ʱ������
@param[in] interval ���û���ס���ĳ����������ʱ��Linux inputϵͳ���͵����μ����Ժ󰴼���Ϣ��ʱ������
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_config_pan_rep_interval(struct pan_device *dev, UINT32 delay, UINT32 interval);

/*!
@brief ���ú����豸���������ֵ���߼���ֵ��ӳ���
@param[in] dev �����ð��������ֵ���߼���ֵ��ӳ����ǰ����豸�ڵ㡣
@param[in] phy_code 	0: read ���ص�codeΪlinux �߼���ֵ��1: read ���ص� code Ϊ�����ֵ��ӳ����е�����ֵ��2: read ���ص�codeΪ�����ֵ��
@param[in] map ӳ������ʼ��ַ��
@param[in] map_len ӳ���ĳ��ȡ�
@param[in] unit_len ӳ�����byteΪ��λ�ĳ��ȡ�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_config_key_map(struct pan_device *dev, UINT8 phy_code, UINT8 *map, UINT32 map_len, UINT32 unit_len);

/*!
@brief ɾ�������豸���������ֵ���߼���ֵ��ӳ���
@param[in] dev �����ð��������ֵ���߼���ֵ��ӳ����ǰ����豸�ڵ㡣
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_del_key_map(struct pan_device *dev);

/*!
@brief ��������豸���������ֵ���߼���ֵ��ӳ���
@param[in] dev �����ð��������ֵ���߼���ֵ��ӳ����ǰ����豸�ڵ㡣
@param[in] phy_code 	0: read ���ص�codeΪlinux �߼���ֵ��1: read ���ص� code Ϊ�����ֵ��ӳ����е�����ֵ��2: read ���ص�codeΪ�����ֵ��
@param[in] map ӳ������ʼ��ַ��
@param[in] map_len ӳ���ĳ��ȡ�
@param[in] unit_len ӳ�����byteΪ��λ�ĳ��ȡ�
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_config_panel_map(struct pan_device *dev, UINT8 phy_code, UINT8 *map, UINT32 map_len, UINT32 unit_len);

/*!
@brief ���ú����豸��Ҫʶ��ĺ�������ʽ��
@param[in] dev �����ú�������ʽ��ǰ����豸�ڵ㡣
@param[in] format ��Ҫʶ��ĺ�������ʽ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_config_ir_format(struct pan_device *dev, UINT32 format);

/*!
@brief �������豸�����֡�
@param[out] dev_name ��������豸���ֵĵ�ַ��
@return INT32��
@retval  0       �ɹ���
@retval  ��0  ʧ�ܡ�
*/
INT32 pan_get_panel_name(INT8 *dev_name);

#endif

/*!
 @}
 */
#ifdef __cplusplus
}
#endif
#endif /* __HLD_PAN_H__ */
