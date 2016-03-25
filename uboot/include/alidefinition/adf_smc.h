#ifndef	__ADF_SMC_H_

#define	__ADF_SMC_H_




#ifdef __cplusplus
extern "C" {
#endif

/*! @addtogroup Devicedriver
 *  @{
    */
 
/*! @addtogroup ALiSMC
 *  @{
    */


#define ATR_MAX_SIZE 		33	//!< Maximum length of ATR.



/*! @enum smc_error_e
@brief Status information of smartcard module. 
*/

typedef enum smc_error_e {

	SMART_NO_ERROR,

	SMART_WRONG_ATR,

	SMART_TOO_MANY_RETRIES,

	SMART_OVERRUN_ERROR,

	SMART_FRAME_ERROR,

	SMART_PTS_NOT_SUPPORTED,

	SMART_INVALID_STATUS_BYTES_SEQUENCE,

	SMART_INVALID_CLASS,

	SMART_INVALID_CODE,

	SMART_INCORRECT_REFERENCE,

	SMART_INCORRECT_LENGTH,

	SMART_NOT_INSERTED,

	SMART_NOT_RESETED,

	SMART_INVALID_PROTOCOL,

	SMART_USER_ABORT,

	SMART_BAD_COMMAND,

	SMART_WRITE_ERROR ,

	SMART_READ_ERROR,

	SMART_NO_ANSWER,

	SMART_PARITY_ERROR,

	SMART_UNKNOWN_ERROR,

} smc_error_e;



#define SMC_MSG_MAX_LEN 32			//!< Maximum length of data sent from kernel to application layer. 



#define BOARD_SUPPORT_CLASS_A		(1<<0)	//!< Class A 

#define BOARD_SUPPORT_CLASS_B		(1<<1)	//!< Class B 

#define BOARD_SUPPORT_CLASS_C		(1<<2)	//!<Class C 



/*! @struct smc_notification_param_t
@brief Smartcard notification information. 
*/

typedef struct smc_notification_param_t {

    uint8 smc_msg_tag;						//!<Smartcard notification tag 

    uint8 smc_msg_len;						//!<Smartcard notification length 

    uint8 smc_msg_buf[SMC_MSG_MAX_LEN];		//!<Smartcard notification address 

}smc_notification_param_t;





/*! @enum smc_notification_e
@brief Smartcard module notification tag. 
*/

typedef enum smc_notification_e {

	SMC_MSG_CARD_STATUS,			//!< Card status 

} smc_notification_e;



/* Card status */

#define SMC_STATUS_OK			0

#define SMC_STATUS_NOT_EXIST	1

#define SMC_STATUS_NOT_RESET	2



#define SMC_IO_ON		1

#define SMC_IO_OFF		0



/*! @enum smc_atr_result
@brief ATR status. 
*/

enum smc_atr_result

{

	SMC_ATR_NONE = 0,

	SMC_ATR_WRONG,

	SMC_ATR_OK

};

/*! @struct smc_atr_t
@brief ATR address and length. 
*/

typedef struct smc_atr_t {

    uint8 atr_buf[ATR_MAX_SIZE];

    unsigned short atr_size;

}smc_atr_t;



/*! @struct smc_iso_transfer_t
@brief ISO transfer command parameters. 
*/
typedef struct smc_iso_transfer_t {

    uint8 *command;

    int32 num_to_write; 

    uint8 *response; 

    int32 num_to_read; 

    int32 actual_size;

    int32 transfer_err;

}smc_iso_transfer_t;



/*! @struct smc_hb_t
@brief History byte parameters. 
*/

typedef struct smc_hb_t

{

	uint8 *hb;

	uint8 hbn;

}smc_hb_t;



/*! @enum class_selection
@brief Card operation class. 
*/

enum class_selection

{

	SMC_CLASS_NONE_SELECT = 0,	//!< No selection. 

	SMC_CLASS_A_SELECT,			//!< Class A 

	SMC_CLASS_B_SELECT,			//!< Class B 

	SMC_CLASS_C_SELECT			//!< Class C 

};



/*! @struct smc_dev_cfg
@brief Initialization configuration parameters. 
*/

struct smc_dev_cfg			

{

	uint32	init_clk_trigger : 1;		//!< 0, Use the default clock frequency 3.579545MHz.

	uint32	def_etu_trigger : 1;		//!< 0, Use the detected ETU 
	           										//!< 1, Use the configured ETU 

	uint32	sys_clk_trigger : 1; 		//!< Do no use anymore.

	uint32	gpio_cd_trigger : 1;		//!< Do not use anymore. 									

	uint32	gpio_cs_trigger : 1;		//!< Do not use anymore. 

	uint32  force_tx_rx_trigger: 1; 	//!<  Auto swith sending/receiving 
	
	uint32	parity_disable_trigger: 1;	//!< 0, Disable odd-even check when getting ATR 

										              //!< 1, Enable odd-even check when getting ATR 

	uint32	parity_odd_trigger: 1; 		//!< 0, Use even check when getting ATR 

									             	//!< 1, Use odd check when getting ATR

	uint32	apd_disable_trigger: 1;		//!< 0, Enable auto drop function 

										//!< 1, Disable auto drop function 

	uint32	type_chk_trigger : 1;		//!< 0, Do not detect the card operation class; 1, Detect the card operation class 				
	uint32 	warm_reset_trigger: 1;		//!<0£¬all resets are cold resets; 1, All resets are warm reset except the first time. 								

	uint32	gpio_vpp_trigger : 1;		//!< Do not use anymore. 

	uint32 	disable_pps: 1;				//!< Whether disable PPS 

	uint32	invert_power: 1;			//!< Whether invert to supply power 

	uint32	invert_detect: 1;			//!< Whether invert to detect card 

	uint32	class_selection_supported: 1;	 	//!<  Whether support multiple operation classes 

	uint32 	board_supported_class: 6;			//!< Supported operation class 

	uint32  board_class: 2;						//!< Current selected operation class 

	uint32 	en_power_open_drain:1;				//!< Whether enable power pin drain open circuit 

	uint32 	en_clk_open_drain:1;				//!<  Whether enable clock pin drain open circuit 

	uint32 	en_data_open_drain:1;				//!<  Whether enable data pin drain open circuit 

	uint32 	en_rst_open_drain:1;				//!< Whether enable reset pin drain open circuit 

	uint32	reserved : 4;									

	uint32	init_clk_number;			//!< Initialize clock frequency number 

	uint32  *init_clk_array;			//!< Initialize clock frequency address

	uint32	default_etu;				//!< Defaule ETU 

	uint32	smc_sys_clk;				//!< System clock frequency 
	
	uint32 	gpio_cd_pol:1;				//!< GPIO polarity 

	uint32 	gpio_cd_io:1;				//!< Set input and output 

	uint32 	gpio_cd_pos: 14;			//!< GPIO index 

	uint32 	gpio_cs_pol:1;				//!< GPIO polarity 

	uint32 	gpio_cs_io:1;				//!<Set input and output 

	uint32 	gpio_cs_pos: 14;			//!< GPIO index 

	uint8   force_tx_rx_cmd;			//!< ommand forced to send/receive 

	uint8   force_tx_rx_cmd_len;		//!< Command length forced to send/receive 

	uint8   intf_dev_type;				//!< Do not use anymore. 

	uint8   reserved1;

	uint32 	gpio_vpp_pol:1;				//!< GPIO polarity 

	uint32 	gpio_vpp_io:1;				//!< Set input and output 

	uint32 	gpio_vpp_pos: 14;			//!< GPIO index 

	uint32	ext_cfg_tag;				//!< Do not use anymore.

	void	*ext_cfg_pointer;			//!< Do not use anymore. 

	void	(*class_select)(enum class_selection );	//!< Callback function of card operation class 

	int32	use_default_cfg;						//!< Whether use the default configuration parameters 

};



/*! @struct smc_t1_trans_t
@brief T1 protocol sending parameter 
*/

typedef struct smc_t1_trans_t {

    uint8 dad;

    void *send_buf; 

    uint32 send_len; 

    void *rcv_buf;

    uint32 rcv_len;

}smc_t1_trans_t;



/*! @struct smc_t1_xcv_t
@brief T1 protocol receiving parameter 
*/

typedef struct smc_t1_xcv_t {

    uint8 *sblock;

    uint32 slen;

    uint8 *rblock;

    uint32 rmax;

    uint32 actual_size;

}smc_t1_xcv_t;



/*! @struct smc_t1_nego_ifsd_t

@brief T1 protocol negotiation parameter.
*/

typedef struct smc_t1_nego_ifsd_t {

    uint32 dad;		//!< Target node address 

    int32 ifsd;		//!< Maximum field length of block information that smartcard can receive 

}smc_t1_nego_ifsd_t;



#ifdef __cplusplus

}

#endif

/*!
 * @}
 */

/*!
 * @}
 */

#endif



