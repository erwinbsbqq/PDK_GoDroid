#
# -= Makefile for project compile =-
#
# Usage:
# . Name this file as "Makefile";
#   Put it in the root of "src" directory.
# . Add a new module name in SMOD;
#   Write the linkage information at the end of file.
#

# List of sub-modules
# Note: Can NOT be the same name as sub-directories.
SMOD = OSAL_PTHREAD HLD_BASE \
       HLD_BUS_DOG \
       HLD_BUS_UART \
       HLD_APE \
       HLD_STO \
       HLD_NIM \
       HLD_CIC \
       HLD_DSC HLD_RTC\
       HLD_STANDBY\
       HLD_DBG \
       HLD_DMX \
       HLD_DECV \
       HLD_GE \
       HLD_VP \
       HLD_VBI \
       HLD_DECA \
       HLD_SND \
       HLD_OSD \
       HLD_SDEC \
       HLD_PAN \
       HLD_SOC \
       HLD_SMC \
       HLD_SCART \
       HLD_IMGDEC \
       HLD_BDMA \
       HLD_NIM_ISDBT \
       HLD_HDMI \
       HLD_AVSYNC \
       HLD_MISC_IPC \
       HLD_MISC_RFK \
       HLD_SURFACE_2D \
       HLD_OTP	\
       HLD_FAKE	\
       HLD_TRNG	\
       HLD_WIFI \
       HLD_CE \
       LIB_ASH LIB_LOADER \
       LIB_UPG LIB_USBUPG LIB_UPG4 \
       LIB_PUB LIB_PUB_ALL LIB_PUB_AS_ENGINE  \
       LIB_GNUZIP LIB_LIB7ZIP LIB_LZMA LIB_UN7ZIP LIB_UNLZO LIB_CHAR \
       LIB_DBC \
       LIB_SI_ENGINE LIB_SI_FILTER LIB_SI_PARSER LIB_SI_PLUGIN_MONITOR LIB_SI_PLUGIN_SEARCH LIB_SI_PLUGIN_TDT \
       LIB_EPG \
       LIB_NVOD \
       LIB_CHUNK LIB_DB5 LIB_DB4 LIB_DB4_T LIB_DB4_ISDBT LIB_DB4_ATSC \
       VERSION_INFO \
       LIB_DVTCA LIB_CONAX7 LIB_DVNCA LIB_CDCA3 LIB_MGCA\
       LIB_FS LIB_STBINFO LIB_STBINFO_NOR LIB_LOGICSTO \
       LIB_FLIST \
       LIB_PVR3_ENG LIB_PVR_DRIVE LIB_PVR3_MGR LIB_PVR3_API36 \
       LIB_GE LIB_AUI LIB_SURFACE_2D \
       LIB_AMP LIB_AMPGST LIB_IMAGE \
       LIB_NETUPG \
       CRYPTON \
       LIB_AD \
       HLD_ENCODER

# Following lines are the common description for all projects.
# Do NOT modify anything, unless you know what you are doing.

# Note: Assume this file is executed under 'src' directory.
include compiler.def
include path.def

ifneq ($(strip $(BUILD_APP_MODE_TEST)),)
SMOD += TEST_CUINT
endif

SMOD_ = ${SMOD:=_}
SMOD__ = ${SMOD:=__}

do_nothing:
	@echo "Do nothing."

-include cstm.def

all: ${SMOD}

clean: ${SMOD_}
	cd ${LIB_DIR}; \
	  rm -f list.mod
	  
ddk_rel: ${SMOD__}
	cd ${LIB_DIR}; \

rebuild: clean all


telelogic: telelogic.mk
	echo generate $< done

telelogic.mk: makefile
	@gawk  -v src_root_path="${BASE_DIR}" -f ../prj/tools/telelogic.awk $<  >$@

telelogic_all: telelogic.mk 
	make -f telelogic.mk ${SMOD}

# End of common description.

# Construct and deconstruct of sub-modules

LIB_AMP:
	cd lib/libapplet/libamp; \
	  ${MAKE}
