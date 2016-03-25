/***************************************************************************************
*    Ali Corp. All Rights Reserved. 2008 Copyright (C)
*
*    File:    if.h
*
*    Description:     Global definitions for the INET interface module.
*    History:
*           Date				Athor		Version          Reason
*	    ==================================	=================
*	1.	02.25.2007		Mao Feng      Ver 0.1		Create file.
*	
***************************************************************************************/

#ifndef _ETHETNET_IF_H
#define _ETHETNET_IF_H


#include "basic_types.h"
//#include <api/libtcpip/netif/etharp.h>

//standard interface flags (netdevice->multicast_flags).
#define	IFF_Promiscuous				0x10		/* receive all packets		*/
#define	IFF_PassAllMulticast				0x20		/* receive all multicast packets*/
#define	IFF_MODE_MASK					0x0F
#define	IFF_PerfectFilterting				0x00
#define	IFF_HashFilterting_OnePecfect	0x01
#define	IFF_Inverse						0x02
#define	IFF_HashFilterting				0x03


struct mii_ioctl_data {
	UINT16		phy_id;
	UINT16		reg_num;
	UINT16		val_in;
	UINT16		val_out;
};


struct ifreq 
{
	union {
		char *	ifru_data;
	} ifr_ifru;
};

#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/


#endif /* _ETHETNET_IF_H */

