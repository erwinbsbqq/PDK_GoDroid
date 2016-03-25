

#if 1
#include <linux/mutex.h>

#if 0
#include <linux/spinlock.h>
#endif

#include "dmx_stack.h"

struct dmx_mutex_module ali_dmx_mutex_module;

#if 0
unsigned long ali_dmx_mutex_irq_flags;
spinlock_t ali_dmx_mutex_irq_spinlock;
#endif

__inline__ __s32 dmx_mutex_output_lock
(
    __u32 dev_id
)
{
    __s32         ret;
    struct mutex *mutex;

#if 0
    spin_lock_irqsave(&ali_dmx_mutex_irq_spinlock, ali_dmx_mutex_irq_flags);
	
    return(0);
#endif

#if 0
    mutex = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    ret = mutex_lock_interruptible(mutex);

    return(ret);
#endif

#if 1
    mutex = &(ali_dmx_mutex_module.output_dev_mutex[0]);

    ret = mutex_lock_interruptible(mutex);

    return(ret);
#endif


}



__inline__ __s32 dmx_mutex_output_unlock
(
    __u32 dev_id
)
{
    struct mutex *mutex;

#if 0
    spin_unlock_irqrestore(&ali_dmx_mutex_irq_spinlock, ali_dmx_mutex_irq_flags);

    return(0);
#endif

#if 0
    mutex = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    mutex_unlock(mutex);

    return(0);
#endif   

#if 1
    mutex = &(ali_dmx_mutex_module.output_dev_mutex[0]);

    mutex_unlock(mutex);

    return(0);
#endif 
}



__inline__ __s32 dmx_mutex_input_lock
(
    __u32 dev_id
)
{
    __s32         ret;
    struct mutex *mutex;

    mutex = &(ali_dmx_mutex_module.input_dev_mutex[dev_id]);

    ret = mutex_lock_interruptible(mutex);

    return(ret);
}



__inline__ __s32 dmx_mutex_input_unlock
(
    __u32 dev_id
)
{
    struct mutex *mutex;

    mutex = &(ali_dmx_mutex_module.input_dev_mutex[dev_id]);

    mutex_unlock(mutex);

    return(0);
}



__inline__ __s32 dmx_mutex_module_init
(
    void
)
{
    __u32 idx;

    for (idx = 0; idx < DMX_LINUX_OUTPUT_DEV_CNT; idx++)
    {
        mutex_init(&(ali_dmx_mutex_module.output_dev_mutex[idx]));
    }

    for (idx = 0; idx < DMX_LINUX_INPUT_DEV_CNT; idx++)
    {
        mutex_init(&(ali_dmx_mutex_module.input_dev_mutex[idx]));
    }

#if 0
    spin_lock_init(&ali_dmx_mutex_irq_spinlock);
#endif

    return(0);
}


__inline__ __s32 dmx_mutex_lock
(
    __u32 dev_id
)
{
    __s32         ret;
    struct mutex *mutex;

    mutex = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    ret = mutex_lock_interruptible(mutex);

    return(ret);
}



__inline__ __s32 dmx_mutex_unlock
(
    __u32 dev_id
)
{
    struct mutex *mutex;

    mutex = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    mutex_unlock(mutex);

    return(0);
}
#else
#include <linux/spinlock.h>
#include "dmx_stack.h"

struct dmx_mutex_module ali_dmx_mutex_module;

__s32 dmx_mutex_output_lock
(
    __u32 dev_id
)
{
    //__s32         ret;
    //struct mutex *mutex;
    spinlock_t *spinlock;
    
    spinlock = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    spin_lock(spinlock);

    return(0);
}



__s32 dmx_mutex_output_unlock
(
    __u32 dev_id
)
{
    //struct mutex *mutex;
    spinlock_t *spinlock;

    spinlock = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    spin_unlock(spinlock);

    return(0);
}



__s32 dmx_mutex_input_lock
(
    __u32 dev_id
)
{
    //__s32         ret;
    //struct mutex *mutex;
    spinlock_t *spinlock;

    spinlock = &(ali_dmx_mutex_module.input_dev_mutex[dev_id]);

    spin_lock(spinlock);

    return(0);
}



__s32 dmx_mutex_input_unlock
(
    __u32 dev_id
)
{
    //struct mutex *mutex;
    spinlock_t *spinlock;

    spinlock = &(ali_dmx_mutex_module.input_dev_mutex[dev_id]);

    spin_unlock(spinlock);

    return(0);
}



__s32 dmx_mutex_module_init
(
    void
)
{
    __u32 idx;

    for (idx = 0; idx < DMX_LINUX_OUTPUT_DEV_CNT; idx++)
    {
        spin_lock_init(&(ali_dmx_mutex_module.output_dev_mutex[idx]));
    }

    for (idx = 0; idx < DMX_LINUX_INPUT_DEV_CNT; idx++)
    {
        spin_lock_init(&(ali_dmx_mutex_module.input_dev_mutex[idx]));
    }

    return(0);
}


__inline__ __s32 dmx_mutex_lock
(
    __u32 dev_id
)
{
    //__s32         ret;
    //struct mutex *mutex;
    spinlock_t *spinlock;
    
    spinlock = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    spin_lock(spinlock);

    return(0);
}



__inline__ __s32 dmx_mutex_unlock
(
    __u32 dev_id
)
{
    //struct mutex *mutex;
    spinlock_t *spinlock;

    spinlock = &(ali_dmx_mutex_module.output_dev_mutex[dev_id]);

    spin_unlock(spinlock);

    return(0);
}

#endif

