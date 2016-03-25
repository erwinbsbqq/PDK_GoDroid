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

#include "mcomm.h"
#include "shm_comm_hw.h"
#include "ali_rpc_ctrl.h"
#include "ali_rpcng.h"



extern MINIRPC_INTERFACE Minirpc_Interface;
extern unsigned int g_local_id;
extern unsigned int g_remote_id;

unsigned int buffer[4*1024];
unsigned char *whole_data_copy = (unsigned char *)buffer;
unsigned int current_data_len = 0;
unsigned int current_data_pos = 0;
unsigned char *rx_buf;


int g_mcommfifoelement = 0; 


extern MINIRPC_SHM_MGMT_BLOCK*  MINIRPC_SHM_Mgmt_Blk;
extern ReturnBuffer returnBufferManager;

#define u32 unsigned int

#define COMM_FIFO_MAX    128

#define WAKEUP_MBX0         0
#define WAKEUP_MBX1         1
#define WAKEUP_MBX2         2
#define WAKEUP_MBX3         3

struct mcomm_devdata {
	void *platform_data;
	unsigned int waitSem;
	unsigned int irq0;      /* Mailbox0 irq */
	unsigned int irq1;      /* Mailbox1 irq */
	unsigned int irq2;      /* Mailbox2 irq */
	unsigned int irq3;      /* Mailbox3 irq */
    unsigned int wakeup;    /* wake up cause */
	unsigned int head;
    unsigned int tail;
    u32 fifo[COMM_FIFO_MAX];
};

/* Only supports a single mcomm region. */
static struct mcomm_devdata _mcomm_devdata;

static struct mcomm_platform_ops *mcomm_platform_ops;

enum irqreturn {
	IRQ_NONE		= (0 << 0),
	IRQ_HANDLED		= (1 << 0),
	IRQ_WAKE_THREAD	= (1 << 1),
};

typedef enum irqreturn irqreturn_t;

typedef  irqreturn_t  (*irq_handler_t)(int, void *);

extern int request_irq(unsigned int irq, unsigned int handler, unsigned long flags,
			const char *name, void *dev);
			
static irqreturn_t mcomm_interrupt(int irq, void *dev_id);

static long mcomm_dev_initialize(struct mcomm_devdata *devdata)
{
	long rc=0;
	int i;

    /* Check mailbox0 int. */
	if (devdata->irq0 != -1) 
	{
	    //Log(LOG_DEBUG, "%s devdata->irq0:%d\n",__func__,devdata->irq0);
		
		rc = request_irq(devdata->irq0, mcomm_interrupt, 0, "mcomm0",devdata);
		if (rc) 
		{
			printf("%s: failed to reserve mailbox0 irq %d\n", __func__,
				   devdata->irq0);
			//goto out;
		}
	}


    /* Check mailbox1 int. */
	if (devdata->irq1 != -1) 
	{
	    //Log(LOG_DEBUG, "%s devdata->irq1:%d\n",__func__,devdata->irq1);
		
		rc = request_irq(devdata->irq1, mcomm_interrupt, 0, "mcomm1",devdata);
		if (rc) 
		{
			printf( "%s: failed to reserve mailbox1 irq %d\n", __func__,
				   devdata->irq1);
			goto out;
		}
	}

    /* Check mailbox2 int. */
	if (devdata->irq2 != -1) 
	{
		rc = request_irq(devdata->irq2, mcomm_interrupt, 0, "mcomm2",devdata);
		if (rc) 
		{
			printf("%s: failed to reserve mailbox2 irq %d\n", __func__,
				   devdata->irq2);
			goto out;
		}
	}

    /* Check mailbox3 int. */
	if (devdata->irq3 != -1) 
	{
		rc = request_irq(devdata->irq3, mcomm_interrupt, 0, "mcomm3",devdata);
		if (rc) 
		{
			printf( "%s: failed to reserve mailbox3 irq %d\n", __func__,
				   devdata->irq3);
			goto out;
		}
	}

	return 0;
out:
	return rc;
}

static long mcomm_dev_ioctl_init(struct mcomm_devdata *devdata)
{
	long rc;

    rc = mcomm_dev_initialize(devdata);
	if (rc)
		return -1;
	return 0;
}






