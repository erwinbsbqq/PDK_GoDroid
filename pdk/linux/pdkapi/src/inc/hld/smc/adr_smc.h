#ifndef __ADR_HLD_SMC_H__
#define __ADR_HLD_SMC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <hld/adr_hld_dev.h>
#include <ali_smc_common.h>


/*! @addtogroup 设备驱动
 *  @{
 */
 
/*! @addtogroup 智能卡
 *  @{
 */


/*! @enum smc_device_ioctrl_command
    @brief 智能卡 驱动IO 命令 集。
*/
enum smc_device_ioctrl_command
{
	SMC_DRIVER_SET_IO_ONOFF = 0,	//!<未实现。	
	SMC_DRIVER_SET_ETU,				//!<设定工作etu。
	SMC_DRIVER_SET_WWT,				//!<设定首字节等待超时，单位毫秒。			
	SMC_DRIVER_SET_GUARDTIME,		//!<未实现。
	SMC_DRIVER_SET_BAUDRATE,		//!<未实现。
	SMC_DRIVER_CHECK_STATUS,		//!<检测卡的状态。
	SMC_DRIVER_CLKCHG_SPECIAL,		//!<未实现。
	SMC_DRIVER_FORCE_SETTING,		//!<未实现。
	SMC_DRIVER_SET_CWT,				//!<设定字节等待超时。
	SMC_DRIVER_GET_F,				//!<获得ATR F参数值，用于计算新ETU 。
	SMC_DRIVER_GET_D,				//!<获得ATR D参数值，用于计算新ETU 。
	SMC_DRIVER_GET_ATR_RESULT,		//!<智能卡ATR 状态。
	SMC_DRIVER_GET_ATR,				//!<智能卡ATR 
	SMC_DRIVER_GET_HB,				//!<获取ATR  History Bytes 。
	SMC_DRIVER_GET_PROTOCOL,		//!<获取智能卡当前工作协议。
	SMC_DRIVER_SET_WCLK,			//!<重新设定智能卡工作频率，新频率在reset 智能卡后生效。
	SMC_DRIVER_SEND_PPS,			//!<发送PPS 命令。
	SMC_DRIVER_SET_PROTOCOL,		//!<设置智能卡当前工作协议，未实现。
	SMC_DRIVER_SET_OPEN_DRAIN,		//!<设置智能卡是否支持open drain, bit0 : en_power_open_drain; bit1 : en_clk_open_drain; bit2 : en_data_open_drain; bit3 : en_rst_open_drain.
	SMC_DRIVER_SET_DEBUG_LEVEL,		//!<设置智能卡调试level
	SMC_DRIVER_GET_CLASS,			//!<获得当前选择的操作类别
	SMC_DRIVER_SET_CLASS			//!<设置操作类别
};

/*! @enum smc_device_status
    @brief 智能卡设备状态。
*/
typedef enum smc_device_status
{
	SMC_DEV_ATTACH = 0,
	SMC_DEV_OPEN,
	SMC_DEV_CLOSE,
	SMC_DEV_DETACH	
}smc_device_status_e;


/*!@struct smc_gpio_info
   @brief 智能卡驱动模块gpio信息。
 */
struct smc_gpio_info		
{
	UINT8	polar;		//!<使能智能卡的gpio极性
	UINT8	io;			//!<设置gpio的输入输出
	UINT8	position;	//!<gpio索引
};

/*!@struct smc_hw_info
   @brief 智能卡驱动模块硬件信息。
 */
struct smc_hw_info
{
	UINT8 io_type;				//!<0 uart io; 1 iso_7816 io. 目前只能选择0
	UINT8 uart_id;				//!<SCI_FOR_RS232 0, SCI_FOR_MDM 1
	UINT8 gpio_control;			//!<1 gpio 控制
	UINT8 ext_clock;			//!<0 内部时钟; 1 外部时钟
	UINT8 shared_uart;			//!<0 ; 1 rs232和uart_switch gpio复用
	UINT8 uart_with_appinit : 1;
	UINT8 reserved : 7;
	
	struct smc_gpio_info prest;
	struct smc_gpio_info power;
	struct smc_gpio_info reset;
	struct smc_gpio_info uart_switch;	
	struct smc_gpio_info clock_switch;
	UINT32 clock;						//!<外部时钟
	UINT32 clock_ext;					//!<外部时钟 2
	UINT32 to_for_atr;					//!<atr超时时间，单位为ms
	UINT32 to_for_cmd;					//!<命令超时时间，单位为ms
};


/*!@struct smc_device
   @brief 智能卡驱动模块设备节点。
 */
struct smc_device
{
	struct hld_device *next;			//!<下一个设备节点 
	UINT32 type;						//!<接口硬件类型
	INT8 name[HLD_MAX_NAME_SIZE];		//!<设备名称

