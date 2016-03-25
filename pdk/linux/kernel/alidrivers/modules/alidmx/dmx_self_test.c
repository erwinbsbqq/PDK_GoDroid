

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/types.h>

#include <asm/io.h>
#include <ali_interrupt.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>

#include <linux/mm.h>
#include <linux/workqueue.h>


#include "dmx_stack.h"


struct workqueue_struct *dmx_test_workqueue;

struct work_struct dmx_test_work;



__s32 dmx_pid_flt_cb
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  cb_param
)
{

    //printk("%s,%d,cb_param:%x\n", __FUNCTION__, __LINE__, cb_param);

    return(0);

}


void dmx_pid_flt_test
(
    void
)
{

    __s32 flt_id_0;
    __s32 flt_id_1;
    __s32 flt_id_2;
    __s32 flt_id_3;
    
    __s32 ret;

    printk("%s,%d\n", __FUNCTION__, __LINE__);

#if 1
    flt_id_0 = dmx_pid_flt_register(0, 0, dmx_pid_flt_cb, 0);

    if (flt_id_0 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
#endif


#if 1
    flt_id_1 = dmx_pid_flt_register(0, 0, dmx_pid_flt_cb, 1);

    if (flt_id_1 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
#endif


#if 1
    ret = dmx_pid_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif



    msleep(5000);

#if 1
    ret = dmx_pid_flt_start(flt_id_1);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif

    msleep(5000);

#if 1
    ret = dmx_pid_flt_unregister(flt_id_1);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


#if 1
    flt_id_2 = dmx_pid_flt_register(0, 1, dmx_pid_flt_cb, 2);

    if (flt_id_2 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
#endif

#if 1
    ret = dmx_pid_flt_start(flt_id_2);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif

    msleep(5000);

#if 1
    ret = dmx_pid_flt_stop(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif

#if 1
    ret = dmx_pid_flt_stop(flt_id_2);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


#if 1
    ret = dmx_pid_flt_unregister(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif
    
#if 1
    ret = dmx_pid_flt_unregister(flt_id_2);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif



#if 1
    flt_id_3 = dmx_pid_flt_register(0, 17, dmx_pid_flt_cb, 3);

    if (flt_id_3 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
#endif


#if 1
    ret = dmx_pid_flt_unregister(flt_id_3);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


#if 1
    flt_id_3 = dmx_pid_flt_register(0, 17, dmx_pid_flt_cb, 3);

    if (flt_id_3 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d\n", __FUNCTION__, __LINE__);
#endif

#if 1
    ret = dmx_pid_flt_start(flt_id_3);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif

#if 1
    ret = dmx_pid_flt_stop(flt_id_3);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif

    msleep(2000);

#if 1
    ret = dmx_pid_flt_start(flt_id_3);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_pid_flt_unregister(flt_id_3);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


    printk("%s,%d\n", __FUNCTION__, __LINE__);

    for (;;)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        msleep(2000);
    }
}



__s32 dmx_ts_flt_cb
(
    struct dmx_ts_pkt_inf *pkt_inf,
    __u32                  cb_param
)
{
    printk("%s,%d,cb_param:%x,pid:%d,conti:%d\n",
           __FUNCTION__, __LINE__, cb_param, pkt_inf->pid, pkt_inf->conti_cnt);

    return(0);
}





void dmx_ts_flt_test
(
    void
)
{
    __s32 ret;
    __s32 flt_id_0;
    __s32 flt_id_1;
    
#if 1
    flt_id_0 = dmx_ts_flt_register(0, 17, dmx_ts_flt_cb, 0);

    if (flt_id_0 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);
#endif

#if 1
    ret = dmx_ts_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_ts_flt_stop(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_ts_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_ts_flt_stop(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_ts_flt_unregister(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


#if 1
    flt_id_0 = dmx_ts_flt_register(0, 16, dmx_ts_flt_cb, 0);

    if (flt_id_0 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);
#endif


#if 1
    flt_id_1 = dmx_ts_flt_register(0, 17, dmx_ts_flt_cb, 0);

    if (flt_id_0 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_1);
#endif


#if 1
    ret = dmx_ts_flt_start(flt_id_1);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif

#if 1
    ret = dmx_ts_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


#if 1
    ret = dmx_ts_flt_unregister(flt_id_1);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


#if 1
    ret = dmx_ts_flt_unregister(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif


    printk("%s,%d\n", __FUNCTION__, __LINE__);
    
    for (;;)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        
        msleep(2000);
    }

    return(0);
}






__s32 dmx_sec_flt_cb
(
    __u8                     *src,
    __s32                     len,
    enum DMX_SEC_FLT_CB_TYPE  cb_type,
    __u32                     cb_param
)
{
    printk("%s,%d,src:%x,len:%d,cb_type:%d,cb_param:%d\n",
           __FUNCTION__, __LINE__, src, len, cb_type, cb_param);


    return(0);
}






void dmx_sec_flt_test
(
    void
)
{
    __s32 ret;
    __s32 flt_id_0;
    __s32 flt_id_1;

    struct Ali_DmxSecMaskInfo SecMask;
    
#if 1
    flt_id_0 = dmx_sec_flt_register(0, 17, NULL, dmx_sec_flt_cb, 0);

    if (flt_id_0 < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);
#endif

#if 1
    ret = dmx_sec_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);

#if 1
    ret = dmx_sec_flt_stop(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);

#if 1
    ret = dmx_sec_flt_start(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);

#if 1
    ret = dmx_sec_flt_stop(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }

    msleep(5000);
#endif


    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);

#if 1
    ret = dmx_sec_flt_unregister(flt_id_0);

    if (ret < 0)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
    }
#endif

    printk("%s,%d,flt_id_0:%d\n", __FUNCTION__, __LINE__, flt_id_0);


}





void dmx_self_test_work
(
    struct work_struct *work
)
{
    //dmx_pid_flt_test();

    //dmx_ts_flt_test();

    dmx_sec_flt_test();

    return;
}





__s32 dmx_self_test_init
(
    void
)
{
    /* dmx_pid_flt_test
    */
    dmx_test_workqueue = create_workqueue("DmxPidFltTst");

    if (!dmx_test_workqueue)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);

        return(-1);
    }

    INIT_WORK(&dmx_test_work, dmx_self_test_work);

    queue_work(dmx_test_workqueue, &dmx_test_work);

    return(0);
}