/*-----------------------------------------------------------
* CtrlMsgWrite
* 
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
Int32 CtrlMsgWrite(Ctrl_Msg_Type msg_type)
	{
		unsigned char *tx_buf = Minirpc_Interface.minirpc_get_buffer(g_local_id);
		Uint32 ctxt=0;
		Int32 rc=0;

		Log(LOG_DEBUG, "[%s] tx_buf=0x%08x\n", __func__, tx_buf);
		
		ctxt = CTRLMSG_CLASS & 0xff;
		
		switch(msg_type)
		{
		  case CONNECT_REQ_E:
			   memcpy(tx_buf,"CONNECT_REQ",sizeof("CONNECT_REQ"));
			   ctxt |= (sizeof("CONNECT_REQ")&0xff)<<MSG_LEN_OFFSET;
			   break;
		  case CONNECT_RES_E:
			   memcpy(tx_buf,"CONNECT_RES",sizeof("CONNECT_RES"));
			   ctxt |= (sizeof("CONNECT_RES")&0xff)<<MSG_LEN_OFFSET;
			   break;
		  case DISCONN_REQ_E:
			   memcpy(tx_buf,"DISCONN_REQ",sizeof("DISCONN_REQ"));
			   ctxt |= (sizeof("DISCONN_REQ")&0xff)<<MSG_LEN_OFFSET;
			   break;
		  case DISCONN_RES_E:
			   memcpy(tx_buf,"DISCONN_RES",sizeof("DISCONN_RES"));
			   ctxt |= (sizeof("DISCONN_RES")&0xff)<<MSG_LEN_OFFSET;
			   break;
		  default:
			   break;	
		}
		
		Minirpc_Interface.minirpc_tx_out(g_remote_id, ctxt);

		minirpc_receive_thread();
	
		return 0;
	}



/*-----------------------------------------------------------
* RpcMsgWrite
* 
*
* Return:  RPC_ERROR_VALUE->Failed,  other value->Success or timeout
*------------------------------------------------------------*/
Int32 RpcMsgWrite(Uint8* buf, Uint32 bufsize)
{	
		Uint32 send_len=0;
		Uint32 ctxt=0;
		Uint32 total_index=0;
		Uint32 current_index=0;
		Uint32 rc=0;
		unsigned char *tx_buf = Minirpc_Interface.minirpc_get_buffer(g_local_id);

		Log(LOG_DEBUG, "[%s] tx_buf=0x%08x\n", __func__, tx_buf);
			
		total_index = bufsize/MINIRPC_MAX_MBX_LEN;
		if(bufsize%MINIRPC_MAX_MBX_LEN>0)
		{
		   total_index++;
		}
		while(bufsize/MINIRPC_MAX_MBX_LEN>0)
		{	
			Log(LOG_DEBUG, "[%s] total_index=%d current_index=%d\n",__func__,total_index,current_index);
	
			memcpy(tx_buf, buf+current_index*MINIRPC_MAX_MBX_LEN, MINIRPC_MAX_MBX_LEN);
			
			while(memcmp(tx_buf, buf+current_index*MINIRPC_MAX_MBX_LEN, MINIRPC_MAX_MBX_LEN));

	        ctxt = 0;
			ctxt |= RPCMSG_CLASS<<MSG_CLASS_TYPE_OFFSET;
			ctxt |= (current_index<<MSG_CURRENT_INDEX_OFFSET);
			ctxt |= (total_index<<MSG_TOTAL_INDEX_OFFSET);
			ctxt |= (MINIRPC_MAX_MBX_LEN-1)<<MSG_LEN_OFFSET;
	
			Minirpc_Interface.minirpc_tx_out(g_remote_id,ctxt);
	
			current_index++;
			bufsize -= MINIRPC_MAX_MBX_LEN;
			send_len += MINIRPC_MAX_MBX_LEN;
		}
		if(bufsize>0)
		{
		 
			
			Log(LOG_DEBUG, "[%s] total_index=%d current_index=%d\n",__func__,total_index,current_index);
			
			memcpy(tx_buf,buf+current_index*MINIRPC_MAX_MBX_LEN,bufsize);

	        ctxt = 0;
			ctxt |= RPCMSG_CLASS<<MSG_CLASS_TYPE_OFFSET;
			ctxt |= (current_index<<MSG_CURRENT_INDEX_OFFSET);
			ctxt |= (total_index<<MSG_TOTAL_INDEX_OFFSET);
			ctxt |= (bufsize-1)<<MSG_LEN_OFFSET;
			
			Minirpc_Interface.minirpc_tx_out(g_remote_id,ctxt);
			
			send_len+=bufsize;
		}

		minirpc_receive_thread();
	
		return send_len;
}


