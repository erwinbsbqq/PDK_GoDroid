
#ifndef __GMAC__UTIL_H
#define __GMAC__UTIL_H

#include "eth_gmac.h"

#define MAX_TOE_PKT_LEN	(64*1024)

#define MAC_FRAME_IP_TCP	0
#define MAC_FRAME_IP_UDP	1
#define MAC_FRAME_IPv6_TCP	2
#define MAC_FRAME_IPv6_UDP	3

#define ETH_IP			0x0800
#define ETH_IPv6		0x86DD

#define MAC_HDR_LEN		14
#define IP_IPSRC_OFF	12
#define IP_IPDES_OFF	16
#define IPv6_IPSRC_OFF	8
#define IPv6_IPDES_OFF	24
#define IPv6_NXT_HDR	40

struct ali_mac_priv_io {
    unsigned short  reg;
    unsigned long  value;
};

struct ali_mac_xmit_io {
	unsigned char  tx_rx; 
	unsigned char  type;   //0: tcpv4, 1: udpv4, 2: tcpv6, 3: udpv6
	unsigned char  toe_tx; //0: disable, 1: enable
	unsigned char  tso_ufo;

	unsigned long  len;    

	unsigned long max_len;

	unsigned short  mtu;    
	unsigned short  max_mtu;
	
	unsigned short  desc_min; 
	unsigned short  desc_max; 
	
	unsigned short vlan;   
	unsigned short  reserve_1;
	
	unsigned long  repeat;   

	unsigned char  dest_type; 
	unsigned char  hop_hd_len;   
	unsigned char  dst_hd_len;   
	unsigned char  rt_hd_len;    

	unsigned char  dst_2_hd_len;
	unsigned char  reserve_2;
	unsigned short reserve_3;
};

void start_xmit_test(pgmac_private pgmac, struct ali_mac_xmit_io* cmd);
#endif //_GMAC_UTIL_H_

