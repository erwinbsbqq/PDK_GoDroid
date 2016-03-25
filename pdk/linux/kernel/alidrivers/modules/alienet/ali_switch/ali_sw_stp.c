/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *
 */
#include <linux/kernel.h>
#include <linux/rculist.h>
#include <linux/etherdevice.h>
#include "ali_sw.h"
#include "ali_sw_stp.h"


/*
 * since time values in bpdu are in jiffies and then scaled (1/256)
 * before sending, make sure that is at least one.
 */
#define MESSAGE_AGE_INCR	((HZ < 256) ? 1 : (HZ/256))

const char *ali_sw_port_state_names[] = {
	[ALI_SW_STATE_DISABLED] = "disabled  ",
	[ALI_SW_STATE_LISTENING] = "listening ",
	[ALI_SW_STATE_LEARNING] = "learning  ",
	[ALI_SW_STATE_FORWARDING] = "forwarding",
	[ALI_SW_STATE_BLOCKING] = "blocking  ",
};

u16 ali_sw_stp_priority = 0x8000;

u8 stp_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

static void ali_sw_init_stp_port(struct net_ali_sw_port *p);

void ali_sw_log_state(const struct net_ali_sw_port *p)
{
	pr_info("stp: port[%d] entering [%s] state\n",
		p->port_no, ali_sw_port_state_names[p->stp_state]);
}

/* called under sw lock */
struct net_ali_sw_port *ali_sw_get_port(struct net_ali_sw *sw, u16 port_no)
{
	struct net_ali_sw_port *p;
	u8 i;

	for(i=0; i<MAX_PORT_NUM;i++)
	{
		p = &(sw->port_list[i]);
		if (p->port_no == port_no)
			return p;		
	}
	sw_error("%s, port_no out of range:%d", __FUNCTION__, port_no);
	return NULL;
}

