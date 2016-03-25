/*
 * Copyright (c) 2010, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#undef DEBUG

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/hugetlb.h>
#include <linux/highmem.h>

#include "mcomm.h"
#include "mcomm_compat.h"
#include "shm_comm_hw.h"

/*Added by tony on 2014/06/08*/
#if defined(CONFIG_ALI_MBX_AS)

#include <linux/delay.h>
#include "shm_comm_hw.h"
#include <ali_mbx_as.h>

Uint8  g_mbx_revd_buf[MBX_RCV_MAX_BUF+0x100];   /*mailbox received buffer*/
Uint32 g_mbx_revd_len = 0;                      /*recevied total length*/
Uint32 g_mbx_handshake_req = 0;
Uint32 g_mbx_handshake_ack = 0;
Uint32 g_mbx_prepare = 0;
/*Handshake Msg*/
const char mbx_handshake_msg_req[] = {"REQ HandShake"};
const char mbx_handshake_msg_ack[] = {"ACK HandShake"};

extern unsigned long __G_ALI_MM_MCAPI_MEM_START_ADDR;
extern Uint32 MbxRegStatus(Uint32 mbxIdx);

//#define MBX_DEBUG

#endif
/*Added end*/

#define COMM_FIFO_MAX    128

#define WAKEUP_MBX0         0
#define WAKEUP_MBX1         1
#define WAKEUP_MBX2         2
#define WAKEUP_MBX3         3

struct mcomm_devdata {
	struct work_struct wq_mbx0;
	struct work_struct wq_mbx1;
	wait_queue_head_t waitCtxt; /*Doesn't be used now*/
	struct semaphore  waitSem;  /*Added by tony*/
	struct cdev cdev;
	struct resource mem;
	void __iomem *mbox_mapped;
	void *platform_data;
	atomic_t refcount;
	unsigned int irq0;      /* Mailbox0 irq */
	unsigned int irq1;      /* Mailbox1 irq */
	unsigned int irq2;      /* Mailbox2 irq */
	unsigned int irq3;      /* Mailbox3 irq */
    atomic_t head;
    atomic_t tail;
    u32 fifo[COMM_FIFO_MAX];
};

/* Only supports a single mcomm region. */
static struct mcomm_devdata _mcomm_devdata;

static struct mcomm_platform_ops *mcomm_platform_ops;

static long mcomm_dev_ioctl_init(struct mcomm_devdata *devdata);

int g_mcommfifoelement = 0; /*Added by tony on 2013/05/13*/

/* Push a context at the head of the mcomm fifo. */
static void mcomm_fifo_push(struct mcomm_devdata *devdata, u32 ctxt)
{
    int head, tail;
    
    head = atomic_read(&devdata->head);
    //tail = atomic_read(&devdata->tail);

    /*for mailbox debug*/
    //printk("<MAIN>======= MAIN mcomm_fifo_push got mailbox data! \n");

    /*Changed by tony on 2013/05/13*/

      g_mcommfifoelement = 1;

        devdata->fifo[head] = ctxt;
        atomic_set(&devdata->head, ((head + 1) & (COMM_FIFO_MAX - 1)));
		pr_debug("%s: push context: %d at: %d\n", __func__, ctxt, head);

	//printk("##### <MAIN> %s debug, head:%d, tail:%d, g_mcommfifoelement:%d\n", __func__, devdata->head, devdata->tail, g_mcommfifoelement);

}

#if defined(CONFIG_ALI_MBX_AS)

void mbx_ctrl_process()
{
	 void *pwbuf = (void *)__G_ALI_MM_MCAPI_MEM_START_ADDR;
	 unsigned char *ptemp = pwbuf;
#ifdef MBX_DEBUG	
	 printk("mbx_ctrl_process, entered!\n");
#endif
	 if(!memcmp(pwbuf, (void*)mbx_handshake_msg_req, strlen(mbx_handshake_msg_req)))
	 {
	   g_mbx_handshake_req = 0x01;
#ifdef MBX_DEBUG
	   printk("mbx_ctrl_process, received REQ!\n");
#endif
	 }
	 else
	 {
	 	 if(!memcmp(pwbuf, (void*)mbx_handshake_msg_ack, strlen(mbx_handshake_msg_ack)))
	   {
	      g_mbx_handshake_ack = 0x01;
#ifdef MBX_DEBUG
	      printk("mbx_ctrl_process, received ACK!\n");
#endif
	   }
	   else
	   	  printk("mbx_ctrl_process received a bad msg!!!, MBX_RBUF_IDX:0x%x, buffer[0~3]:0x%x,0x%x,0x%x,0x%x,\n", MBX_RBUF_IDX, ptemp[0],ptemp[1],ptemp[2],ptemp[3]);
	 }
	 	 
}