LIB_AMP_:
	cd lib/libapplet/libamp; \
	  ${MAKE} clean
LIB_AMP__:
	cd lib/libapplet/libamp; \
	  ${MAKE} ddk_rel

LIB_AMPGST:
	cd lib/libapplet/libampgst; \
	  ${MAKE}
LIB_AMPGST_:
	cd lib/libapplet/libampgst; \
	  ${MAKE} clean
LIB_AMPGST__:
	cd lib/libapplet/libampgst; \
	  ${MAKE} ddk_rel

LIB_IMAGE:
	cd lib/libapplet/libimage; \
	  ${MAKE}
LIB_IMAGE_:
	cd lib/libapplet/libimage; \
	  ${MAKE} clean
LIB_IMAGE__:
	cd lib/libapplet/libimage; \
	  ${MAKE} ddk_rel
	  
LIB_PUB_ALL:
	cd lib/libapplet/libpub/pub; \
	${MAKE} 
LIB_PUB_ALL_:
	cd lib/libapplet/libpub/pub; \
	${MAKE} clean
LIB_PUB_ALL__:
	cd lib/libapplet/libpub/pub; \
	${MAKE} ddk_rel

LIB_PUB_AS_ENGINE:
	cd lib/libapplet/libpub/as_engine; \
	${MAKE} 
LIB_PUB_AS_ENGINE_:
	cd lib/libapplet/libpub/as_engine; \
	${MAKE} clean
LIB_PUB_AS_ENGINE__:
	cd lib/libapplet/libpub/as_engine; \
	${MAKE} ddk_rel

LIB_FS:
	cd lib/libstorage/libfs; \
	  ${MAKE}
LIB_FS_:
	cd lib/libstorage/libfs; \
	  ${MAKE} clean
LIB_FS__:
	cd lib/libstorage/libfs; \
	  ${MAKE} ddk_rel

LIB_STBINFO:
	cd lib/libstorage/libstbinfo; \
	  ${MAKE}
LIB_STBINFO_:
	cd lib/libstorage/libstbinfo; \
	  ${MAKE} clean
LIB_STBINFO__:
	cd lib/libstorage/libstbinfo; \
	  ${MAKE} ddk_rel
	  
LIB_STBINFO_NOR:
	cd lib/libstorage/libstbinfo_nor; \
	  ${MAKE}
LIB_STBINFO_NOR_:
	cd lib/libstorage/libstbinfo_nor; \
	  ${MAKE} clean
LIB_STBINFO_NOR__:
	cd lib/libstorage/libstbinfo_nor; \
	  ${MAKE} ddk_rel

LIB_LOGICSTO:
	cd lib/libstorage/liblogicsto; \
	  ${MAKE}
LIB_LOGICSTO_:
	cd lib/libstorage/liblogicsto; \
	  ${MAKE} clean
LIB_LOGICSTO__:
	cd lib/libstorage/liblogicsto; \
	  ${MAKE} ddk_rel
	  
LIB_PVR_DRIVE:
	cd lib/libapplet/libpvr3/drive; \
	  ${MAKE}
LIB_PVR_DRIVE_:
	cd lib/libapplet/libpvr3/drive; \
	  ${MAKE} clean
LIB_PVR_DRIVE__:
	cd lib/libapplet/libpvr3/drive; \
	  ${MAKE} ddk_rel
	  	  
OSAL_PTHREAD:
	cd osal/pthread; \
	  ${MAKE}
OSAL_PTHREAD_:
	cd osal/pthread; \
	  ${MAKE} clean
OSAL_PTHREAD__:
	cd osal/pthread; \
	  ${MAKE} ddk_rel

HLD_BASE:
	cd hld; \
	  ${MAKE}
HLD_BASE_:
	cd hld; \
	  ${MAKE} clean
HLD_BASE__:
	cd hld; \
	  ${MAKE} ddk_rel

HLD_BUS_UART:
	cd hld/bus/sci; \
	  ${MAKE}