struct _FindCtrlMsgType
{
    Ctrl_Msg_Type ctrl_msg_type;
	char str[256];
};

typedef struct _FindCtrlMsgType FindCtrlMsgType;

FindCtrlMsgType g_FindCtrlMsgType_s[]=
{
   	{
		CONNECT_REQ_E,
	    "CONNECT_REQ",
	},
    {
		CONNECT_RES_E,
	    "CONNECT_RES",
	},
	{
		DISCONN_REQ_E,
	    "DISCONN_REQ",
	},
	{
		DISCONN_RES_E,
	    "DISCONN_RES",
	},

};

Ctrl_Msg_Type GetCtrlMsgType(char *s)
{
   unsigned int i=0;
   for(i=0;i<sizeof(g_FindCtrlMsgType_s)/sizeof(g_FindCtrlMsgType_s[0]);i++)
   {
       if(strcmp(g_FindCtrlMsgType_s[i].str,s)==0)
	   	return g_FindCtrlMsgType_s[i].ctrl_msg_type;
   }
}


static long CtrlMsgProc(u32 param)
{
		unsigned char *rev_msg = NULL;
		unsigned char *rx_buf = NULL;
		unsigned char msg_len = 0;
		Ctrl_Msg_Type ctrl_msg_type = 0;
		int rc=0;
	
		rx_buf = Minirpc_Interface.minirpc_get_buffer(g_remote_id);  //MINIRPC_SHM_Mgmt_Blk->minirpc_shm_buff;
		msg_len = GET_MINIRPC_MSG_LEN(param);
		rev_msg = (char *)malloc(msg_len);
		memcpy(rev_msg, rx_buf, msg_len);
	
		Log(LOG_DEBUG, "[%s]rev_msg:%s\n",__func__,rev_msg);
	
		ctrl_msg_type = GetCtrlMsgType(rev_msg);
	
		switch(ctrl_msg_type)
		{
		   case CONNECT_REQ_E:
			   Log(LOG_DEBUG, "[%s] Receive CONNECT_REQ_E\n",__func__);
			   g_param.g_RPCRunning = RPCST_MINIRPC_RUNNING;
			   CtrlMsgWrite(CONNECT_RES_E);
			   break;
		   case CONNECT_RES_E:
			   Log(LOG_DEBUG, "[%s] Receive CONNECT_RES_E\n",__func__);
			   g_param.g_RPCRunning = RPCST_MINIRPC_RUNNING;
			   break;
		   case DISCONN_REQ_E:
			   Log(LOG_DEBUG, "[%s] Receive DISCONN_REQ_E\n",__func__);
			   g_param.g_RPCRunning = RPCST_MINIRPC_STOPPING;
			   CtrlMsgWrite(DISCONN_RES_E);			   
			   break;
		   case DISCONN_RES_E:
			   Log(LOG_DEBUG, "[%s] Receive DISCONN_RES_E\n",__func__);
			   g_param.g_RPCRunning = RPCST_MINIRPC_STOPPING;
			   break;
		   default:
			   Log(LOG_DEBUG, "[%s] Error Ctrl Msg!!\n",__func__);
			   break;
		}
		
		Minirpc_Interface.minirpc_free_buffer(rx_buf);
		
		return rc;
		  
}



