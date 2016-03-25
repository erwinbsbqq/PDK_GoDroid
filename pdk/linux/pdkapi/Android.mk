LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libadr_hld
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := 			\
	src/hld/hld_dev.c 		\
	src/hld/ape/adr_ape_dbg.c 	\
	src/hld/ape/adr_ape_playback.c 	\
	src/hld/ape/ape.c 		\
	src/hld/ape/ape_priv.c		\
	src/hld/avsync/adr_avsync_dbg.c	\
	src/hld/avsync/avsync.c		\
	src/hld/bdma/ALiBDMA.cpp	\
	src/hld/bdma/ALiDMA_API.cpp	\
	src/hld/bus/dog/dog_api.c	\
	src/hld/bus/sci/uart_api.c	\
	src/hld/bus/sci/uart_vsprintf.c	\
	src/hld/dbg/adr_dbg_serv.c	\
	src/hld/dbg/adr_dbg_parser.c	\
	src/hld/dbg/adr_dbg_tool.c	\
	src/hld/dbg/adr_dbg_bw.c	\
	src/hld/dbg/adr_dbg_osd.c	\
	src/hld/dbg/adr_dbg_adc.c	\
	src/hld/dbg/adr_dbg_smc.c	\
	src/hld/dbg/adr_dbg_dmx_old.c	\
	src/hld/deca/deca.c	\
	src/hld/deca/adr_deca_dbg.c	\
	src/hld/decv/decv.c	\
	src/hld/decv/adr_decv_dbg.c	\
	src/hld/dis/adr_vpo_dbg.c	\
	src/hld/dis/vpo.c		\
	src/hld/dmx/dmx.c		\
	src/hld/dmx/Ali_DmxLib.c	\
	src/hld/dmx/adr_dmx_dbg.c	\
	src/hld/dsc/dsc.c		\
	src/hld/dsc/dsc_ca.c		\
	src/hld/dsc/dsc_tds.c		\
	src/hld/hdmi/hdmi_hld.c		\
	src/hld/misc/ipc/adr_ipc.c	\
	src/hld/misc/rfk/rfk.c		\
	src/hld/nim/nim.c		\
	src/hld/nim/adr_tuner_dbg.c	\
	src/hld/osd/osddrv.c		\
	src/hld/osd/adr_osddrv_dbg.c	\
	src/hld/pan/pan.c		\
	src/hld/pan/adr_pan_dbg.c	\
	src/hld/rtc/rtc.c		\
	src/hld/smc/smc.c		\
	src/hld/snd/snd.c		\
	src/hld/snd/adr_snd_dbg.c	\
	src/hld/soc/soc.c		\
	src/hld/sto/sto.c		\
	src/hld/sto/sto_nand.c		\
	src/hld/sto/nand_api.c		\
	src/hld/sto/libmtd.c		\
	src/osal/pthread/osal_flag.c	\
	src/osal/pthread/osal_msgq.c	\
	src/osal/pthread/osal_mutex.c	\
	src/osal/pthread/osal_sema.c	\
	src/osal/pthread/osal_task.c	\
	src/osal/pthread/osal_timer.c	\
	src/osal/pthread/select_timer.c	\
	src/lib/libcunit/Util.c		\
	src/lib/libcunit/MyMem.c	\
	src/lib/libcunit/CUError.c	\
	src/lib/libcunit/TestDB.c	\
	src/lib/libcunit/TestRun.c	\
	src/lib/libcunit/Automated.c	\
	src/lib/libcunit/Basic.c	\
	src/lib/libcunit/Console.c	\

	
#src/hld/standby/fast_standby.c	\
#src/hld/standby/pmu_standby.c	\
	
	

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/inc $(LOCAL_PATH)/include/alidrivers/include/ $(LOCAL_PATH)/include/kernel/linux-linaro-3.4-rc3/include
LOCAL_C_INCLUDES += system/core/include/cutils

LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_CFLAGS = -g -O2 -Wreturn-type 
LOCAL_CFLAGS += -Wno-return-type


include $(BUILD_SHARED_LIBRARY)