HLD_BUS_UART_:
	cd hld/bus/sci; \
	  ${MAKE} clean
HLD_BUS_UART__:
	cd hld/bus/sci; \
	  ${MAKE} ddk_rel

HLD_BUS_DOG:
	cd hld/bus/dog; \
	  ${MAKE}
HLD_BUS_DOG_:
	cd hld/bus/dog; \
	  ${MAKE} clean
HLD_BUS_DOG__:
	cd hld/bus/dog; \
	  ${MAKE} ddk_rel
	  
HLD_APE:
	cd hld/ape; \
		$(MAKE)
HLD_APE_:
	cd hld/ape; \
		$(MAKE) clean
HLD_APE__:
	cd hld/ape; \
		$(MAKE) ddk_rel

HLD_RTC:
	cd hld/rtc; \
	  ${MAKE}
HLD_RTC_:
	cd hld/rtc; \
	  ${MAKE} clean
HLD_RTC__:
	cd hld/rtc; \
	  ${MAKE} ddk_rel
	  
HLD_STANDBY:
	cd hld/standby; \
	  ${MAKE}
HLD_STANDBY_:
	cd hld/standby; \
	  ${MAKE} clean
HLD_STANDBY__:
	cd hld/standby; \
	  ${MAKE} ddk_rel
	  
HLD_STO:
	cd hld/sto; \
	  ${MAKE}
HLD_STO_:
	cd hld/sto; \
	  ${MAKE} clean
HLD_STO__:
	cd hld/sto; \
	  ${MAKE} ddk_rel

HLD_IMGDEC:
	cd hld/imgdec; \
	  ${MAKE}
HLD_IMGDEC_:
	cd hld/imgdec; \
	  ${MAKE} clean
HLD_IMGDEC__:
	cd hld/imgdec; \
	  ${MAKE} ddk_rel

HLD_BDMA:
	cd hld/bdma; \
	  ${MAKE}
HLD_BDMA_:
	cd hld/bdma; \
	  ${MAKE} clean
HLD_BDMA__:
	cd hld/bdma; \
	  ${MAKE} ddk_rel	  

HLD_NIM:
	cd hld/nim; \
	  ${MAKE}
HLD_NIM_:
	cd hld/nim; \
	  ${MAKE} clean
HLD_NIM__:
	cd hld/nim; \
	  ${MAKE} ddk_rel

HLD_HDMI:
	cd hld/hdmi; \
	  ${MAKE}
HLD_HDMI_:
	cd hld/hdmi; \
	  ${MAKE} clean
HLD_HDMI__:
	cd hld/hdmi; \
	  ${MAKE} ddk_rel
	  
HLD_NIM_ISDBT:
	cd hld/nim/isdbt; \
	  ${MAKE}
HLD_NIM_ISDBT_:
	cd hld/nim/isdbt; \
	  ${MAKE} clean
HLD_NIM_ISDBT__:
	cd hld/nim/isdbt; \
	  ${MAKE} ddk_rel

HLD_DMX:
	cd hld/dmx; \
	  ${MAKE}
HLD_DMX_:
	cd hld/dmx; \
	  ${MAKE} clean
HLD_DMX__:
	cd hld/dmx; \
	  ${MAKE} ddk_rel

HLD_DBG:
	cd hld/dbg; \
	  ${MAKE}
HLD_DBG_:
	cd hld/dbg; \
	  ${MAKE} clean
HLD_DBG__:
	cd hld/dbg; \
	  ${MAKE} ddk_rel
	  
HLD_AVSYNC:
	cd hld/avsync; \
	  ${MAKE}
HLD_AVSYNC_:
	cd hld/avsync; \
	  ${MAKE} clean
HLD_AVSYNC__:
	cd hld/avsync; \
	  ${MAKE} ddk_rel

HLD_DECV:
	cd hld/decv; \
	  ${MAKE}
HLD_DECV_:
	cd hld/decv; \
	  ${MAKE} clean
HLD_DECV__:
	cd hld/decv; \
	  ${MAKE} ddk_rel

