/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    adr_rtc.h
*
*    Description:    This file contains sto_device structure define in HLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.20.2012      xuhua huang       Ver 0.1    Create file.
*****************************************************************************/

#ifndef __ADR_HLD_RTC_H__
#define __ADR_HLD_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <adr_basic_types.h>

#define RTC_STATE_DETACH   0
#define RTC_STATE_ATTACH   1

#define RTC_CMD_BASE				0x00
#define RTC_CMD_SET_TIME			(RTC_CMD_BASE	+ 1)//!<����RTC ʱ��
#define RTC_CMD_GET_TIME			(RTC_CMD_BASE	+ 2)//!<��ȡRTC ʱ��
#define RTC_CMD_SET_ALARM		(RTC_CMD_BASE	+ 3)//!<����ALARM ʱ��
#define RTC_CMD_GET_ALARM		(RTC_CMD_BASE	+ 4)//!<��ȡALARM ʱ��
#define RTC_CMD_AIE_OFF			(RTC_CMD_BASE	+ 5)//!<�ر�ALARM 
#define RTC_CMD_AIE_ON				(RTC_CMD_BASE	+ 6)//!<����ALARM 

#define PMU_CMD_BASE				0x00
#define PMU_CMD_SET_TIME					(PMU_CMD_BASE	+ 1)//!<����standby��ǰʱ��
#define PMU_CMD_SET_WAKETIME			(PMU_CMD_BASE	+ 2)//!<����wakeup ʱ��
#define PMU_CMD_ENTER_STANDBY		(PMU_CMD_BASE	+ 3)//!<����standbyģʽ
#define PMU_CMD_SET_POWER_KEY		(PMU_CMD_BASE	+ 4)//!<����powerkey��ֵ

#define POWER_KEY_NUM			8
struct rtc_time_hld
{
    UINT16  year;        /**< ��*/
    UINT8  month;       /**< ��*/
    UINT8  date;        /**< ��*/
    UINT8  hour;        /**< ʱ*/
    UINT8  minute;      /**< ��*/
    UINT8  second;      /**< ��*/
};

struct rtc_wkalrm_hld 
{
	struct rtc_time_hld time;	/**<Alarm���õ�ʱ�� */
	UINT8 enabled;	/**< 0 = alarm disabled, 1 = alarm enabled */
	UINT8 pending; /**< 0 = alarm not pending, 1 = alarm pending */
};

struct rtc_device
{
	struct rtc_device  *next; //!< �ڲ�ʹ�á�
	UINT32 type; //!< �ڲ�ʹ�á�
	INT8 name[16]; //!< �ڲ�ʹ�á�
	UINT16		flags;				//!<Interface flags, status and ability 
	INT32 handle;
};

struct pmu_device
{
	struct pmu_device  *next; //!< �ڲ�ʹ�á�
	UINT32 type; //!< �ڲ�ʹ�á�
	INT8 name[16]; //!< �ڲ�ʹ�á�
	UINT16		flags;				//!<Interface flags, status and ability 
	INT32 handle;//!< �ڲ�ʹ�á�
	struct rtc_time_hld time;//!< �ڲ�ʹ�á�
	UINT32 duration;//!< �ڲ�ʹ�á�
};

/*!
@brief ��ʼ��rtc ģ�顣
*/
RET_CODE rtc_attach(void);

/*!
@brief �ͷ�rtc ģ�顣
*/
RET_CODE rtc_dettach(struct rtc_device *dev);