static long RpcMsgProc(u32 param)
{
	  unsigned char type = 0;
	  unsigned char current_index = 0;
	  unsigned char total_index = 0;
	  unsigned short len = 0;
	  int ret = 0;
	  Uint32  rid = 0;
	  unsigned long i = 0;
	  static unsigned char compare_current_index = 0;
	  static unsigned char compare_total_index = 0;
	  static unsigned int msg_find = 0;
	  rx_buf = Minirpc_Interface.minirpc_get_buffer(g_remote_id);
	
	  current_index = GET_MINIRPC_MSG_CURRENT_INDEX(param);
	  total_index = GET_MINIRPC_MSG_TOTAL_INDEX(param);
	
	  Log(LOG_DEBUG, "[%s] total_index:%d current_index:%d\n",__func__,total_index,current_index);
	
	  if(msg_find == 0)
	  {
		if((total_index > 0)&&(current_index == 0))
		{	
		   compare_total_index = total_index;
		   compare_current_index = 0;
		   msg_find = 1;
	
		   memset(whole_data_copy, 0, 16*1024);
		   current_data_len = 0;
		   current_data_pos = 0;
			
		}
		else
		{
		   goto exit;
		}
	  }
	  
	  if(msg_find == 1)
	  {
		 if((current_index == compare_current_index)&&(total_index == compare_total_index))
		 {
			if(current_index < compare_total_index)
			{
				compare_current_index++;
	
				Log(LOG_DEBUG, "[%s] A total_index:%d current_index:%d current_pos:%d\n",__func__,total_index, current_index, current_data_pos);	 
	
				len=GET_MINIRPC_MSG_LEN(param)+1;
					
				memcpy(whole_data_copy + current_data_pos, rx_buf, len);	 
	
	/*For debug usage*/
#if 0
	
				for(i=0;i<len;i++)
				{
				  Log(LOG_ERR,"current_pos:%d  whole_data_copy[%d]:0x%x\n", current_data_pos, current_data_pos+i, *(whole_data_copy+current_data_pos+i));
				}
#endif
				
				current_data_len += len;
				current_data_pos += len;

	
			}
			else
			{
				goto exit;
			}
	
			if(compare_current_index == compare_total_index)
			{	
				Log(LOG_DEBUG, "[%s] whole message has received whole_data_copy:0x%08x current_data_len:%d\n",__func__, whole_data_copy, current_data_len);
				
				ret=FormatCheckPacketType(whole_data_copy, current_data_len, &rid);
				if (ret == 0)
				{
					/*It's a caller encoded packet*/
					if (ProcessCallPacket(whole_data_copy, current_data_len) == RPC_SUCCESS_VALUE)
					{
						//g_TotalRcvMsgCall++;
					}
					else
					{
						Log(LOG_EMERG, "[%s] BadPacketDebugInfo, caller packet!\n",__func__);
						//g_TotalRcvBadMsgPacket++;
					}
				}
				else if (ret == 1)
				{
					/*It's a return result packet*/
					if (ProcessReturnPacket(whole_data_copy, current_data_len, rid) == RPC_SUCCESS_VALUE)
					{
						//g_TotalRcvMsgRet++;
					}
					else
					{
						Log(LOG_EMERG, "[%s] RecordRcvBadPacketDebugInfo, ret packet!\n",__func__);
						//g_TotalRcvBadMsgPacket++;
					}
				}
				else
				{
					Log(LOG_EMERG, "[%s]  !!Received a bad packet!!\n",__func__);
					/*Record the debug variable status*/
					//g_TotalRcvBadMsgPacket++;
	
				}
	
				Log(LOG_DEBUG, "[%s]  whole msg process finish\n",__func__);
				
				compare_total_index = 0;
				compare_current_index = 0;
				msg_find = 0;	

				return 0;
	
			}
	
			Minirpc_Interface.minirpc_free_buffer(rx_buf);
	
			return 1; 
			
		}
		else
		{
			goto exit;
		}
	
	  }
	  else
	  {
		 goto exit;
	  }
	 
	exit:  
		
		Log(LOG_ERR, "[%s] msg error and exit\n",__func__);
		
		Minirpc_Interface.minirpc_free_buffer(rx_buf);
	
		return ret;
			  
}