HLD_GE:
	cd hld/ge; \
	  ${MAKE}
HLD_GE_:
	cd hld/ge; \
	  ${MAKE} clean
HLD_GE__:
	cd hld/ge; \
	  ${MAKE} ddk_rel	  

HLD_VP:
	cd hld/dis; \
	  ${MAKE}
HLD_VP_:
	cd hld/dis; \
	  ${MAKE} clean
HLD_VP__:
	cd hld/dis; \
	  ${MAKE} ddk_rel

HLD_VBI:
	cd hld/vbi; \
	  ${MAKE}
HLD_VBI_:
	cd hld/vbi; \
	  ${MAKE} clean
HLD_VBI__:
	cd hld/vbi; \
	  ${MAKE} ddk_rel

HLD_DECA:
	cd hld/deca; \
	  ${MAKE}
HLD_DECA_:
	cd hld/deca; \
	  ${MAKE} clean
HLD_DECA__:
	cd hld/deca; \
	  ${MAKE} ddk_rel

HLD_SND:
	cd hld/snd; \
	  ${MAKE}
HLD_SND_:
	cd hld/snd; \
	  ${MAKE} clean
HLD_SND__:
	cd hld/snd; \
	  ${MAKE} ddk_rel

HLD_OSD:
	cd hld/osd; \
	  ${MAKE}
HLD_OSD_:
	cd hld/osd; \
	  ${MAKE} clean
HLD_OSD__:
	cd hld/osd; \
	  ${MAKE} ddk_rel

HLD_CIC:
	cd hld/cic; \
	  ${MAKE}
HLD_CIC_:
	cd hld/cic; \
	  ${MAKE} clean
HLD_CIC__:
	cd hld/cic; \
	  ${MAKE} ddk_rel
	  
HLD_SDEC:
	cd hld/sdec; \
	  ${MAKE}
HLD_SDEC_:
	cd hld/sdec; \
	  ${MAKE} clean
HLD_SDEC__:
	cd hld/sdec; \
	  ${MAKE} ddk_rel

HLD_PAN:
	cd hld/pan; \
	  ${MAKE}
HLD_PAN_:
	cd hld/pan; \
	  ${MAKE} clean
HLD_PAN__:
	cd hld/pan; \
	  ${MAKE} ddk_rel

HLD_SOC:
	cd hld/soc; \
	  ${MAKE}
HLD_SOC_:
	cd hld/soc; \
	  ${MAKE} clean
HLD_SOC__:
	cd hld/soc; \
	  ${MAKE} ddk_rel
	  	  
HLD_SCART:
	cd hld/scart; \
	  ${MAKE}
HLD_SCART_:
	cd hld/scart; \
	  ${MAKE} clean
HLD_SCART__:
	cd hld/scart; \
	  ${MAKE} ddk_rel

HLD_SMC:
	cd hld/smc; \
	  ${MAKE}
HLD_SMC_:
	cd hld/smc; \
	  ${MAKE} clean
HLD_SMC__:
	cd hld/smc; \
	  ${MAKE} ddk_rel
	  
HLD_OTP:
	cd hld/otp; \
	  ${MAKE}
HLD_OTP_:
	cd hld/otp; \
	  ${MAKE} clean
HLD_OTP__:
	cd hld/otp; \
	  ${MAKE} ddk_rel
	  
HLD_FAKE:
	cd hld/fake; \
	  ${MAKE}
HLD_FAKE_:
	cd hld/fake; \
	  ${MAKE} clean
HLD_FAKE__:
	cd hld/fake; \
	  ${MAKE} ddk_rel

HLD_TRNG:
	cd hld/trng; \
	  ${MAKE}
HLD_TRNG_:
	cd hld/trng; \
	  ${MAKE} clean
HLD_TRNG__:
	cd hld/trng; \
	  ${MAKE} ddk_rel
	  
HLD_CE:
	cd hld/ce; \
	  ${MAKE}
HLD_CE_:
	cd hld/ce; \
	  ${MAKE} clean
HLD_CE__:
	cd hld/ce; \
	  ${MAKE} ddk_rel

