#ifndef	__ADR_HLD_IRX_H__
#define	__ADR_HLD_IRX_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! @addtogroup pan
 *  @{
 */
 
/*! @enum IR_CODE_FORMAT
    @brief  ��������ʽ��
*/
enum IR_CODE_FORMAT{
	IR_NEC_FORMAT = 0,	//!<NEC��
	IR_LAB_FORMAT		//!<LAB��
};

/*! @enum IR_PULSE_POLAR
    @brief  �������弫�ԡ�
*/
enum IR_PULSE_POLAR{
	IR_PULSE_STANDARD = 0,
	IR_PULSE_INVERTED
};

/*! @enum IR_CONNECT_MODE
    @brief  ��������ģʽ��
*/
enum IR_CONNECT_MODE{
	IR_CONNECT_BY_CABLE = 0,
	IR_CONNECT_BY_LED	
};

/*!@struct IRX_CONFIG_PARAM
   @brief �������ò�����
*/
struct IRX_CONFIG_PARAM{
	enum IR_CODE_FORMAT format;
	enum IR_PULSE_POLAR polar;
	enum IR_CONNECT_MODE connect;
	UINT32 reserved2;
};

/*!
 @}
 */
#ifdef __cplusplus
	}
#endif

#endif/*__HLD_IRX_H__*/

