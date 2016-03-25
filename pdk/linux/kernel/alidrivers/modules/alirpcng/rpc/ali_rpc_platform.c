/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2013 Copyright (C)
 *
 *  File: ali_rpc.c
 *
 *  Description: Ali Remote Prcedure Call driver between main and see CPU.
 *
 *  History:
 *      Date              Author              Version              Comment
 *      ====            ======          =======         =======
 *  1.  2013.03.14     Tony.Zh            0.1.000             Initial
 ****************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_ALI_CHIP_M3921)
#include <asm/uaccess.h>
#endif
#include <ali_reg.h>
#include "ali_rpcng.h"

//#define  RPC_SAMPLE_TEST

//#define  RPC_SAMPLE_TEST_TSTRUCT
#define  RPC_SAMPLE_TEST_OPAQUE

//#define  RPC_CONSUME_CPU_TEST

Ulong  g_rpctestcalltime = 0;
static Thread  rpcinittid;

void stop_watchdog()
{
#if defined(CONFIG_ALI_CHIP_M3921)
	printk("watch dog disable doesn't be implemented here!\n");
#else	
	*(volatile Uint8 *)__REGALIRAW(0x18018504) = 0;	//disable watch dog
	printk("watch dog disable,count=0x%08x \n",*(unsigned long *)__REGALIRAW(0x18018500));
#endif
}


#ifdef RPC_SAMPLE_TEST


#define MAX_TEST_THREAD  3
Thread gtid[MAX_TEST_THREAD];
Int32  gthreadContex[MAX_TEST_THREAD];
Thread gtcpu;

/*Services*/
static Int32 Demo_Service1(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service1 execution!\n");

        return  0;
}

static Int32 Demo_Service2(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service2 execution!\n");

        return  0;
}

static Int32 Demo_Service3(Param *arg1, Param *arg2, Param *arg3, Param *arg4, Param *arg5, Param *arg6, Param *arg7, Param *arg8)
{
        Log(LOG_INFO, "<MAIN> Entered Demo_Service3 execution!\n");

        return  0;
}

EXPORT_RPC(Demo_Service1);
EXPORT_RPC(Demo_Service2);
EXPORT_RPC(Demo_Service3);



/*Test RpcCall common thread*/
static void TestRpcThreadCommon(void *contex)
{
        Int32 threadindex = *((Int32*)contex);
        RpcCd cd;
        struct timeval start, end;
        Ulong interval = 0;
        Int32 ret, result;
	Ulong fv;

        Log(LOG_INFO,"TestRpcThreadCommon threadindex:%d\n", threadindex);
        while(1){
                //PR_Sleep(threadindex+2);
                //PR_uSleep(20*1000);
                PR_uSleep(25*1000);
                Log(LOG_DEBUG,"{threadindex:%d} will send RpcCall for Demo_Service%d\n", threadindex, threadindex+2);
                do_gettimeofday(&start);

                if(threadindex==0)
                {
                        ret = RpcCallCompletion(Demo_Service2, NULL);
                }
                else if(threadindex==1)
                {
                        ret = RpcCallCompletion(Demo_Service3, NULL);
                }
                else if(threadindex==2)
                {
                        ret = RpcCallCompletion(Demo_Service4, NULL);
                }
                else if(threadindex==3)
                {
                        ret = RpcCallCompletion(Demo_Service5, NULL);
                }
                else if(threadindex==4)
                {
                        ret = RpcCallCompletion(Demo_Service6, NULL);
                }
                else if(threadindex==5)
                {
                        ret = RpcCallCompletion(Demo_Service7, NULL);
                }
                else if(threadindex==6)
                {
                        ret = RpcCallCompletion(Demo_Service8, NULL);
                }
                else if(threadindex==7)
                {
                        ret = RpcCallCompletion(Demo_Service9, NULL);
                }

                do_gettimeofday(&end);
                interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
		fv = interval;

                Log(LOG_INFO,"{threadindex:%d} RpcCallCompletion return, ret:%d, result:%d, consumed time: %d(ns)\n", threadindex, ret, result, (Int32)fv);
                //Log(LOG_INFO,"{threadindex:%d} RpcCallCompletion return, ret:%d, result:%d, \n", threadindex, ret, result);
        }

}


/*Consume cpu thread for Rpc pressure test*/
static void CpuConsumeTestRpcThread(void *contex)
{
	Ulong ul, ut;
	while(1){
		for(ul=0;ul<0x20000000;ul++){
			ut += 1;
		}
		PR_uSleep(1000);
	}

}

