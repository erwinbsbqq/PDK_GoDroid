/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: ali_transport.h
 *  (I)
 *  Description: ali transport driver helps to the communication between kernel 
 *			  and user space. it is build by the netlink
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.11				Sam		Create
 ****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>

#include <net/netlink.h>
#include <linux/semaphore.h>

#include <linux/ali_transport.h>

#if 0
#define PRF printk
#else
#define PRF(...) 	do{}while(0)
#endif

static struct semaphore m_sema;
static struct sock *m_sock = NULL;

static void recv_handler(struct sk_buff *skb)
{
	struct sk_buff *skb_r = NULL;
	struct nlmsghdr *nlhdr = NULL;

	skb_r = skb_get(skb);
	nlhdr = (struct nlmsghdr *)skb_r->data;

	PRF("receive data pid %d\n", nlhdr->nlmsg_pid);
}

int ali_transport_send_msg(int port_id, void *msg_buf, int msg_size)
{
	struct nlmsghdr *nlhdr = NULL;	
	struct sk_buff *skb = NULL;
	int ret = 0;

	if((msg_size > MAX_TRANSPORT_MSG_SIZE)                                   
			|| (port_id == -1)                                               
			|| (port_id == 0)                                                
			|| (msg_size <= 0)){
		PRF("port_id %d or msg_size %d fail\n", port_id, msg_size);
		return -1;
	}

	if(down_interruptible(&m_sema)){
		PRF("%s : down sem fail\n", __FUNCTION__);
		return -1;	
	}

	skb = alloc_skb(NLMSG_SPACE(msg_size)
		, GFP_KERNEL);
	if(skb == NULL) {
		PRF("%s : create send skb fail\n", __FUNCTION__);
		ret = -1;
		goto EXIT;
	}

	nlhdr = nlmsg_put(skb, port_id, 0, NLMSG_MIN_TYPE + 2, msg_size, 0);
	if (!nlhdr) {
		PRF("nlmsg put fail\n");
		ret = -1;
		goto EXIT;
	}	

	memcpy(NLMSG_DATA(nlhdr), msg_buf, msg_size);

	ret = netlink_unicast(m_sock, skb, port_id, MSG_DONTWAIT);
	PRF("netlink unicast pid %d ret %d\n", port_id, ret);

EXIT:
	up(&m_sema);
	return ret;
}

EXPORT_SYMBOL(ali_transport_send_msg);

static int __init ali_transport_init(void)
{
extern struct net init_net;

	m_sock = netlink_kernel_create(&init_net
		, NETLINK_ALITRANSPORT, 0, recv_handler, NULL, THIS_MODULE);
	if(m_sock == NULL) {
		PRF("%s : create sock fail\n", __FUNCTION__);
		return - 1;
	}
	
//	init_MUTEX(&m_sema);
	sema_init(&m_sema, 1);

	PRF("init ali transport done\n");
	return 0;
}

static void __exit ali_transport_exit(void)
{
	if(m_sock != NULL)
		netlink_kernel_release(m_sock);
}

module_init(ali_transport_init);
module_exit(ali_transport_exit);
 
MODULE_AUTHOR("ALi (Shanghai) Corporation");
MODULE_DESCRIPTION("ali Transport driver for dual CPU platform");
MODULE_LICENSE("GPL");

