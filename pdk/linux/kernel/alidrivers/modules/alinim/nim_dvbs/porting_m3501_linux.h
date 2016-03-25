#ifndef __PORTING_M3501_LINUX_H__
#define __PORTING_M3501_LINUX_H__





#define nim_print  	printk



#define LNB_CMD_BASE			0xf0
#define LNB_CMD_ALLOC_ID		(LNB_CMD_BASE+1)
#define LNB_CMD_INIT_CHIP		(LNB_CMD_BASE+2)
#define LNB_CMD_SET_POLAR		(LNB_CMD_BASE+3)
#define LNB_CMD_POWER_EN		(LNB_CMD_BASE+4)

#define NIM_PORLAR_HORIZONTAL	0x00
#define NIM_PORLAR_VERTICAL		0x01
#define NIM_PORLAR_LEFT			0x02
#define NIM_PORLAR_RIGHT		0x03

#define NIM_PORLAR_REVERSE		0x01
#define NIM_PORLAR_SET_BY_22K	0x02



#define I2C_ERROR_BASE			-200

#define ERR_I2C_SCL_LOCK		(I2C_ERROR_BASE - 1)	/* I2C SCL be locked */
#define ERR_I2C_SDA_LOCK		(I2C_ERROR_BASE - 2)	/* I2C SDA be locked */
#define ERR_I2C_NO_ACK			(I2C_ERROR_BASE - 3)	/* I2C slave no ack */
#define S3501_ERR_I2C_NO_ACK	ERR_I2C_NO_ACK

#define OSAL_INVALID_ID			0xFFFFFFFF


#define comm_malloc(x)				kmalloc((x),GFP_KERNEL)//kmalloc((x), GFP_ATOMIC)
#define comm_memset 				memset
#define comm_free 					kfree

#define comm_sleep 					msleep
#define comm_memcpy 				memcpy
#define nim_i2c_read				ali_i2c_read
#define nim_i2c_write				ali_i2c_write
#define nim_i2c_write_read			ali_i2c_write_read
#define comm_delay(x)				\
	do \
	{\
	   if(x>1000000)\
	   {\
	   	  mdelay(x/1000000);\
	   }\
	   else if(x>1000)\
	   {\
	   	  ndelay(x/1000);\
	   }\
	   else\
	   {\
	   	  udelay(x);\
	   }\
	}while(0)  

#define OSAL_TWF_ORW				0
#define OSAL_TWF_ANDW 				0
#define OSAL_TWF_CLR				0

#define NIM_MUTEX_ENTER(priv)  \
	do \
	{ \
		mutex_lock(&priv->i2c_mutex); \
	}while(0)

#define NIM_MUTEX_LEAVE(priv) \
	do\
	{ \
		mutex_unlock(&priv->i2c_mutex);\
	}while(0)





typedef struct NIM_AUTO_SCAN         NIM_AUTO_SCAN_T;
typedef struct NIM_CHANNEL_CHANGE    NIM_CHANNEL_CHANGE_T;


struct nim_s3501_tsk_status
{
    UINT32 					m_lock_flag;
    UINT32					m_task_id;
    UINT32 					m_sym_rate;
    UINT8 					m_work_mode;
    UINT8 					m_map_type;
    UINT8 					m_code_rate;
    UINT8 					m_info_data;
};
struct nim_s3501_tparam
{
    INT32 					t_last_snr;
    INT32 					t_last_iter;
    INT32 					t_aver_snr;
    INT32 					t_snr_state;
    INT32 					t_snr_thre1;
    INT32 					t_snr_thre2;
    INT32 					t_snr_thre3;
    INT32 					t_phase_noise_detected;
    INT32	 				t_dynamic_power_en;
    UINT32 					phase_noise_detect_finish;
    UINT32					t_reg_setting_switch;
    UINT8 					t_i2c_err_flag;
};
struct nim_s3501_lstatus
{
    UINT32					nim_s3501_sema;
    INT32					ret;
    UINT8 					s3501_autoscan_stop_flag;
    UINT8 					s3501_chanscan_stop_flag;
    UINT32 					old_ber ;
    UINT32 					old_per ;
    UINT32 					old_ldpc_ite_num;
    UINT8 					*adcdata;// = (unsigned char *)__MM_DMX_FFT_START_BUFFER;//[2048];
    UINT8 					*adcdata_malloc_addr;
    UINT8 					*adcdata_raw_addr;
    INT32 					m_freq[256];
    UINT32 					m_rs[256];
    INT32 					FFT_I_1024[1024];
    INT32 					FFT_Q_1024[1024];
    UINT8 					m_crnum;
    UINT32 					m_cur_freq;
    UINT8 					c_rs ;
    UINT32 					m_step_freq;
    pfn_nim_reset_callback 	                m_pfn_reset_s3501;
    UINT8 					m_enable_dvbs2_hbcd_mode;
    UINT8 					m_dvbs2_hbcd_enable_value;
    UINT8 					s3501d_lock_status;
    UINT32 					phase_err_check_status;
    UINT32 					m_s3501_type;
    UINT32 					m_s3501_sub_type;
    UINT32 					m_setting_freq;
    UINT32 					m_err_cnts;
    UINT8 					m_hw_timeout_thr;

