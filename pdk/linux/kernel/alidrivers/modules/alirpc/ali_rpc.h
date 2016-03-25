#ifndef __DRIVERS_ALI_RPC_H
#define __DRIVERS_ALI_RPC_H

#if 0
#define RPC_PRF printk
#else
#define RPC_PRF(...) 	do{}while(0)
#endif

#define RPC_NUM			5

#define GET_DWORD(addr)            (*(volatile UINT32 *)(addr))
#define SET_DWORD(addr, d)         (*(volatile UINT32 *)(addr)) = (d)
#define SETB_DWORD(addr, bit)      *(volatile UINT32 *)(addr) |= (bit)
#define SETB_BYTE(addr, bit)       *(volatile UINT8 *)(addr) |= (bit)
#define GET_BYTE(addr)            (*(volatile UINT8 *)(addr))
#define SET_BYTE(addr, bit)       (*(volatile UINT8 *)(addr)) = (bit)

struct remote_call_msg
{
    UINT32 size:30;
    volatile UINT32 flag:2;
    UINT8  func;
    UINT8  pri;
    UINT8  resv;
    UINT8  module;
    UINT32 caller;
    UINT32 retbuf;
    UINT32 parasize;
    UINT32 para[16];
};

struct remote_response_msg
{
    UINT32 size:30;
    UINT32 flag:2;
    UINT32 ret;
    UINT32 caller;
    UINT32 retbuf;
};


//Serialize structure 
//被串行化结构的描述
struct desc_storage_struct
{
    UINT16 size;  //size of this structure 结构大小
    UINT8  flag;  //flag of this structure 结构标志
    UINT8  ind;   //index in out para list
    UINT32 off;   //offset of message buffer for storage 结构的存储位置
    UINT32 wp;    //write back address for out parameter
};
//指向被串行化结构的指针的描述
//不支持不对齐的指针
struct desc_pointer_struct
{              
    UINT16 pflag:2; //0: pointer of this structure is a parameter  
                    //1: pointer of this structure is member of another structure
    UINT16 poff:14; //pointer offset if pflag == 1, max struct 16K
                    //index of parameter list if  pflag == 0 
    UINT8  pind;    //index of desc_struct list for pointer 
    UINT8  sind;    //index of desc_struct list for pointed
};

//Attribute of pointer
#define DESC_P_IS_PARA(x)    (((x)&0x1)  == 0)
#define DESC_P_IS_MEMBER(x)  (((x)&0x1)  == 1)
#define DESC_P_IS_RET(x)     (((x)&0x1)  == 0)

#define DESC_STRU_IS_OUTPUT(x) (((x)&0x1) == 1)
#define DESC_STRU_IS_CONSTR(x) (((x)&0x2) == 2)  
#define DESC_STRU_IS_LIST(x)   (((x)&0x4) == 4) 
#define DESC_STRU_IS_DYNAMIC(x)   ((x)&0xe) 

#define TYPE_OUTPUT            1
#define TYPE_STRING            2
#define TYPE_LIST              4
#define TYPE_SIZE_UB           0x10
#define TYPE_SIZE_UH           0x20
#define TYPE_SIZE_UW           0x00

typedef void (*NORMAL_CALLEE)(UINT8 *);

/* actively call the remote device */
#define MAILBOX_GET_REMOTE_CALL		1
/* passively reply to the remote device */
#define MAILBOX_GET_REMOTE_RET		2

#define CALL_MSG_BUFFER_SIZE 		(MSG_BUFFER_SIZE * 32)//(MSG_BUFFER_SIZE * 16)
#define RET_MSG_BUFFER_SIZE			(MSG_BUFFER_SIZE * 32)//(MSG_BUFFER_SIZE * 16)

#define MSG_RECEIVED 					0xffffffff
#define RPC_DISABLE                     0xdeaddead

#define REMOTE_CALL_TIME_OUT			(HZ * 100)

struct ali_rpc_private
{
	/* device name */
	char *name;

	/* semaphore for call */
	struct semaphore sem_call;

	/* semaphore for mutex operation */
	struct semaphore sem_mutex;

	/* control flag */
	volatile unsigned long flag;
	
	/* remote call wait queue */
	wait_queue_head_t wait_call;

	/* kernel thread to deal with the local call */
	struct task_struct *thread_call;

	/* out and in msg buffer */			
	void *out_msg_buf[RPC_NUM];
	int out_msg_buf_valid[RPC_NUM];
	volatile unsigned long rpc_flag[RPC_NUM];
	
	void *in_msg_buf;

	/* isr status */
	unsigned long isr_call:1;
	unsigned long isr_ret:1;
	unsigned long res:30;

};

//add by phil for boot-media
void rpc_remote_boot_media(void);
void rpc_remote_dev_init(void);
void ali_rpc_hld_base_callee(UINT8 *msg);

#endif