void RpcCallTest(Int32 localNode, Int32 remoteNode)
{
	Int32 ret, result, count, rid, i; 
	struct timeval start, end;
	Ulong interval = 0;
	Ulong fv;

	Param argtest, argtest_ret;
	XDR   xdr, *pxdr;
	Int32 ii = -111;
	Uint32 uii = 0x55aa;
	char  cc = 'T';
	Uchar ucc = 0x33;
	Long  ll = 0xaaaa;
	Ulong ull = 0xaaaa5555;
	Bool  bb = True;
	char name[64];
	Uchar  opaquebuf[750];

	TestStruct  tstruct, tstruct_ret;


	 Log(LOG_INFO, "Will startting RpcCallTest! localNode:%d, remoteNode:%d\n", localNode, remoteNode);

#ifdef RPC_CONSUME_CPU_TEST
	if(PR_ThreadCreate(&gtcpu, &CpuConsumeTestRpcThread, NULL, 0, 0, "RPC_CPU_CONSUME")!=RPC_OS_OK)
	{		
			Log(LOG_ERR,"RpcCallTest create cpu consume test thread failed!\n");
			return;
	}

#endif

#if  0 
	/*startup multi threads to do parallel rpcCall test*/
	for(i=0;i<MAX_TEST_THREAD;i++){
		gthreadContex[i] = i;
		sprintf(name, "rpc_test_thr%d",i+1);
		if(PR_ThreadCreate(&gtid[i], &TestRpcThreadCommon, (void*)&gthreadContex[i], 0, 0, name)!=RPC_OS_OK)
		{		
			Log(LOG_ERR,"RpcCallTest create test thread failed!,i:%d\n",i);
			return;
		}
		PR_uSleep(100);
	}
#endif

	tstruct.ii = -111;
	tstruct.uii = 0x55aa;
	tstruct.cc = 'T';
	tstruct.ucc= 0x33;
	tstruct.ll = 0xaaaa;
	tstruct.ull = 0xaaaa5555;
	tstruct.bb = True;

#ifdef RPC_SAMPLE_TEST_TSTRUCT
  	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct);
  	RPC_PARAM_CREATE(p2, PARAM_OUT, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct_ret);
#endif

#ifdef RPC_SAMPLE_TEST_OPAQUE
  	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_OPAQUE, 128, opaquebuf);
