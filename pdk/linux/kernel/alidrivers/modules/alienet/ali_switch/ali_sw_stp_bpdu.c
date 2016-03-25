/*
 *	Linux ALi switch
 *
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *
 */

#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/llc.h>
#include <net/net_namespace.h>
#include <net/llc.h>
#include <net/llc_pdu.h>
#include <net/stp.h>
#include <asm/unaligned.h>

#include "ali_sw.h"
#include "ali_sw_stp.h"

#define STP_HZ		256
#define LLC_RESERVE sizeof(struct llc_pdu_un)

static void ali_sw_send_bpdu(struct net_ali_sw_port *p,
			 const unsigned char *data, int length)
{
	struct sk_buff *skb;
	unsigned short len;
	struct ethhdr *eth;
	
	if(!p){
		sw_trace("p is NULL!");
		return;
	}
	
	skb = dev_alloc_skb(length+LLC_RESERVE);
	if (!skb)
		return;

	skb->dev = p->p_P0->dev;
	skb->protocol = htons(ETH_P_802_2);

	skb_reserve(skb, LLC_RESERVE);
	memcpy(__skb_put(skb, length), data, length);

	llc_pdu_header_init(skb, LLC_PDU_TYPE_U, LLC_SAP_BSPAN, LLC_SAP_BSPAN, LLC_PDU_CMD);
	llc_pdu_init_as_ui_cmd(skb);

//---------------------------------------------------------------------
	//llc_mac_hdr_init(skb, ((p->p_P0)->dev)->dev_addr, p->sw->group_addr);
	/* originally, it is 'llc_mac_hdr_init', but there is 'relocation truncated to fit: R_MIPS_26' error. */
	len = skb->len;

	skb_push(skb, sizeof(*eth));
	skb_reset_mac_header(skb);
	eth = eth_hdr(skb);
	eth->h_proto = htons(len);
	memcpy(eth->h_dest, p->sw->group_addr, ETH_ALEN);
	memcpy(eth->h_source, p->mac_addr, ETH_ALEN);

	skb_reset_mac_header(skb);
//---------------------------------------------------------------------
	SW_OUTPUT_SKB_CB(skb)->skb_id[0] = 'a';
	SW_OUTPUT_SKB_CB(skb)->skb_id[1] = 'l';
	SW_OUTPUT_SKB_CB(skb)->skb_id[2] = 'i';
	SW_OUTPUT_SKB_CB(skb)->pkt_id[0] = 's';
	SW_OUTPUT_SKB_CB(skb)->pkt_id[1] = 't';
	SW_OUTPUT_SKB_CB(skb)->pkt_id[2] = 'p';

	SW_OUTPUT_SKB_CB(skb)->forward_ports = 1<<p->port_no;
	/*	#define PORT0_EN			1
		#define PORT1_EN			2
		#define PORT2_EN			4
		#define PORT3_EN			8*/
	
#if 0
	sw_trace("\nstp: sending bpdu..., skb=0x%x, skb->data=0x%x, forward_ports=0x%x(hex)", \
				skb, skb->data, SW_OUTPUT_SKB_CB(skb)->forward_ports);
	//p0_print_line(skb->data, skb->len);
#endif

	dev_queue_xmit(skb);
	/*	maybe we should call "p0_start_xmit" directly, instead of dev_queue_xmit!
		question. */
}

static inline void ali_sw_set_ticks(unsigned char *dest, int j)
{
	unsigned long ticks = (STP_HZ * j)/ HZ;

	put_unaligned_be16(ticks, dest);
}

static inline int ali_sw_get_ticks(const unsigned char *src)
{
	unsigned long ticks = get_unaligned_be16(src);

	return DIV_ROUND_UP(ticks * HZ, STP_HZ);
}

/* called under sw lock */
void ali_sw_send_config_bpdu(struct net_ali_sw_port *p, struct ali_sw_config_bpdu *bpdu)
{
	unsigned char buf[35];

	if (p->sw->stp_enabled != ALI_SW_KERNEL_STP)
		return;

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = BPDU_TYPE_CONFIG;
	buf[4] = (bpdu->topology_change ? 0x01 : 0) |
		(bpdu->topology_change_ack ? 0x80 : 0);
	buf[5] = bpdu->root.prio[0];
	buf[6] = bpdu->root.prio[1];
	buf[7] = bpdu->root.addr[0];
	buf[8] = bpdu->root.addr[1];
	buf[9] = bpdu->root.addr[2];
	buf[10] = bpdu->root.addr[3];
	buf[11] = bpdu->root.addr[4];
	buf[12] = bpdu->root.addr[5];
	buf[13] = (bpdu->root_path_cost >> 24) & 0xFF;
	buf[14] = (bpdu->root_path_cost >> 16) & 0xFF;
	buf[15] = (bpdu->root_path_cost >> 8) & 0xFF;
	buf[16] = bpdu->root_path_cost & 0xFF;
	buf[17] = bpdu->sw_id.prio[0];
	buf[18] = bpdu->sw_id.prio[1];
	buf[19] = bpdu->sw_id.addr[0];
	buf[20] = bpdu->sw_id.addr[1];
	buf[21] = bpdu->sw_id.addr[2];
	buf[22] = bpdu->sw_id.addr[3];
	buf[23] = bpdu->sw_id.addr[4];
	buf[24] = bpdu->sw_id.addr[5];
	buf[25] = (bpdu->port_id >> 8) & 0xFF;
	buf[26] = bpdu->port_id & 0xFF;

	ali_sw_set_ticks(buf+27, bpdu->message_age);
	ali_sw_set_ticks(buf+29, bpdu->max_age);
	ali_sw_set_ticks(buf+31, bpdu->hello_time);
	ali_sw_set_ticks(buf+33, bpdu->forward_delay);//question.

	ali_sw_send_bpdu(p, buf, 35);
}