	UINT16 flags;						//!<不再使用
	UINT32 base_addr;					//!<设备基地址 
	UINT8  irq;							//!<中断号
										//!<硬件私有结构Hardware privative structure 
	void *priv;							//!<私有数据指针
 
    UINT8 blockable;					//!<块是能标志
	void	(*callback)(UINT32 param);
};


/*!
@brief 挂接智能卡设备 。
@param[in] dev_id 需挂接的智能卡设备号，0 或1。
@param[in] config_param 智能卡初始化配置结构体指针。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_dev_attach(int dev_id, struct smc_dev_cfg * config_param);

/*!
@brief 卸载智能卡设备 。
@param[in] dev_id 需挂接的智能卡设备号，0 或1。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_dev_dettach(int dev_id);

/*!
@brief 卸载智能卡设备 。
@param[in] dev_id 需挂接的智能卡设备号，0 或1。
@param[in] use_default_cfg 是否使用驱动默认的配置参数。
@param[in] cfg 新的配置参数。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_attach(int dev_id, int use_default_cfg,
                 struct smc_dev_cfg *cfg);

/*!
@brief 打开智能卡设备 。
@param[in] dev 需打开的智能卡设备节点。
@param[in] callback 应用层注册的callback函数，用于通知应用层卡插入拔出情况。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param));

/*!
@brief 关闭智能卡设备 。
@param[in] dev 需关闭的智能卡设备节点。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_close(struct smc_device *dev);

/*!
@brief 检测智能卡设备是否插入 。
@param[in] dev 需检测的智能卡设备节点。
@return INT32。
@retval  0       卡插入。
@retval  非0  卡拔出。
*/
INT32 smc_card_exist(struct smc_device *dev);

/*!
@brief 重启智能卡。
@param[in] dev 需重启的智能卡设备节点。
@param[in] buffer 接收ATR 数据的地址。
@param[out] *atr_size 实际接收的ATR 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);

/*!
@brief 休眠智能卡。
@param[in] dev 需休眠的智能卡设备节点。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_deactive(struct smc_device *dev);

/*!
@brief 从智能卡读取数据。
@param[in] dev待读取数据的智能卡设备节点。
@param[in] buffer 接收智能卡 数据的地址。
@param[in] size 期望读取的 数据长度。
@param[out] *actlen 实际接收的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  未读取任何数据。
*/
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);

/*!
@brief 向智能卡发送数据。
@param[in] dev待发送数据的智能卡设备节点。
@param[in] buffer 待发送 数据的地址。
@param[in] size 期望发送的 数据长度。
@param[out] *actlen 实际发送的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  发送失败。
*/
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
/*!
@brief 遵循ISO7816 T0 协议的数据传输接口。
@param[in] dev待传输数据的智能卡设备节点。
@param[in] command 待发送 的命令及数据的地址。
@param[in] num_to_write 期望发送的 命令和数据的总长度。
@param[in] response 接收 数据的地址。
@param[in] num_to_read 期望接收的 数据长度。
@param[out] *actual_size 实际接收的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size);

/*!
@brief 遵循ISO7816 T1 协议的数据传输接口。
@param[in] dev待传输数据的智能卡设备节点。
@param[in] command 待发送 的命令及数据的地址。
@param[in] num_to_write 期望发送的 命令和数据的总长度。
@param[in] response 接收 数据的地址。
@param[in] num_to_read 期望接收的 数据长度。
@param[out] *actual_size 实际接收的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);

/*!
@brief 遵循ISO7816 T14 协议的数据传输接口。
@param[in] dev待传输数据的智能卡设备节点。
@param[in] command 待发送 的命令及数据的地址。
@param[in] num_to_write 期望发送的 命令和数据的总长度。
@param[in] response 接收 数据的地址。
@param[in] num_to_read 期望接收的 数据长度。
@param[out] *actual_size 实际接收的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);

/*!
@brief 智能卡 模块的io contorl 操作。
@param[in] dev 指向智能卡模块 的指针。
@param[in] cmd 操作 的命令类型。参考smc_device_ioctrl_command定义。
@param[in,out] param 操作的参数，根据不同的命令会有不同的参数。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param);

/*!
@brief 遵循ISO7816 T1 协议的数据传输接口。
@param[in] dev待传输数据的智能卡设备节点。
@param[in] dad 目标结点地址。
@param[in] snd_buf 发送数据地址。
@param[in] snd_len 发送数据长度。
@param[in] rcv_buf 接收的数据地址。
@param[in] rcv_len 接收的 数据长度。
@return INT32。
@retval  0       成功。
@retval  非0  失败。
*/
INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len);


#ifdef __cplusplus
}
#endif

/*!
 @}
 */
 
 /*!
 @}
 */
 
#endif /* __HLD_SMC_H__ */

