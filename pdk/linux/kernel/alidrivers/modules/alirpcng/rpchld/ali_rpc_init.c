/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *  (C)
 *  File: ali_rpc_init.c
 *  (I)
 *  Description: RPC mutex,spinlock create/delete and share memory malloc/free
 *               These code are ported from ali_oldrpc.c:
 *               //depot/SCT/Branch/Users/David/RPC/main/kernel/alidrivers/alirpc/ali_oldrpc.c#5
 *               but it still is useful for the RPCng.
 *			  
 *  (S)
 ****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <ali_rpcng.h>
#include <ali_shm.h>
#include <ali_reg.h>

#if defined(CONFIG_ALI_MBX_AS)
#include <ali_mbx_as.h>
extern unsigned long __G_ALI_MM_MCAPI_MEM_START_ADDR;
#endif
/* RPC hw infomation. if you don't know detail about the below setting,
keep it as default is the best idea */
extern unsigned long __G_ALI_MM_PRIVATE_AREA_TOP_ADDR;
extern unsigned long __G_ALI_MM_PRIVATE_AREA_START_ADDR;
extern unsigned long __G_ALI_MM_SHARED_MEM_TOP_ADDR;
extern unsigned long __G_ALI_MM_SHARED_MEM_START_ADDR;
extern unsigned long __G_ALI_MM_SHARED_MEM_SIZE;

struct RpcStatstics
{
	UINT32 M_ScmIsrCnt;
	UINT32 M_SrmIsrCnt;
	UINT32 M_McsReqCnt;
	UINT32 M_MrsReqCnt;

    UINT32 S_ScmReqCnt;
    UINT32 S_SrmReqCnt;
    UINT32 S_McsIsrCnt;
    UINT32 S_MrsIsrCnt;
};

#if defined(CONFIG_ALI_MBX_AS)
#define RPC_MEM_LEN  					(MBX_FREE_BUF_LEN) 
#define RPC_BASE_ADDR 					__VMEMALI(__G_ALI_MM_MCAPI_MEM_START_ADDR+MBX_FREE_BUF_IDX)

#else
#define RPC_MEM_LEN  					(__G_ALI_MM_SHARED_MEM_SIZE) 

/*Changed by tony on 2013/06/06*/
#define RPC_BASE_ADDR 					__VMEMALI(__G_ALI_MM_SHARED_MEM_START_ADDR)

#endif

#define ALLOC_SM_LEN    					(RPC_MEM_LEN-12)
#define ALLOC_SM_ADDR   				(RPC_BASE_ADDR)

#define READ_DUAL_MUTEX_ADDR 			((ALLOC_SM_ADDR + ALLOC_SM_LEN))
#define WRITE_DUAL_MUTEX_ADDR 		((ALLOC_SM_ADDR + ALLOC_SM_LEN)+4)
#define SEE_EXCEPTION_HANDLER_ADDR 	(ALLOC_SM_ADDR + ALLOC_SM_LEN + 8)


static UINT32 *m_rpc_sm_head;

static unsigned long flags;  

#define DUAL_MUTEX_NUM 32
#define MUTEX_UNUSED 0
#define MUTEX_USED 1

#define GET_BIT(data, n) ((1<<n)&data)
#define SET_BIT(data, n, v) data = ((data & (~(1<<n))) | (v<<n))

static volatile UINT32 *g_dual_mutex_read;
static volatile UINT32 *g_dual_mutex_write;
static UINT8 g_dual_mutex_cnt = 0; 
static UINT8 g_dual_mutex_status[DUAL_MUTEX_NUM];
static struct semaphore g_dual_mutex_local[DUAL_MUTEX_NUM];

#define IS_MUTEX_LOCKED(n) GET_BIT((*g_dual_mutex_read), n) 
#define IS_MUTEX_WRITTEN(n) GET_BIT((*g_dual_mutex_write), n) 

#if 0
#define LOCK_MUTEX(n) SET_BIT((*g_dual_mutex_write), n, 1) 
#define UNLOCK_MUTEX(n) SET_BIT((*g_dual_mutex_write), n, 0) 
#else
#define LOCK_MUTEX(n) 	{\
						UINT32 data;\
						SET_BIT((*g_dual_mutex_write), n, 1);\
						 data = GET_BIT((*g_dual_mutex_write), n);}
#define UNLOCK_MUTEX(n) {\
						UINT32 data;\
						SET_BIT((*g_dual_mutex_write), n, 0);\
						data = GET_BIT((*g_dual_mutex_write), n);}
#endif


void ali_rpc_spinlock(int mutex_id)
{
	unsigned long val;

	preempt_disable();
	local_irq_save(flags);
	
	while(*(volatile UINT32 *)(__PREG32ALI(0x18041000)) == 1)
		val = *(volatile UINT32 *)(__PREG32ALI(0x18000000));
}
EXPORT_SYMBOL(ali_rpc_spinlock);