/* 
*	the assumpation is root_port
*	loop to see if other port has a high priority vector...
*	called under sw lock 
*/
static int ali_sw_should_become_root_port(const struct net_ali_sw_port *p, u16 root_port)
{
	struct net_ali_sw *sw;
	struct net_ali_sw_port *rp;
	int t;

	sw = p->sw;
	if (p->stp_state == ALI_SW_STATE_DISABLED || ali_sw_is_designated_port(p))
		return 0;

	//firstly, compare switch id to see which is the smallest.
	if (memcmp(&sw->sw_id, &p->designated_root, 8) <= 0)//to see if it's root sw.
		return 0;
	//before this line, it is we call "early_return".

	//the following indicates p->designated_root has higher priority than sw->sw_id
	//root_port is what we think as the root port of a sw, but we are not sure, need to compare through a looping.
	if (root_port==MAX_PORT_NUM)//root_port is port_no. this is changed, is it right? original it's '!root_port' 
		return 1;

	rp = ali_sw_get_port(sw, root_port);
    if (!rp) {
        return 0;
    }

	t = memcmp(&p->designated_root, &rp->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;
	
	//designated_cost is the accumulated one, path_cost is not the accumulated one.
	if (p->designated_cost + p->path_cost <
	    rp->designated_cost + rp->path_cost)
		return 1;
	else if (p->designated_cost + p->path_cost >
		 rp->designated_cost + rp->path_cost)
		return 0;

	t = memcmp(&p->designated_sw, &rp->designated_sw, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->designated_port < rp->designated_port)
		return 1;
	else if (p->designated_port > rp->designated_port)
		return 0;

	if (p->port_id < rp->port_id)
		return 1;

	return 0;
}

/* called under sw lock */
static void ali_sw_root_selection(struct net_ali_sw *sw)
{
	/* to find out the ROOT switch, and also the ROOT port of this switch. */
	struct net_ali_sw_port *p;
	u8 i;
	u16 root_port = MAX_PORT_NUM;

	for(i=0; i<MAX_PORT_NUM;i++)
	{
		p = &(sw->port_list[i]);
		if (p->stp_state != ALI_SW_STATE_DISABLED && ali_sw_should_become_root_port(p, root_port))
			root_port = p->port_no;
	}
	
	sw->root_port = root_port;

	if (root_port==MAX_PORT_NUM) //root_port is port_no. this is changed, is it right? original it's '!root_port' , question
	{
	/*entering this branch, it means 'ali_sw_should_become_root_port' is 'early_return', 
	(if the call back has 'ali_sw_supersedes_port_info', it is impossible.)
	then the switch should become the root switch. but why not call the funcstion @'ali_sw_become_root_sw' directly?
		I think @'ali_sw_root_selection' is called within @'ali_sw_configuration_update',
		@'ali_sw_become_root_sw' is also called along with @'ali_sw_configuration_update',
		entering this branch doesn't mean the switch is finally become the root switch,
		may just be temporary, so we don't call @'ali_sw_become_root_sw'*/	
		sw->designated_root = sw->sw_id;
		sw->root_path_cost = 0;
#ifdef ALI_SW_DEBUG_STP		
		stp_trace("root_port==MAX_PORT_NUM in root selection");
#endif
	} 
	else 
	{
		p = ali_sw_get_port(sw, root_port);
        if (!p) {
            return;
        }
		sw->designated_root = p->designated_root;
		sw->root_path_cost = p->designated_cost + p->path_cost;//the accumulated one,  the desgnated  one + local path cost.
	}
}

/* called under sw lock */
void ali_sw_become_root_sw(struct net_ali_sw *sw)
{
	//sw_max_age, sw_hello_time, sw_forward_delay is this switch's parameter, other switch may have different values.
	sw->max_age = sw->sw_max_age;
	sw->hello_time = sw->sw_hello_time;
	sw->forward_delay = sw->sw_forward_delay;

	pr_info("stp: become root switch\n");
	
	ali_sw_topology_change_detection(sw);
	del_timer(&sw->tcn_timer);
	
	ali_sw_config_bpdu_generation(sw);
	mod_timer(&sw->hello_timer, jiffies + sw->hello_time);	
}

/* called under sw lock */
void ali_sw_transmit_config(struct net_ali_sw_port *p)
{
	struct ali_sw_config_bpdu bpdu;
	struct net_ali_sw *sw;


	if (timer_pending(&p->hold_timer)) {
		p->config_pending = 1;
		return;
	}

	sw = p->sw;

	bpdu.topology_change = sw->topology_change;
	bpdu.topology_change_ack = p->topology_change_ack;
	bpdu.root = sw->designated_root;
	bpdu.root_path_cost = sw->root_path_cost;
	bpdu.sw_id = sw->sw_id;
	bpdu.port_id = p->port_id;
	
	if (ali_sw_is_root_sw(sw))
		bpdu.message_age = 0;
	else {
		struct net_ali_sw_port *root
			= ali_sw_get_port(sw, sw->root_port);
        if (!root) {
            return;
        }
		bpdu.message_age = sw->max_age
			- (root->message_age_timer.expires - jiffies)
			+ MESSAGE_AGE_INCR;
/***************************************************************************
	when receiving,
		|			  jiffies(message_age)		expires				 |
		|-------------^----------------------^------------------|
	when transmiting,
		|			  	jiffies(message_age)	expires				 |
		|--------------------^---------------^------------------|
****************************************************************************/
	}
	
	bpdu.max_age = sw->max_age;
	bpdu.hello_time = sw->hello_time;
	bpdu.forward_delay = sw->forward_delay;

	if (bpdu.message_age < sw->max_age) {
		ali_sw_send_config_bpdu(p, &bpdu);
		p->topology_change_ack = 0;
		p->config_pending = 0;
		mod_timer(&p->hold_timer, round_jiffies(jiffies + ALI_SW_HOLD_TIME));
	}

}

/* called under sw lock */
static inline void ali_sw_record_config_information(struct net_ali_sw_port *p,
						const struct ali_sw_config_bpdu *bpdu)
{
	p->designated_root = bpdu->root;
	p->designated_cost = bpdu->root_path_cost;
	
	p->designated_sw = bpdu->sw_id;//thus makes it not a designated itself.
	p->designated_port = bpdu->port_id;//thus makes it not a designated itself.

	mod_timer(&p->message_age_timer, jiffies + (p->sw->max_age - bpdu->message_age));
	//if the config BPDU keeps coming in the allowed time, the message_age_timer will never expire.
}

/* called under sw lock */
static inline void ali_sw_record_config_timeout_values(struct net_ali_sw *sw,
					    const struct ali_sw_config_bpdu *bpdu)
{
	sw->max_age = bpdu->max_age;
	sw->hello_time = bpdu->hello_time;
	sw->forward_delay = bpdu->forward_delay;
	sw->topology_change = bpdu->topology_change;
}

/* called under sw lock */
void ali_sw_transmit_tcn(struct net_ali_sw *sw)
{
	ali_sw_send_tcn_bpdu(ali_sw_get_port(sw, sw->root_port));
}

/* called under sw lock */
static int ali_sw_should_become_designated_port(const struct net_ali_sw_port *p)
{
	struct net_ali_sw *sw;
	int t;
/*********************************************************************************************************
	entering this, the following parameters have been set. 
	p is the one port that received the BPDU, other ports' record haven't been updated.
		p->designated_root
		p->designated_cost
		p->designated_sw
		p->port_id
		sw->root_port
		sw->designated_root
		sw->root_path_cost
*********************************************************************************************************/
	sw = p->sw;
	if (ali_sw_is_designated_port(p))
		return 1;

	if (memcmp(&p->designated_root, &sw->designated_root, 8))//sw->designated_root should be smallest after 'ali_sw_root_selection'.
		return 1;

	if (sw->root_path_cost < p->designated_cost)
		return 1;
	else if (sw->root_path_cost > p->designated_cost)
		return 0;/*<---------let's imagine, a switch with lower priority is added to a topology,
				1. it claims itself as root switch, all its ports are designated ports, sending out config BPDUs.
				2. when other switch receive its config BPDUs, they will reply with a config BPDU that has high priority vector.
				3. some port of this switch receives other switch's config BPDU, it will record parameters from BPDU such as:
						p->designated_root = bpdu->root;
						p->designated_cost = bpdu->root_path_cost;
						p->designated_sw = bpdu->sw_id;
						p->designated_port = bpdu->port_id;
				    so the port receiving the BPDU is not a designated port anymore. (eg. port A)
				...............................................................................................................>>record information
				    this switch also updates its information if some port receive a config BPDU that has high priority vector.
						sw->designated_root = p->designated_root;
						sw->root_path_cost = p->designated_cost + p->path_cost;
				    so the switch is not the root switch anymore, it will delete its hello_timer
				...............................................................................................................>>choose the root switch
				    the swithch will also loop all its ports to see which port has a better priority vector
				    	(designated_root, designated_cost, designated_sw, designated_port)
				    so the switch will find out its root port, (eg. it is port A)
				...............................................................................................................>>choose the root port.
				    except the root port, all the other ports remain the designated ports or make it BLOCKING, 
				    for designated ports, they will update priority vector
				...............................................................................................................>>choose the designated port.
				    reset ports' state to LISTENING, restart their forward_timer.
				4. this switch will propagate the BPDU to other switches through out its designated ports.
	<-----------anything, I want to say it here is:
				    designated port is seen from a LAN's perspective, it should have smallest path cost to root switch.
				    so the comparision doesn't add this LAN's local path cost, just between switch's root path cost and BPDU's root path cost.*/

	t = memcmp(&sw->sw_id, &p->designated_sw, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->port_id < p->designated_port)
		return 1;//how come?

	return 0;
}

/* called under sw lock */
static void ali_sw_designated_port_selection(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;

	for(i=0; i<MAX_PORT_NUM; i++)
	{
		p = &(sw->port_list[i]);
		if (p->stp_state != ALI_SW_STATE_DISABLED && ali_sw_should_become_designated_port(p)){
			ali_sw_become_designated_port(p);	
#ifdef ALI_SW_DEBUG_STP
			stp_trace("stp: port[%d] become desgnated port, %x%x%x%x%x%x,%d; %x%x%x%x%x%x,0x%x; %x%x%x%x%x%x,%d.", 
					p->port_no,
					p->designated_root.addr[0], p->designated_root.addr[1], p->designated_root.addr[2],
					p->designated_root.addr[3], p->designated_root.addr[4], p->designated_root.addr[5],
					p->designated_cost, 
					p->designated_sw.addr[0], p->designated_sw.addr[1], p->designated_sw.addr[2],
					p->designated_sw.addr[3], p->designated_sw.addr[4], p->designated_sw.addr[5],
					p->designated_port,
					sw->designated_root.addr[0], sw->designated_root.addr[1], sw->designated_root.addr[2],
					sw->designated_root.addr[3], sw->designated_root.addr[4], sw->designated_root.addr[5],
					sw->root_path_cost);	
#endif		
		}
	}
}

/* called under sw lock */
static int ali_sw_supersedes_port_info(struct net_ali_sw_port *p, struct ali_sw_config_bpdu *bpdu)
{
	int t;

	//has a better root, return 1
	t = memcmp(&bpdu->root, &p->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	//same root, but has a better path cost, return 1
	if (bpdu->root_path_cost < p->designated_cost)
		return 1;
	else if (bpdu->root_path_cost > p->designated_cost)
		return 0;

	//same root, same path cost, but has a better designated switch
	t = memcmp(&bpdu->sw_id, &p->designated_sw, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	//the BPDU is not from this switch, return 1
	if (memcmp(&bpdu->sw_id, &p->sw->sw_id, 8))
		return 1;

	//same root, path cost, designated sw, but a better port id, return 1
	if (bpdu->port_id <= p->designated_port)
		return 1;

	return 0;
}

/* called under sw lock */
static inline void ali_sw_topology_change_acknowledged(struct net_ali_sw *sw)
{
	sw->topology_change_detected = 0;
	del_timer(&sw->tcn_timer);
}

/* called under sw lock */
void ali_sw_topology_change_detection(struct net_ali_sw *sw)
{
	int isroot = ali_sw_is_root_sw(sw);

	pr_info("topology_change_detection: topology change detected, %s\n", 
		isroot ? "propagating..." : "sending tcn bpdu...");

	if (isroot)
	{	//if it's root switch, send out TC periodically after get TCN or detect topology change
		sw->topology_change = 1;
		mod_timer(&sw->topology_change_timer, jiffies + sw->sw_forward_delay + sw->sw_max_age);
	} 
	else if (!sw->topology_change_detected)
	{	//other switches send TCN.
		ali_sw_transmit_tcn(sw);
		mod_timer(&sw->tcn_timer, jiffies + sw->sw_hello_time);
	}
	//detect topology change happened
	sw->topology_change_detected = 1;
	
	ali_sw_alr_fast_aging_ext(15);
	mod_timer(&sw->fast_aging_timer, jiffies + sw->sw_forward_delay);
}

/* called under sw lock */
void ali_sw_config_bpdu_generation(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;
	
	for(i=0; i<MAX_PORT_NUM;i++)
	{
		p = &(sw->port_list[i]);
		if (p->stp_state != ALI_SW_STATE_DISABLED && ali_sw_is_designated_port(p))
			ali_sw_transmit_config(p);
	}
}

/* called under sw lock */
static inline void ali_sw_reply(struct net_ali_sw_port *p)
{
	ali_sw_transmit_config(p);
}

/* called under sw lock */
void ali_sw_configuration_update(struct net_ali_sw *sw)
{
/*	@'ali_sw_root_selection' finds out:
		sw->root_port
		sw->designated_root
		sw->root_path_cost*/
	ali_sw_root_selection(sw);
	ali_sw_designated_port_selection(sw);
}

/* called under sw lock */
void ali_sw_become_designated_port(struct net_ali_sw_port *p)
{
	struct net_ali_sw *sw;

	sw = p->sw;
	p->designated_root = sw->designated_root;
	p->designated_cost = sw->root_path_cost;//this is accumulated
	
	//if some port is designated port, the switch it belongs to must be a designated switch.
	p->designated_sw = sw->sw_id;
	p->designated_port = p->port_id;
}


/* called under sw lock */
static void ali_sw_make_blocking(struct net_ali_sw_port *p)
{
	if (p->stp_state != ALI_SW_STATE_DISABLED &&  p->stp_state != ALI_SW_STATE_BLOCKING) 
	{
		if (p->stp_state == ALI_SW_STATE_FORWARDING || p->stp_state == ALI_SW_STATE_LEARNING)
			ali_sw_topology_change_detection(p->sw);

		//p->stp_state = ALI_SW_STATE_BLOCKING;
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_BLOCKING);
		ali_sw_log_state(p);
		del_timer(&p->forward_delay_timer);
	}
}

/* called under sw lock */
static void ali_sw_make_changing(struct net_ali_sw_port *p)
{
	/*ALI_SW_STATE_LISTENING, ALI_SW_STATE_LEARNING, ALI_SW_STATE_FORWARDING all forward STP pkts.
	this routine only means forward STP pkts (BPDU), doesn't mean forward data pkts. */
	struct net_ali_sw *sw = p->sw;
	
	if (p->stp_state != ALI_SW_STATE_BLOCKING)
		return;

	if (sw->forward_delay == 0) 
	{
		//enter this maybe is because forward_delay is set to 0 by user tools,
		//so we dont need forward_delay_timer anymore.
		//p->stp_state = ALI_SW_STATE_FORWARDING;
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_FORWARDING);
		ali_sw_topology_change_detection(sw);
		del_timer(&p->forward_delay_timer);
	}
	else if (p->sw->stp_enabled == ALI_SW_KERNEL_STP)
	{	
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_LISTENING);
	}
	else
	{	
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_LEARNING);
	}
	
	ali_sw_log_state(p);

	if (sw->forward_delay != 0)
		mod_timer(&p->forward_delay_timer, jiffies + sw->forward_delay);
}