HLD_DSC:
	cd hld/dsc; \
	  ${MAKE}
HLD_DSC_:
	cd hld/dsc; \
	  ${MAKE} clean
HLD_DSC__:
	cd hld/dsc; \
	  ${MAKE} ddk_rel

HLD_WIFI:
	cd hld/wifi; \
	  ${MAKE}
HLD_WIFI_:
	cd hld/wifi; \
	  ${MAKE} clean
HLD_WIFI__:
	cd hld/wifi; \
	  ${MAKE} ddk_rel

LIB_NETUPG:
	cd lib/libapplet/libnetupg/;  \
	  ${MAKE}
LIB_NETUPG_:
	cd lib/libapplet/libnetupg/;  \
	  ${MAKE} clean
LIB_NETUPG__:
	cd lib/libapplet/libnetupg/;  \
	  ${MAKE} ddk_rel

LIB_PUB:
	cd lib/libapplet/libpub/;  \
	  ${MAKE}
LIB_PUB_:
	cd lib/libapplet/libpub/;  \
	  ${MAKE} clean
LIB_PUB__:
	cd lib/libapplet/libpub/;  \
	  ${MAKE} ddk_rel

LIB_DVTCA:
	cd lib/libcas/dvt; \
	  ${MAKE}
LIB_DVTCA_:
	cd lib/libcas/dvt; \
	  ${MAKE} clean
LIB_DVTCA__:
	cd lib/libcas/dvt; \
	  ${MAKE} ddk_rel	  

LIB_DVNCA:
	cd lib/libcas/dvn; \
	  ${MAKE}
LIB_DVNCA_:
	cd lib/libcas/dvn; \
	  ${MAKE} clean
LIB_DVNCA__:
	cd lib/libcas/dvn; \
	  ${MAKE} ddk_rel	 

LIB_CDCA3:
	cd lib/libcas/cdca30a; \
	  ${MAKE}
LIB_CDCA3_:
	cd lib/libcas/cdca30a; \
	  ${MAKE} clean
LIB_CDCA3__:
	cd lib/libcas/cdca30a; \
	  ${MAKE} ddk_rel

LIB_MGCA :
	cd lib/libcas/mg; \
	  ${MAKE}

LIB_MGCA_ :
	cd lib/libcas/mg; \
	  ${MAKE} clean
	  
LIB_MGCA__ :
	cd lib/libcas/mg; \
	  ${MAKE} ddk_rel	  
	  	  
LIB_ASH:
	cd lib/libapplet/libash; \
	  ${MAKE}
LIB_ASH_:
	cd lib/libapplet/libash; \
	  ${MAKE} clean
LIB_ASH__:
	cd lib/libapplet/libash; \
	  ${MAKE} ddk_rel 

LIB_LOADER:
	cd lib/libloader; \
	  ${MAKE}
LIB_LOADER_:
	cd lib/libloader; \
	  ${MAKE} clean
LIB_LOADER__:
	cd lib/libloader; \
	  ${MAKE} ddk_rel 

LIB_UPG:
	cd lib/libapplet/libupg; \
	  ${MAKE}
LIB_UPG_:
	cd lib/libapplet/libupg; \
	  ${MAKE} clean
LIB_UPG__:
	cd lib/libapplet/libupg; \
	  ${MAKE} ddk_rel

LIB_UPG4:
	cd lib/libapplet/libupg4; \
	  ${MAKE}
LIB_UPG4_:
	cd lib/libapplet/libupg4; \
	  ${MAKE} clean
LIB_UPG4__:
	cd lib/libapplet/libupg4; \
	  ${MAKE} ddk_rel
	  
LIB_USBUPG:
	cd lib/libapplet/libusbupg; \
	  ${MAKE}
LIB_USBUPG_:
	cd lib/libapplet/libusbupg; \
	  ${MAKE} clean
LIB_USBUPG__:
	cd lib/libapplet/libusbupg; \
	  ${MAKE} ddk_rel


