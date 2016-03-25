/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *
 */
#ifndef _ALI_SW_PRIVATE_STP_H
#define _ALI_SW_PRIVATE_STP_H

#include <linux/netdevice.h>
#include "ali_sw.h"

#define BPDU_TYPE_CONFIG 0
#define BPDU_TYPE_TCN 0x80

/* called under sw lock */
static inline int ali_sw_is_root_sw(const struct net_ali_sw *sw)
{
	return !memcmp(&sw->sw_id, &sw->designated_root, 8);
}



struct ali_sw_config_bpdu
{
	unsigned			topology_change:1;
	unsigned			topology_change_ack:1;
	stp_sw_id		root;
	int				root_path_cost;
	stp_sw_id		sw_id;
	stp_port_id		port_id;
	int				message_age;
	int				max_age;
	int				hello_time;
	int				forward_delay;
};

/* called under switch lock */
static inline int ali_sw_is_designated_port(const struct net_ali_sw_port *p)
{
	return !memcmp(&p->designated_sw, &p->sw->sw_id, 8) &&
		(p->designated_port == p->port_id);
	/*
		if a port is designated_port, its switch must be designated_sw.
	*/
}


static inline int ali_sw_is_root_port(const struct net_ali_sw_port *p)
{
	return (p->port_no == p->sw->root_port);
}

/* ali_sw_stp.c */
extern int ali_sw_stp_init(void);

extern void ali_sw_become_root_sw(struct net_ali_sw *sw);
extern void ali_sw_config_bpdu_generation(struct net_ali_sw *);
extern void ali_sw_configuration_update(struct net_ali_sw *);
extern void ali_sw_port_state_selection(struct net_ali_sw *);
extern void ali_sw_received_config_bpdu(struct net_ali_sw_port *p, struct ali_sw_config_bpdu *bpdu);
extern void ali_sw_received_tcn_bpdu(struct net_ali_sw_port *p);
extern void ali_sw_transmit_config(struct net_ali_sw_port *p);
extern void ali_sw_transmit_tcn(struct net_ali_sw *sw);
extern void ali_sw_topology_change_detection(struct net_ali_sw *sw);

extern void ali_sw_log_state(const struct net_ali_sw_port *p);
extern struct net_ali_sw_port *ali_sw_get_port(struct net_ali_sw *sw, u16 port_no);
//extern void ali_sw_init_stp_port(struct net_ali_sw_port *p);
extern void ali_sw_become_designated_port(struct net_ali_sw_port *p);


/* ali_sw_stp_bpdu.c */
extern void ali_sw_send_config_bpdu(struct net_ali_sw_port *, struct ali_sw_config_bpdu *);
extern void ali_sw_send_tcn_bpdu(struct net_ali_sw_port *);
//extern void ali_sw_stp_rcv(struct sk_buff *skb);


extern void ali_sw_stp_timer_init(struct net_ali_sw *sw);
extern void ali_sw_stp_port_timer_init(struct net_ali_sw_port *p);
extern unsigned long ali_sw_timer_value(const struct timer_list *timer);





#endif