/* called under sw lock */
void ali_sw_port_state_selection(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;

	for(i=0; i<MAX_PORT_NUM;i++) {
		p = &(sw->port_list[i]);
		if (p->stp_state != ALI_SW_STATE_DISABLED) 	{
			if (p->port_no == sw->root_port) {
				//root port
				p->config_pending = 0;
				p->topology_change_ack = 0;
				ali_sw_make_changing(p);
			} else if (ali_sw_is_designated_port(p)){
				 //designated port
				//we only transmit config BPDU, don't receive, so delete message_age_timer 
				del_timer(&p->message_age_timer);
				ali_sw_make_changing(p);
			} else	{	 
				//others, blocking
				p->config_pending = 0;
				p->topology_change_ack = 0;
				ali_sw_make_blocking(p);
			}
		}
	}
}

/* called under sw lock */
static inline void ali_sw_topology_change_acknowledge(struct net_ali_sw_port *p)
{
	p->topology_change_ack = 1;
	ali_sw_transmit_config(p);
}

/* called under sw lock */
void ali_sw_received_config_bpdu(struct net_ali_sw_port *p, struct ali_sw_config_bpdu *bpdu)
{
	struct net_ali_sw *sw;
	int was_root;

	sw = p->sw;
	was_root = ali_sw_is_root_sw(sw);
	
	/*@'ali_sw_supersedes_port_info' returns 1 when:
			the BPDU has higher priority vector	*/
	if (ali_sw_supersedes_port_info(p, bpdu)) 
	{
#ifdef ALI_SW_DEBUG_STP	
		stp_trace("\nstp: port[%d] recv config BPDU, %x%x%x%x%x%x,%d; %x%x%x%x%x%x,%d; %x%x%x%x%x%x,0x%x.", 
				p->port_no,
				p->designated_root.addr[0], p->designated_root.addr[1], p->designated_root.addr[2],
				p->designated_root.addr[3], p->designated_root.addr[4], p->designated_root.addr[5],
				p->designated_cost, 
				bpdu->root.addr[0], bpdu->root.addr[1], bpdu->root.addr[2],
				bpdu->root.addr[3], bpdu->root.addr[4], bpdu->root.addr[5],
				bpdu->root_path_cost,
				bpdu->sw_id.addr[0], bpdu->sw_id.addr[1], bpdu->sw_id.addr[2],
				bpdu->sw_id.addr[3], bpdu->sw_id.addr[4], bpdu->sw_id.addr[5],
				bpdu->port_id);
#endif		
		
		/*@'ali_sw_record_config_information' changes items according the BPDU: 
			p->designated_root = bpdu->root;
			p->designated_cost = bpdu->root_path_cost; //the one from BPDU, not add local path cost yet.
			p->designated_sw = bpdu->sw_id;
			p->designated_port = bpdu->port_id;
			renew message_age_timer 
		originally, a port might be the designated port of switch, but after this, it isn't anymore,
			because it changes port's designated switch and designated port (designated_sw&designated_port)*/	
		ali_sw_record_config_information(p, bpdu);

		/*@'ali_sw_configuration_update'	will find out:
			which is the root switch
			which is the root port of this switch
			which is the designated port of this switch
		@'ali_sw_port_state_selection' is often called with
			@'ali_sw_configuration_update'*/
		ali_sw_configuration_update(sw);
		ali_sw_port_state_selection(sw);

		if (!ali_sw_is_root_sw(sw) && was_root) {
			//enter means the switch was root switch but now it is not.
			del_timer(&sw->hello_timer);
			if (sw->topology_change_detected) {
			/*topology_change_detected is set in @'ali_sw_topology_change_detection'
			@'ali_sw_topology_change_detection' is called in mainly 4 conditions:
				some port changes its status from 'FORWARDING'/'LEARNING' to 'BLOCKING'
				some port changes its status from 'LEARNING' to 'FORWARDING', and this switch has designated ports
				when the switch receive TCN BPDUs						
				this switch now become the root switch, previously it wasn't. 	
			@'ali_sw_transmit_tcn' send TCN BPDU only through switch's root port.*/
				del_timer(&sw->topology_change_timer);
				ali_sw_transmit_tcn(sw);
				mod_timer(&sw->tcn_timer, jiffies + sw->sw_hello_time);
			}
		}

		if (p->port_no == sw->root_port) {
			//the BPDU comes from the root switch.
			/*if the BPDU has TC, record in @'ali_sw_record_config_timeout_values'.
			if the BPDU has TCA, deal with it in @'ali_sw_topology_change_acknowledged'.*/
			ali_sw_record_config_timeout_values(sw, bpdu);

			/*if this switch has designated ports, it will propagate the BPDU to other switches.*/
			ali_sw_config_bpdu_generation(sw);

			/*TCN is send from this switch's root port to its designated switch's designated ports
			Config BPDU(with TCA) is received from this switch's root port.*/
			if (bpdu->topology_change_ack)
				ali_sw_topology_change_acknowledged(sw);
		}
	} 
	else if (ali_sw_is_designated_port(p)) 	{
		//he BPDU comes from other switch with lower priority, which may just enter the topology and claims itself as the root switch.
		/*if it receives config BPDU with lower priority from its designated ports, 
		then reply with its config BPDU, which has a higher priority.*/
		ali_sw_reply(p);
	}
}

