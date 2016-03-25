
#ifndef __SYS_CONFIG_QT_MEDIA_PLAYER_H__
#define __SYS_CONFIG_QT_MEDIA_PLAYER_H__
//Warning: Do not modify this file by yourselfy.
//It will be changed automaticly by script : gencompiler.sh

#if 0
//Used to fast boot, espacially in AS mode.
/*
QT Mediaplayer Object:

QT_MEDIAPLA_LIBS += -lomxil-bellagio -lavformat -lavutil -lavcodec -lavdevice -lswscale
QT_MEDIAPLA_LIBS += -lQtGui -lQtCore -lQtNetwork -lomx_media_player -lqt_media_player

we only need to dlopen: liblqt_media_player.so
*/
#ifndef DLOPEN_DYNAMIC_LIBRARY_SUPPORT
#define DLOPEN_DYNAMIC_LIBRARY_SUPPORT
#endif
#define QT_MEDIA_PLAYER_SUPPORT
#define LIBQT_MODULE_SUPPORT

//#define QT_ALI_OPENVG_SUPPORT

#else

#undef QT_MEDIA_PLAYER_SUPPORT

#endif

#endif