#endif

	while(1){

		Log(LOG_INFO, "~~~~~MAIN Will send RpcCall~~~~~~\n");
#ifdef RPC_SAMPLE_TEST_TSTRUCT
		RPC_PARAM_UPDATE(p1, PARAM_IN, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct);
  		RPC_PARAM_UPDATE(p2, PARAM_OUT, PARAM_TESTSTRUCT, sizeof(TestStruct), &tstruct_ret);
		memset(&tstruct_ret,0,sizeof(TestStruct));
#endif

#ifdef RPC_SAMPLE_TEST_OPAQUE
   		RPC_PARAM_UPDATE(p1, PARAM_OUT, PARAM_OPAQUE, 750, opaquebuf);
		//memset(opaquebuf, 0x55, 128);
		opaquebuf[0] = (Int8)0x88;
		opaquebuf[1] = (Int8)0x44;
		opaquebuf[748] = (Int8)0x99;
		opaquebuf[749] = (Int8)0x66;
		Log(LOG_ERR,"opaquebuf[0]:0x%x, opaquebuf[1]:0x%x, opaquebuf[748]:0x%x, opaquebuf[749]:0x%x\n", opaquebuf[0], opaquebuf[1],opaquebuf[748],opaquebuf[749] );
#endif
		do_gettimeofday(&start);

#if 1 

#ifdef RPC_SAMPLE_TEST_TSTRUCT    
   	ret = RpcCallCompletion(Demo_Service1, &p1, &p2, NULL);
#endif

#ifdef RPC_SAMPLE_TEST_OPAQUE
        ret = RpcCallCompletion(Demo_Service10, &p1, NULL);
#endif
		do_gettimeofday(&end);
		interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
		fv = interval;

		g_rpctestcalltime = fv;
		Log(LOG_INFO,"RpcCallCompletion return, ret:%d, consumed time: %d(ns)\n",ret, (Int32)fv);
#ifdef RPC_SAMPLE_TEST_TSTRUCT
		Log(LOG_INFO,"RpcCallCompletion, [peer tstruct_ret result] ii:%d, uii:0x%x, cc:%c, ucc:0x%x, ll:0x%x, ull:0x%x, bb:0x%x\n", tstruct_ret.ii, tstruct_ret.uii, tstruct_ret.cc, tstruct_ret.ucc, tstruct_ret.ll, tstruct_ret.ull, tstruct_ret.bb);
#endif	

#ifdef RPC_SAMPLE_TEST_OPAQUE
		Log(LOG_ERR,"RpcCallCompletion, [peer opaque return result] buf[0]:%x, buf[1]:%x, buf[748]:%x, buf[749]:%x",opaquebuf[0],opaquebuf[1],opaquebuf[748], opaquebuf[749]);
#endif



#endif

/*For non-blocking rpcCall*/
#if 0
   	rid = RpcCall(Demo_Service1,&p1, &p2, NULL);
		Log(LOG_DEBUG,"RpcCall non-blocking call return,[rid]:%d\n",rid);
		if(rid != RPC_ERROR_VALUE){
		
			ret = RpcWait( rid, 1500);
			if(ret == RPC_OS_OK){

				result = RpcExtractRet(rid);
				Log(LOG_DEBUG,"RpcCall returned result:%d from peer\n",result);	
			}
			else{
				Log(LOG_DEBUG,"RpcWait timeout or error return, ret:%d\n",ret);
			}
			RpcFreeRetBuf(rid);
		}
		else
			Log(LOG_INFO,"RpcCall failed!\n");
#endif
/*Non-blocking Call end*/


		PR_uSleep(100*1000);
		//PR_uSleep(5*1000);
		//PR_uSleep(1*1000);
		//PR_uSleep(10*1000);
		//PR_uSleep(20*1000);
		//PR_uSleep(2*1000);
	}


	Log(LOG_DEBUG, "Completing RpcCallTest!\n");
}

#endif

/*Proc based contron intertace*/
#define RPC_DEBUG_PROC_DIR "alirpc"
#define RPC_DEBUG_PROC_LEVEL "debuglevel"
#define RPC_DEBUG_PROC_CALTIME "caltime"
#define RPC_DEBUG_PROC_INFO "debuginfo"

struct proc_dir_entry *rpc_proc_dir = NULL;
struct proc_dir_entry *rpc_proc_debug_file = NULL;
struct proc_dir_entry *rpc_proc_caltime_file = NULL;
struct proc_dir_entry *rpc_proc_dbginfo_file = NULL;
extern Int32    g_dbglevel;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t rpc_debuglevel_procfile_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	Int32 ret;
	char buffer[64];
		Log(LOG_DEBUG,"rpc_debuglevel_procfile_read, size:%d\n", size);
		if(size>0){
			buffer[0] = g_dbglevel+0x30;
			ret = 1;
		}
		else{
			ret = 0;
		}
		return simple_read_from_buffer(ubuf, size, ppos, buffer, ret);  
}

static ssize_t rpc_debuglevel_procfile_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	Int8 buf[2];
	Int32 itemp, ret;

	Log(LOG_DEBUG,"rpc_debuglevel_procfile_write, count:%d\n",count);
	if(count<=0)
		return 0;
	if(copy_from_user(buf, buffer, 1))  
         	return -EFAULT;  
        Log(LOG_DEBUG,"rpc_debuglevel_procfile_write, received buf:%c \n",buf[0]);
	itemp = buf[0]-0x30;
	if(itemp>=LOG_EMERG && itemp<=LOG_DEBUG)
 		g_dbglevel = itemp;
	/*Call remote debug level setting*/
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &g_dbglevel);
        ret = RpcCallCompletion(RPC_DebugLevel_Service, &p1, NULL);
        if(ret!=0)
                Log(LOG_EMERG,"Set remote debug level failed, ret:%d\n",ret);

	Log(LOG_DEBUG,"rpc_debuglevel_procfile_write completed!"); 
	return count;
}

#else
int rpc_debuglevel_procfile_read(char*buffer, char**buffer_localation, off_t offset,int buffer_length,int* eof, void *data )
{
	Int32 ret;

	if(offset>0){
		*eof = 1;
		return 0;
	}
	else{
		Log(LOG_DEBUG,"rpc_debuglevel_procfile_read, buffer_length:%d\n", buffer_length);
		if(buffer_length>0){
			buffer[0] = g_dbglevel+0x30;
			ret = 1;
		}
		else{
			ret = 0;
		}
		*eof = 1;
	}
	return ret;
}