/* called under sw lock */
void ali_sw_received_tcn_bpdu(struct net_ali_sw_port *p)
{
	if (ali_sw_is_designated_port(p)){
		pr_info("received_tcn_bpdu at port[%d]\n", p->port_no);
		ali_sw_topology_change_detection(p->sw);
		ali_sw_topology_change_acknowledge(p);
	}
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/* called under sw lock */
static int ali_sw_is_designated_for_some_port(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;

	for(i=0; i<MAX_PORT_NUM;i++)
	{
		p = &(sw->port_list[i]);
		if (p->stp_state != ALI_SW_STATE_DISABLED &&
		    !memcmp(&p->designated_sw, &sw->sw_id, 8))
			return 1;
	}
	return 0;
}

static void ali_sw_hello_timer_expired(unsigned long arg)
{
	struct net_ali_sw *sw = (struct net_ali_sw *)arg;
#if 0
	unsigned long flags;

	pr_info("sw: hello timer expired\n");
	spin_lock_irqsave(&sw->lock, flags);
#endif
	ali_sw_config_bpdu_generation(sw);
	mod_timer(&sw->hello_timer, round_jiffies(jiffies + sw->hello_time));
#if 0	
	spin_unlock_irqrestore(&sw->lock, flags);
#endif
}

static void ali_sw_message_age_timer_expired(unsigned long arg)
{
	struct net_ali_sw_port *p = (struct net_ali_sw_port *) arg;
	struct net_ali_sw *sw = p->sw;
	const stp_sw_id *id = &p->designated_sw;
	int was_root;
	/*unsigned long flags;*/

	if (p->stp_state == ALI_SW_STATE_DISABLED)
		return;

	pr_info("stp: msg_timer_expired on port[%d], %.2x-%.2x-%.2x-%.2x-%.2x-%.2x lost\n",
		p->port_no,
		id->addr[0], id->addr[1], id->addr[2],
		id->addr[3], id->addr[4], id->addr[5]);

	/*
	 * According to the spec, the message age timer cannot be
	 * running when we are the root sw. So..  this was_root
	 * check is redundant. I'm leaving it in for now, though.
	 */
	//spin_lock_irqsave(&sw->lock, flags);
	if (p->stp_state == ALI_SW_STATE_DISABLED)
		goto unlock;
	was_root = ali_sw_is_root_sw(sw);

	ali_sw_become_designated_port(p);
	ali_sw_configuration_update(sw);
	ali_sw_port_state_selection(sw);
	if (ali_sw_is_root_sw(sw) && !was_root)
		ali_sw_become_root_sw(sw);
 unlock:
	//spin_unlock_irqrestore(&sw->lock, flags);
	return;
}

static void ali_sw_forward_delay_timer_expired(unsigned long arg)
{
	struct net_ali_sw_port *p = (struct net_ali_sw_port *) arg;
	struct net_ali_sw *sw = p->sw;
#if 0
	unsigned long flags;
	pr_info("sw: port[%d] forward delay timer\n", p->port_no);
	spin_lock_irqsave(&sw->lock, flags);
#endif
	
	if (p->stp_state == ALI_SW_STATE_LISTENING) {
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_LEARNING);
		mod_timer(&p->forward_delay_timer, jiffies + sw->forward_delay);
	} 
	else if (p->stp_state ==  ALI_SW_STATE_LEARNING) {
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_FORWARDING);
		if (ali_sw_is_designated_for_some_port(sw))
			ali_sw_topology_change_detection(sw);
	}
	
	ali_sw_log_state(p);
	
	//spin_unlock_irqrestore(&sw->lock, flags);
}