/* called under sw lock */
void ali_sw_send_tcn_bpdu(struct net_ali_sw_port *p)
{
	unsigned char buf[4];
    if(!p) {
        return;
    }
	if (p->sw->stp_enabled != ALI_SW_KERNEL_STP)
		return;

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = BPDU_TYPE_TCN;
	ali_sw_send_bpdu(p, buf, 4);
}

void ali_sw_stp_rcv(struct sk_buff *skb)
{
	struct net_ali_sw_port *p=NULL;
	struct net_ali_sw *sw;
	unsigned char *buf;
	u8	port_no;
	/* unsigned long flags; */

	if((SW_INPUT_SKB_CB(skb)->skb_id[0] != 'a')||(SW_INPUT_SKB_CB(skb)->skb_id[1] != 'l')\
			||(SW_INPUT_SKB_CB(skb)->skb_id[2] != 'i'))
		goto err;

	port_no = SW_INPUT_SKB_CB(skb)->port_no;
	
#if 1
	if(test_bit(5, &dbg_runtime_val))
		sw_trace("stp_bpdu_rcv: port_no=%d", port_no);
#endif

	sw = gp_sw;
	p = &(gp_sw->port_list[port_no]);

	if (!p)
		goto err;

	if (!pskb_may_pull(skb, 4))
		goto err;

	/* compare of protocol id and version, skb->data points to STP position in packet */
	buf = skb->data;

	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 0)
	{
		sw_error("%s: wrong packet", __FUNCTION__);
		goto err;
	}

	//spin_lock_irqsave(&sw->lock, flags);//question, tbd, shouldn't use such heave locks.

	if (sw->stp_enabled != ALI_SW_KERNEL_STP)
		goto out;

	if (p->stp_state == ALI_SW_STATE_DISABLED)
		goto out;

	buf = skb_pull(skb, 3);

	if (buf[0] == BPDU_TYPE_CONFIG) {
		struct ali_sw_config_bpdu bpdu;

		if (!pskb_may_pull(skb, 32))
			goto out;

		buf = skb->data;
		bpdu.topology_change = (buf[1] & 0x01) ? 1 : 0;
		bpdu.topology_change_ack = (buf[1] & 0x80) ? 1 : 0;

		bpdu.root.prio[0] = buf[2];
		bpdu.root.prio[1] = buf[3];
		bpdu.root.addr[0] = buf[4];
		bpdu.root.addr[1] = buf[5];
		bpdu.root.addr[2] = buf[6];
		bpdu.root.addr[3] = buf[7];
		bpdu.root.addr[4] = buf[8];
		bpdu.root.addr[5] = buf[9];
		bpdu.root_path_cost =
			(buf[10] << 24) |
			(buf[11] << 16) |
			(buf[12] << 8) |
			buf[13];
		bpdu.sw_id.prio[0] = buf[14];
		bpdu.sw_id.prio[1] = buf[15];
		bpdu.sw_id.addr[0] = buf[16];
		bpdu.sw_id.addr[1] = buf[17];
		bpdu.sw_id.addr[2] = buf[18];
		bpdu.sw_id.addr[3] = buf[19];
		bpdu.sw_id.addr[4] = buf[20];
		bpdu.sw_id.addr[5] = buf[21];
		bpdu.port_id = (buf[22] << 8) | buf[23];

		bpdu.message_age = ali_sw_get_ticks(buf+24);
		bpdu.max_age = ali_sw_get_ticks(buf+26);
		bpdu.hello_time = ali_sw_get_ticks(buf+28);
		bpdu.forward_delay = ali_sw_get_ticks(buf+30);

		ali_sw_received_config_bpdu(p, &bpdu);
	}
	else if (buf[0] == BPDU_TYPE_TCN) {
		ali_sw_received_tcn_bpdu(p);
	}
	
 out:
	//spin_unlock_irqrestore(&sw->lock, flags);
 err:
	kfree_skb(skb);
}





