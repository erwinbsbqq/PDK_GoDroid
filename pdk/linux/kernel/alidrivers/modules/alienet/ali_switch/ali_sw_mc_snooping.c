/*
 *	Linux ALi switch, 
 *	Multicast (IGMP/MLD snooping) Support.
 *	Authors:
 *	peter.li <gilbertjuly@gmail.com>/<peter.li@alitech.com>
 *
 *	As you can guess, IGMP/MLD snooping is not fully supported,
 *	currently, only join/leave message are processed.
 *	in future, it should recognize where the multicast route is,
 * 	and direct associated messages to the port which the route connects.
 *	Also, for each mulitcast client/route, it should has a timer to detect if it still exists.
 *
 */
 
#include <linux/err.h>
#include <linux/if_ether.h>
#include <linux/igmp.h>
#include <linux/jhash.h>
#include <linux/kernel.h>
#include <linux/log2.h>
#include <linux/netdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/random.h>
#include <linux/rculist.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <net/ip.h>
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
#include <net/ipv6.h>
//#include <net/mld.h>//this is new added, peter
#include <net/addrconf.h>
#include <net/ip6_checksum.h>
#endif

#include "ali_sw.h"
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
#include "ali_sw_mld.h"
#endif

#define mlock_dereference(X, br) \
	rcu_dereference_protected(X, lockdep_is_held(&br->multicast_lock))

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static inline int ipv6_is_transient_multicast(const struct in6_addr *addr)
{
	if (ipv6_addr_is_multicast(addr) && ((addr)->s6_addr[1] & 0x10))//IPV6_ADDR_MC_FLAG_TRANSIENT
		return 1;
	return 0;
}
#endif

static void ipv4_multicast_2_mac_addr(u8 *mac_addr, u32 groupv4)
{
	mac_addr[5] = groupv4>>24&0xff;
	mac_addr[4] = (groupv4>>16)&0xff;
	mac_addr[3] = (groupv4>>8)&0x7f;
	mac_addr[2] = 0x5e;
	mac_addr[1] =  00;
	mac_addr[0] =  01;

	sw_trace("ipv4group 0x%08x (host order) -->  mac 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x", \
			groupv4, mac_addr[0] , mac_addr[1], mac_addr[2], mac_addr[3] , mac_addr[4], mac_addr[5]);
}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static void ipv6_multicast_2_mac_addr(u8 *mac_addr, const struct in6_addr *groupv6)
{
	mac_addr[5] = groupv6->s6_addr[15];
	mac_addr[4] = groupv6->s6_addr[14];
	mac_addr[3] = groupv6->s6_addr[13];
	mac_addr[2] = groupv6->s6_addr[12];
	mac_addr[1] =  0x33;
	mac_addr[0] =  0x33;

	sw_trace("ipv6group 0x%08x-0x%08x-0x%08x-0x%08x (host order) -->  mac 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x", \
			groupv6->s6_addr32[0], groupv6->s6_addr32[1], groupv6->s6_addr32[2], groupv6->s6_addr32[3],
			mac_addr[0] , mac_addr[1], mac_addr[2], mac_addr[3] , mac_addr[4], mac_addr[5]);
}
#endif