int rpc_debuglevel_procfile_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
	Int8 buf[2];
	Int32 itemp, ret;

	Log(LOG_DEBUG,"rpc_debuglevel_procfile_write, count:%d\n",count);
	if(count<=0)
		return 0;
	if(copy_from_user(buf, buffer, 1))  
         	return -EFAULT;  
        Log(LOG_DEBUG,"rpc_debuglevel_procfile_write, received buf:%c \n",buf[0]);
	itemp = buf[0]-0x30;
	if(itemp>=LOG_EMERG && itemp<=LOG_DEBUG)
 		g_dbglevel = itemp;
	/*Call remote debug level setting*/
	RPC_PARAM_CREATE(p1, PARAM_IN, PARAM_INT32, sizeof(Int32), &g_dbglevel);
        ret = RpcCallCompletion(RPC_DebugLevel_Service, &p1, NULL);
        if(ret!=0)
                Log(LOG_EMERG,"Set remote debug level failed, ret:%d\n",ret);

	Log(LOG_DEBUG,"rpc_debuglevel_procfile_write completed!"); 
	return count;
}
#endif

/*Cacualte the call costed time*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t rpc_caltime_procfile_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	        Int32 ret, itemp;
	        char buffer[64];//??seq_printf maybe best choice
       
                Log(LOG_DEBUG,"rpc_caltime_procfile_read, size:%d, caltime:%d\n", size, g_rpctestcalltime);
                if(size>1){
			itemp = g_rpctestcalltime%10000000;
			
			buffer[0] = itemp/1000000+0x30;
                        itemp = itemp%1000000;
                        buffer[1] = itemp/100000+0x30;
                        itemp = itemp%100000;
			buffer[2] = itemp/10000+0x30;
                        itemp = itemp%10000;
                        buffer[3] = itemp/1000+0x30;
			itemp = itemp%1000;	
                        buffer[4] = itemp/100+0x30;
			itemp = itemp%100;
                        buffer[5] = itemp/10+0x30;
			buffer[6] = itemp%10+0x30;
                        ret = 7;
                }
                else{
                        ret = 0;
                }
         return simple_read_from_buffer(ubuf, size, ppos, buffer, ret);       
}
static ssize_t rpc_caltime_procfile_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	return count;
}
#else
int rpc_caltime_procfile_read(char*buffer, char**buffer_localation, off_t offset,int buffer_length,int* eof, void *data )
{
        Int32 ret, itemp;

        if(offset>0){
                *eof = 1;
                return 0;
        }
        else{
                Log(LOG_DEBUG,"rpc_caltime_procfile_read, buffer_length:%d, caltime:%d\n", buffer_length, g_rpctestcalltime);
                if(buffer_length>1){
			itemp = g_rpctestcalltime%10000000;
			
			buffer[0] = itemp/1000000+0x30;
                        itemp = itemp%1000000;
                        buffer[1] = itemp/100000+0x30;
                        itemp = itemp%100000;
			buffer[2] = itemp/10000+0x30;
                        itemp = itemp%10000;
                        buffer[3] = itemp/1000+0x30;
			itemp = itemp%1000;	
                        buffer[4] = itemp/100+0x30;
			itemp = itemp%100;
                        buffer[5] = itemp/10+0x30;
			buffer[6] = itemp%10+0x30;
                        ret = 7;
                }
                else{
                        ret = 0;
                }
                *eof = 1;
        }
        return ret;
}

int rpc_caltime_procfile_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
        return count;
}
#endif 
/*Process debug info*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
ssize_t rpc_dbginfo_procfile_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
		RPCDBGINFO local,remote, *pdbg;
	Int32 len = 0, ret;
	char buffer[1024];//??seq_printf maybe best choice

	

	memset(&local, 0, sizeof(RPCDBGINFO));
	memset(&remote, 0, sizeof(RPCDBGINFO));

	/*Local debug info*/
	len += sprintf(&buffer[len],"[Local Debug Info]\n");
	if(!RpcGetDetailInfo(&local, False)){
		pdbg = &local;
		len += sprintf(&buffer[len],"RunningStatus:%d\nrpcversion:%d\nmcapiversion:%d\ntotalrunningInvoker:%d\ntotalsleepInvoker:%d\ntotalserviceQtask:%d\ntotalRegisteredService:%d\ntotalsendmsgcall:%d\ntotalsendpktcall:%d\ntotalrcvmsgcall:%d\ntotalrcvpktcall:%d\ntotalrcvmsgret:%d\ntotalrcvpktret:%d\ntotalrcvbadmsgpacket:%d\ntotalrcvbadpktpacket:%d\ntotalrcvbadfuncidcall:%d\n\n",pdbg->RunningStatus, pdbg->rpcversion, pdbg->mcapiversion, pdbg->totalrunningInvoker, pdbg->totalsleepInvoker, pdbg->totalserviceQtask,pdbg->totalRegisteredService,pdbg->totalsendmsgcall,pdbg->totalsendpktcall,pdbg->totalrcvmsgcall,pdbg->totalrcvpktcall,pdbg->totalrcvmsgret,pdbg->totalrcvpktret,pdbg->totalrcvbadmsgpacket,pdbg->totalrcvbadpktpacket,pdbg->totalrcvbadfuncidcall);
	}
	/*Remote debug info*/
	len += sprintf(&buffer[len],"[Remote Debug Info]\n");
	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_RPCDBGINFO, sizeof(RPCDBGINFO), &remote);	
	ret = RpcCallCompletion(RPC_DebugInfo_Service, &p1, NULL);	
	if(ret!=0){
		Log(LOG_EMERG,"Get debug info failed from see, ret:%d\n",ret);
		len += sprintf(&buffer[len],"Connection failed!\n");
	}
	else{
		pdbg = &remote;
                len += sprintf(&buffer[len],"RunningStatus:%d\nrpcversion:%d\nmcapiversion:%d\ntotalrunningInvoker:%d\ntotalsleepInvoker:%d\ntotalserviceQtask:%d\ntotalRegisteredService:%d\ntotalsendmsgcall:%d\ntotalsendpktcall:%d\ntotalrcvmsgcall:%d\ntotalrcvpktcall:%d\ntotalrcvmsgret:%d\ntotalrcvpktret:%d\ntotalrcvbadmsgpacket:%d\ntotalrcvbadpktpacket:%d\ntotalrcvbadfuncidcall:%d\n\n",pdbg->RunningStatus, pdbg->rpcversion, pdbg->mcapiversion, pdbg->totalrunningInvoker, pdbg->totalsleepInvoker, pdbg->totalserviceQtask,pdbg->totalRegisteredService,pdbg->totalsendmsgcall,pdbg->totalsendpktcall,pdbg->totalrcvmsgcall,pdbg->totalrcvpktcall,pdbg->totalrcvmsgret,pdbg->totalrcvpktret,pdbg->totalrcvbadmsgpacket,pdbg->totalrcvbadpktpacket,pdbg->totalrcvbadfuncidcall);
	}
	
	return simple_read_from_buffer(ubuf, size, ppos, buffer, len);
}