int checkChunkBuffer(u32 ctxt)
{
	 int ret = 0;
	 unsigned int pktlen, pkttol, pktcur, pkttype; 
	 void *pwbuf = (void *)__G_ALI_MM_MCAPI_MEM_START_ADDR;
	
	 pktlen = MBX_REG_GET_LEN(ctxt);
	 pkttol = MBX_REG_GET_TOL_PKT(ctxt);
	 pktcur = MBX_REG_GET_CUR_PKT(ctxt);
	 pkttype = MBX_REG_GET_MSG_TYPE(ctxt);
#ifdef MBX_DEBUG	 
	 printk("checkChunkBuffer, pktlen:0x%x, pkttol:0x%x, pktcur:0x%x, pkttype:0x%x\n", pktlen, pkttol, pktcur, pkttype);
#endif	 
	 if(pkttype == MBX_CTRL_MSG)
	 	  mbx_ctrl_process();
	 else if(pkttype == MBX_DATA_MSG)
	 {
	 	  if(pktcur==0)
	 	  {
	 	     g_mbx_revd_len = 0;
	 	  }
	 	  if((g_mbx_revd_len+pktlen)>MBX_RCV_MAX_BUF || pktlen>MBX_RBUF_LEN)
	 	  {
	 	  	 printk("checkChunkBuffer, Received data overflow!!! pktlen:0x%x\n", pktlen);
	 	  	 ret = 0;
	 	  }
	 	  else
	 	  {	
	 	     memcpy((void*)&g_mbx_revd_buf[g_mbx_revd_len], (void*)&pwbuf[MBX_RBUF_IDX], pktlen);
	 	     g_mbx_revd_len += pktlen;
	 	  }
	 	  if((pktcur+1) >= pkttol)
	 	  {
	 	  	ret = 1;  /*Means received the last chunk data*/
#ifdef MBX_DEBUG	 	  	
	 	  	printk("checkChunkBuffer, received the last chunk data, g_mbx_revd_len:0x%x\n", g_mbx_revd_len);
#endif
	 	  }
	 }
	 else
	 {
	 	  printk("checkChunkBuffer got a invalid Msg!\n");
	 }
	
	 writel(MBX_RECEIVED_MASK, REG_SEE_MBX1_SEND); //tell the peer we have received the msg
	
	 return ret;
}

#endif

/** Wake up the process(es) corresponding to the mailbox(es) which just received
 *  packets, since there is no way to make wake_up_interruptible known which isr
 *  waking up in the multi isr services, thus, I divide the pending process to 5
 *  wake_up_interruptible conditions.
 */
static irqreturn_t mcomm_interrupt(int irq, void *dev_id)
{
    u32 ctxt;
	struct mcomm_devdata *devdata = dev_id;

    if (irq == devdata->irq0) {
        ctxt = mcomm_platform_ops->recv(MBX0);
        
#if defined(CONFIG_ALI_MBX_AS)          
        if(!g_mbx_prepare)
        	return IRQ_HANDLED;
#endif
	/*for mailbox debug*/
	//printk("<MAIN>====== MAIN mcomm_interrupt got mailbox0 sync from see! ctxt:%d\n",ctxt);
   	/*Changed by tony on 2013/05/21*/
	if (ctxt == SEE_ACK) {
            //mcomm_platform_ops->sync();
            schedule_work(&devdata->wq_mbx0);
        }
    }
    if (irq == devdata->irq1) {
        /* An interrupt from See to transfer mailbox context. */
        ctxt = mcomm_platform_ops->recv(MBX1);
#if defined(CONFIG_ALI_MBX_AS)        
        if(!g_mbx_prepare)
        	return IRQ_HANDLED;
#endif
	/*for mailbox debug*/
	//printk("<MAIN>====== MAIN mcomm_interrupt got mailbox1 data! ctxt:%d, g_mcommfifoelement:%d\n",ctxt, g_mcommfifoelement);
#if defined(CONFIG_ALI_MBX_AS)
     if(checkChunkBuffer(ctxt))
     {
#ifdef MBX_DEBUG     	
     	 printk("Mbx got a last chunk buffer!\n");
#endif
#endif	
	
	     if(g_mcommfifoelement){
		      printk("mcomm_interrupt, get a unexpteced mailbox1 interrupt!\n");
	     }
 	     else{
		      mcomm_fifo_push(devdata, ctxt);
		      /*Changed by tony*/
        	//wake_up_interruptible(&devdata->waitCtxt);
        	//up(&devdata->waitSem);
        	schedule_work(&devdata->wq_mbx1);
	     }
#if defined(CONFIG_ALI_MBX_AS)
     }
#endif
	
    }
    if (irq == devdata->irq2) {
    	  
        ctxt = mcomm_platform_ops->recv(MBX2);
#if defined(CONFIG_ALI_MBX_AS)        
        if(!g_mbx_prepare)
        	return IRQ_HANDLED;
#endif        	
    }
    if (irq == devdata->irq3) {
    	  
        ctxt = mcomm_platform_ops->recv(MBX3);
#if defined(CONFIG_ALI_MBX_AS)        
        if(!g_mbx_prepare)
        	return IRQ_HANDLED;
#endif        	
    }

	return IRQ_HANDLED;
}