static int ali_sw_multicast_leave_group(struct net_ali_sw_port *p, struct sw_mc_addr *group_info)
{
	struct net_ali_sw *sw = NULL;
	u8 flag=0;
	struct mc_alr_entry *p_entry;
	struct alr_wdata alr_entry;

	if (!p){
		sw_trace("%s: illegal port!", __FUNCTION__, __LINE__);
		return -1;
	}
	sw = p->sw;
	list_for_each_entry(p_entry, &p->sw->mc_alr_entry_list, list) {
		if(compare_ether_addr(p_entry->mc_mac_addr, group_info->mac_addr)==0){
			flag = 1;
			if(p->port_no==0)
				p_entry->forward_ports&=~PORT0_EN;
			else if(p->port_no==1)
				p_entry->forward_ports&=~PORT1_EN;
			else if(p->port_no==2)
				p_entry->forward_ports&=~PORT2_EN;
			else if(p->port_no==3)
				p_entry->forward_ports&=~PORT3_EN;	

			sw_trace("MC: find a MC entry already, delete port[%d]", p->port_no);
			break;
		}
	}
	
	if(flag == 1){
		if(p_entry->forward_ports==0){
			alr_entry.ports = 0;
			alr_entry.filter = 0;
			alr_entry.static_entry = 0;
			alr_entry.valid = 0;// '0' means to delete an entry
			alr_entry.age = 1;

			alr_entry.mac_addr_high16 = (p_entry->mc_mac_addr[0]<<8)|p_entry->mc_mac_addr[1];
			alr_entry.mac_addr_low32 = (p_entry->mc_mac_addr[2]<<24)|(p_entry->mc_mac_addr[3]<<16)|(p_entry->mc_mac_addr[4]<<8)|p_entry->mc_mac_addr[5];

			flag = ali_sw_alr_write_entry(&alr_entry);
			sw_trace("MC: alr entry has been deleted!");
		}else{
			alr_entry.ports = p_entry->forward_ports;
			alr_entry.filter = 0;
			alr_entry.static_entry = 1;
			alr_entry.valid = 0;// '1' means to update an entry
			alr_entry.age = 1;

			alr_entry.mac_addr_high16 = (p_entry->mc_mac_addr[0]<<8)|p_entry->mc_mac_addr[1];
			alr_entry.mac_addr_low32 = (p_entry->mc_mac_addr[2]<<24)|(p_entry->mc_mac_addr[3]<<16)|(p_entry->mc_mac_addr[4]<<8)|p_entry->mc_mac_addr[5];

			flag = ali_sw_alr_write_entry(&alr_entry);
			sw_trace("MC: alr entry has been updated (leave)!");
		}
		return flag;
	}

	return 0;
}

static int ali_sw_multicast_add_group(struct net_ali_sw_port *p, struct sw_mc_addr *group_info)
{
	struct net_ali_sw *sw = NULL;
	u8 flag=0;
	struct mc_alr_entry *p_entry;
	struct alr_wdata alr_entry;

	//spin_lock(&sw->multicast_lock);
	memset(&alr_entry, 0, sizeof(alr_entry));
	if (!p){
		sw_trace("%s: illegal port!", __FUNCTION__, __LINE__);
		return -1;
	}
    sw = p->sw;
	//sw_trace("%s:port_no=%d, mc_mac_addr=0x%x", __FUNCTION__, p->port_no, group_info->mac_addr);

	list_for_each_entry(p_entry, &p->sw->mc_alr_entry_list, list) {
		if(compare_ether_addr(p_entry->mc_mac_addr, group_info->mac_addr)==0){
			flag = 1;
			if(p->port_no==0)
				p_entry->forward_ports|=PORT0_EN;
			else if(p->port_no==1)
				p_entry->forward_ports|=PORT1_EN;
			else if(p->port_no==2)
				p_entry->forward_ports|=PORT2_EN;
			else if(p->port_no==3)
				p_entry->forward_ports|=PORT3_EN;	

			sw_trace("MC: find a MC entry already, add port[%d]", p->port_no);
			break;
		}
	}

	if(flag == 0){
		p_entry = kzalloc(sizeof(*p_entry), GFP_ATOMIC);
		memcpy(p_entry->mc_mac_addr, group_info->mac_addr, 6);
		
		if(p->port_no==0)
			p_entry->forward_ports|=PORT0_EN;
		else if(p->port_no==1)
			p_entry->forward_ports|=PORT1_EN;
		else if(p->port_no==2)
			p_entry->forward_ports|=PORT2_EN;
		else if(p->port_no==3)
			p_entry->forward_ports|=PORT3_EN;
		
		list_add(&p_entry->list, &p->sw->mc_alr_entry_list);
		sw_trace("MC: create a new MC entry, add port[%d]: ", p->port_no);
	}


	alr_entry.ports = p_entry->forward_ports;
	alr_entry.filter = 0;
	alr_entry.static_entry = 1;
	alr_entry.valid = 1;// '1' means to update an entry
	alr_entry.age = 1;

	alr_entry.mac_addr_high16 = (p_entry->mc_mac_addr[0]<<8)|p_entry->mc_mac_addr[1];
	alr_entry.mac_addr_low32 = (p_entry->mc_mac_addr[2]<<24)|(p_entry->mc_mac_addr[3]<<16)|(p_entry->mc_mac_addr[4]<<8)|p_entry->mc_mac_addr[5];
				
	flag = ali_sw_alr_write_entry(&alr_entry);
	sw_trace("MC: alr entry has been updated (add)!");

	//spin_unlock(&sw->multicast_lock);
out:
	return flag;
}

