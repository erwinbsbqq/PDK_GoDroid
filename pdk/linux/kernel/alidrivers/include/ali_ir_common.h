#ifndef _ALI_IR_COMMON_H_
#define _ALI_IR_COMMON_H_

/*! @addtogroup Devicedriver 
 *  @{
 */

/*! @addtogroup ALiIR
 *  @{
 */

/* ALI25C01 key code, 60keys & ali new RC */
#define IR_ALI01_HKEY_0			0x60df926d	//!< ALI25C01 key code key0
#define IR_ALI01_HKEY_1			0x60dfc837	//!< ALI25C01 key code key1	
#define IR_ALI01_HKEY_2			0x60df08f7	//!< ALI25C01 key code key2
#define IR_ALI01_HKEY_3			0x60df8877	//!< ALI25C01 key code key3
#define IR_ALI01_HKEY_4			0x60dff00f	//!< ALI25C01 key code key4
#define IR_ALI01_HKEY_5			0x60df30cf	//!< ALI25C01 key code key5
#define IR_ALI01_HKEY_6			0x60dfb04f	//!< ALI25C01 key code key6
#define IR_ALI01_HKEY_7			0x60dfd02f	//!< ALI25C01 key code key7
#define IR_ALI01_HKEY_8			0x60df10ef	//!< ALI25C01 key code key8
#define IR_ALI01_HKEY_9			0x60df906f	//!< ALI25C01 key code key9

#define IR_ALI01_HKEY_LEFT		0x60df38c7	//!< ALI25C01 key code key "left"
#define IR_ALI01_HKEY_RIGHT		0x60df12ed	//!< ALI25C01 key code key "right"
#define IR_ALI01_HKEY_UP		0x60df22dd	//!< ALI25C01 key code key "up"
#define IR_ALI01_HKEY_DOWN		0x60dfb847	//!< ALI25C01 key code key "down"

#define IR_ALI01_HKEY_V_UP		0X60df28d7	//!< ALI25C01 key code key "volume up"
#define IR_ALI01_HKEY_V_DOWN	0x60df48b7	//!< ALI25C01 key code key "volume down"
//no channel key, use pre ,next
#define IR_ALI01_HKEY_C_UP		0x60df20df	//!< ALI25C01 key code key "previous"
#define IR_ALI01_HKEY_C_DOWN	0x60df0af5	//!< ALI25C01 key code key "next"
#define IR_ALI01_HKEY_ENTER		0x60df3ac5	//!< ALI25C01 key code key "enter"
#define IR_ALI01_HKEY_P_UP		0x60dfd22d	//!< ALI25C01 key code key "page up"
#define IR_ALI01_HKEY_P_DOWN	0x60dfe01f	//!< ALI25C01 key code key "page down"
#define IR_ALI01_HKEY_TEXT		0x60df827d	//!< ALI25C01 key code key "text"
#define IR_ALI01_HKEY_POWER		0x60df708f	//!< ALI25C01 key code key "power"
#define IR_ALI01_HKEY_PREV		0x60df20df	//!< ALI25C01 key code key 
#define IR_ALI01_HKEY_NEXT		0x60df0af5	//!< ALI25C01 key code key 
#define IR_ALI01_HKEY_AUDIO		0x60df629d	//!< ALI25C01 key code key "audio"
#define IR_ALI01_HKEY_SUBTITLE	0x60df807f	//!< ALI25C01 key code key "subtitle"

#define IR_ALI01_HKEY_NEWS		0x60df50af	//!< MP(60key RC)

#define IR_ALI01_HKEY_VOD		0x60df9867	//!< SLEEP(60key RC)

#define IR_ALI01_HKEY_MAIL		0x60df6a95	//!< PIP(60key RC)
#define IR_ALI01_HKEY_LOCALPLAY	0x60df7887	//!< ALI25C01 key code key "local play"

#define IR_ALI01_HKEY_FIND		0x60dfe21d	//!< ALI25C01 key code key "find"
#define IR_ALI01_HKEY_MUTE		0x60dfa05f	//!< ALI25C01 key code key "mute"
#define IR_ALI01_HKEY_SLEEP		0x60df9867	//!< ALI25C01 key code key "sleep"
#define IR_ALI01_HKEY_PAUSE		0x60df7a85	//!< ALI25C01 key code key "pause"
#define IR_ALI01_HKEY_INFOR		0x60df6897	//!< ALI25C01 key code key "info"
#define IR_ALI01_HKEY_EXIT		0x60df42bd	//!< ALI25C01 key code key "exit"
#define IR_ALI01_HKEY_TVSAT		0x60df52ad	//!< ALI25C01 key code key "tvsat"
#define IR_ALI01_HKEY_TVRADIO	0x60df02fd	//!< ALI25C01 key code key "TV Radio"
#define IR_ALI01_HKEY_FAV		0x60dfc23d	//!< ALI25C01 key code key "FAV"
#define IR_ALI01_HKEY_ZOOM		0x60df40bf	//!< ALI25C01 key code key "zoom"
#define IR_ALI01_HKEY_EPG		0x60df00ff	//!< ALI25C01 key code key "EPG"
#define IR_ALI01_HKEY_MENU		0x60df2ad5	//!< ALI25C01 key code key "menu"
#define IR_ALI01_HKEY_RECALL	0x60dfc03f	//!< ALI25C01 key code key "recall"