static void mcomm_wq_mbx0(struct work_struct *work)
{
	struct mcomm_devdata *dev = container_of(work,
					struct mcomm_devdata, wq_mbx0);
	/** Sync local process.
	 */
	mcomm_platform_ops->sync();
}

static void mcomm_wq_mbx1(struct work_struct *work)
{
	struct mcomm_devdata *dev = container_of(work,
					struct mcomm_devdata, wq_mbx1);
	/** Sync local process.
	 */
	up(&dev->waitSem);
}

static long mcomm_fd_ioctl_wait_read(struct mcomm_devdata *devdata, u32 *ctxt)
{
    int head, tail, ret = 0;

    if (!devdata || !ctxt){
	printk("<MAIN>mcomm_fd_ioctl_wait_read, got dev handle failed!\n");
        return -1;
    }
	//printk("<MAIN>====== MAIN Mcapi wait data pre\n");
	/*Changed by tony on 2013/05/20*/
	down(&devdata->waitSem);
	//printk("<MAIN>====== MAIN Mcapi wait data return, g_mcommfifoelement:%d\n", g_mcommfifoelement);

        /* Get the context. */
        head = atomic_read(&devdata->head);
        tail = atomic_read(&devdata->tail);

	//printk("==============<MAIN>%s: head:%d, tail:%d, g_mcommfifoelement:%d.\n", __func__, head, tail, g_mcommfifoelement);
        *ctxt = devdata->fifo[tail];
        atomic_set(&devdata->tail, ((tail + 1) & (COMM_FIFO_MAX - 1)));
	
	if(g_mcommfifoelement)  
              	g_mcommfifoelement = 0;

    return (long)ret;
}

static long mcomm_fd_ioctl_notify(struct mcomm_devdata *devdata,
                                  mcomm_core_t target_core, u32 data)
{
	if (target_core == SEE_CPU_ID) {
		mcomm_platform_ops->send(target_core, data);
        return 0;
	}

	return -1;
}

static long mcomm_fd_ioctl(struct file *fp, unsigned int ioctl,
                           unsigned long arg)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;
	void __user *userptr = (void __user *)arg;
	long rc = 0;

	switch (ioctl) {
	case MCOMM_LOCAL_CPUID: {
		u32 cpuid = mcomm_platform_ops->cpuid(0);

		rc = -EFAULT;
		if (copy_to_user(userptr, &cpuid, sizeof(cpuid)) == 0)
			rc = 0;
		break;
	}

	case MCOMM_REMOTE_CPUID: {
		u32 cpuid = mcomm_platform_ops->cpuid(1);

		rc = -EFAULT;
		if (copy_to_user(userptr, &cpuid, sizeof(cpuid)) == 0)
			rc = 0;
		break;
	}

	case MCOMM_WAIT_READ: {
        /** Returns 0 when mailbox context available, others the context invalid.
         */
        u32 ctxt;
        rc = mcomm_fd_ioctl_wait_read(devdata, &ctxt);
		if (rc == 0) {
            pr_debug("%s: received 0x%x\n", __func__, ctxt);
            copy_to_user(userptr, &ctxt, sizeof(ctxt));
        }
		break;
	}

    /**
     * Besides just notifying the peer, we deliver (send) mailbox context yet.
     */
	case MCOMM_SEND: {
        SHM_COMM_DATA data;

		rc = -EFAULT;
		if (copy_from_user(&data, userptr, sizeof(data)) == 0) {
			pr_debug("%s: send (0x%x) to (0x%x)\n", __func__, data.data, data.remoteId);
			rc = mcomm_fd_ioctl_notify(devdata, data.remoteId, data.data);
		}

		break;
	}

	/*Added by tony on 2013/05/22*/
        case MCOMM_SYNCACK: {
		//printk("<MAIN>MCOMM_SYNCACK will send ack to see II!\n");
                /* Ack the peer as we have processed. */
                mcomm_platform_ops->ack(SEE_CPU_ID, MAIN_ACK);
                break;
        }

	default:
		rc = -EINVAL;
	}

	return rc;
}

