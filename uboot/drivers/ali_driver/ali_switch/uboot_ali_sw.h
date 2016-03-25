/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *	
 *	this file should contain the prototype of the data structure
 *	things like STP, IGMP.... should be defined here.
 *
 *
 */
#ifndef _ALI_SW_PRIVATE_H
#define _ALI_SW_PRIVATE_H

#include "uboot_ali_sw_types.h"
#include "uboot_ali_sw_p0.h"

#define DRV_NAME "ALI_SW"
#define DRV_VER "Ver 0.1"

#define ALI_SW_HASH_BITS 	8
#define ALI_SW_HASH_SIZE 	(1 << ALI_SW_HASH_BITS)

#define ALI_SW_PORT_BITS	2
//#define ALI_SW_MAX_PORTS	(1<<ALI_SW_PORT_BITS)
#define MAX_PORT_NUM 4

#define ALI_SW_HOLD_TIME 	(1*HZ)

#define ALI_SW_DEBUG_TRACE
#define ALI_SW_DEBUG_WARN
//#define ALI_SW_DEBUG_INFO
#define ALI_SW_DEBUG_ERROR

#define ALI_SW_DEBUG_IPV6_UDP
#define ALI_SW_DEBUG1

#ifdef ALI_SW_DEBUG_INFO
#	define ali_info(msg,args...) \
		printf(msg "\n", ## args)
#else
#	define ali_info(msg,args...)
#endif


#ifdef ALI_SW_DEBUG_WARN
#	define ali_warn(msg,args...) \
		printf(msg "\n", ## args)
#else
#	define ali_warn(msg,args...)
#endif

#ifdef ALI_SW_DEBUG_ERROR
#	define ali_error(msg,args...) \
		printf(msg "\n", ## args)
#else
#	define ali_error(msg,args...)
#endif

#define ALI_SW_TRACE

#ifdef ALI_SW_TRACE
#define ali_trace(level, msg, args...) \
   {if (test_bit(level, &dbg_runtime_val)) {printf(msg "\n", ##args);}}
#else
#define ali_trace(level, msg, args...)
#endif 

typedef struct mac_addr 	mac_addr;

struct ali_sw_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

struct ali_sw_ip
{
	union {
		__be32	ip4;
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
		struct in6_addr ip6;
#endif
	} u;
	__be16		proto;
};

enum PORT_PATH_COST{
	STP_SPEED_10M = 0,
	STP_SPEED_100M = 1,
	STP_SPEED_1000M = 2,
	STP_SPEED_RESERVED = 3,
};


/****************************************************************************
p0_private, net_ali_sw, net_ali_sw_port
	how to organize these three?
	in br*.c, br*.h impliments:
		struct net_bridge contains a list of struct net_bridge_port, 
		struct net_bridge_port contains net_device
	struct net_ali_sw should be just like struct net_bridge,
		it contains 4 ports (struct net_ali_sw_port)
		clearly, we don't need net_device in net_ali_sw_port.
	struct net_ali_sw_port:
		SW maintained member -- id, timer, role, cost, status(duplicated? question)
		HW maintained member -- status, (id? question)
	struct net_ali_sw:
		SW maintained member -- id, timer, role, cost
		HW maintained member -- no?? question
		TX/RX bpdu is through net_ali_sw, so net_ali_sw should contain p0_private or net_device
			TX question, call dev_queue_xmit?????????????????????????????????????????????

we use the simplest way to allocate memory,
	p0_private contains net_ali_sw,
	net_ali_sw contains MAX_PORT_NUM numbers of net_ali_sw_port
	in ali_sw_dev_attach, we call 'alloc_etherdev'.
	maybe we need a global points to p0_private, a global points to net_ali_sw.

question:
	do we need have a copy of ALR in software??????????????????????????????????????????????

The above, we only have discussed impliments of STP,
	what to do about MCS, that's a TBD!!!
	
*****************************************************************************/
enum LINK_STATUS_CHECK{
	LINK_OKAY_TO_TRANSMIT = 1,
	LINK_DISCONNECT = 0,
};

enum LINK_DUPLEX_CHECK{
	LINK_HALF_DUPLEX = 0,
	LINK_FULL_DUPLEX = 1,
};

enum LINK_SPEED_CHECK{
	LINK_SPEED_10M = 0,
	LINK_SPEED_100M = 1,
	LINK_SPEED_1000M = 2,
	LINK_SPEED_RESERVED = 3,
};

struct net_ali_sw_port
{
	p_p0_private 			p_P0;
	struct net_ali_sw		*sw;
	u8					*mac_addr;
	u8 					got_phy:1;
	u8 					phy_rst:1;
	u8 					link_established:1;
	u8 					transmit_okay:1;	/*range in LINK_STATUS_CHECK*/
	u8 					link_spd:2;		/*range in LINK_SPEED_CHECK*/
	u8 					link_dup:1;		/*range in LINK_DUPLEX_CHECK*/
	u8					rx_pause:1;
	u8 					tx_pause:1;
	u16 			    link_partner;	
    u8                  port_no;
};

typedef struct net_ali_sw_port *p_ali_sw_port, ali_sw_port;

enum {
	ALI_SW_NO_STP = 0, 
	ALI_SW_KERNEL_STP = 1,
};

enum {
	ALI_SW_STP_NOT_WORKING = 0, 
	ALI_SW_STP_WORKING = 1,
};

struct sw_txrx_stats{
	u32		rx_runt_pkts_port[MAX_PORT_NUM-1];
	u32		rx_long_frame_port[MAX_PORT_NUM-1];
	u32		rx_crc_err_pkts_port[MAX_PORT_NUM-1];
	u32		rx_alig_err_frame_port[MAX_PORT_NUM-1];
	u32		tx_pkts_port[MAX_PORT_NUM-1];
	//question, no RX stats?
	//half-duplex, TBD.
};

struct net_ali_sw
{
	p_p0_private 			p_P0;
	u8					sw_name[4];
	struct net_ali_sw_port  port_list[MAX_PORT_NUM];
	u8 					ports_link_status;
	struct sw_txrx_stats 	sw_stats;
	
	//the following is for root switch.
	unsigned long			sw_max_age;	//normally, it's 20 secs
	unsigned long			ageing_time;		//fdb, normally 300 secs
	unsigned long			sw_hello_time;
	unsigned long			sw_forward_delay;

	u8					topology_change;
	u8					topology_change_detected;
};
typedef struct net_ali_sw *p_ali_sw, ali_sw;

#define DEBUG_DRIVER_LEVEL			0
#define DISPLAY_MC_SNOOPING_LIST	1
#define DISPLAY_STP_INFO			2
#define START_FAST_ALR_AGING		3
#define STOP_FAST_ALR_AGING		4

typedef struct dbg_setting{
	u8 dbg_no;
	u8 dbg_val;
	u8 dbg_cmd;
} * pdbg_setting;

typedef struct phy_mdio_ctl
{
	u8 port_no;
	u8 phy_reg_addr;
}*pphy_mdio_ctl;

#define WAN_PORT_SELECTION		1
#define ROUTE_FORWARDING_TEST	2
typedef struct _ali_reg_t {
	u32 cmd;
	u32 phy_addr;
	u32 val;
} ali_reg_t;

#define SW_INPUT_SKB_CB(__skb)		((struct ali_sw_input_skb_cb *)(__skb)->cb)
#define SW_OUTPUT_SKB_CB(__skb)	((struct ali_sw_output_skb_cb *)(__skb)->cb)

extern struct net_ali_sw *gp_sw;
extern u32 ali_sw_base;
extern bool ali_sw_check_port_link_status(struct net_ali_sw_port *p);
extern u32 dbg_runtime_val;
#endif