LIB_GNUZIP:
	cd lib/libcommon/libzip/gnuzip; \
	  ${MAKE}
LIB_GNUZIP_:
	cd lib/libcommon/libzip/gnuzip; \
	  ${MAKE} clean
LIB_GNUZIP__:
	cd lib/libcommon/libzip/gnuzip; \
	  ${MAKE} ddk_rel

LIB_LZMA:
	cd lib/libcommon/libzip/lzma; \
	  ${MAKE}
LIB_LZMA_:
	cd lib/libcommon/libzip/lzma; \
	  ${MAKE} clean
LIB_LZMA__:
	cd lib/libcommon/libzip/lzma; \
	  ${MAKE} ddk_rel

LIB_LIB7ZIP:
	cd lib/libcommon/libzip/lib7zip; \
	  ${MAKE}
LIB_LIB7ZIP_:
	cd lib/libcommon/libzip/lib7zip; \
	  ${MAKE} clean
LIB_LIB7ZIP__:
	cd lib/libcommon/libzip/lib7zip; \
	  ${MAKE} ddk_rel

LIB_UN7ZIP:
	cd lib/libcommon/libzip/un7zip; \
	  ${MAKE}
LIB_UN7ZIP_:
	cd lib/libcommon/libzip/un7zip; \
	  ${MAKE} clean
LIB_UN7ZIP__:
	cd lib/libcommon/libzip/un7zip; \
	  ${MAKE} ddk_rel

LIB_UNLZO:
	cd lib/libcommon/libzip/unlzo; \
	  ${MAKE}
LIB_UNLZO_:
	cd lib/libcommon/libzip/unlzo; \
	  ${MAKE} clean
LIB_UNLZO__:
	cd lib/libcommon/libzip/unlzo; \
	  ${MAKE} ddk_rel

LIB_CHAR:
	cd lib/libcommon/libchar; \
	  ${MAKE}
LIB_CHAR_:
	cd lib/libcommon/libchar; \
	  ${MAKE} clean
LIB_CHAR__:
	cd lib/libcommon/libchar; \
	  ${MAKE} ddk_rel

LIB_DBC:
	cd lib/libdvb/libdbc; \
	  ${MAKE}
LIB_DBC_:
	cd lib/libdvb/libdbc; \
	  ${MAKE} clean
LIB_DBC__:
	cd lib/libdvb/libdbc; \
	  ${MAKE} ddk_rel
  
LIB_SI_ENGINE:
	cd lib/libdvb/libsi/engine; \
	  ${MAKE}
LIB_SI_ENGINE_:
	cd lib/libdvb/libsi/engine; \
	  ${MAKE} clean
LIB_SI_ENGINE__:
	cd lib/libdvb/libsi/engine; \
	  ${MAKE} ddk_rel

LIB_SI_ENGINE_SPL:
	cd lib/libdvb/libsi/engine_spl; \
	  ${MAKE}
LIB_SI_ENGINE_SPL_:
	cd lib/libdvb/libsi/engine_spl; \
	  ${MAKE} clean
LIB_SI_ENGINE_SPL__:
	cd lib/libdvb/libsi/engine_spl; \
	  ${MAKE} ddk_rel

LIB_SI_PLUGIN_SEARCH:
	cd lib/libdvb/libsi/plugin/search; \
	  ${MAKE}
LIB_SI_PLUGIN_SEARCH_:
	cd lib/libdvb/libsi/plugin/search; \
	  ${MAKE} clean
LIB_SI_PLUGIN_SEARCH__:
	cd lib/libdvb/libsi/plugin/search; \
	  ${MAKE} ddk_rel

LIB_SI_PLUGIN_MONITOR:
	cd lib/libdvb/libsi/plugin/monitor; \
	  ${MAKE}
LIB_SI_PLUGIN_MONITOR_:
	cd lib/libdvb/libsi/plugin/monitor; \
	  ${MAKE} clean
LIB_SI_PLUGIN_MONITOR__:
	cd lib/libdvb/libsi/plugin/monitor; \
	  ${MAKE} ddk_rel