long mcomm_ioctl(unsigned int ioctl, void *arg)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;
	long rc = 0;

	switch (ioctl) {
    case MCOMM_INIT: {
        rc = mcomm_dev_ioctl_init(devdata);
        break;
    }

	case MCOMM_LOCAL_CPUID: {
		u32 cpuid = mcomm_platform_ops->cpuid(0);

		rc = -EFAULT;
        if (arg) {
            *((u32 *)(arg)) = cpuid;
            rc = 0;
        }
		break;
	}

	case MCOMM_REMOTE_CPUID: {
		u32 cpuid = mcomm_platform_ops->cpuid(1);

		rc = -EFAULT;
        if (arg) {
            *((u32 *)(arg)) = cpuid;
            rc = 0;
        }
		break;
	}

	case MCOMM_WAIT_READ: {
        /** Returns 0 when mailbox context available, others the context invalid.
         */
        u32 ctxt;
        rc = mcomm_fd_ioctl_wait_read(devdata, &ctxt);

/*Added by tony on 2014/06/08 for MBX AS solution*/
#if defined(CONFIG_ALI_MBX_AS)
        
        if (arg){
                *((u32 *)(arg)) = g_mbx_revd_len;
        }
#else
		if (rc == 0) {
            pr_debug("%s: received 0x%x\n", __func__, ctxt);
            if (arg) {
                *((u32 *)(arg)) = ctxt;
            }
        }

#endif
		break;
	}

    /**
     * Besides just notifying the peer, we deliver (send) mailbox context yet.
     */
	case MCOMM_SEND: {
        SHM_COMM_DATA *pdata = (SHM_COMM_DATA*)arg;

		rc = -EFAULT;
        if (pdata) {
			rc = mcomm_fd_ioctl_notify(devdata, pdata->remoteId, pdata->data);
        }

		break;
	}
	/*Added by tony on 2013/05/22*/
	case MCOMM_SYNCACK: {
		//printk("<MAIN>MCOMM_SYNCACK will send ack to see!\n");		
		/* Ack the peer as we have processed. */
       		mcomm_platform_ops->ack(SEE_CPU_ID, MAIN_ACK);
		break;
	}

	default:
		rc = -EINVAL;
	}

	return rc;
}
EXPORT_SYMBOL(mcomm_ioctl);

#if defined(CONFIG_ALI_MBX_AS)

extern unsigned long __G_ALI_MM_MCAPI_MEM_SIZE;
extern unsigned long __G_ALI_MM_MCAPI_MEM_START_ADDR;

int mbx_hs_send_req()
{
	  void *pwbuf = (void *)__G_ALI_MM_MCAPI_MEM_START_ADDR;
	  unsigned char *ptemp = pwbuf;
	  unsigned int len = strlen(mbx_handshake_msg_req);
	  unsigned int data;
#ifdef MBX_DEBUG	  
	  printk("<<main>> mbx_hs_send_req entered!\n");
#endif	  
    memcpy((void*)&pwbuf[MBX_WBUF_IDX], (void*)mbx_handshake_msg_req, len);
    //printk("handshake, main MBX_WBUF_IDX:%d, buf[0~3]:0x%x,0x%x,0x%x,0x%x\n", MBX_WBUF_IDX, ptemp[0],ptemp[1],ptemp[2],ptemp[3] );
    data = MBX_REG_WRITE_PREPARE_CONTENT(len, 1, 0, MBX_CTRL_MSG); 
    /*Trigger send*/ 
    //printk("<<main>> HandShake mbx_hs_send_req trigger send, Senseflag:0x%x\n", readl(REG_MAIN_SENSE_MBX));
    MbxHwMain_Tx(MBX0, data);
    
    /*Waiting the peer received*/
      unsigned int counter = 0;
      while(1)
      {
      	data = readl(REG_MAIN_MBX0_SEND);
      	if(data == MBX_RECEIVED_MASK)
      		break;
      	counter++;
#ifdef CONFIG_MIPS
        if(counter>0xA0000)
#else
      	if(counter>0x30000) /*the default value is 0xa600*/
#endif
      	{
      		printk("mbx_hs_send_req send peer no respond!!!REG_MAIN_MBX0_SEND:0x%x, value:0x%x\n",REG_MAIN_MBX0_SEND,readl(REG_MAIN_MBX0_SEND));
      		return -1;
      	}
      }
#ifdef MBX_DEBUG      
      printk("<<main>> HandShake mbx_hs_send_req end!counter:0x%x\n",counter);
#endif      
      return 0;
     
}
int mbx_hs_send_ack()
{
	  void *pwbuf = (void *)__G_ALI_MM_MCAPI_MEM_START_ADDR;
	  unsigned int len = strlen(mbx_handshake_msg_ack);
	  unsigned int data;
#ifdef MBX_DEBUG	  
	  printk("<<main>> mbx_hs_send_ack 1\n");
#endif
	  memcpy((void*)&pwbuf[MBX_WBUF_IDX], (void*)mbx_handshake_msg_ack, len);
    data = MBX_REG_WRITE_PREPARE_CONTENT(len, 1, 0, MBX_CTRL_MSG); 
    /*Trigger send*/ 
#ifdef MBX_DEBUG    
    printk("<<main>> HandShake mbx_hs_send_ack will trigger send!\n");
#endif
    MbxHwMain_Tx(MBX0, data);
    /*Waiting the peer received*/
      unsigned int counter = 0;
      while(1)
      {
      	data = readl(REG_MAIN_MBX0_SEND);
      	if(data == MBX_RECEIVED_MASK)
      		break;
      	counter++;
#ifdef CONFIG_MIPS
        if(counter>0xA0000)
#else
      	if(counter>0x30000)
#endif
      	{
      		printk("mbx_hs_send_req send peer no respond!!!\n");
      		return -1;
      	}
      }
      return 0;
}