static void ali_sw_tcn_timer_expired(unsigned long arg)
{
	struct net_ali_sw *sw = (struct net_ali_sw *) arg;
	/* unsigned long flags; */
	
	pr_info("sw: tcn timer expired\n");
	
	//spin_lock_irqsave(&sw->lock, flags);

	ali_sw_transmit_tcn(sw);
	mod_timer(&sw->tcn_timer,jiffies + sw->sw_hello_time);
	
	//spin_unlock_irqrestore(&sw->lock, flags);
}

static void ali_sw_topology_change_timer_expired(unsigned long arg)
{
	struct net_ali_sw *sw = (struct net_ali_sw *) arg;
#if 0
	unsigned long flags;
	pr_info("sw: topology change timer expired\n");
	spin_lock_irqsave(&sw->lock, flags);
#endif
	sw->topology_change_detected = 0;
	sw->topology_change = 0;
#if 0
	spin_unlock_irqrestore(&sw->lock, flags);
#endif
}

static void ali_sw_fast_aging(unsigned long arg)
{
	struct net_ali_sw *sw = (struct net_ali_sw *) arg;
	pr_info("sw: alr fast aging timer expired\n");
	del_timer(&sw->fast_aging_timer);
	ali_sw_alr_fast_aging_ext(300);
}