/*!
@brief ��rtc ģ�顣
@param[in] dev ָ��rtcģ�� ��ָ�롣
@param[in] init ��ʼ��������
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE rtc_open(struct rtc_device * dev);

/*!
@brief �ر�rtc  ģ�顣
@param[in] dev ָ��rtc ģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE rtc_close(struct rtc_device * dev);

/*!
@brief rtc ģ���io contorl ������
@param[in] dev ָ��rtc ģ�� ��ָ�롣
@param[in] dwCmd ���� ���������͡��ο�RTC_CMD_XX���塣
@param[in,out] dwParam �����Ĳ��������ݲ�ͬ��������в�ͬ�Ĳ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  IO����dwCmd ����:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>

  <tr align="center">
    <td>RTC_CMD_SET_TIME</td>    
    <td>struct  rtc_time_hld *</td>    
    <td>����</td>
    <td>����ʱ��</td>
  </tr>

  <tr align="center">
    <td>RTC_CMD_GET_TIME</td>    
    <td>struct rtc_time_hld *</td>    
    <td>���</td>
    <td>���ʱ��</td>
  </tr>

  <tr align="center">
    <td>RTC_CMD_SET_ALARM</td>    
    <td>struct rtc_wkalrm_hld *</td>    
    <td>����</td>
    <td>��������ʱ��</td>
  </tr>  
  
  <tr align="center">
    <td>RTC_CMD_GET_ALARM</td>    
    <td>struct  rtc_wkalrm_hld *</td>    
    <td>���</td>
    <td>��ȡ����ʱ��</td>
  </tr>  

  <tr align="center">
    <td>RTC_CMD_AIE_OFF</td>    
    <td>UINT32</td>    
    <td>����</td>
    <td>�ر�����</td>
  </tr>  

  <tr align="center">
    <td>RTC_CMD_AIE_ON</td>    
    <td>UINT32</td>    
    <td>����</td>
    <td>��������</td>
  </tr>  
*/
RET_CODE rtc_io_control(struct rtc_device * dev, UINT32 dwCmd, UINT32 dwParam);

/*!
@brief ��ʼ��pmu ģ�顣
*/
RET_CODE pmu_attach(void);

/*!
@brief �ͷ�pmu ģ�顣
*/
RET_CODE pmu_dettach(struct pmu_device *dev);

/*!
@brief ��pmu ģ�顣
@param[in] dev ָ��pmuģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE pmu_open(struct pmu_device *dev);

/*!
@brief �ر�pmu  ģ�顣
@param[in] dev ָ��pmu ģ�� ��ָ�롣
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
*/
RET_CODE pmu_close(struct pmu_device * dev);

/*!
@brief pmu ģ���io contorl ������
@param[in] dev ָ��pmu ģ�� ��ָ�롣
@param[in] dwCmd ���� ���������͡��ο�PMU_CMD_XX���塣
@param[in,out] dwParam �����Ĳ��������ݲ�ͬ��������в�ͬ�Ĳ�����
@return RET_CODE��
@retval  RET_SUCCESS       ���� �ɹ���
@retval  !RET_SUCCESS     ���� ʧ�ܣ����������״̬����
@note  IO����dwCmd ����:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">����</th>
    <th width="200">����</th>
    <th width="80">��������</th>
    <th width="320">��ע</th>
  </tr>
  <tr align="center">
    <td>PMU_CMD_SET_TIME</td>    
    <td>struct rtc_time_hld *</td>    
    <td>����</td>
    <td>�������ϵͳ����standby��ʱ��</td>
  </tr>
  <tr align="center">
    <td>PMU_CMD_SET_WAKETIME</td>    
    <td>UINT32</td>    
    <td>����</td>
    <td>����ϵͳ��������ʱ��</td>
  </tr>

  <tr align="center">
    <td>PMU_CMD_ENTER_STANDBY</td>    
    <td>UINT32</td>    
    <td>����</td>
    <td>����ϵͳ�������</td>
  </tr>  
  
  <tr align="center">
    <td>PMU_CMD_SET_POWER_KEY</td>    
    <td>UINT32 *</td>    
    <td>����</td>
    <td>����ϵͳpower��ֵ,����Ĳ���Ϊ���飬������POWER_KEY_NUM����ֵ</td>
  </tr>  

*/
RET_CODE pmu_io_control(struct pmu_device * dev, UINT32 dwCmd, UINT32 dwParam);
#ifdef __cplusplus
}
#endif


#endif  /* __ADR_HLD_RTC_H__ */