int mbx_handshake(void)
{
		int ret = 0;
		int counter = 0;
#ifdef MBX_DEBUG	  
	  printk("<<main>> mbx_handshake entered!\n");
#endif
    g_mbx_prepare = 1;

    while(1)
	  {
	  	if(!mbx_hs_send_req())
	  	  break;
	  	msleep(20);
	  }
	  
    while(1)
    {
    	if(g_mbx_handshake_req)
	  		break;
	  	msleep(20);
    }
#ifdef MBX_DEBUG
	  printk("$$$mbx_handshake Success!\n");
#endif
	
	  return ret;
}

int mbx_send_data(void *buffer, Uint32 len)
{
    int ret = 0;
    SHM_COMM_DATA comm;
    unsigned int tolpkt, curpkt;
    void *pwbuf = (void *)__G_ALI_MM_MCAPI_MEM_START_ADDR;
    unsigned int data = 0;
    unsigned counter = 0;
    
    if(!buffer || !len)
    {
    	printk("mbx_send_data invalid argument\n");
    	return MBX_ERR_SEND_BUF_NULL;
    }
    if(len>MBX_RCV_MAX_BUF)
    {
    	printk("mbx_send_data the sent data overflow!\n");
    	return MBX_ERR_SEND_BUF_OVERFLOW;
    }
    curpkt = 0;
    tolpkt = len/MBX_WBUF_LEN;
    if((len%MBX_WBUF_LEN) >0)
    	tolpkt += 1;
#ifdef MBX_DEBUG    
    printk("mbx_send_data, pwbuf:0x%x, tolpkt:0x%x, len:0x%x\n", pwbuf, tolpkt, len);
#endif
    while(len>MBX_WBUF_LEN)
    {
    	memcpy((void*)&pwbuf[MBX_WBUF_IDX], (void*)&buffer[curpkt*MBX_WBUF_LEN], MBX_WBUF_LEN);
    	data = MBX_REG_WRITE_PREPARE_CONTENT(MBX_WBUF_LEN, tolpkt, curpkt, MBX_DATA_MSG); 
      /*Trigger send*/ 
#ifdef MBX_DEBUG
      printk("^^^ mbx_send_data will trigger send! curpkt:0x%x, tolpkt:0x%x, MBX_WBUF_LEN:0x%x, MBX_WBUF_IDX:0x%x\n",curpkt, tolpkt, MBX_WBUF_LEN, MBX_WBUF_IDX);
#endif
      MbxHwMain_Tx(MBX0, data);
      /*Waiting the peer received*/
      counter = 0;
      while(1)
      {
      	data = readl(REG_MAIN_MBX0_SEND);
      	if(data == MBX_RECEIVED_MASK)
      		break;
      	counter++;
#ifdef CONFIG_MIPS
        if(counter>0xA0000)
#else
      	if(counter>0x30000)
#endif
      	{
      		printk("$$$$Mailbox send peer no respond!!!\n");
      		return MBX_ERR_SEND_BUF_COMMON;
      	}
      }
      
      curpkt++; 	
    	len -= MBX_WBUF_LEN;
    }
    /*send the last pkt to peer*/
    memcpy((void*)&pwbuf[MBX_WBUF_IDX], (void*)&buffer[curpkt*MBX_WBUF_LEN], len);
    data = MBX_REG_WRITE_PREPARE_CONTENT(len, tolpkt, curpkt, MBX_DATA_MSG);
    /*Trigger send*/ 
#ifdef MBX_DEBUG    
    printk("VVV mbx_send_data will trigger the last send! curpkt:0x%x, tolpkt:0x%x, len:0x%x, MBX_WBUF_IDX:0x%x\n",curpkt, tolpkt, len, MBX_WBUF_IDX);
#endif
    comm.remoteId = SEE_CPU_ID;
    comm.data = data;
    ret = mcomm_ioctl(MCOMM_SEND, &comm);
#ifdef MBX_DEBUG    
    printk("mbx_send_data complete, ret:%d\n", ret);
#endif 
	  return ret;
}