static void ali_sw_hold_timer_expired(unsigned long arg)
{
	struct net_ali_sw_port *p = (struct net_ali_sw_port *) arg;
#if 0
	unsigned long flags;
	pr_debug("sw: %d hold timer expired\n", p->sw->sw_name,  p->port_no);
	spin_lock_irqsave(&p->sw->lock, flags);
#endif
	if (p->config_pending)
		ali_sw_transmit_config(p);
#if 0
	spin_unlock_irqrestore(&p->sw->lock, flags);
#endif
}

#if 0
void ali_sw_fdb_cleanup(unsigned long arg)
{
	//THINGS LIKE br_fdb_cleanup, tbd
	sw_trace("%s", __FUNCTION__);
}
#endif

void ali_sw_stp_timer_init(struct net_ali_sw *sw)
{
	//send config BPDU at regular time(2HZ).
	setup_timer(&sw->hello_timer, ali_sw_hello_timer_expired, (unsigned long) sw);

	//send TCN BPDU at regular time(2HZ).
	setup_timer(&sw->tcn_timer, ali_sw_tcn_timer_expired, (unsigned long) sw);

	//how long does root switch inform other switches about the topology_change(35HZ = MAX_AGE+FORWARD_DEPLAY).
	setup_timer(&sw->topology_change_timer, ali_sw_topology_change_timer_expired, (unsigned long) sw);

	//used to check FDB, this is only to periodically check cleanup condition, cleanup is done(delete the expired entries) if condition is meet.
	//an entry expires if it has not been used for at least 5 minuttes. --> this is used no matter if STP is enabled or not.
	//if STP TC is detected, the entry expiring time is 15HZ(FORWARD_DELAY).
	//we don't need this, HW will take care of this itself
	//setup_timer(&sw->gc_timer, ali_sw_fdb_cleanup, (unsigned long) sw);
	
	setup_timer(&sw->fast_aging_timer, ali_sw_fast_aging, (unsigned long) sw);
}

void ali_sw_stp_port_timer_init(struct net_ali_sw_port *p)
{
	//The information known to a port has expired. (20HZ)
	setup_timer(&p->message_age_timer, ali_sw_message_age_timer_expired, (unsigned long) p);

	//to change port status, the status machine. (15HZ)
	setup_timer(&p->forward_delay_timer, ali_sw_forward_delay_timer_expired, (unsigned long) p);

	//can't send two BPDUs in 1HZ.
	setup_timer(&p->hold_timer, ali_sw_hold_timer_expired, (unsigned long) p);
}