void ali_rpc_spinunlock(int mutex_id)
{
	unsigned long val;

	*(volatile UINT32 *)(__REGALIRAW(0x18041000)) = 0;
	val = *(volatile UINT32 *)(__REGALIRAW(0x18000000));
	local_irq_restore(flags);
	preempt_enable();
}
EXPORT_SYMBOL(ali_rpc_spinunlock);

/* Name : 
			ali_rpc_mutex_create
    Description :
			create the rpc mutex for the communication during the dual CPUs
    Arguments : 
    			void
    Return value:
    			<= 0 	---> fail
    			others 	---> mutex id
*/
int ali_rpc_mutex_create(void)
{
	int id = -1;
	int i = 0;

	
//	down(&m_rpc_sem_mutex);

	if(g_dual_mutex_cnt >= DUAL_MUTEX_NUM)
		goto EXIT;

	for(i = 0;i < DUAL_MUTEX_NUM;i++){
		if(g_dual_mutex_status[i] == MUTEX_UNUSED){
			g_dual_mutex_cnt++;
			g_dual_mutex_status[i] = MUTEX_USED;
			id = i + 1;
			break;
		}
	}

	if(id > 0){
		ali_rpc_spinlock(0);

		SET_BIT(*g_dual_mutex_read, i, 0);
		SET_BIT(*g_dual_mutex_write, i, 0);		

		ali_rpc_spinunlock(0);		
	}
		
EXIT:
//	up(&m_rpc_sem_mutex);
	return id;
}
EXPORT_SYMBOL(ali_rpc_mutex_create);

/* Name : 
			ali_rpc_mutex_delete
    Description :
			delete the rpc mutex
    Arguments : 
    			void
    Return value:
    			<= 0 	---> fail
    			others 	---> ok
*/
int ali_rpc_mutex_delete(int mutex_id)
{
	int id = mutex_id - 1;


	if(g_dual_mutex_status[id] != MUTEX_USED)
		return -1;
	
//	down(&m_rpc_sem_mutex);
	
	g_dual_mutex_status[id] = MUTEX_UNUSED;
	g_dual_mutex_cnt--;
	
//	up(&m_rpc_sem_mutex);	
	return 0;
}
EXPORT_SYMBOL(ali_rpc_mutex_delete);

/*
Another solution to do dual cpu procetion without any hardware help
cpu 1         cpu 2
w11+r1+r2   w21+r2+r1
if(cpu 1 get mutex)
  r1 = 1 & r2 = 0
*/
/*
Local exclusive access by local mutex 
To do:
wake up remote processor by interrupt?
*/
int ali_rpc_mutex_lock(int mutex_id, int timeout)
{
	int id = mutex_id - 1;


	down(&g_dual_mutex_local[id]);
	
	while(1)
	{
		ali_rpc_spinlock(0);
		if(IS_MUTEX_WRITTEN(id))
		{
			ali_rpc_spinunlock(0);
			msleep(1);
			continue;
		}
		LOCK_MUTEX(id);
		ali_rpc_spinunlock(0);
		break;
	}
	return 0;
}

EXPORT_SYMBOL(ali_rpc_mutex_lock);

/* Name : 
			ali_rpc_mutex_unlock
    Description :
			unlock the rpc mutex
    Arguments : 
    			int mutex_id ---> id of the rpc mutex
    Return value:
    			<= 0 	---> fail
    			others 	---> ok
*/
int ali_rpc_mutex_unlock(int mutex_id)
{
	int id = mutex_id - 1;
	
    	ali_rpc_spinlock(0);
	UNLOCK_MUTEX(id);
	ali_rpc_spinunlock(0);
	up(&g_dual_mutex_local[id]);
	return 0;
}
EXPORT_SYMBOL(ali_rpc_mutex_unlock);


/* Name : 
			ali_rpc_malloc_shared_mm
    Description :
			malloc the shared memory for the comminuction during the dual CPUs
    Arguments : 
    			unsigned long size ---> size of the needed buffer
    Return value:
    			NULL 	---> fail
    			others 	---> the shared memory buffer start address 
*/
void *ali_rpc_malloc_shared_mm(unsigned long size)
{
	UINT32 head = m_rpc_sm_head[0];
	UINT32 flen, off;
	UINT32 *ptr, *prev, *rptr;
	UINT32 *fptr, *fprev; 

	
	if(!size)
		return NULL; 
	
	//Alignment by DWORD
	size = (size + 3) & ~0x3;

	//printk("%s : size %d\n", __FUNCTION__, size);
	
	//Allocate shared memory in free list, fit block first, head block last 
	fptr = fprev = NULL;
	prev = m_rpc_sm_head;
	ptr  = (UINT32 *)((head>>16) + ALLOC_SM_ADDR); 
	//printk("ali_rpc_malloc_shared_mm, prev:0x%x, ptr:0x%x, flen:0x%x\n", prev, ptr, ptr[0]&0xffff);     
	while(1){
		head = ptr[0];
		flen  = head&0xffff; 
		if(flen == size)
			break;
		if(flen > size){
			fptr  = ptr;
			fprev = prev;
		};
		if(ptr == m_rpc_sm_head){
			if(fptr == NULL)
				return NULL;
			break;
		}
		prev = ptr; 
		ptr = (UINT32 *)((head>>16) + ALLOC_SM_ADDR); 
	}

	if(flen == size)
		prev[0] = (prev[0]&0xffff)|(ptr[0]&0xffff0000);
	else{
		ptr = fptr; prev = fprev;
		rptr = (UINT32 *)((UINT32)ptr + size);
		off  = (UINT32)rptr - ALLOC_SM_ADDR; 
		if(ptr == prev)
			rptr[0] = (flen - size)|(off<<16);
		else{
			rptr[0] = (flen - size)|(ptr[0]&0xffff0000);
			prev[0] = (prev[0]&0xffff)|(off<<16);
		}    
	}
	if(ptr == m_rpc_sm_head){
		m_rpc_sm_head = (UINT32 *)((UINT32)ptr + size); 
	}
	return ptr;     
}
EXPORT_SYMBOL(ali_rpc_malloc_shared_mm);


