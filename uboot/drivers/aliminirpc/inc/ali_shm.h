/*
 * ali_interrupt.h
 *
 * Copyright (C) 2013 ALi, Inc.
 *
 * Author:
 *	Tony Zhang <tony.zhang@alitech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
	 

#ifndef __ALI_SHM_H
#define __ALI_SHM_H

#if defined(CONFIG_ALI_CHIP_M3921)
	#define ALI_VMEM_BASE   0x00000000
#else
	#define ALI_VMEM_BASE   0xA0000000
#endif

#define __VMEMALI(x)    (((unsigned long)x) | ALI_VMEM_BASE)

#if defined(CONFIG_ALI_CHIP_M3921)
/*Main to SEE Address translation*/
#define __VMTSALI(x)    ((((unsigned long)x) & 0x1FFFFFFF) | 0xA0000000)

/*SEE to Main Address translation*/
#define __VSTMALI(x)    ((((unsigned long)x) & 0x1FFFFFFF) | 0xC0000000)
#else
/*Main to SEE Address translation*/
#define __VMTSALI(x)    ((unsigned long)x)

/*SEE to Main Address translation*/
#define __VSTMALI(x)    ((unsigned long)x)
#endif

typedef struct GLOBAL_PARAM
{
	unsigned long	g_RPCOS;			/*0->means Linux, 1->means TDS*/
	unsigned long	g_RPCRunning;		/* 0->Idle, 1->Running, 2->Initialing, 3->Stopping*/
	unsigned long	g_RPCVersion;
	unsigned long	g_dbglevel;
	unsigned long	g_BigEndian;
	unsigned long   g_local_cpu_id;
	unsigned long   g_remote_cpu_id;
	unsigned long   g_local_node_id;
	unsigned long   g_remote_node_id; 

	unsigned long  g_TotalSendMsgCall;
	unsigned long  g_TotalRcvMsgCall;
	unsigned long  g_TotalRcvMsgRet;
	unsigned long  g_TotalRcvBadMsgPacket;
	unsigned long  g_TotalSendPktCall;
	unsigned long  g_TotalRcvPktCall;
	unsigned long  g_TotalRcvPktRet;
	unsigned long  g_TotalRcvBadPktPacket;
	unsigned long  g_TotalRcvBadFuncIDCall;
	unsigned long  g_TotalService;

}global_param;

extern global_param g_param;

int mcomm_ali_modinit(void);

#endif

