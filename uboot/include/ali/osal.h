
#ifndef _OSAL_H_
#define _OSAL_H_

#include <ali/basic_types.h>
//#include <ali/retcode.h>


typedef INT32  OSAL_ER;
typedef UINT32 OSAL_ID;

#define OSAL_INVALID_ID     0
#define OSAL_WAIT_FOREVER_TIME 0xff

typedef void (*OSAL_T_LSR_PROC_FUNC_PTR)(UINT32);
typedef void (*OSAL_T_HSR_PROC_FUNC_PTR)(UINT32);


#define osal_task_sleep(ms)  mdelay(ms)
#define osal_delay(us)  udelay(us)

#define osal_mutex_lock(...) do{}while(0)
#define osal_mutex_unlock(...) do{}while(0)

#define osal_interrupt_enable(...) do{}while(0)
#define osal_interrupt_disable(...) do{}while(0)

#define osal_interrupt_unregister_lsr(...) do{}while(0)

static inline OSAL_ID osal_mutex_create()
{
    return (OSAL_ID)1 ;
}
#define osal_interrupt_register_lsr(...) do{}while(0)


#define osal_cache_invalidate_all(...)	do{}while(0)
#define osal_cache_flush				flush_dcache_range
#define osal_cache_invalidate			invalidate_dcache_range
#define MUTEX_ENTER()                   do{}while(0)
#define MUTEX_LEAVE()               	 do{}while(0)    


#define DELAY_MS(ms)  		osal_task_sleep(ms)
#define DELAY_US(us)		osal_delay(us)


#endif