/* Name : 
			ali_rpc_free_shared_mm
    Description :
			free the malloced shared memory
    Arguments : 
    			void *buf ---> the shared memory buffer start address
    			unsigned logn size ---> size of the buffer
    Return value:
    			<= 0 	---> fail
    			others 	---> ok
*/
void ali_rpc_free_shared_mm(void *buf, unsigned long size)
{
	UINT32 head;
	UINT32 *fptr = buf;
	UINT32 *tprev, *tfptr;
	UINT32 *ptr, *prev;  

	
	if(!size)
		return; 
	
	//Alignment by DWORD
	size = (size + 3)&~0x3;
	prev = ptr  = m_rpc_sm_head;          
	//Find out previus and next blocks
	while(1){
		head = ptr[0];
		ptr  = (UINT32 *)((head>>16) + ALLOC_SM_ADDR);
		if(((prev < fptr) || (prev == m_rpc_sm_head)) && fptr < ptr)
			break; 
		if(ptr == m_rpc_sm_head)
			return;
		prev = ptr;          
	}
	tprev = (UINT32 *)((UINT32)prev + (prev[0]&0xffff));
	tfptr = (UINT32 *)((UINT32)fptr + size);
	if(fptr == tprev){
		//Merge with last free block
		size += (prev[0]&0xffff);
		prev[0] = (prev[0]&0xffff0000)|size;
		fptr = prev;
	}
	else
		prev[0] = (prev[0]&0xffff)|(((UINT32)fptr - ALLOC_SM_ADDR)<<16);	

	if(tfptr == ptr){
		//Merge with next free block
		if(prev == ptr)
			fptr[0] = (((UINT32)fptr- ALLOC_SM_ADDR)<<16)|(size + (ptr[0]&0xffff));
		else
			fptr[0] = (ptr[0]&0xffff0000)|(size + (ptr[0]&0xffff));
		if(tfptr == m_rpc_sm_head)
			m_rpc_sm_head = fptr; 
	}
	else{
		fptr[0] = size |(((UINT32)ptr - ALLOC_SM_ADDR)<<16);
	}
}
EXPORT_SYMBOL(ali_rpc_free_shared_mm);

static void rpc_mutex_init(void)
{
	int i;
	
	//init_MUTEX(&m_rpc_priv->sem_mutex);
	g_dual_mutex_read = (volatile UINT32 *)(READ_DUAL_MUTEX_ADDR);
	g_dual_mutex_write = (volatile UINT32 *)(WRITE_DUAL_MUTEX_ADDR);	
	*g_dual_mutex_read = 0;
	*g_dual_mutex_write = 0;
	g_dual_mutex_cnt = 0;
	memset(g_dual_mutex_status, MUTEX_UNUSED, DUAL_MUTEX_NUM);	
	for(i = 0; i < DUAL_MUTEX_NUM; i++)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
		init_MUTEX(&g_dual_mutex_local[i]);
#else
		sema_init(&g_dual_mutex_local[i], 1);
#endif
}

int ali_rpc_init_mutex(void)
{
	m_rpc_sm_head = (UINT32 *)ALLOC_SM_ADDR;
	m_rpc_sm_head[0] = ALLOC_SM_LEN;
	rpc_mutex_init();

	//printk("__G_ALI_MM_SHARED_MEM_START_ADDR:0x%08x, ALLOC_SM_ADDR:0x%08x ALLOC_SM_LEN:0x%08x\n", __G_ALI_MM_SHARED_MEM_START_ADDR, ALLOC_SM_ADDR
	//	, ALLOC_SM_LEN);	
#if 0
	printk("ali_rpc_init_mutex, m_rpc_sm_head:0x%x, m_rpc_sm_head[0]:%d\n", m_rpc_sm_head, m_rpc_sm_head[0]);
	void *ptr = NULL;
	ptr = ali_rpc_malloc_shared_mm(0x10);
	printk("ali_rpc_init_mutex, call share mem allocated 0x10 return:0x%x\n", ptr);
	if(ptr)
		ali_rpc_free_shared_mm(ptr, 0x10);			
#endif
}
//EXPORT_SYMBOL(ali_rpc_init_mutex);