int mbx_send_sync(void)
{
	  int dummydata;
    int rc;
    
    rc = mcomm_ioctl(MCOMM_SYNCACK, &dummydata);
	  
	  return rc;
}

int mbx_read_data(void **buffer, unsigned int *len)
{
	  struct shm_comm_device *mcomm_dev = NULL;
    int rc;
#ifdef MBX_DEBUG
    printk("mbx_read_data 1");    
#endif
    if(!buffer)
    {
    	 printk("mbx_read_data argument error!");
    	 return -1;
    }

	  rc = mcomm_ioctl(MCOMM_WAIT_READ, (void *)len);
	  *buffer = (void*)g_mbx_revd_buf;
#ifdef MBX_DEBUG	
	  printk("mbx_read_data 2");
#endif
	  
	  return rc;
}

int mbx_init(void)
{
	  int rc;
	  
	  printk("__G_ALI_MM_MCAPI_MEM_START_ADDR:0x%x, share_mem_buf_base:0x%x\n", __G_ALI_MM_MCAPI_MEM_START_ADDR, (__G_ALI_MM_MCAPI_MEM_START_ADDR+MBX_FREE_BUF_IDX));
	  rc = mcomm_ioctl(MCOMM_INIT, 0);
	  return rc;
}

#endif

static int __mcomm_follow_pte(struct mm_struct *mm, unsigned long address,
		pte_t **ptepp, spinlock_t **ptlp)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;

	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	pud = pud_offset(pgd, address);
	if (pud_none(*pud) || unlikely(pud_bad(*pud)))
		goto out;

	pmd = pmd_offset(pud, address);
	VM_BUG_ON(pmd_trans_huge(*pmd));
	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
		goto out;

	/* We cannot handle huge page PFN maps. Luckily they don't exist. */
	if (pmd_huge(*pmd))
		goto out;

	ptep = pte_offset_map_lock(mm, pmd, address, ptlp);
	if (!ptep)
		goto out;
	if (!pte_present(*ptep))
		goto unlock;
	*ptepp = ptep;
	return 0;
unlock:
	pte_unmap_unlock(ptep, *ptlp);
out:
	return -EINVAL;
}

static inline int mcomm_follow_pte(struct mm_struct *mm, unsigned long address,
			     pte_t **ptepp, spinlock_t **ptlp)
{
	int res;

	/* (void) is needed to make gcc happy */
	(void) __cond_lock(*ptlp,
			   !(res = __mcomm_follow_pte(mm, address, ptepp, ptlp)));
	return res;
}

#ifdef CONFIG_HAVE_IOREMAP_PROT
static int mcomm_follow_phys(struct vm_area_struct *vma,
		unsigned long address, unsigned int flags,
		unsigned long *prot, resource_size_t *phys)
{
	int ret = -EINVAL;
	pte_t *ptep, pte;
	spinlock_t *ptl;

	if (!(vma->vm_flags & (VM_IO | VM_PFNMAP)))
		goto out;

	if (mcomm_follow_pte(vma->vm_mm, address, &ptep, &ptl))
		goto out;
	pte = *ptep;

	if ((flags & FOLL_WRITE) && !pte_write(pte))
		goto unlock;

	*prot = pgprot_val(pte_pgprot(pte));
	*phys = (resource_size_t)pte_pfn(pte) << PAGE_SHIFT;

	ret = 0;
unlock:
	pte_unmap_unlock(ptep, ptl);
out:
	return ret;
}

static int mcomm_access_phys(struct vm_area_struct *vma, unsigned long addr,
                             void *buf, int len, int write)
{
	resource_size_t phys_addr = 0;
	unsigned long prot = 0;
	void __iomem *maddr;
	int offset = addr & (PAGE_SIZE-1);

	if (mcomm_follow_phys(vma, addr, write, &prot, &phys_addr))
		return -EINVAL;

	maddr = ioremap_prot(phys_addr, PAGE_SIZE, prot);
	if (write)
		memcpy_toio(maddr + offset, buf, len);
	else
		memcpy_fromio(buf, maddr + offset, len);
	iounmap(maddr);

	return len;
}
#endif