LIB_SI_PLUGIN_TDT:
	cd lib/libdvb/libsi/plugin/tdt; \
	  ${MAKE}
LIB_SI_PLUGIN_TDT_:
	cd lib/libdvb/libsi/plugin/tdt; \
	  ${MAKE} clean
LIB_SI_PLUGIN_TDT__:
	cd lib/libdvb/libsi/plugin/tdt; \
	  ${MAKE} ddk_rel

LIB_SI_FILTER:
	cd lib/libdvb/libsi/filter; \
	  ${MAKE}
LIB_SI_FILTER_:
	cd lib/libdvb/libsi/filter; \
	  ${MAKE} clean
LIB_SI_FILTER__:
	cd lib/libdvb/libsi/filter; \
	  ${MAKE} ddk_rel

LIB_SI_PARSER:
	cd lib/libdvb/libsi/parser; \
	  ${MAKE}
LIB_SI_PARSER_:
	cd lib/libdvb/libsi/parser; \
	  ${MAKE} clean
LIB_SI_PARSER__:
	cd lib/libdvb/libsi/parser; \
	  ${MAKE} ddk_rel

LIB_EPG:
	cd lib/libdvb/libepg; \
	  ${MAKE}
LIB_EPG_:
	cd lib/libdvb/libepg; \
	  ${MAKE} clean
LIB_EPG__:
	cd lib/libdvb/libepg; \
	  ${MAKE} ddk_rel

LIB_NVOD:
	cd lib/libdvb/libnvod; \
	  ${MAKE}
LIB_NVOD_:
	cd lib/libdvb/libnvod; \
	  ${MAKE} clean
LIB_NVOD__:
	cd lib/libdvb/libnvod; \
	  ${MAKE} ddk_rel

LIB_CHUNK:
	cd lib/libstorage/libchunk; \
	  ${MAKE}
LIB_CHUNK_:
	cd lib/libstorage/libchunk; \
	  ${MAKE} clean
LIB_CHUNK__:
	cd lib/libstorage/libchunk; \
	  ${MAKE} ddk_rel

LIB_DB5:
	cd lib/libstorage/libdb/libdb5; \
	  ${MAKE}
LIB_DB5_:
	cd lib/libstorage/libdb/libdb5; \
	  ${MAKE} clean
LIB_DB5__:
	cd lib/libstorage/libdb/libdb5; \
	  ${MAKE} ddk_rel

LIB_DB4:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE}
LIB_DB4_:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} clean
LIB_DB4__:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} ddk_rel

LIB_DB4_T:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_t
LIB_DB4_T_:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_t clean
LIB_DB4_T__:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_t ddk_rel

LIB_DB4_ISDBT:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_isdbt
LIB_DB4_ISDBT_:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_isdbt clean
LIB_DB4_ISDBT__:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_isdbt ddk_rel

LIB_DB4_ATSC:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_atsc
LIB_DB4_ATSC_:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_atsc clean
LIB_DB4_ATSC__:
	cd lib/libstorage/libdb/libdb4; \
	  ${MAKE} -f Makefile_atsc ddk_rel
 	  
VERSION_INFO:
	cd version/; \
	  ${MAKE}
VERSION_INFO_:
	cd version/; \
	  ${MAKE} clean

LIB_FLIST:
	cd lib/libapplet/libflist; \
	  ${MAKE}
LIB_FLIST_:
	cd lib/libapplet/libflist; \
	  ${MAKE} clean
LIB_FLIST__:
	cd lib/libapplet/libflist; \
	  ${MAKE} ddk_rel

LIB_PVR3_ENG:
	cd lib/libapplet/libpvr3/eng; \
	  ${MAKE}
LIB_PVR3_ENG_:
	cd lib/libapplet/libpvr3/eng; \
	  ${MAKE} clean
LIB_PVR3_ENG__:
	cd lib/libapplet/libpvr3/eng; \
	  ${MAKE} ddk_rel

LIB_PVR3_MGR:
	cd lib/libapplet/libpvr3/mgr; \
	  ${MAKE}
