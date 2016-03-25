Main Directory:
..\Elephant\src\lld\nim\dvbc\tun_tda18250ab\

Tuner API:
	tun_tda18250ab.c
	tun_tda18250ab.h

Low Level Driver£º
	tmbslTDA18250A.c
	tmbslTDA18250A.h
	tmbslTDA18250A_Config_DVBC.h
	tmbslTDA18250A_Config_DVBT.h
	tmbslTDA18250A_Config_DTMB.h
	tmbslTDA18250A_Config_ISDBT.h
	tmbslTDA18250A_Config_ANALOG.h
	tmbslTDA18250A_Config_ATSC.h
	tmbslTDA18250A_Config_Common.h
	tmbslTDA18250A_Local.h


High Level Driver:
	tmbslTDA18250A_Advanced.c
	tmbslTDA18250A_Advanced.h
	tmbslTDA18250InstanceCustom.h
	tmbslTDA18250local.h

Driver API£º
	

Head Files£º
	
	Standard Files£º
		tmNxTypes.h	(include tmFlags.h)
		tmCompId.h
		tmFrontEnd.h
		tmUnitParams.h
		tmsysFrontEndTypes.h
		tmsysScanXpress.h
		tmUnitParams.h

	Driver Head Files£º
		tmbslTDA18250A.h

Note:
1 Simple initialization code as M3202C:
INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
/*How to carry out the tuner config/init setting in root.c/board_config.c. Please look at below.*/

		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xb8;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x14;
	//joey 20080504. change IF agc setting according to Jack's test result.	
	//trueve 20080806. restore IF AGC setting to max limitation according to Joey's suggestion.
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x80;
	
		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_NXP;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;//7; /*1 for single AGC, 7 for dual AGC */
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC0;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 16;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 5000;// 8M/6M also is 5000 IF.
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_TYPE_SCB0;
	//
		extern INT32 tun_tda18250ab_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
		extern INT32 tun_tda18250ab_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_tda18250ab_status(UINT32 Tun_id, UINT8 *lock);
		extern INT32 tun_tda18250ab_get_rf_level(UINT32 tuner_id, UINT32 *rf_level);

	
		ptrQAM_Tuner->nim_Tuner_Init					= tun_tda18250ab_init;
		ptrQAM_Tuner->nim_Tuner_Control					= tun_tda18250ab_control;
		ptrQAM_Tuner->nim_Tuner_Status					= tun_tda18250ab_status;
		ptrQAM_Tuner->nim_Tuner_Get_rf_level				= (void *)tun_tda18250ab_get_rf_level;

}
2 Support 6M &8M mode:
	6M/8M mode, wTuner_IF_Freq	= 5000;