static int ali_sw_ip4_multicast_add_group(struct net_ali_sw_port *p,  __be32 group)
{
	struct sw_mc_addr sw_group;

	if (ipv4_is_local_multicast(group))
		return 0;

	sw_group.u.ip4 = group;
	sw_group.proto = htons(ETH_P_IP);
	ipv4_multicast_2_mac_addr(sw_group.mac_addr, group);

	return ali_sw_multicast_add_group(p, &sw_group);
}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static int ali_sw_ip6_multicast_add_group(struct net_ali_sw_port *p,  const struct in6_addr *group)
{
	struct sw_mc_addr sw_group;

	if (!ipv6_is_transient_multicast(group))
		return 0;

	sw_group.u.ip6 = *group;
	sw_group.proto = htons(ETH_P_IPV6);
	ipv6_multicast_2_mac_addr(sw_group.mac_addr, group);

	return ali_sw_multicast_add_group(p, &sw_group);
}
#endif

static int ali_sw_ip4_multicast_leave_group(struct net_ali_sw_port *p,  __be32 group)
{
	struct sw_mc_addr sw_group;

	if (ipv4_is_local_multicast(group))
		return;

	sw_group.u.ip4 = group;
	sw_group.proto = htons(ETH_P_IP);
	ipv4_multicast_2_mac_addr(sw_group.mac_addr, group);

	return ali_sw_multicast_leave_group(p, &sw_group);
}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static int ali_sw_ip6_multicast_leave_group(struct net_ali_sw_port *p,  const struct in6_addr *group)
{
	struct sw_mc_addr sw_group;

	if (!ipv6_is_transient_multicast(group))
		return;

	sw_group.u.ip6 = *group;
	sw_group.proto = htons(ETH_P_IPV6);
	ipv6_multicast_2_mac_addr(sw_group.mac_addr, group);

	return ali_sw_multicast_leave_group(p, &sw_group);
}
#endif

static int ali_sw_ip4_multicast_igmp3_report(struct sk_buff *skb, struct net_ali_sw_port *p)
{
	//sw_trace("%s, port_no=%d", __FUNCTION__, p->port_no);
	struct igmpv3_report *ih;
	struct igmpv3_grec *grec;
	int i;
	int len;
	int num;
	int type;
	int err = 0;
	__be32 group;

	if (!pskb_may_pull(skb, sizeof(*ih))){
		sw_trace("%s, %d, pskb_may_pull", __FUNCTION__, __LINE__);
		if(p->port_no!=0)
			return -EINVAL;
	}

	ih = igmpv3_report_hdr(skb);
	num = ntohs(ih->ngrec);
	len = sizeof(*ih);

	for (i = 0; i < num; i++) {
		len += sizeof(*grec);
		if (!pskb_may_pull(skb, len)){
			sw_trace("%s, %d, pskb_may_pull", __FUNCTION__, __LINE__);
			if(p->port_no!=0)
				return -EINVAL;
		}
		grec = (void *)(skb->data + len - sizeof(*grec));
		group = grec->grec_mca;
		type = grec->grec_type;

		len += ntohs(grec->grec_nsrcs) * 4;
		if (!pskb_may_pull(skb, len)){
			sw_trace("%s, %d, pskb_may_pull", __FUNCTION__, __LINE__);
			if(p->port_no!=0)
				return -EINVAL;
		}
		/* We treat this as an IGMPv2 report for now. */
		switch (type) {
		case IGMPV3_MODE_IS_INCLUDE:
		case IGMPV3_MODE_IS_EXCLUDE:
		case IGMPV3_CHANGE_TO_INCLUDE:
			/*INCLUDE source multicast ip address, if NULL, means leave group*/
			/*so we only assume it has no source mc ip address, to fully support IGMPv3, a lot needs to be done, TBD.*/
			err = ali_sw_ip4_multicast_leave_group(p, group);			
			break;
		case IGMPV3_CHANGE_TO_EXCLUDE:
			/*EXCLUDE source multicast ip address, if NULL, means join group*/
			/*so we only assume it has no source mc ip address, to fully support IGMPv3, a lot needs to be done, TBD.*/
			err = ali_sw_ip4_multicast_add_group(p, group);	
			break;
		case IGMPV3_ALLOW_NEW_SOURCES:
		case IGMPV3_BLOCK_OLD_SOURCES:
			break;

		default:
			continue;
		}
/*
		err = ali_sw_ip4_multicast_add_group(p, group);
		if (err)
			break;
*/			
	}

	return err;	
}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static int ali_sw_ip6_multicast_mld2_report(struct net_ali_sw_port *p,
										struct sk_buff *skb)
{
	struct icmp6hdr *icmp6h;
	struct mld2_grec *grec;
	int i;
	int len;
	int num;
	int err = 0;