LIB_PVR3_MGR_:
	cd lib/libapplet/libpvr3/mgr; \
	  ${MAKE} clean
LIB_PVR3_MGR__:
	cd lib/libapplet/libpvr3/mgr; \
	  ${MAKE} ddk_rel

LIB_PVR3_API36:
	cd lib/libapplet/libpvr3/m36; \
	  ${MAKE}
LIB_PVR3_API36_:
	cd lib/libapplet/libpvr3/m36; \
	  ${MAKE} clean
LIB_PVR3_API36__:
	cd lib/libapplet/libpvr3/m36; \
	  ${MAKE} ddk_rel

LIB_GE:
	cd lib/libui/libge; \
	  ${MAKE}
LIB_GE_:
	cd lib/libui/libge; \
	  ${MAKE} clean
LIB_GE__:
	cd lib/libui/libge; \
	  ${MAKE} ddk_rel

LIB_AUI:
	cd lib/libui/libaui; \
	  ${MAKE} -f Makefile.cmd
LIB_AUI_:
	cd lib/libui/libaui; \
	  ${MAKE} -f Makefile.cmd clean
LIB_AUI__:
	cd lib/libui/libaui; \
	  ${MAKE} -f Makefile.cmd

LIB_SURFACE_2D:
	cd lib/libui/libsurface2d; \
	  ${MAKE} 
LIB_SURFACE_2D_:
	cd lib/libui/libsurface2d; \
	  ${MAKE} clean
LIB_SURFACE_2D__:
	cd lib/libui/libsurface2d; \
	  ${MAKE} ddk_rel

HLD_MISC_IPC:
	cd hld/misc/ipc;\
	 ${MAKE}
HLD_MISC_IPC_:
	cd hld/misc/ipc;\
	 ${MAKE} clean	
HLD_MISC_IPC__:
	cd hld/misc/ipc;\
	 ${MAKE} ddk_rel
	  
HLD_MISC_RFK:
	cd hld/misc/rfk;\
	 ${MAKE}
HLD_MISC_RFK_:
	cd hld/misc/rfk;\
	 ${MAKE} clean	
HLD_MISC_RFK__:
	cd hld/misc/rfk;\
	 ${MAKE} ddk_rel

CRYPTON:
	cd lib/libcas/crypton; \
	 ${MAKE}	
CRYPTON_:
	cd lib/libcas/crypton; \
	 ${MAKE} clean	

LIB_CONAX7:
	cd lib/libcas/conax7; \
	 ${MAKE}	
LIB_CONAX7_:
	cd lib/libcas/conax7; \
	 ${MAKE} clean
	 
LIB_AD:
	cd lib/libad; \
	$(MAKE)
LIB_AD_:
	cd lib/libad; \
	${MAKE} clean
LIB_AD__:
	cd lib/libad; \
	${MAKE} ddk_rel

HLD_SURFACE_2D:
	cd hld/surface2d; \
	  ${MAKE}
HLD_SURFACE_2D_:
	cd hld/surface2d; \
	  ${MAKE} clean
HLD_SURFACE_2D__:
	cd hld/surface2d; \
	  ${MAKE} ddk_rel

TEST_CUINT:
	cd test/cunit; \
	  ${MAKE} 
TEST_CUINT_:
	cd test/cunit; \
	  ${MAKE} clean 
TEST_CUINT__:
	cd test/cunit; \
	  ${MAKE} ddk_rel

LIB_CUNITCORE:
	cd lib/libcunit; \
	  ${MAKE} 
LIB_CUNITCORE_:
	cd lib/libcunit; \
	  ${MAKE} clean 
LIB_CUNITCORE__:
	cd lib/libcunit; \
	  ${MAKE} ddk_rel


HLD_ENCODER:
	cd hld/encoder; \
	  ${MAKE}
HLD_ENCODER_:
	cd hld/encoder; \
	  ${MAKE} clean
HLD_ENCODER__:
	cd hld/encoder; \
	  ${MAKE} ddk_rel