/* Report ticks left (in USER_HZ) used for API */
unsigned long ali_sw_timer_value(const struct timer_list *timer)
{
	return timer_pending(timer) ? jiffies_to_clock_t(timer->expires - jiffies) : 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void ali_sw_set_port_stp_status(struct net_ali_sw_port *p, u8 stp_status)
{
/*//	the HW status is a little different with the p->stp_state, since HW status lacks one state.*/
	p->stp_state = stp_status;
	ali_sw_set_port_stp_hwstatus(p->port_no, stp_status);	
}

void ali_sw_start_port_stp(struct net_ali_sw_port *p) {
	/*struct net_ali_sw *sw = p->sw; */
	//spin_lock(&sw->lock);
	sw_trace("start_port_stp: port_id=0x%x", p->port_id);
	
	ali_sw_init_stp_port(p);//port_id, status(blocking), role(designated)
	ali_sw_port_state_selection(p->sw);//status(listening), could receive and forward STP pkts(BPDU).
	//ali_sw_log_state(p);
	//spin_unlock(&sw->lock);
}

void ali_sw_stop_port_stp(struct net_ali_sw_port *p)//called when ports' link is down
{
//question, this sequence might have some problem...!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	struct net_ali_sw *sw = p->sw;
	int wasroot;
	
	//spin_lock(&sw->lock);
	sw_trace("stop_port_stp: port_id=0x%x", p->port_id);
	
	wasroot = ali_sw_is_root_sw(sw);//'1' means yes, it was.
	
	ali_sw_become_designated_port(p);
	
	p->topology_change_ack = 0;
	p->config_pending = 0;

	del_timer(&p->message_age_timer);
	del_timer(&p->forward_delay_timer);
	del_timer(&p->hold_timer);

//	br_fdb_delete_by_port(br, p, 0);
//	br_multicast_disable_port(p);			//TBD

	ali_sw_configuration_update(sw);
	ali_sw_port_state_selection(sw);

	//p->stp_state = ALI_SW_STATE_DISABLED;
	ali_sw_set_port_stp_status(p, ALI_SW_STATE_DISABLED);
	
	if (ali_sw_is_root_sw(sw) && !wasroot)
		ali_sw_become_root_sw(sw);

	//spin_unlock(&sw->lock);
}

void ali_sw_start_sw_stp(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;
	bool link_status = 0;
	
/****************************************************************************
	On starting up, the switch thinks itself as the ROOT switch,
	the ports belonging to the switch are assigned the role of "designated",
	and their status is "blocking".
	however, the status of "blocking" will soon be changed to "listening"!
****************************************************************************/

	//spin_lock(&sw->lock);
	mod_timer(&sw->hello_timer, jiffies + sw->hello_time);
	//mod_timer(&sw->gc_timer, jiffies + HZ/10);

	for(i=0;i<MAX_PORT_NUM;i++){
		p = &(sw->port_list[i]);
		if(ali_sw_check_port_link_status(p)==LINK_OKAY_TO_TRANSMIT)	{
			ali_sw_start_port_stp(p);
			if(i!=0)
				link_status = 1;
		}
	}

	if(link_status==0)
		sw_warn("%s: no external ports are linked up!", __FUNCTION__);

	sw->stp_is_working = ALI_SW_STP_WORKING;

	//sw_trace("...sending configuation BPDU!");
	ali_sw_config_bpdu_generation(sw);
	//spin_unlock(&sw->lock);
}

void ali_sw_stop_sw_stp(struct net_ali_sw *sw)
{
	struct net_ali_sw_port *p;
	u8 i;

	//spin_lock(&sw->lock);

	for(i=0;i<MAX_PORT_NUM;i++)
	{
		p = &(sw->port_list[i]);
		ali_sw_stop_port_stp(p);
	}
	
	del_timer_sync(&sw->hello_timer);
	del_timer_sync(&sw->topology_change_timer);
	del_timer_sync(&sw->tcn_timer);
	del_timer_sync(&sw->fast_aging_timer);	
	
	sw->stp_is_working = ALI_SW_STP_NOT_WORKING;

	//spin_unlock(&sw->lock);
}

static void ali_sw_init_stp_port(struct net_ali_sw_port *p)
{
	p->port_id = (u16)((p->priority<<8)|(p->port_no));
	ali_sw_become_designated_port(p);
	ali_sw_set_port_stp_status(p, ALI_SW_STATE_BLOCKING);
	
	p->topology_change_ack = 0;
	p->config_pending = 0;
	p->path_cost = ali_sw_get_port_cost(p);//this is local, not the accumulated one 

	sw_trace("init_port_stp: port_no:%d, port_id=0x%x, path_cost=%d\n", p->port_no, p->port_id, p->path_cost);
}

int ali_sw_stp_init(void)
{
	u8 i;
	p_ali_sw_port p;
/****************************************************************************
	net_device <--> p0_private <--> net_ali_sw
									|-----net_ali_sw_port controls PORT0
									|-----net_ali_sw_port controls PORT1
									|-----net_ali_sw_port controls PORT2
									|-----net_ali_sw_port controls PORT3
****************************************************************************/
	sw_trace("initializing STP facility...");
	
	/*** step1. initialize switch itself, id, cost.... ***/ 
	//NOTE: equals to 'br_add_bridge', and 'new_bridge_dev'
	if(gp_sw==NULL) {
		sw_error("gp_sw == NULL!");
        return -1;
    }

	gp_sw->sw_name[0] = 'A';
	gp_sw->sw_name[1] = 'L';
	gp_sw->sw_name[2] = 'i';
	gp_sw->sw_name[3] = '0';

	gp_sw->group_addr = stp_group_address;

	gp_sw->sw_id.prio[0] = (ali_sw_stp_priority>>8)&0xff;
	gp_sw->sw_id.prio[1] = ali_sw_stp_priority&0xff;
	//assume P0 has the smallest MAC address, TBD, needs a compare before.
	memcpy(gp_sw->sw_id.addr, (gp_sw->port_list[0]).mac_addr, ETH_ALEN);

	gp_sw->designated_root = gp_sw->sw_id;
	gp_sw->root_path_cost = 0;
	gp_sw->root_port = MAX_PORT_NUM;//question, this is 0? or we make it impossible? originally it's 0

	gp_sw->sw_hello_time = gp_sw->hello_time = 2 * HZ;
	gp_sw->sw_max_age = gp_sw->max_age = 20 * HZ;
	gp_sw->sw_forward_delay = gp_sw->forward_delay = 15 * HZ;
	gp_sw->topology_change = 0;
	gp_sw->topology_change_detected = 0;
	gp_sw->ageing_time = 300 * HZ;//question, we can't control it

	ali_sw_stp_timer_init(gp_sw);

	/*** step2. initialize ports: set status, set id (priority), cost.... ***/
	//NOTE: equals to 'br_add_if', 'new_nbp'
	for(i=0;i<MAX_PORT_NUM;i++)
	{	
		//this is something need to be initialized no matter port's link is down or up...
		p = &(gp_sw->port_list[i]);

		if(p->port_no!=i)
		{	
			sw_warn("weird, %s, %d", __FUNCTION__, __LINE__);
			p->port_no = i;
		}
		p->priority = 0x80;
		
		//port_id, status(blocking), role(designated)
		ali_sw_init_stp_port(p);
		//at beginning, make it DISABLED; if link down, make it DISABLED
		ali_sw_set_port_stp_status(p, ALI_SW_STATE_DISABLED);
		//buid up timer
		ali_sw_stp_port_timer_init(p);
		
		//br_multicast_add_port, TBD, question.
	}

	sw_trace("...STP initialization is done!");
		
//step3. get everything start to work and send BPDUs.
	//NOTE: equals to 'br_dev_open' 'br_stp_enable_bridge', will call 'br_stp_enable_port'.
	// claim it is the root bridge, cfg BPDU sending through it's ports whose link status is UP.
	
	//ali_sw_start_sw_stp(gp_sw);
	return 0;
}


void ali_sw_show_stp_info(void)
{
	u32 tmp1, tmp2;
	u8 idx;
	struct net_ali_sw_port *p;
	const stp_sw_id *id = &gp_sw->sw_id;
	
	pr_info("-----------------------------------------------------------------\n");
	//TBD, stb enable?
	pr_info("my sw:		%.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 	
										id->prio[0], id->prio[1],
										id->addr[0], id->addr[1], id->addr[2],
										id->addr[3], id->addr[4], id->addr[5]);
	id = &gp_sw->designated_root;
	pr_info("root sw:	%.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", 	
										id->prio[0], id->prio[1],
										id->addr[0], id->addr[1], id->addr[2],
										id->addr[3], id->addr[4], id->addr[5]);
	pr_info("root port: %d				path cost: %d\n", gp_sw->root_port, gp_sw->root_path_cost);
	pr_info("max age: %ld				switch max age: %ld\n", gp_sw->max_age/HZ, gp_sw->sw_max_age/HZ);
	pr_info("hello time: %ld				switch hello time: %ld\n", gp_sw->hello_time/HZ, gp_sw->sw_hello_time/HZ);
	pr_info("forward delay: %ld			switch forward delay: %ld\n", gp_sw->forward_delay/HZ, gp_sw->sw_forward_delay/HZ);
	pr_info("ageing time: %ld			(unit is second)\n", gp_sw->ageing_time/HZ);

	for(idx=0; idx<MAX_PORT_NUM; idx++)
	{
		p = ali_sw_get_port(gp_sw, idx);
        if (!p) {
            return;
        }
		pr_info("\nport no: %d, port id: 0x%x\n", p->port_no, p->port_id);
		pr_info("    stp state: %s		link: %s,	path cost:%d\n", \
			ali_sw_port_state_names[p->stp_state], ali_sw_check_port_link_status(p)==LINK_OKAY_TO_TRANSMIT?"up":"down",
			p->path_cost);

		pr_info("    root port: %c			designated port: %c\n", ali_sw_is_root_port(p)? 'Y':'N',ali_sw_is_designated_port(p)? 'Y':'N');
		id = &p->designated_root;
		pr_info("    root sw:       %.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",	
										id->prio[0], id->prio[1],
										id->addr[0], id->addr[1], id->addr[2],
										id->addr[3], id->addr[4], id->addr[5]);
		id = &p->designated_sw;
		pr_info("    designated sw: %.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",	
								id->prio[0], id->prio[1],
								id->addr[0], id->addr[1], id->addr[2],
								id->addr[3], id->addr[4], id->addr[5]);
		
		pr_info("    designated port: 0x%x		designated cost:%d\n", p->designated_port, p->designated_cost);
	}

	SW_W8(0x1f8, 0);
	tmp1 = SW_R32(0x14);
	tmp2 = SW_R32(0);	
	SW_W8(0x1f8, 4);
	sw_trace("p0[0]=0x%x, p0[0x14]=0x%x, fabric[0]=0x%x, fabric[0x148]=0x%x, fabric[0x188]=0x%x", tmp2, tmp1,
				SW_R32(0), SW_R32(0x148), SW_R32(0x188));
	
	pr_info("-----------------------------------------------------------------\n");
	
	return;
}