	if (!pskb_may_pull(skb, sizeof(*icmp6h)))
		return -EINVAL;

	icmp6h = icmp6_hdr(skb);
	num = ntohs(icmp6h->icmp6_dataun.un_data16[1]);
	len = sizeof(*icmp6h);

	for (i = 0; i < num; i++) {
		__be16 *nsrcs, _nsrcs;

		nsrcs = skb_header_pointer(skb,
					   len + offsetof(struct mld2_grec,
							  grec_nsrcs),
					   sizeof(_nsrcs), &_nsrcs);
		if (!nsrcs)
			return -EINVAL;

		if (!pskb_may_pull(skb,
				   len + sizeof(*grec) +
				   sizeof(struct in6_addr) * ntohs(*nsrcs)))
			return -EINVAL;

		grec = (struct mld2_grec *)(skb->data + len);
		len += sizeof(*grec) +
		       sizeof(struct in6_addr) * ntohs(*nsrcs);

		/* We treat these as MLDv1 reports for now. */
		switch (grec->grec_type) {
		case MLD2_CHANGE_TO_INCLUDE:
			err = ali_sw_ip6_multicast_leave_group(p, &grec->grec_mca);
			if (!err)
				break;
		case MLD2_CHANGE_TO_EXCLUDE:
			err = ali_sw_ip6_multicast_add_group(p, &grec->grec_mca);
			if (!err)
				break;
		case MLD2_MODE_IS_INCLUDE:
		case MLD2_MODE_IS_EXCLUDE:
		case MLD2_ALLOW_NEW_SOURCES:
		case MLD2_BLOCK_OLD_SOURCES:
			sw_error("unsupported yet (0x%x)!", grec->grec_type);
			break;

		default:
			continue;
		}

	}

	return err;
}
#endif


