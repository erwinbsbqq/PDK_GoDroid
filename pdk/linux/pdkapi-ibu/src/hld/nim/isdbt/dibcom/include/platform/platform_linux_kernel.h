#ifndef DIBTYPES_H
#define DIBTYPES_H
#include <sys/types.h>
//#include <asm/types.h>
//#include <linux/version.h>
//#include <linux/string.h>
//#include <linux/slab.h>
//#include <linux/delay.h>
#include <basic_types.h>
#include <api/libc/string.h>
//#include <api/libc/alloc.h>
//#include <api/libfs2/types.h>
#include <osal/osal_timer.h>
#include <osal/osal_task.h>

//typedef char			INT8;
//typedef short			INT16;
//typedef long			INT32;

#if 1
#define uint32_t unsigned long
#define  int32_t long
#define uint16_t unsigned short
#define  int16_t short
#define  uint8_t unsigned char
#define   int8_t char
#endif
#define MemAlloc(size)       malloc(size)//, GFP_KERNEL)
#define	MemFree(chunk, size) free(chunk)

#define DibMemAlloc MemAlloc
#define DibMemFree  MemFree

#define DibMoveMemory ali_memmove
#define DibZeroMemory(pointer, size) ali_memset(pointer, 0, size)
#define DibSetMemory  ali_memset
// Sleep
#define DibMSleep(v) OS_TaskSleep(v)
#define DibUSleep(v) osal_delay(v)

typedef int DIB_LOCK;

#define DibInitLock(lock)
#define DibFreeLock(lock)
#define DibAcquireLock(lock)
#define DibReleaseLock(lock)
#define DibAllocateLock(lock)
#define DibDeAllocateLock(lock)


typedef int DIB_EVENT;
#define DibAllocateEvent(event)
#define DibDeAllocateEvent(event)
#define DibInitNotificationEvent(event)
#define DibSetEvent(event)
#define DibResetEvent(a)
#define DibWaitForEvent(event, timeout)

#endif