Uint32 MiniRpcRecvProc(u32 param)
{
        int ret=-1;
		unsigned char MsgType=0;
		
		MsgType = GET_MINIRPC_MSG_TYPE(param);
	
		Log(LOG_DEBUG,"[%s] param:0x%08x, msg_type:%d\n", __func__,param, MsgType);
	
		switch(MsgType)
		{
			case CTRLMSG_CLASS:
				 ret = CtrlMsgProc(param);
				 break;
			case RPCMSG_CLASS:
				 ret = RpcMsgProc(param);
				 break;
			default:
				 break;
		}

	    return ret;
	
}


extern int shm_linux_wait_notify(unsigned int *ctxt);

void minirpc_receive_thread()
{
    unsigned int ctxt;
    int rc;

	Log(LOG_DEBUG,"[%s] enter\n",__func__);

    do 
	{	
        rc = shm_linux_wait_notify(&ctxt);
        if(rc)
		{    
		    PR_LOG("[%s] unexpected data\n",__func__);
            continue;
        }

		rc = MiniRpcRecvProc(ctxt);

		Log(LOG_DEBUG,"[%s] rc=%d",__func__,rc);
        if(rc == 1)
        {
           continue;
        }
		if(rc == 0)
        {
           break;
        }

    } while (1);

	Log(LOG_DEBUG,"[%s] exit\n",__func__);

}


/* Push a context at the head of the mcomm fifo. */
static void mcomm_fifo_push(struct mcomm_devdata *devdata, u32 ctxt)
{
      int head, tail;
	  
      g_mcommfifoelement = 1;
      head = devdata->head;
      devdata->fifo[devdata->head] = ctxt;
      devdata->head = (head + 1)&(COMM_FIFO_MAX - 1);
      Log(LOG_DEBUG,"[%s] push context: 0x%08x at: %d\n", __func__, ctxt, head);

	//printk("##### <MAIN> %s debug, head:%d, tail:%d, g_mcommfifoelement:%d\n", __func__, devdata->head, devdata->tail, g_mcommfifoelement);

}


/** Wake up the process(es) corresponding to the mailbox(es) which just received
 *  packets, since there is no way to make wake_up_interruptible known which isr
 *  waking up in the multi isr services, thus, I divide the pending process to 5
 *  wake_up_interruptible conditions.
 */
static irqreturn_t mcomm_interrupt(int irq, void *dev_id)
{
    u32 ctxt;
	struct mcomm_devdata *devdata = dev_id;
	unsigned int type=0;

    /*irq0*/
    if (irq == devdata->irq0) 
	{
        ctxt = mcomm_platform_ops->recv(MBX0);

		Log(LOG_DEBUG, "%s: irq0 ctxt:0x%08x\n",__func__,ctxt);

		if (ctxt == SEE_ACK) 
		{
            mcomm_platform_ops->sync();
        }
    }
	
	/*irq1*/
    if (irq == devdata->irq1) 
	{
        ctxt = mcomm_platform_ops->recv(MBX1);

		Log(LOG_DEBUG, "%s: irq1 ctxt:0x%08x\n",__func__,ctxt);

		/*receive message process func*/
		if(g_mcommfifoelement)
		{
			printf("mcomm_interrupt, get a unexpteced mailbox1 interrupt!\n");
		}
		else
		{
        	devdata->wakeup |= (1 << WAKEUP_MBX1);
			mcomm_fifo_push(devdata, ctxt);
			
        	devdata->waitSem = 1;
			
	    }
		///MiniRpcRecvProc(ctxt);
	   
    }

	/*irq2*/
    if (irq == devdata->irq2) 
	{
        ctxt = mcomm_platform_ops->recv(MBX2);
    }

	/*irq3*/
    if (irq == devdata->irq3) 
	{
        ctxt = mcomm_platform_ops->recv(MBX3);
    }

	return IRQ_HANDLED;
}

static void mcomm_sync_tasklet(unsigned long priv)
{
    mcomm_platform_ops->sync();
}

