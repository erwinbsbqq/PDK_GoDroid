
#ifndef _NIM_TDA10025_H_
#define _NIM_TDA10025_H_



INT32 	nim_tda10025_dev_init(struct nim_device *dev);
INT32   nim_tda10025_dev_close(struct nim_device *dev);

INT32 	nim_tda10025_get_lock(struct nim_device *dev, UINT8 *lock );
INT32 	nim_tda10025_get_freq(struct nim_device *dev, UINT32 *freq );
INT32 	nim_tda10025_get_qam(struct nim_device *dev, UINT8 *qam_order );
INT32 	nim_tda10025_get_snr(struct nim_device *dev, UINT8 *snr );
INT32 	nim_tda10025_get_symbolrate(struct nim_device *dev, UINT32 *sym_rate );
INT32 	nim_tda10025_get_ber(struct nim_device *dev, UINT32 *err_count );
INT32 	nim_tda10025_get_agc(struct nim_device *dev, UINT8 *agc );

INT32 	nim_tda10025_I2C_switch( BOOL bOn );
INT32 	nim_tda10025_enter_standby(struct nim_device *dev);





#endif