static ssize_t rpc_dbginfo_procfile_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	return count;
}
#else
int rpc_dbginfo_procfile_read(char*buffer, char**buffer_localation, off_t offset,int buffer_length,int* eof, void *data )
{
	RPCDBGINFO local,remote, *pdbg;
	Int32 len = 0, ret;

	if(offset>0){
		Log(LOG_DEBUG,"rpc_dbginfo_procfile_read offset:%d\n",offset);
		*eof = 1;
		return 0;
	}

	memset(&local, 0, sizeof(RPCDBGINFO));
	memset(&remote, 0, sizeof(RPCDBGINFO));

	/*Local debug info*/
	len += sprintf(&buffer[len],"[Local Debug Info]\n");
	if(!RpcGetDetailInfo(&local, False)){
		pdbg = &local;
		len += sprintf(&buffer[len],"RunningStatus:%d\nrpcversion:%d\nmcapiversion:%d\ntotalrunningInvoker:%d\ntotalsleepInvoker:%d\ntotalserviceQtask:%d\ntotalRegisteredService:%d\ntotalsendmsgcall:%d\ntotalsendpktcall:%d\ntotalrcvmsgcall:%d\ntotalrcvpktcall:%d\ntotalrcvmsgret:%d\ntotalrcvpktret:%d\ntotalrcvbadmsgpacket:%d\ntotalrcvbadpktpacket:%d\ntotalrcvbadfuncidcall:%d\n\n",pdbg->RunningStatus, pdbg->rpcversion, pdbg->mcapiversion, pdbg->totalrunningInvoker, pdbg->totalsleepInvoker, pdbg->totalserviceQtask,pdbg->totalRegisteredService,pdbg->totalsendmsgcall,pdbg->totalsendpktcall,pdbg->totalrcvmsgcall,pdbg->totalrcvpktcall,pdbg->totalrcvmsgret,pdbg->totalrcvpktret,pdbg->totalrcvbadmsgpacket,pdbg->totalrcvbadpktpacket,pdbg->totalrcvbadfuncidcall);
	}
	/*Remote debug info*/
	len += sprintf(&buffer[len],"[Remote Debug Info]\n");
	RPC_PARAM_CREATE(p1, PARAM_OUT, PARAM_RPCDBGINFO, sizeof(RPCDBGINFO), &remote);	
	ret = RpcCallCompletion(RPC_DebugInfo_Service, &p1, NULL);	
	if(ret!=0){
		Log(LOG_EMERG,"Get debug info failed from see, ret:%d\n",ret);
		len += sprintf(&buffer[len],"Connection failed!\n");
	}
	else{
		pdbg = &remote;
                len += sprintf(&buffer[len],"RunningStatus:%d\nrpcversion:%d\nmcapiversion:%d\ntotalrunningInvoker:%d\ntotalsleepInvoker:%d\ntotalserviceQtask:%d\ntotalRegisteredService:%d\ntotalsendmsgcall:%d\ntotalsendpktcall:%d\ntotalrcvmsgcall:%d\ntotalrcvpktcall:%d\ntotalrcvmsgret:%d\ntotalrcvpktret:%d\ntotalrcvbadmsgpacket:%d\ntotalrcvbadpktpacket:%d\ntotalrcvbadfuncidcall:%d\n\n",pdbg->RunningStatus, pdbg->rpcversion, pdbg->mcapiversion, pdbg->totalrunningInvoker, pdbg->totalsleepInvoker, pdbg->totalserviceQtask,pdbg->totalRegisteredService,pdbg->totalsendmsgcall,pdbg->totalsendpktcall,pdbg->totalrcvmsgcall,pdbg->totalrcvpktcall,pdbg->totalrcvmsgret,pdbg->totalrcvpktret,pdbg->totalrcvbadmsgpacket,pdbg->totalrcvbadpktpacket,pdbg->totalrcvbadfuncidcall);
	}

	*eof = 1;
        return len;
}

