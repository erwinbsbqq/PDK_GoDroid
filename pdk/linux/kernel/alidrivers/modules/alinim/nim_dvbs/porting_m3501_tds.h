#ifndef __PORTING_M3501_TDS_H__
#define __PORTING_M3501_TDS_H__






#define nim_print  		libc_printf



#define S3501_ERR_I2C_NO_ACK	ERR_I2C_NO_ACK




#define comm_malloc 			MALLOC
#define comm_memset				MEMSET
#define comm_free				FREE
#define comm_delay				nim_comm_delay
#define comm_sleep				osal_task_sleep
#define comm_memcpy				memcpy

#define nim_i2c_read			i2c_read
#define nim_i2c_write			i2c_write
#define nim_i2c_write_read		i2c_write_read










// use mutex
#define NIM_MUTEX_CREATE	osal_semaphore_create
#define NIM_MUTEX_DELETE	osal_semaphore_delete

#define NIM_MUTEX_ENTER(priv)  \
	if(priv->ul_status.nim_s3501_sema != OSAL_INVALID_ID) \
	{ \
		do{ \
			priv->ul_status.ret= osal_semaphore_capture(priv->ul_status.nim_s3501_sema, 1000); \
			if(priv->ul_status.ret!= OSAL_E_OK) \
				{NIM_PRINTF("nim_s3501_sema[%d] capture timeout, retry...\n", \
					priv->ul_status.nim_s3501_sema);} \
		}while(priv->ul_status.ret != OSAL_E_OK); \
	}

#define NIM_MUTEX_LEAVE(priv) \
	if(priv->ul_status.nim_s3501_sema != OSAL_INVALID_ID) \
	{ \
		osal_semaphore_release(priv->ul_status.nim_s3501_sema); \
	}

// use flag
#if 0
#define NIM_FLAG_CREATE osal_flag_create
#define nim_flag_del osal_flag_delete
#define nim_flag_read osal_flag_wait
#define nim_flag_set osal_flag_set
#define nim_flag_clear osal_flag_clear
#endif


typedef struct NIM_AUTO_SCAN       NIM_AUTO_SCAN_T;





struct nim_s3501_tsk_status
{
    UINT32 					m_lock_flag;
    ID 						m_task_id;
    UINT32 					m_sym_rate;
    UINT8 					m_work_mode;
    UINT8 					m_map_type;
    UINT8 					m_code_rate;
    UINT8 					m_info_data;
};
struct nim_s3501_tparam
{
    int 					t_last_snr;
    int 					t_last_iter;
    int 					t_aver_snr;
    int 					t_snr_state;
    int 					t_snr_thre1;
    int 					t_snr_thre2;
    int 					t_snr_thre3;
    INT32 					t_phase_noise_detected;
    INT32	 				t_dynamic_power_en;
    UINT32 					phase_noise_detect_finish;
    UINT32					t_reg_setting_switch;
    UINT8 					t_i2c_err_flag;
};
struct nim_s3501_lstatus
{
    ID 						nim_s3501_sema;
    ER 						ret;
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
    pfn_nim_reset_callback 	m_pfn_reset_s3501;
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


enum NIM_BLSCAN_MODE
{
    NIM_SCAN_FAST = 0,
    NIM_SCAN_SLOW = 1,
};






struct nim_s3501_private
{
    INT32 							(*nim_tuner_init) (UINT32 *, struct QPSK_TUNER_CONFIG_EXT *);	// Tuner Initialization Function
    INT32 							(*nim_tuner_control) (UINT32, UINT32, UINT32);	// Tuner Parameter Configuration Function
    INT32 							(*nim_tuner_status) (UINT32, UINT8 *);
    struct QPSK_TUNER_CONFIG_DATA 	tuner_config_data;
    UINT32 							tuner_id;
    UINT32 							i2c_type_id;
    UINT32 							polar_gpio_num;
    UINT32 							sys_crystal;
    UINT32 							sys_clock;
    UINT16 							pre_freq;
    UINT16 							pre_sym;
    INT8 							autoscan_stop_flag;
    struct nim_device_stats 		stats;
    UINT8 							chip_id;
    struct EXT_DM_CONFIG 			ext_dm_config;
    struct nim_s3501_lstatus 		ul_status;
    INT32 							ext_lnb_id;
    INT32 							(*ext_lnb_control) (UINT32, UINT32, UINT32);
    struct nim_s3501_tsk_status 	tsk_status;
    struct nim_s3501_tparam 		t_param;
    UINT32 							cur_freq;
    UINT32 							cur_sym;
    UINT32 							flag_id;
    enum NIM_BLSCAN_MODE 			blscan_mode;
    OSAL_ID 						m3501_mutex;
	UINT8 							work_alive;
};






DWORD 		nim_s3501_multu64div(UINT32 v1, UINT32 v2, UINT32 v3);
void 		nim_comm_delay(UINT32 us);

INT32 		nim_callback(NIM_AUTO_SCAN_T *pst_auto_scan, void *pfun, UINT8 status, UINT8 polar, 
                             UINT32 freq, UINT32 sym, UINT8 fec, UINT8 stop);
UINT32 		nim_flag_read(struct nim_s3501_private *priv, UINT32 T1, UINT32 T2, UINT32 T3);
UINT32 		nim_flag_create(struct nim_s3501_private *priv);
UINT32 		nim_flag_set(struct nim_s3501_private *priv, UINT32 value);
UINT32 		nim_flag_clear(struct nim_s3501_private *priv, UINT32 value);
UINT32 		nim_flag_del(struct nim_s3501_private *priv);



#endif