static long mcomm_fd_ioctl_wait_read(struct mcomm_devdata *devdata, u32 *ctxt)
{
    int head, tail, ret = 0;

    if (!devdata || !ctxt)
	{
		printf("<MAIN>mcomm_fd_ioctl_wait_read, got dev handle failed!\n");
        return -1;
    }

    Log(LOG_DEBUG,"[%s] wait data...\n",__func__);
	
	while(1!=devdata->waitSem)
	{
	   mdelay(3);
	   //printf(".");
	}

	devdata->waitSem = 0;
	
	Log(LOG_DEBUG,"[%s] wait data return, g_mcommfifoelement:%d\n",__func__,g_mcommfifoelement);

	if(!g_mcommfifoelement){
		//devdata->wakeup = 0;
		return -1;	
	}
  	else
	{
        /* Clear wake up flag. */
        //devdata->wakeup &= ~(1 << WAKEUP_MBX1);
        devdata->wakeup = 0;

        /* Get the context. */
        head = devdata->head;
        tail = devdata->tail;
		
        /*if (head == tail) {
              pr_debug("%s: Should never be here.\n", __func__);
              return -1;
       	 }*/

	    *ctxt = devdata->fifo[tail];
	    devdata->tail = (tail + 1)&(COMM_FIFO_MAX - 1);
		 
		
		if(g_mcommfifoelement>0)  
	    {
	       g_mcommfifoelement = 0;
		}
	
		Log(LOG_DEBUG,"[%s]: wakeup, *ctxt:0x%08x  head:%d, tail:%d, g_mcommfifoelement:%d.\n", __func__, *ctxt, head, tail, g_mcommfifoelement);

    }


    return (long)ret;
}

static long mcomm_fd_ioctl_notify(struct mcomm_devdata *devdata,
                                  mcomm_core_t target_core, u32 data)
{
    Log(LOG_DEBUG, "%s: target_core:0x%08x  data:0x%08x\n",__func__, target_core, data);

	if (target_core == SEE_CPU_ID) 
	{
		mcomm_platform_ops->send(target_core, data);
        return 0;
	}
	return -1;
}

long mcomm_ioctl(unsigned int ioctl, void *arg)
{
	struct mcomm_devdata *devdata = &_mcomm_devdata;
	long rc=0;
	u32 cpuid = 0;
	u32 ctxt = 0;
	SHM_COMM_DATA *pdata = NULL;
	
	switch (ioctl) 
	{
	    case MCOMM_INIT:
	        rc = mcomm_dev_ioctl_init(devdata);
	        break;
		case MCOMM_LOCAL_CPUID:
			cpuid = mcomm_platform_ops->cpuid(0);
			rc = -1;
	        if (arg) 
			{
	            *((u32 *)(arg)) = cpuid;
	            rc = 0;
	        }
			break;
		case MCOMM_REMOTE_CPUID:
			cpuid = mcomm_platform_ops->cpuid(1);
			rc = -1;
	        if (arg) {
	            *((u32 *)(arg)) = cpuid;
	            rc = 0;
	        }
			break;
		case MCOMM_WAIT_READ:
	        rc = mcomm_fd_ioctl_wait_read(devdata, &ctxt);
			if (rc == 0) 
			{
	            if (arg) 
				{
	                *((u32 *)(arg)) = ctxt;
	            }
	        }
			break;
		case MCOMM_SEND: 
	        pdata = (SHM_COMM_DATA*)arg;
			rc = -1;
	        if (pdata) 
			{
				rc = mcomm_fd_ioctl_notify(devdata, pdata->remoteId, pdata->data);
	        }
			break;
		case MCOMM_SYNCACK: 
	        mcomm_platform_ops->ack(SEE_CPU_ID, MAIN_ACK);
			break;
		default:
			rc = -1;
			break;
	}

	return rc;
}
EXPORT_SYMBOL(mcomm_ioctl);


int mcomm_init(struct mcomm_platform_ops *ops, struct module *module)

{
	int rc;
	struct mcomm_devdata *devdata = &_mcomm_devdata;

	devdata->irq0 = MAIN_MBX0_INT;
    devdata->irq1 = MAIN_MBX1_INT;
    devdata->irq2 = MAIN_MBX2_INT;
    devdata->irq3 = MAIN_MBX3_INT;

	mcomm_platform_ops = ops;
	
	return 0;
}

void mcomm_exit(void)
{
	return 0;
}