int rpc_dbginfo_procfile_write(struct file *filp, const char *buffer,unsigned long count,void *data)
{
        return count;
}
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static const struct file_operations alirpc_debug_fops = {
	.read = rpc_debuglevel_procfile_read,
	.write = rpc_debuglevel_procfile_write,
	.llseek = default_llseek,
};

static const struct file_operations alirpc_caltime_fops = {
	.read = rpc_caltime_procfile_read,
	.write = rpc_caltime_procfile_write,
	.llseek = default_llseek,
};

static const struct file_operations alirpc_dbginfo_fops = {
	.read = rpc_dbginfo_procfile_read,
	.write = rpc_dbginfo_procfile_write,
	.llseek = default_llseek,
};

#endif
/*rpc proc debug initial*/
static int  rpc_debug_procfs_init(void)
{

     	rpc_proc_dir = proc_mkdir(RPC_DEBUG_PROC_DIR, NULL);  

     	if (rpc_proc_dir == NULL) {  
		Log(LOG_ERR,"rpc_debug_procfs_init create dir alirpc failed!!\n");
		return -1;
     	} 

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
        rpc_proc_debug_file = proc_create(RPC_DEBUG_PROC_LEVEL,0644,rpc_proc_dir, &alirpc_debug_fops);
#else
        rpc_proc_debug_file = create_proc_entry(RPC_DEBUG_PROC_LEVEL,0644,rpc_proc_dir);
#endif
        if(rpc_proc_debug_file == NULL)
        {
                remove_proc_entry(RPC_DEBUG_PROC_DIR, NULL);
                Log(LOG_ERR,"Error:could not initialize /proc/alirpc/%s\n",RPC_DEBUG_PROC_LEVEL);
                return -1;
        }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))        
        rpc_proc_debug_file->read_proc = rpc_debuglevel_procfile_read;
        rpc_proc_debug_file->write_proc = rpc_debuglevel_procfile_write;

	/*For RpcCall time costed view*/
	rpc_proc_caltime_file = create_proc_entry(RPC_DEBUG_PROC_CALTIME,0644,rpc_proc_dir);