#define IR_ALI01_HKEY_RED		0x60df609f	//!< ALI25C01 key code key "red"
#define IR_ALI01_HKEY_GREEN		0x60df7887	//!< ALI25C01 key code key "green"
#define IR_ALI01_HKEY_YELLOW	0x60dff807	//!< ALI25C01 key code key "yellow"
#define IR_ALI01_HKEY_BLUE		0x60dfba45	//!< ALI25C01 key code key "blue"

#define IR_ALI01_HKEY_V_FORMAT 	0x60dfa25d	//!< ALI25C01 key code key "video format"
#define IR_ALI01_HKEY_PLAY		0x60df18e7	//!< ALI25C01 key code key "play"
#define IR_ALI01_HKEY_STOP		0x60dfe817	//!< ALI25C01 key code key "stop"

#define IR_ALI01_HKEY_RECORD        0x60dfa857	//!< ALI25C01 key code key "record"
#define IR_ALI01_HKEY_PVR_INFO		0x60dfca35	//!< ALI25C01 key code key "PVR info" 	
#define IR_ALI01_HKEY_FILELIST		0x60dfb24d	//!< ALI25C01 key code key "file list"	
#define IR_ALI01_HKEY_DVRLIST		0x60df8a75	//!< ALI25C01 key code key "DVR list"	
#define IR_ALI01_HKEY_USBREMOVE		0x60df1ae5	//!< ALI25C01 key code key "usb remove"	
#define IR_ALI01_HKEY_PIP_LIST		0x60df9a65	//!< ALI25C01 key code key "PIP list"	
#define IR_ALI01_HKEY_PIP			0x60df6a95	//!< ALI25C01 key code key "PIP"
#define IR_ALI01_HKEY_SWAP			0x60df5aa5	//!< ALI25C01 key code key "swap"	
#define IR_ALI01_HKEY_MOVE			0x60dfda25	//!< ALI25C01 key code key "move"	
#define IR_ALI01_HKEY_REPEATAB		0x60dfea15	//!< ALI25C01 key code key "repeatab"
#define IR_ALI01_HKEY_FB			0x60df58a7	//!< ALI25C01 key code key "frame back"	
#define IR_ALI01_HKEY_FF		    0x60dfd827	//!< ALI25C01 key code key "frame forward"
#define IR_ALI01_HKEY_SLOW  		0x60dfaa55	//!< ALI25C01 key code key "slow"
#define IR_ALI01_HKEY_REVSLOW  		0x60df4ab5	//!< ALI25C01 key code key "REVSLOW"

/* add Keys and buttons for 3921 , begin , cari chen */

#define KEY_STB				0x220	//!< key "STB"
#define KEY_FORMAT			0x221	//!< key "FORMAT"
#define KEY_FAVUP			0x222	//!< key "FAVUP"
#define KEY_FAVDOWN			0x223	//!< key "FAVDOWN"
#define KEY_DVRLIST			0x224	//!< key "DVRLIST"
#define KEY_REVSLOW			0x225	//!< key "REVSLOW"
#define KEY_DVR_INFO		0x226	//!< key "DVR_INFO"
#define KEY_PIP				0x227	//!< key "PIP"
#define KEY_USBREMOVE		0x228	//!< key "USBREMOVE"
#define KEY_PIP_LIST		0x229	//!< key "PIP_LIST"
#define KEY_SWAP			0x22a	//!< key "SWAP"
#define KEY_STOCK			0x22b	//!< key "STOCK"
#define KEY_NVOD			0x22c	//!< key "NVOD"

/* add Keys and buttons for 3921 , end , cari chen */
/*! @struct linux_ir_key_map_t
 @brief This structure defines map relationship between physics key and logic key.
 */
struct linux_ir_key_map_t
{
	unsigned long ir_code;//!< Physics key
	unsigned short key;//!< Logic key
};

/*!
@}
*/

/*!
@}
*/

#endif
