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

#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/version.h>
#include "ali_sw_p0.h"
//#include "ali_sw_stp.h"
//#include "ali_sw_mc.h"

#define DRV_NAME "ALI_SW"
#define DRV_VER "Ver 0.1"

#define ALI_SW_HASH_BITS 	8
#define ALI_SW_HASH_SIZE 	(1 << ALI_SW_HASH_BITS)

#define ALI_SW_PORT_BITS	2
//#define ALI_SW_MAX_PORTS	(1<<ALI_SW_PORT_BITS)
#define MAX_PORT_NUM 4

#define ALI_SW_HOLD_TIME 	(1*HZ)

//#define CONFIG_ALi_SW_STP
//#define CONFIG_ALi_MC_SNOOPING

#define ALI_SW_DEBUG_TRACE
#define ALI_SW_DEBUG_WARN
#define ALI_SW_DEBUG_INFO
#define ALI_SW_DEBUG_ERROR


//#define ALI_SW_DEBUG_STP
#define ALI_SW_DEBUG_IPV6_UDP
#define ALI_SW_DEBUG1
//#define FPGA_PHY_MAC_SET

#ifdef ALI_SW_DEBUG_TRACE
#	define sw_trace(msg,args...) \
		printk(KERN_INFO msg "\n", ## args)
#else
#	define sw_trace(msg,args...)
#endif

#ifdef ALI_SW_DEBUG_INFO
#	define sw_info(msg,args...) \
		printk(KERN_INFO msg "\n", ## args)
#else
#	define sw_info(msg,args...)
#endif


#ifdef ALI_SW_DEBUG_WARN
#	define sw_warn(msg,args...) \
		printk(KERN_WARNING msg "\n", ## args)
#else
#	define sw_warn(msg,args...)
#endif

#ifdef ALI_SW_DEBUG_ERROR
#	define sw_error(msg,args...) \
		printk(KERN_ERR msg "\n", ## args)
#else
#	define sw_error(msg,args...)
#endif

#define ALI_SW_TRACE
#ifdef ALI_SW_TRACE
#define ali_trace(level, msg, args...) \
   {if (test_bit(level, &dbg_runtime_val)) {printk(KERN_INFO msg "\n", ##args);}}
#else
#define ali_trace(level, msg, args...)
#endif 

#define ALI_SW_DEBUG_TX_L1
#ifdef ALI_SW_DEBUG_TX_L1
#	define trace_tx_l1(msg,args...) \
		printk(KERN_INFO  msg "\n", ## args)
#else
#	define trace_tx_l1(msg,args...)	
#endif

#if	1
#	define trace_async(msg,args...) \
		printk(msg, ## args)
#else
#	define trace_async(msg,args...)
#endif



#define ALI_SW_STP_DEBUG
#ifdef ALI_SW_DEBUG_TRACE
#	define stp_trace(msg,args...) \
		printk(KERN_INFO msg "\n", ## args)
#else
#	define stp_trace(msg,args...)
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
#define UINT32 unsigned int 
#else
#endif
typedef struct ali_sw_id 	stp_sw_id;
typedef struct mac_addr 	mac_addr;
typedef u16 				stp_port_id;

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

/*
struct net_ali_sw_port_group {
	struct net_ali_sw_port		*port;
	struct net_ali_sw_port_group	*next;
	struct hlist_node			mglist;
	struct rcu_head			rcu;
	struct timer_list			timer;
	struct timer_list			query_timer;
	struct ali_sw_id			addr;
	u32						queries_sent;
};

struct net_ali_sw_mdb_entry
{
	struct hlist_node			hlist[2];
	struct hlist_node			mglist;
	struct net_ali_sw			*sw;
	struct net_ali_sw_mdb_entry	*ports;
	struct rcu_head			rcu;
	struct timer_list			timer;
	struct timer_list			query_timer;
	struct ali_sw_ip			addr;
	u32						queries_sent;
};

struct net_ali_sw_mdb_htable
{
	struct hlist_head				*mhash;
	struct rcu_head				rcu;
	struct net_ali_sw_mdb_htable	*old;
	u32							size;
	u32							max;
	u32							secret;
	u32							ver;
};
*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//STP SW status
#define ALI_SW_STATE_DISABLED		0
#define ALI_SW_STATE_LISTENING		1
#define ALI_SW_STATE_LEARNING		2
#define ALI_SW_STATE_FORWARDING	3
#define ALI_SW_STATE_BLOCKING		4

//STP HW status, HW has one less state than SW status
enum PORT_STP_STATUS{
	STP_STATE_HW_DISABLED = 0,
	STP_STATE_HW_BLOCKING_LISTENING = 1,
	STP_STATE_HW_LEARNING = 2,
	STP_STATE_HW_FORWARDING = 3,
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
	u16 					link_partner;	
	
//-------------------------------------------------------------------------
	/*** STP, should be quite straghtforward ***/
	u8					priority;
	u8					port_no;	
	stp_port_id			port_id;	

	u8					stp_state;		
	u8					topology_change_ack;
	u8					config_pending;

	stp_port_id			designated_port;		//designated_port is designated_sw's designated_port
	stp_sw_id			designated_root;
	stp_sw_id			designated_sw;
	u32					path_cost;			//local path cost
	u32					designated_cost;		//retrieved from BPDU? accumulated?

	struct timer_list		forward_delay_timer;	//control the stp state machine
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;

//-------------------------------------------------------------------------
	/*** MC snooping ***/
#ifdef CONFIG_ALi_MC_SNOOPING
#if 0
	u32					multicast_startup_queries_sent;
	unsigned char			multicast_router;
	struct timer_list		multicast_router_timer;
	struct timer_list		multicast_query_timer;
	struct hlist_head		mglist;
	struct hlist_node		rlist;
#endif	
#endif

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
	UINT32		rx_runt_pkts_port[MAX_PORT_NUM-1];
	UINT32		rx_long_frame_port[MAX_PORT_NUM-1];
	UINT32		rx_crc_err_pkts_port[MAX_PORT_NUM-1];
	UINT32		rx_alig_err_frame_port[MAX_PORT_NUM-1];
	UINT32 		tx_pkts_port[MAX_PORT_NUM-1];
	//question, no RX stats?
	//half-duplex, TBD.
};

struct mc_alr_entry{
	u8 					mc_mac_addr[6];
	u8					forward_ports;
	struct list_head		list;	
};

struct net_ali_sw
{
	spinlock_t			lock;
	p_p0_private 			p_P0;
	u8					sw_name[4];
	struct net_ali_sw_port  port_list[MAX_PORT_NUM];
	unsigned int ports_link_status;
	struct sw_txrx_stats 	sw_stats;
//-------------------------------------------------------------------------
	/*** STP ***/
	u8					stp_enabled:1;		//seems useless, stp_is_working is enough? TBD
	u8					stp_is_working:1;
	u8					*group_addr;
	
	stp_sw_id			designated_root;
	stp_sw_id			sw_id;
	u32					root_path_cost;// it equals to BPDU's 'designated_cost' + 'path_cost'

	//the following three can be found in fields of Configuration BPDU.
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	
	//the following is for root switch.
	unsigned long			sw_max_age;	//normally, it's 20 secs
	unsigned long			ageing_time;		//fdb, normally 300 secs
	unsigned long			sw_hello_time;
	unsigned long			sw_forward_delay;

	stp_port_id			root_port;

	u8					topology_change;
	u8					topology_change_detected;
//	u8 					alr_fast_aging;

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		fast_aging_timer;	

//-------------------------------------------------------------------------
	struct list_head		mc_alr_entry_list;	
#ifdef CONFIG_ALi_MC_SNOOPING
//	unsigned char			multicast_router; //what for?
/*
	u8					multicast_disabled:1;
	u32					multicast_last_member_count;
	u32					multicast_startup_queries_sent;
	u32					multicast_startup_query_count;

	unsigned long			multicast_last_member_interval;
	unsigned long			multicast_membership_interval;
	unsigned long			multicast_querier_interval;
	unsigned long			multicast_query_interval;
	unsigned long			multicast_query_response_interval;
	unsigned long			multicast_startup_query_interval;

	spinlock_t				multicast_lock;
	struct ali_sw_mdb_htable *mdb;
//	struct hlist_head		router_list;

	struct timer_list		multicast_router_timer;
	struct timer_list		multicast_querier_timer;
	struct timer_list		multicast_query_timer;
*/
	
#endif

};
typedef struct net_ali_sw *p_ali_sw, ali_sw;


struct sw_mc_addr
{
	union {
		__be32	ip4;
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
		struct in6_addr ip6;
#endif
	} u;
	__be16		proto;
	u8 	mac_addr[6];
};

extern const u8 ali_sw_group_address[ETH_ALEN];


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//ALR related structure
typedef struct alr_rdata
{
	u16 mac_addr_high16;
	u8 ports:		4;
	u8 filter:		1;
	u8 static_entry:1;
	u8 EOT:		1;
	u8 valid:		1;
	u8 reserved;
	u32 mac_addr_low32;//question: is there a difference between u32 with UINT32??
	u8 entery_idx;
} * p_alr_rdata;

typedef struct alr_wdata
{
	u16 mac_addr_high16;
	u8 ports:		4;
	u8 filter:		1;
	u8 static_entry:1;
	u8 age:		1;
	u8 valid:		1;
	u8 reserved;
	u32 mac_addr_low32;
	//u8 entery_addr;
}* p_alr_wdata;


typedef struct vlut_data
{
	u32 vid:12;
	u32 port0_member:1;
	u32 port1_member:1;
	u32 port2_member:1;
	u32 port3_member:1;	
	u32 port0_tag:1;
	u32 port1_tag:1;
	u32 port2_tag:1;
	u32 port3_tag:1;
	u32 reserved20_31: 12;	
}* p_vlut_data;

typedef struct vlan_port_info{
	u8 port_no;
	u8 ppriority;
	u8 port_type;
	u16 pvid;
	u8 fff_chg:1;
	u8 igrs_filter:1;
	u8 tag_chg:1;
	u8 tag_only:1;
	u8 untag_only:1;	
} * pvlan_port_info;


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

#define EGRS_QUEUE0		0
#define EGRS_QUEUE1		1
#define EGRS_QUEUE2		2
#define EGRS_QUEUE3		3
#define EGRS_PORT		4
#define EGRS_NONE		5

typedef struct egrs_info{
	u8 	port_no;
	u8 	mode;
	u16 	scale;
} *pegrs_info;

#define IGRS_EN			9
#define IGRS_NONE		10
#define PORT0_EN			1
#define PORT1_EN			2
#define PORT2_EN			4
#define PORT3_EN			8
#define IGRS_INFO_DSCP_EN		1
#define IGRS_INFO_ACL_EN		2
#define IGRS_INFO_VLAN_EN		4

typedef struct igrs_info{
	u8		priority_mode;
	u8		igrs_mode;
	u16		p0_scale;	
	u32		p1_cbs;
	u32		p1_ebs; 
	u32		p23_cbs;
	u32		p23_ebs;	
	u16		p1_tc[16];
	u16		p2_tc[16];
	u16		p3_tc[16];
	u8		ports_enable;
} *pigrs_info;

#define FLOWCTL_DISABLE 	0
#define FLOWCTL_ENABLE 	1
#define P123_FLOWCTL_NONE			0
#define P123_FLOWCTL_PORT_ONLY	1
#define P123_FLOWCTL_PRIOR_ONLY	2
#define P123_FLOWCTL_PORT_PRIOR	3
typedef struct flowctl_info{
	u8 	port_no;
	u8 	p0_blks;
	u8 	p123_mode;
} *pflowctl_info;


typedef struct bandrate_info{
	u8 	port_no;
	u16 	interval;
} *pbandrate_info;


#define ACL_WRITE_ENTRY	1
#define ACL_READ_ENTRY		0
#define ACL_MODE_SRC_SRC 	4
#define ACL_MODE_SRC_MSK	0
#define ACL_MODE_DST_DST	5
#define ACL_MODE_DST_MSK	1
#define ACL_PROTO_TCP	1
#define ACL_PROTO_UDP	2
#define ACL_PROTO_TCP_UDP	3

typedef struct acl_info{
	u8   	acl_proto;
	u8	acl_command;
	u8 	acl_mode;
	u8 	acl_idx;
	u8 	ports_enable;
	u8 	priority_sel;
	u32	ip1;
	u32	ip2;
	u16 port1_start;
	u16 port1_end;
	u16 port2_start;
	u16 port2_end;

}*pacl_info;


#define STP_OPERATION_DISABLE		0
#define STP_OPERATION_ENABLE		1
#define STP_OPERATION_INFO		2
#define STP_SET_PRIORITY			3
typedef struct stp_info
{
	u8 stp_op;
	u16 stp_priority;
}*pstp_info;



typedef struct phy_mdio_ctl
{
	u8 port_no;
	u8 phy_reg_addr;
}*pphy_mdio_ctl;

struct alr_age
{
	u8 aging_type;
	u32 aging_time;
};

struct igrs2_info
{
	u8 	port;
	u16 	tc;
	u32 	cbs;
	u32 	ebs;
};

#define WAN_PORT_SELECTION		1
#define ROUTE_FORWARDING_TEST	2
struct route_info{
	u8 route_cmd;
	union{
		u8 wan_port;
		u16 forward_ports;
	}un;
};

typedef struct _ali_reg_t {
	u32 cmd;
	u32 phy_addr;
	u32 val;
} ali_reg_t;
#define SHOW_DROP_STATISTICS		0
#define SHOW_LINK_STATUS			1
#define READ_PHY_MDIO				2
#define VLAN_OPERATION				3
#define ALR_AGING_TEST				4
#define QOS_ACL_SETTING			5
#define QOS_VLAN_SETTING			6
#define QOS_DSCP_SETTING			7
#define IGRS2_DBG_SETING			8
#define ROUTE_SETTING				9
#define ALIREG_SETTING              10
#define ALIREG_GETTING              11

typedef struct misc_info{
	unsigned char misc_cmd;
	union {
		struct phy_mdio_ctl 	mdio_val;
		u8					vlan_en;
		struct alr_age			age_val;
		u8					qos_en;
		struct igrs2_info		igrs2_val;
		struct route_info		route_val;
	} misc_val;
} misc_info_t, *pmisc_info;
#define misc_alireg misc_val.ali_reg

/*
union {
	u32 reg_val;
	struct vlut_data vlut_data_val;
} vlut_data_reg;
*/

//__attribute__((packed))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct ali_sw_input_skb_cb {
	u8   skb_id[3];
	u8	port_no;	
	int 	igmp;
	int 	mrouters_only;	
};

struct ali_sw_output_skb_cb {
	u8	forward_ports;	
	u8   skb_id[3];
	u8   pkt_id[4];
};

#define SW_INPUT_SKB_CB(__skb)		((struct ali_sw_input_skb_cb *)(__skb)->cb)
#define SW_OUTPUT_SKB_CB(__skb)	((struct ali_sw_output_skb_cb *)(__skb)->cb)


/*
struct ali_sw_skb_cb {
	u8   skb_id[3];
	u8	port_no;	
	int 	igmp;
	int 	mrouters_only;	
};
#define ALI_SW_SKB_CB(__skb)	((struct ali_sw_skb_cb *)(__skb)->cb)
*/
/*
#ifdef CONFIG_ALi_MC_SNOOPING
# define ALI_SW_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(ALI_SW_INPUT_SKB_CB(__skb)->mrouters_only)
#else
# define ALI_SW_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(0)
#endif
*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/* ali_sw.c */


int ali_sw_alr_write_entry(p_alr_wdata p_entry);

void ali_sw_alr_fast_aging(u8 flag);
void ali_sw_alr_fast_aging_ext(u32 aging_time);

extern struct net_ali_sw *gp_sw;
extern UINT32 ali_sw_base;
void ali_sw_set_port_stp_hwstatus(u8 port_no, u8 stp_status);
extern int ali_sw_stp_init(void);
extern u8 ali_sw_get_port_cost(struct net_ali_sw_port *p);
//extern u8 ali_sw_check_port_link_status(u8 port_no);
extern bool ali_sw_check_port_link_status(struct net_ali_sw_port *p);
#ifdef ALI_SW_DEBUG1
extern void p0_print(void*p, unsigned short len);
extern void p0_print_line(unsigned char *p, unsigned short len);
#endif

/* ali_sw_stp.c */
//ali_sw_multicast_rcv
extern u16 ali_sw_stp_priority ;

extern void ali_sw_stp_rcv(struct sk_buff *skb);
void ali_sw_start_sw_stp(struct net_ali_sw *sw);
void ali_sw_stop_sw_stp(struct net_ali_sw *sw);
extern void ali_sw_start_port_stp(struct net_ali_sw_port *p);
extern void ali_sw_stop_port_stp(struct net_ali_sw_port *p);
extern struct net_ali_sw_port *ali_sw_get_port(struct net_ali_sw *sw, u16 port_no);
//extern const char *ali_sw_port_state_names;
extern void ali_sw_set_port_stp_status(struct net_ali_sw_port *p, u8 stp_status);



/* ali_sw_mc_snooping.c*/
extern struct sk_buff *ali_sw_multicast_rcv( struct sk_buff *skb);
extern struct net_ali_sw_mdb_entry *ali_sw_mdb_get(struct net_ali_sw *sw,   struct sk_buff *skb);
//extern void ali_sw_multicast_init();
extern void ali_sw_multicast_deliver(struct net_ali_sw_mdb_entry *mdst, struct sk_buff *skb);
extern void ali_sw_multicast_forward(struct net_ali_sw_mdb_entry *mdst, struct sk_buff *skb, struct sk_buff *skb2);
int ali_sw_mc_snooping_init(void);
 
 void ali_sw_show_stp_info(void);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


extern unsigned long dbg_runtime_val;




#endif
