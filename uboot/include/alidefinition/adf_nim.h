/*******************************************************************************

File name   : adf_nim.h

Description : nim driver LINUX platform OS  API header file

Author      : kent.kang

Create date : Jan 20, 2014

COPYRIGHT (C) ALi Corporation 2013

Revision history:
--------------------------------------------------------------------------------
Date                        Modification           Revision              Name
-------------               ------------          ----------         -----------
Jan 20, 2014                   Created               V0.1             kent.kang

*******************************************************************************/

/* Define to prevent recursive inclusion */

#ifndef __ALI_ADFNIM_H__
#define __ALI_ADFNIM_H__


enum nim_dvbc_mode
{
    NIM_DVBC_J83AC_MODE  = 0x00,
	NIM_DVBC_J83B_MODE  = 0x01,
};

/*!@enum nim_sample_clk 
@brief nim sample clock select
*/
enum nim_sample_clk
{
    NIM_SAMPLE_CLK_27M  = 0x00,
    NIM_SAMPLE_CLK_54M  = 0x10,
};



#endif /* __ALI_ADFNIM_H__ */