#else
	/*For RpcCall time costed view*/
	rpc_proc_caltime_file = proc_create(RPC_DEBUG_PROC_CALTIME,0644,rpc_proc_dir, &alirpc_caltime_fops);
#endif
        if(rpc_proc_caltime_file == NULL)
        {
		remove_proc_entry(RPC_DEBUG_PROC_LEVEL, rpc_proc_dir);
                remove_proc_entry(RPC_DEBUG_PROC_DIR, NULL);
                Log(LOG_ERR,"Error:could not initialize /proc/alirpc/%s\n",RPC_DEBUG_PROC_CALTIME);
                return -1;
        }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))          
        rpc_proc_caltime_file->read_proc = rpc_caltime_procfile_read;
        rpc_proc_caltime_file->write_proc = rpc_caltime_procfile_write;

	/*For Debug info*/
        rpc_proc_dbginfo_file = create_proc_entry(RPC_DEBUG_PROC_INFO,0644,rpc_proc_dir);
#else
	/*For Debug info*/
        rpc_proc_dbginfo_file = proc_create(RPC_DEBUG_PROC_INFO,0644,rpc_proc_dir, &alirpc_dbginfo_fops);
#endif
        if(rpc_proc_dbginfo_file == NULL)
        {
		remove_proc_entry(RPC_DEBUG_PROC_LEVEL, rpc_proc_dir);
		remove_proc_entry(RPC_DEBUG_PROC_CALTIME, rpc_proc_dir);
                remove_proc_entry(RPC_DEBUG_PROC_DIR, NULL);
                Log(LOG_ERR,"Error:could not initialize /proc/alirpc/%s\n",RPC_DEBUG_PROC_INFO);
                return -1;
        }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))        
        rpc_proc_dbginfo_file->read_proc = rpc_dbginfo_procfile_read;
        rpc_proc_dbginfo_file->write_proc = rpc_dbginfo_procfile_write;
#endif

        Log(LOG_DEBUG,"/proc/alirpc/%s is created\n",RPC_DEBUG_PROC_LEVEL);
        return 0;
}


static void __exit rpc_debug_procfs_exit(void)
{
        remove_proc_entry(RPC_DEBUG_PROC_LEVEL, rpc_proc_dir);
        remove_proc_entry(RPC_DEBUG_PROC_CALTIME, rpc_proc_dir);
        remove_proc_entry(RPC_DEBUG_PROC_INFO, rpc_proc_dir);
	remove_proc_entry(RPC_DEBUG_PROC_DIR, NULL);
        Log(LOG_DEBUG,"/proc/alirpc/%s has been removed\n",RPC_DEBUG_PROC_LEVEL);
}




void  g_see_boot();
extern void rpc_remote_dev_init(void);

void RpcInitThread(void *contex)
{
	//Log(LOG_DEBUG,"MAIN, RpcInitThread enered!\n");
	
#ifdef __BOOT_SEE_IN_UBOOT__
	Log(LOG_ERR,"++++++MAIN Will not startup the see!\n");
#else	
	Log(LOG_ERR,"++++++MAIN Will startup the see!\n");
	g_see_boot();
	PR_uSleep(100000);
	Log(LOG_ERR,"++++++MAIN startup see completed!\n");
#endif

#if 1 //masked for temp s3921 debug
	RpcInit(0);
	rpc_remote_dev_init(); /*Attach all see drivers*/
#endif
#ifdef RPC_SAMPLE_TEST
	RpcCallTest(0, 1);
#endif		

}

void rpc_ali_modinit(void)
{
#ifdef RPC_SAMPLE_TEST
	/*Create a thread to init the RPC*/	
	if(PR_ThreadCreate(&rpcinittid, (ThreadFunc)&RpcInitThread, 0, 0, 0, "rpc_test_main")!=RPC_OS_OK)
        {
		Log(LOG_ERR,"Create RpcInitThread thread failed!\n");
                return;
        }
#else
	RpcInitThread(NULL);
#endif	
}

module_init(rpc_debug_procfs_init);

static void rpc_ali_modexit(void)
{
	RpcDeinit();	

}
module_exit(rpc_debug_procfs_exit);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Tony.Zh");
MODULE_DESCRIPTION("ALi RPC");