static const struct vm_operations_struct mmap_mcomm_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
	.access = mcomm_access_phys
#endif
};

static int mcomm_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;
	unsigned long start_page;
	unsigned long pos;

	if ((vma->vm_end - vma->vm_start) > resource_size(&devdata->mem))
		return -ENOMEM;

	vma->vm_page_prot = mcomm_platform_ops->mmap_pgprot(vma);
	vma->vm_ops = (struct vm_operations_struct *)&mmap_mcomm_ops;

    vma->vm_flags |= VM_IO;
    pos = (unsigned long)(devdata->mem.start & 0x0FFFFFFF);
	start_page = pos >> PAGE_SHIFT;
    //start_page = devdata->mem.start >> PAGE_SHIFT;
	return remap_pfn_range(vma, vma->vm_start,
	                       start_page + vma->vm_pgoff,
	                       vma->vm_end - vma->vm_start,
	                       vma->vm_page_prot);
}

static int mcomm_fd_release(struct inode *inode, struct file *fp)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;

	/* XXX what happens to a thread blocked in ioctl? */

	if (atomic_dec_and_test(&devdata->refcount)) {
		if (devdata->irq0 != NO_IRQ)
			free_irq(devdata->irq0, devdata);
		if (devdata->irq1 != NO_IRQ)
			free_irq(devdata->irq1, devdata);
		if (devdata->irq2 != NO_IRQ)
			free_irq(devdata->irq2, devdata);
		if (devdata->irq3 != NO_IRQ)
			free_irq(devdata->irq3, devdata);
		iounmap(devdata->mbox_mapped);
	}

	return 0;
}

static struct file_operations mcomm_fd_fops = {
	.release        = mcomm_fd_release,
	.unlocked_ioctl = mcomm_fd_ioctl,
	.compat_ioctl   = mcomm_fd_ioctl,
	.mmap           = mcomm_mmap,
};

extern unsigned long __G_ALI_MM_SHARED_MEM_SIZE;

static long mcomm_dev_initialize(struct mcomm_devdata *devdata)
{
	resource_size_t mbox_paddr;
	long rc;

	/* Map only the memory encompassing the mailboxes. */
	mbox_paddr = devdata->mem.start;
	devdata->mbox_mapped = mcomm_platform_ops->map(mbox_paddr,
	                                               __G_ALI_MM_SHARED_MEM_SIZE);
	if (devdata->mbox_mapped == NULL) {
		printk(KERN_ERR "%s: failed to map the mailboxes shared memory.\n", __func__);
		rc = -EFAULT;
		goto out1;
	}

    /* Check mailbox0 int. */
	if (devdata->irq0 != NO_IRQ) {
		rc = request_irq(devdata->irq0, mcomm_interrupt, IRQF_DISABLED, "mcomm",
						 devdata);
		if (rc) {
			printk(KERN_ERR "%s: failed to reserve mailbox0 irq %d\n", __func__,
				   devdata->irq0);
			goto out2;
		}
	}

    /* Check mailbox1 int. */
	if (devdata->irq1 != NO_IRQ) {
		rc = request_irq(devdata->irq1, mcomm_interrupt, IRQF_DISABLED, "mcomm",
						 devdata);
		if (rc) {
			printk(KERN_ERR "%s: failed to reserve mailbox1 irq %d\n", __func__,
				   devdata->irq1);
			goto out2;
		}
	}

    /* Check mailbox2 int. */
	if (devdata->irq2 != NO_IRQ) {
		rc = request_irq(devdata->irq2, mcomm_interrupt, IRQF_DISABLED, "mcomm",
						 devdata);
		if (rc) {
			printk(KERN_ERR "%s: failed to reserve mailbox2 irq %d\n", __func__,
				   devdata->irq2);
			goto out2;
		}
	}

    /* Check mailbox3 int. */
	if (devdata->irq3 != NO_IRQ) {
		rc = request_irq(devdata->irq3, mcomm_interrupt, IRQF_DISABLED, "mcomm",
						 devdata);
		if (rc) {
			printk(KERN_ERR "%s: failed to reserve mailbox3 irq %d\n", __func__,
				   devdata->irq3);
			goto out2;
		}
	}

    /* Init mcomm filo. */
    atomic_set(&devdata->head, 0);
    atomic_set(&devdata->tail, 0);
    g_mcommfifoelement = 0;    
/*We will disable all the mailbox interrupt when initial interrupt system*/
#ifdef CONFIG_MIPS
  printk("***Will enable the maibox interrupt!\n");
  *((unsigned long *) 0xb804003c) |= 0xf0000000;
#endif

	return 0;

out2:
	iounmap(devdata->mbox_mapped);
out1:
	return rc;
}