    UINT8 					m_tso_mode;
    UINT8 					m_tso_status;

};




#if 1
struct nim_s3501_private
{
    INT32 							(*nim_tuner_init) (UINT32 *, struct QPSK_TUNER_CONFIG_EXT *);	// Tuner Initialization Function
    INT32 							(*nim_tuner_control) (UINT32, UINT32, UINT32);	// Tuner Parameter Configuration Function
    INT32 							(*nim_tuner_status) (UINT32, UINT8 *);

    void							(*nim_tuner_close)(void);

    struct QPSK_TUNER_CONFIG_DATA 	tuner_config_data;
    UINT32 							tuner_id;
    UINT32 							i2c_type_id;
    UINT32 							polar_gpio_num;
    UINT32 							sys_crystal;
    UINT32 							sys_clock;
    UINT16 							pre_freq ;
    UINT16 							pre_sym ;
    INT8 							autoscan_stop_flag ;

    UINT8 							chip_id;
    //UINT8							diseqc_typex;				/* NIM DiSEqC Device Type */
    //UINT8							diseqc_portx;				/* NIM DiSEqC Device Port */
    struct t_diseqc_info 			diseqc_info;			/* NIM DiSEqC Device Information Structure */
    struct EXT_DM_CONFIG 			ext_dm_config;
    struct nim_s3501_lstatus 		ul_status;
    INT32							ext_lnb_id;
    INT32 							(*ext_lnb_control) (UINT32, UINT32, UINT32);
    struct nim_s3501_tsk_status 	tsk_status;
    struct nim_s3501_tparam 		t_param;
    UINT32 							cur_freq;
    UINT32 							cur_sym;

    rwlock_t 						flagid_rwlk;  /*Protect the following flag_id*/
    UINT32 							flag_id; 	/*Asynchronous channel change control flag*/
    enum NIM_BLSCAN_MODE 			blscan_mode;

    struct QPSK_TUNER_CONFIG_EXT 	tuner_config;
    struct mutex 					i2c_mutex;

    NIM_AUTO_SCAN_T 					as_info;
    AUTO_SCAN_PARA 					blind_msg;
    wait_queue_head_t           	as_sync_wait_queue;
    unsigned char 					as_status;
    UINT8 							work_alive;
    UINT8							tuner_opened;
    BOOL 							yet_return;
	UINT8 							dev_idx;
	struct nim_m3501_sig_status	      m3501_sig_status;  //add by dennis on 2014-08-01
	struct nim_m3501_quality_info     m3501_quality_info;//add by dennis on 2014-08-04
};


#endif


DWORD 		nim_s3501_multu64div(UINT32 v1, UINT32 v2, UINT32 v3);
UINT32 		os_get_tick_count();



RET_CODE 	nim_send_as_msg(int id, unsigned char lck, unsigned char polar, unsigned short freq, 
                                unsigned int sym, unsigned char fec, unsigned char as_stat);

UINT32 		dvbs_as_cb2_ui(void *p_priv, unsigned char lck, unsigned char polar, unsigned short freq, 
                               unsigned int sym, unsigned char fec, unsigned char as_stat);

INT32 		nim_callback(NIM_AUTO_SCAN_T *pst_auto_scan, void *pfun, UINT8 status, UINT8 polar, 
                             UINT16 freq, UINT32 sym, UINT8 fec, UINT8 stop);

UINT32 		nim_flag_read(struct nim_s3501_private *priv, UINT32 T1, UINT32 T2, UINT32 T3);
UINT32 		nim_flag_create(struct nim_s3501_private *priv);
UINT32 		nim_flag_set(struct nim_s3501_private *priv, UINT32 value);
UINT32 		nim_flag_clear(struct nim_s3501_private *priv, UINT32 value);
UINT32 		nim_flag_del(struct nim_s3501_private *priv);





#endif
