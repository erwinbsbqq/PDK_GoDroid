#
# linux/drivers/media/dvb/frontends/ali_nim/Makefile
#

EXTRA_CFLAGS	+= -D__NIM_LINUX_PLATFORM__

obj-y += porting_linux_common.o
obj-y += tun_common.o

obj-$(CONFIG_ALI_QAM) += nim_dvbc/nim_s3281_linux.o
obj-$(CONFIG_ALI_QAM) += nim_dvbc/nim_s3281_ic.o 
obj-$(CONFIG_ALI_QAM) += nim_dvbc/nim_s3281_im.o

#CONFIG_ALI_QPSK
#CONFIG_ALI_COFDM

obj-$(CONFIG_ALI_QPSK) += nim_dvbs/porting_m3501_linux.o 
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_common_linux.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_linux.o 
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_im.o 
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_im_tr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_im_cr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_ic.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_ic_tr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_m3501_ic_cr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_linux.o 
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_im.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_im_tr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_im_cr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_ic.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_ic_tr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3503_ic_cr.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3501_autosearch.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/nim_s3501_autosearch_fft.o
obj-$(CONFIG_ALI_QPSK) += nim_dvbs/R2FFT.o


#CONFIG_TDA10025
obj-$(CONFIG_TDA10025) += nim_TDA10025/nim_tda10025_linux.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/nim_tda10025_im.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslHCDP.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslHCDP_Advanced.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslHCDP_Instance.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslHCDP_Reg.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslTDA10025.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslTDA10025_Cfg.o
obj-$(CONFIG_TDA10025) += nim_TDA10025/tmbslTDA10025Instance.o

#CONFIG_CXD2837
obj-$(CONFIG_CXD2837) += nim_cxd2837/nim_cxd2837_linux.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/nim_cxd2837_im.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_common.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbc.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbc_monitor.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbt.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbt_monitor.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbt2.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_dvbt2_monitor.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_i2c.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_demod_integ.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_math.o
obj-$(CONFIG_CXD2837) += nim_cxd2837/sony_stopwatch_port.o

#CONFIG_CXD2838
obj-$(CONFIG_CXD2838) += nim_cxd2838/nim_cxd2838_linux.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/nim_cxd2838_im.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_common.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_demod.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_demod_i2c.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_demod_isdbt.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_integ_isdbt.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_demod_isdbt_monitor.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_math.o
obj-$(CONFIG_CXD2838) += nim_cxd2838/sony_stopwatch_port.o
#CONFIG_OTHER_NIM
obj-$(CONFIG_OTHER_NIM) += nim_other/atbm886x.o

#CONFIG_RT810
obj-$(CONFIG_RT810) += tun_RT810/RT810.o
obj-$(CONFIG_RT810) += tun_RT810/tun_rt810.o

#CONFIG_RT820C
obj-$(CONFIG_RT820C) += tun_RT820C/R820C.o
obj-$(CONFIG_RT820C) += tun_RT820C/tun_rt820.o

#CONFIG_TDA18250
obj-$(CONFIG_TDA18250) += tun_TDA18250/System_Lo_Detuning.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tmbslTDA18250.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tmbslTDA18250AdvanceCustom.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tmbslTDA18250Instance.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tmddTDA18250.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tmddTDA18250Instance.o
obj-$(CONFIG_TDA18250) += tun_TDA18250/tun_tda18250.o

#CONFIG_TDA18250_AB
obj-$(CONFIG_TDA18250_AB) += tun_TDA18250AB/tmbslTDA18250A.o
obj-$(CONFIG_TDA18250_AB) += tun_TDA18250AB/tmbslTDA18250A_Advanced.o
obj-$(CONFIG_TDA18250_AB) += tun_TDA18250AB/tun_tda18250ab.o
#CONFIG_MXL603
obj-$(CONFIG_MXL603) += tun_MXL603/MxL603_TunerApi.o
obj-$(CONFIG_MXL603) += tun_MXL603/MxL603_TunerCfg.o
obj-$(CONFIG_MXL603) += tun_MXL603/tun_mxl603.o

#CONFIG_MXL203
obj-$(CONFIG_MXL203) += tun_MXL203/MxL_User_Define.o
obj-$(CONFIG_MXL203) += tun_MXL203/MxL203RF.o
obj-$(CONFIG_MXL203) += tun_MXL203/MxL203RF_API.o
obj-$(CONFIG_MXL203) += tun_MXL203/tun_mxl203rf.o

#CONFIG_AV2012
obj-$(CONFIG_ALI_PM) += tun_AV2012/tun_av2011.o

#CONFIG_CXD2872
obj-$(CONFIG_CXD2872) += tun_cxd2872/sony_tuner_common.o
obj-$(CONFIG_CXD2872) += tun_cxd2872/sony_tuner_ascot3.o
obj-$(CONFIG_CXD2872) += tun_cxd2872/sony_tuner_i2c.o
obj-$(CONFIG_CXD2872) += tun_cxd2872/sony_ascot3.o
#CONFIG_DCT70701
#CONFIG_OTHER_TUNER
obj-$(CONFIG_OTHER_TUNER) += tun_other/tun_dct70701.o

#
#CONFIG_SHARP_VZ7306
obj-$(CONFIG_SHARP_VZ7306) += tun_SHARP/sharp_vz7306.o