static long mcomm_dev_ioctl_init(struct mcomm_devdata *devdata)
{
	long rc;

	if (atomic_inc_return(&devdata->refcount) > 1) {
		printk(KERN_ERR "%s: try to reconfigur.\n", __func__);
		rc = -EBUSY;
		goto out1;
	} else {
		rc = mcomm_dev_initialize(devdata);
		if (rc)
			goto out1;
	}

	return mcomm_anon_inode_getfd("mcomm", &mcomm_fd_fops, devdata, O_RDWR);

out1:
	atomic_dec(&devdata->refcount);
	return rc;
}

static long mcomm_dev_ioctl(struct file *fp, unsigned int ioctl,
                            unsigned long arg)
{
	/**
	 * David - MCOMM_INIT no need to do copy.
	 */

	long rc;
    struct mcomm_devdata *devdata = &_mcomm_devdata;
    
	switch (ioctl) {
	case MCOMM_INIT: {
		rc = mcomm_dev_ioctl_init(devdata);
		break;
	}

	default:
		rc = -EINVAL;
	}

	return rc;
}

static int mcomm_dev_open(struct inode *inode, struct file *fp)
{
	return 0;
}

static struct file_operations mcomm_dev_fops = {
	.open           = mcomm_dev_open,
	.unlocked_ioctl = mcomm_dev_ioctl,
	.compat_ioctl   = mcomm_dev_ioctl,
	.mmap           = mcomm_mmap,
};


static ssize_t mcomm_show_region_addr(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;

	return sprintf(buf, "0x%llx\n", (unsigned long long)devdata->mem.start);
}
static DEVICE_ATTR(address, 0444, mcomm_show_region_addr, NULL);

static ssize_t mcomm_show_region_size(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;

	return sprintf(buf, "0x%llx\n",
	               (unsigned long long)resource_size(&devdata->mem));
}
static DEVICE_ATTR(size, 0444, mcomm_show_region_size, NULL);

static struct attribute *mcomm_attributes[] = {
	&dev_attr_size.attr,
	&dev_attr_address.attr,
	NULL
};

static struct attribute_group mcomm_attr_group = {
	.attrs = mcomm_attributes,
};

struct miscdevice mcomm_misc_dev = {
	.fops = &mcomm_dev_fops,
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcomm0",
};

int mcomm_new_region(struct device *dev, struct resource *mem,
                     struct resource *irq)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;
	int rc;
	static int initialized;

	if (initialized++)
		return -EEXIST;

	INIT_WORK(&devdata->wq_mbx0, mcomm_wq_mbx0);
	INIT_WORK(&devdata->wq_mbx1, mcomm_wq_mbx1);
	/*Changed by tony*/
	//init_waitqueue_head(&devdata->waitCtxt);
	sema_init(&devdata->waitSem, 0);
	devdata->mem = *mem;
    /* Total 4 mailbox ints. */
    devdata->irq0 = irq[0].start;
    devdata->irq1 = irq[1].start;
    devdata->irq2 = irq[2].start;
    devdata->irq3 = irq[3].start;

	rc = sysfs_create_group(&dev->kobj, &mcomm_attr_group);
	if (rc) {
		printk(KERN_WARNING "%s: Failed to register sysfs attributes.\n",
		       __func__);
		goto out1;
	}

	rc = misc_register(&mcomm_misc_dev);
	if (rc) {
		printk("%s misc_register error %d\n", __func__, rc);
		goto out2;
	}

	return 0;

out2:
	sysfs_remove_group(&dev->kobj, &mcomm_attr_group);
out1:
	return rc;
}
EXPORT_SYMBOL(mcomm_new_region);

void mcomm_remove_region(struct device *dev)
{
	misc_deregister(&mcomm_misc_dev);
	sysfs_remove_group(&dev->kobj, &mcomm_attr_group);
}
EXPORT_SYMBOL(mcomm_remove_region);

int mcomm_init(struct mcomm_platform_ops *ops, struct module *module)
{
	int rc = 0;
	if(!ops)
		return -1;
	//rc = mcomm_init_anon_inodes();
//	if (rc)
//		goto out1;

	mcomm_init_anon_inodes();
	mcomm_platform_ops = ops;

	mcomm_dev_fops.owner = module;
	mcomm_fd_fops.owner = module;

	return 0;

out1:
	return rc;
}
EXPORT_SYMBOL(mcomm_init);

void mcomm_exit(void)
{
	mcomm_exit_anon_inodes();
}
EXPORT_SYMBOL(mcomm_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David.L");
MODULE_DESCRIPTION("Shared memory communications channel");