static int ali_sw_multicast_ipv4_rcv( struct net_ali_sw_port *p,  struct sk_buff *skb)
{
	struct sk_buff *skb2 = skb;
	struct iphdr *iph;
	struct igmphdr *ih;
	unsigned len;
	unsigned offset;
	int err;

	/* We treat OOM as packet loss for now. */
	if (!pskb_may_pull(skb, sizeof(*iph)))
		return -EINVAL;

	iph = ip_hdr(skb);

	if (iph->ihl < 5 || iph->version != 4)
		return -EINVAL;
	
	if (!pskb_may_pull(skb, ip_hdrlen(skb)))
		return -EINVAL;

	iph = ip_hdr(skb);

	if (unlikely(ip_fast_csum((u8 *)iph, iph->ihl)))
		return -EINVAL;

	if (iph->protocol != IPPROTO_IGMP)
		return 0;

	len = ntohs(iph->tot_len);
	if (skb->len < len || len < ip_hdrlen(skb))
		return -EINVAL;

	if (skb->len > len) {
		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (!skb2)
			return -ENOMEM;

		err = pskb_trim_rcsum(skb2, len);
		if (err)
			goto err_out;
	}
	
	len -= ip_hdrlen(skb2);
	offset = skb_network_offset(skb2) + ip_hdrlen(skb2);
	__skb_pull(skb2, offset);
	skb_reset_transport_header(skb2);

	err = -EINVAL;
	if (!pskb_may_pull(skb2, sizeof(*ih)))
		goto out;

	switch (skb2->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (!csum_fold(skb2->csum))
			break;
		/* fall through */
	case CHECKSUM_NONE:
		skb2->csum = 0;
		if (skb_checksum_complete(skb2))
			goto out;
	}
	err = 0;

	SW_INPUT_SKB_CB(skb)->igmp = 1;
	ih = igmp_hdr(skb2);
	switch (ih->type) {
	case IGMP_HOST_MEMBERSHIP_REPORT:
	case IGMPV2_HOST_MEMBERSHIP_REPORT:
		SW_INPUT_SKB_CB(skb2)->mrouters_only = 1;
		sw_trace("recv IGMP(V1_V2)_HOST_MEMBERSHIP_REPORT");
		err = ali_sw_ip4_multicast_add_group(p, ih->group);
		break;
	case IGMPV3_HOST_MEMBERSHIP_REPORT:
		sw_trace("recv IGMPV3_HOST_MEMBERSHIP_REPORT");
		err = ali_sw_ip4_multicast_igmp3_report(skb2, p);
		break;
	case IGMP_HOST_MEMBERSHIP_QUERY:
		sw_trace("recv IGMP_HOST_MEMBERSHIP_QUERY");
		//err = ali_sw_ip4_multicast_query(sw, port, skb2);//TBD, unsupported yet.
		break;
	case IGMP_HOST_LEAVE_MESSAGE:
		sw_trace("recv IGMP_HOST_LEAVE_MESSAGE");
		ali_sw_ip4_multicast_leave_group(p, ih->group);
		break;
	default:
		sw_trace("processing IGMP, ih->type=0x%x", ih->type);
	}

out:
	__skb_push(skb2, offset);
err_out:
	if (skb2 != skb)
		kfree_skb(skb2);
	return err;	
}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
static int ali_sw_multicast_ipv6_rcv( struct net_ali_sw_port *p,  struct sk_buff *skb)
{
	struct sk_buff *skb2;
	struct ipv6hdr *ip6h;
	struct icmp6hdr *icmp6h;
	__be16 frag_offp;
	u8 nexthdr;
	unsigned len;
	int offset;
	int err;

	if (!pskb_may_pull(skb, sizeof(*ip6h)))
		return -EINVAL;

	ip6h = ipv6_hdr(skb);

	/*
	 * We're interested in MLD messages only.
	 *  - Version is 6
	 *  - MLD has always Router Alert hop-by-hop option
	 *  - But we do not support jumbrograms.
	 */
	if (ip6h->version != 6 ||
	    ip6h->nexthdr != IPPROTO_HOPOPTS ||
	    ip6h->payload_len == 0)
		return 0;

	len = ntohs(ip6h->payload_len) + sizeof(*ip6h);
	if (skb->len < len)
		return -EINVAL;

	nexthdr = ip6h->nexthdr;
	offset = ipv6_skip_exthdr(skb, sizeof(*ip6h), &nexthdr, &frag_offp);

	if (offset < 0 || nexthdr != IPPROTO_ICMPV6)
		return 0;

	/* Okay, we found ICMPv6 header */
	skb2 = skb_clone(skb, GFP_ATOMIC);
	if (!skb2)
		return -ENOMEM;

	err = -EINVAL;
	if (!pskb_may_pull(skb2, offset + sizeof(struct icmp6hdr)))
		goto out;

	len -= offset - skb_network_offset(skb2);

	__skb_pull(skb2, offset);
	skb_reset_transport_header(skb2);

	icmp6h = icmp6_hdr(skb2);

	switch (icmp6h->icmp6_type) {
	case ICMPV6_MGM_QUERY:
	case ICMPV6_MGM_REPORT:
	case ICMPV6_MGM_REDUCTION:
	case ICMPV6_MLD2_REPORT:
		break;
	default:
		err = 0;
		goto out;
	}

	/* Okay, we found MLD message. Check further. */
	if (skb2->len > len) {
		err = pskb_trim_rcsum(skb2, len);
		if (err)
			goto out;
	}

	switch (skb2->ip_summed) {
	case CHECKSUM_COMPLETE:
		if (!csum_fold(skb2->csum))
			break;
		/*FALLTHROUGH*/
	case CHECKSUM_NONE:
		skb2->csum = 0;
		if (skb_checksum_complete(skb2))
			goto out;
	}

	err = 0;

	SW_INPUT_SKB_CB(skb)->igmp = 1;
	switch (icmp6h->icmp6_type) {
	case ICMPV6_MGM_REPORT:
	    {
		sw_trace("recv ICMPV6_MGM_REPORT");
		struct mld_msg *mld;
		if (!pskb_may_pull(skb2, sizeof(*mld))) {
			err = -EINVAL;
			goto out;
		}
		mld = (struct mld_msg *)skb_transport_header(skb2);
		SW_INPUT_SKB_CB(skb2)->mrouters_only = 1;
		err = ali_sw_ip6_multicast_add_group(p, &mld->mld_mca);
		break;
	    }
	case ICMPV6_MLD2_REPORT:
	    {		
		sw_trace("recv ICMPV6_MLD2_REPORT");
		err = ali_sw_ip6_multicast_mld2_report(p, skb2);
		break;
	    }
	case ICMPV6_MGM_QUERY:
	    {			
		//err = ali_sw_ip6_multicast_query(p, skb2);//TBD, unsupported yet.
		sw_trace("recv ICMPV6_MGM_QUERY");
		break;
	    }		
	case ICMPV6_MGM_REDUCTION:
	    {
		sw_trace("recv ICMPV6_MGM_REDUCTION");
		struct mld_msg *mld;
		if (!pskb_may_pull(skb2, sizeof(*mld))) {
			err = -EINVAL;
			goto out;
		}
		mld = (struct mld_msg *)skb_transport_header(skb2);
		ali_sw_ip6_multicast_leave_group(p, &mld->mld_mca);
	    }
	}

out:
	kfree_skb(skb2);
	return err;
	
}
#endif

