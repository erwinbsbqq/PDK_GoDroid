#ifndef __ALI_OPEN_VG_H
#define __ALI_OPEN_VG_H

#ifdef __cplusplus
extern "C" {
#endif

#define OVG_Enable_CHK 1

#include <pthread.h>
#include <hld/bdma/ALiDMA_API.h>

class ALiBDMA_thread
{
public:
	ALiBDMA_thread();	//throws bad_alloc
	~ALiBDMA_thread();
	
	//Device for ioctl
	int flag;
	pthread_t thread_id;
	int devfd;

	bool ALiOpen_Done;
#ifdef OVG_Enable_CHK
	bool Ovg_exist;
	bool Ovg_sync;
#endif
};
#define ALiDMAGetThreadID() \
	pthread_t getthread = pthread_self();\
	int id;\
	for(id = 0; id<THREAD_NUM; id++){\
		if(threads_bdma[id].thread_id == getthread){\
			break;\
		}\
	}

#ifdef __cplusplus 
} /* extern "C" */
#endif
#endif  /* __ALI_OPEN_VG_H */
