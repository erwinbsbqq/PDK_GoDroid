/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    ethernet_mii.c
*
*    Description:    Mii management.
*
*    History:
*		Date			Athor		Version		Reason
*	    =======================================================
* 1.    25.02.2008	   Mao Feng		Ver 1.0		Original MAC
* 3.	23.04.2009		   		    Ver 2.0		Cost Down MAC
******************************************************************************/
#include <common.h>
#include "types_def.h"

#include "net.h"
#include "ethtool.h"
#include "if.h"
#include "eth_mii.h"

#include "uboot_eth_reg.h"
#include "uboot_eth_toe2.h"


RET_CODE mii_ioctl(struct mii_if_info *mii_if, struct mii_ioctl_data *mii_data, UINT16 cmd, UINT32 *duplex_chg_out)
{
	pmac_adapter padapter = (mac_adapter *)mii_if->dev->priv;
	RET_CODE rc = SUCCESS;
	UINT32 duplex_changed = 0;

	mii_data->phy_id &= mii_if->phy_id_mask;
	mii_data->reg_num &= mii_if->reg_num_mask;

	switch(cmd) {

	case MAC_G_MII_PHY:
		mii_data->phy_id = mii_if->phy_id;

	case MAC_G_MII_REG:
		mii_data->val_out =
			(UINT16)mac_mdio_read(padapter, (UINT32)mii_data->phy_id, (UINT32)mii_data->reg_num);
		break;

	case MAC_S_MII_REG: {
		UINT16 val = mii_data->val_in;

		if (mii_data->phy_id == mii_if->phy_id) {
			switch(mii_data->reg_num) {
			case PhyBasicModeCtrl: {
				UINT32 new_duplex = 0;
				if (val & (BMCRReset|BMCRANEnable))
					mii_if->force_media = 0;
				else
					mii_if->force_media = 1;
				if (mii_if->force_media &&
				    (val & BMCRDuplexMode))
					new_duplex = 1;
				if (mii_if->full_duplex != new_duplex) {
					duplex_changed = 1;
					mii_if->full_duplex = new_duplex;
				}
				break;
			}
			
			case PhyNWayAdvert:
				mii_if->advertising = val;
				break;
				
			default:
				break;
			}
		}

		mac_mdio_write(padapter, (UINT32)mii_data->phy_id, (UINT32)mii_data->reg_num, (UINT32)val);
		break;
	}

	default:
		rc = ERR_FAILURE; //ERR_FAILUE;
		MAC_PRINTF("mii_ioctl: command not supported .\n");
		break;
	}

	if ((rc == 0) && (duplex_chg_out) && (duplex_changed))
		*duplex_chg_out = 1;

	return rc;
}
