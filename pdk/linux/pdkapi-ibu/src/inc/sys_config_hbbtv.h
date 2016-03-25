

#ifndef __SYS_CONFIG_HBBTV_H
#define __SYS_CONFIG_HBBTV_H

//Warning: Do not modify this file by yourselfy.
//It will be changed automaticly by script : gencompiler.sh

#if 0
//If hbbyv function is enabled, this lines should be worked
#ifndef HBBTV_MODULE_SUPPORT
#define HBBTV_MODULE_SUPPORT
#endif

#ifndef HBBTV_AIT_ENABLE
#define HBBTV_AIT_ENABLE
#endif

#ifndef HBBTV_DEBUG_WITH_GDB
#define HBBTV_DEBUG_SEGERR_WITH_GDB
#endif

#ifndef DIRECT_FB_OSD_SUPPORT
#define DIRECT_FB_OSD_SUPPORT
#endif

//Solve the time.h bug /./platform/inc/api/libc/time.h
#define USE_EXTERN_STANDARD_INCLUDE 

#ifdef QT_MEDIA_PLAYER_SUPPORT
	#error "Error: QT Media Player cannot be work with hbbtv module."
#endif

//For Web Browser.
#ifndef HBBTV_WEB_BROWSER_ENABLE
//#define HBBTV_WEB_BROWSER_ENABLE
#endif

//For DSM-CC 

//for test only
#ifndef HBBTV_MIT_TESTSUITE_LOCAL_SERVER
//#define HBBTV_MIT_TESTSUITE_LOCAL_SERVER
#endif

#ifndef HBBTV_DSMCC_SECT_ENABLE
// close it, because dsmcc currently not work.
//#define HBBTV_DSMCC_SECT_ENABLE 
#endif
//Used to fast boot, espacially in AS mode.
/*
QT Mediaplayer Object:

QT_MEDIAPLA_LIBS += -lomxil-bellagio -lavformat -lavutil -lavcodec -lavdevice -lswscale
QT_MEDIAPLA_LIBS += -lQtGui -lQtCore -lQtNetwork -lomx_media_player -lqt_media_player

we only need to dlopen: liblqt_media_player.so

* These module will be opened
HbbTV Project:
*  ooif.so : This lib is loaded by opera, but we can try to open it by our self.[TODO, NEED check]..
....TBD
*
*/
#ifndef DLOPEN_DYNAMIC_LIBRARY_SUPPORT
#define DLOPEN_DYNAMIC_LIBRARY_SUPPORT
#endif
#ifndef CHECK_NETWORK_LINK_STATUS_SUPPORT
#define CHECK_NETWORK_LINK_STATUS_SUPPORT
#endif

#ifdef CHECK_NETWORK_LINK_STATUS_SUPPORT

//Enable this, when network hardware is down, a popup message will be shown on screen.
#define SHOW_NETWORK_STATUS_MSG_ON_POPUP_WINDOW_SUPPORT

#endif //CHECK_NETWORK_LINK_STATUS_SUPPORT
#else

//Check macro.
#ifdef HBBTV_MODULE_SUPPORT
	#error "Error: HBBTV_MODULE_SUPPORT has been enabled. it should be undefined"
#endif	

#endif

#endif