struct sk_buff *ali_sw_multicast_rcv(struct sk_buff *skb)
{
	struct net_ali_sw *sw = NULL;
	struct net_ali_sw_port* p=NULL;
	u8	port_no;
	unsigned long flags = 0;

	//sw_trace("%s:%d, gp_sw=0x%x", __FUNCTION__, __LINE__, gp_sw);

	if((SW_INPUT_SKB_CB(skb)->skb_id[0] != 'a')||(SW_INPUT_SKB_CB(skb)->skb_id[1] != 'l')\
			||(SW_INPUT_SKB_CB(skb)->skb_id[2] != 'i')){	
		sw_warn("%s, receive pocket we shouldn't handler", __FUNCTION__);
		goto drop;
	}
	
	port_no = SW_INPUT_SKB_CB(skb)->port_no;
	//sw_trace("%s:%d, port_no=%d", __FUNCTION__, __LINE__, port_no);
	
	SW_INPUT_SKB_CB(skb)->igmp = 0;
	SW_INPUT_SKB_CB(skb)->mrouters_only = 0;

	sw = gp_sw;
	p = &(gp_sw->port_list[port_no]);
	if (!p){
		sw_warn("%s, port_no unrecognized!", __FUNCTION__);
		goto drop;
	}
	//sw_trace("%s:%d", __FUNCTION__, __LINE__);

	spin_lock_irqsave(&sw->lock, flags);
	switch (skb->protocol) 
	{
		case htons(ETH_P_IP):
			//sw_trace("%s:%d", __FUNCTION__, __LINE__);
			ali_sw_multicast_ipv4_rcv(p, skb);
			break;
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)			
		case htons(ETH_P_IPV6):
			ali_sw_multicast_ipv6_rcv(p, skb);
			break;
#endif
		default:
			sw_warn("MC didn't recognized!");
			break;			
	}
	spin_unlock_irqrestore(&sw->lock, flags);
	//sw_trace("%s:%d", __FUNCTION__, __LINE__);
drop:
/*	should not be free directly, TBD.
	if it's a report IGMP/MLD, we need to transmit this packet out to the port which muliticast route is connected.
	if it's a query IGMP/MLD, we need to reley(flood?) this packet. */	
	kfree_skb(skb);
	return 0;
}


int ali_sw_mc_snooping_init()
{
	//spin_lock_init(&gp_sw->multicast_lock);

	printk(KERN_INFO "ali_sw_mc_snooping_init entered!\n");
	return 0;
}

/*
224.0.0.0 － Base address
224.0.0.1 － 网段中所有支持多播的主机
224.0.0.2 － 网段中所有支持多播的路由器
224.0.0.4 － 网段中所有的DVMRP路由器
224.0.0.5 － 所有的OSPF路由器
224.0.0.6 － 所有的OSPF指派路由器
224.0.0.7 － 所有的ST路由器
224.0.0.8 － 所有的ST主机
224.0.0.9 － 所有RIPv2路由器
224.0.0.10 － 网段中所有支的路由器
224.0.0.11 － Mobile-Agents
224.0.0.12 － DHCP server / relay agent.
224.0.0.13 － 所有的PIM路由器
224.0.0.22 － 所有的IGMP路由器（V3版本）
224.0.0.251 － 所有的支持组播的DNS服务器
*/

