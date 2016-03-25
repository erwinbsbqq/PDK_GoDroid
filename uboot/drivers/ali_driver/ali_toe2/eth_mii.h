/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: ethernet_mii.h
 *
 *  Description: head file for ethernet mac mii management.
 *
 *  History:
 *      Date		Author		Version		Comment
 *      ====		======		=======	=======
 *  1.  2008.02.25  Mao Feng		0.1			Create file.
 *
 ****************************************************************************/

#ifndef _ETHERNET_MII_H_
#define _ETHERNET_MII_H_

#include "types_def.h"
#include "net.h"
#include "ethtool.h"
#include "if.h"
#include "net_dev.h"

struct mii_if_info {
	UINT32 phy_id;
	UINT16 advertising;
	UINT16 phy_id_mask;
	UINT16 reg_num_mask;

	UINT32 full_duplex : 1;		/* is full duplex? */
	UINT32 force_media : 1;	/* is autoneg. disabled? */

	struct net_device *dev;
};

struct ethtool_ops {
	RET_CODE (*get_settings)(struct net_device *, struct ethtool_cmd *);
	RET_CODE (*set_settings)(struct net_device *, struct ethtool_cmd *);
	BOOL (*get_link)(struct net_device *);
	RET_CODE (*nway_reset)(struct net_device *);
	void (*get_mac_stats)(struct net_device *, mac_device_stats *);
};


RET_CODE mii_get_settings(struct mii_if_info *mii, struct ethtool_cmd *ecmd);
RET_CODE mii_set_settings(struct mii_if_info *mii, struct ethtool_cmd *ecmd);

UINT16 mii_nway_result (UINT16 negotiated);
RET_CODE mii_nway_restart (struct mii_if_info *mii);
BOOL mii_link_ok (struct mii_if_info *mii);
RET_CODE mii_ioctl(struct mii_if_info *mii_if, struct mii_ioctl_data *mii_data, UINT16 cmd, UINT32 *duplex_chg_out);

#endif //_ETHERNET_MII_H_

