#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <linux/sem.h>
#include <linux/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <hld_cfg.h>

#include <pthread.h>
#ifdef ADR_ALIDROID
#include <cutils/ashmem.h>
#endif
#include <time.h>

#include <hld/adr_hld_dev.h>
#include <adr_basic_types.h>
#include <hld/dsc/adr_dsc.h>

#include <osal/osal_task.h>

#include <string.h>

#include <unistd.h>

#include <errno.h>

//#include <linux/dvb/ali_dmx.h>
#include <ali_dmx_common.h>
#include <linux/Ali_DmxLibInternal.h>
#include <linux/Ali_DmxLib.h>
#include <ali_tsi_common.h>
//#include <hld/signal/adr_signal.h>


#include <sched.h>

#define ALI_DMX_LIB_SEC_SLOT_BUF_SIZE 4096

#ifdef ADR_ALIDROID
int ALI_DMX_LIB_SEC_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_SEC_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_TS_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_TS_LAYER_SHM_KEY = -1;

int ALI_DMX_LIB_VIDEO_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_AUDIO_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_PCR_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_PCR_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_TS_IN_RAM_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_SRC_CTRL_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_DESCRAM_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY = -1;


int ALI_DMX_LIB_TP_LAYER_SEM_KEY = -1;

int ALI_DMX_LIB_TP_LAYER_SHM_KEY = -1;

#else
#define ALI_DMX_LIB_SEC_LAYER_SEM_KEY (0x5111)

#define ALI_DMX_LIB_SEC_LAYER_SHM_KEY (0x5112)


#define ALI_DMX_LIB_TS_LAYER_SEM_KEY (0x5113)

#define ALI_DMX_LIB_TS_LAYER_SHM_KEY (0x5114)



#define ALI_DMX_LIB_VIDEO_LAYER_SEM_KEY (0x5115)

#define ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY (0x5116)


#define ALI_DMX_LIB_AUDIO_LAYER_SEM_KEY (0x5117)

#define ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY (0x5118)


#define ALI_DMX_LIB_PCR_LAYER_SEM_KEY (0x5119)

#define ALI_DMX_LIB_PCR_LAYER_SHM_KEY (0x5120)


#define ALI_DMX_LIB_TS_IN_RAM_LAYER_SEM_KEY (0x5121)

#define ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY (0x5122)


#define ALI_DMX_LIB_SRC_CTRL_LAYER_SEM_KEY (0x5123)

#define ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY (0x5124)


#define ALI_DMX_LIB_DESCRAM_LAYER_SEM_KEY (0x5125)

#define ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY (0x5126)


#define ALI_DMX_LIB_TP_LAYER_SEM_KEY (0x5127)

#define ALI_DMX_LIB_TP_LAYER_SHM_KEY (0x5128)
#endif

#define ALI_DMX_LIB_SEC_STREAM_CNT 512

#define ALI_DMX_LIB_SEC_SLOT_CNT 256

#define ALI_DMX_LIB_SEC_CH_CNT 128

#define ALI_DMX_LIB_SEC_FLT_CNT 128

#define ALI_DMX_LIB_TS_STREAM_CNT 96

#define ALI_DMX_LIB_STREAM_POLL_CNT 256

#define ALI_DMX_LIB_VIDEO_STREAM_CNT 1

#define ALI_DMX_LIB_AUDIO_STREAM_CNT 1

#define ALI_DMX_LIB_PCR_STREAM_CNT 1

#define ALI_DMX_LIB_TS_IN_RAM_STREAM_CNT 1

#define ALI_DMX_LIB_SRC_CTRL_STREAM_CNT 1

#define ALI_DMX_LIB_DESCRAM_KEY_STREAM_CNT 32

#define ALI_DMX_LIB_TP_STREAM_CNT 1

#define ALI_DMX_LIB_SUBT_STREAM_CNT 1

#if 0
#define DMX_LIB_DBG_PRINT(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DMX_LIB, fmt, ##args); \
			} while(0)

#else
#define ALI_DMX_LIB_PRINT(...) do{}while(0)
#endif

#define DMX_LIB_DBG_ERR(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(DMX_LIB, fmt, ##args); \
			} while(0)


struct ALi_DmxSecStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    /* ID of this sec slots.
     */
    __s32 Idx; 

    /*  Ali dmx linux kernel driver file descriptor.
     */
    __s32 Fd; 

    /* Param configed to kernel driver.
     */
    struct Ali_DmxSecStreamParam StreamParam; 

	__u32 InByteCnt;

	struct Ali_DmxLibSecStrmStatInfo StatInfo;
};



struct ALi_DmxSecSlot
{
    /* State of this section slot.
     */
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    /* ID of this sec slots.
     */
    __s32 Idx; 

    /*  section stream id this slot depends on.
     */
    __s32 SecStreamId; 

    /* Internal Buffer to store section data. 
     */
    __u8 *SecBuf;

    /* Start time of last returning from SecRcvCallback(), used for timeout
     * calculation, counted in milliscond.
     */
    __u32 StartTime;

    /* Configured by user, section masks for this section slot.
     */
    struct Ali_DmxSecSlotParam SlotParam;

    pthread_t WorkThreadId;

	__u32 InByteCnt;
	
	struct Ali_DmxLibSecSlotStatInfo StatInfo;
};


struct ALi_DmxSecCh
{
    /* State of this section slot.
     */
    enum ALI_DMX_STREAM_STATE State;

    __u32 DmxId;

    /* ID of this sec channel.
     */
    __s32 Idx; 

    /* Configured by user, channel parameters for this section channel.
     */
    struct Ali_DmxSecChParam ChParam;

	__u32 InByteCnt;

	struct Ali_DmxLibSecChStatInfo StatInfo;
};




struct ALi_DmxSecFlt
{
    /* State of this section slot.
     */
    enum ALI_DMX_STREAM_STATE State;

    /* ID of this sec filter.
     */
    __s32 Idx; 

    __s32 ChIdx;

    __s32 SecSlotId;

    /* Configured by user, section masks for this section slot.
     */
    struct Ali_DmxSecFltParam FltParam;

    __u32 InByteCnt;//!<Section slot总共收到的byte总数。
    
	struct Ali_DmxLibSecFltStatInfo StatInfo;
};




struct Ali_DmxLibSecLayer
{
    /* Semaphore ID for protecting againest concurrent accesss to dmx lib 
     * section layer.
     */
    __s32 SemId; 

    /* Share memory ID for storing section lib layer info.
     */
    __s32 ShmId; 

    /* Used to avoid semphore self-lock.
     */
    pthread_t SemLockThreadId;

    __s32 SemLockTimes;

    /* Stream.
     */
    __s32 TotalStreamCnt;

    struct ALi_DmxSecStream Streams[ALI_DMX_LIB_SEC_STREAM_CNT];

    /* Slot.
     */
    __s32 TotalSlotCnt;

    struct ALi_DmxSecSlot Slots[ALI_DMX_LIB_SEC_SLOT_CNT];

    /* Channel-filter.
     */
    __s32 TotalChCnt;

    __s32 TotalFltCnt;

    struct ALi_DmxSecCh Chs[ALI_DMX_LIB_SEC_CH_CNT];

    struct ALi_DmxSecFlt Flts[ALI_DMX_LIB_SEC_FLT_CNT];
};





struct ALi_DmxVideoStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    __u32 Idx; /* Idex in ALi_DmxSecStreamSlotArray. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxVideoStreamParam StreamParam; 

    __u32 InByteCnt;//!<VIdoe stream总共收到的byte总数.

	struct Ali_DmxLibVideoStrmStatInfo StatInfo;
};



struct Ali_DmxLibVideoLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxVideoStream Streams[ALI_DMX_LIB_VIDEO_STREAM_CNT];
};



struct ALi_DmxAudioStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    __u32 Idx; /* Idex in ALi_DmxSecStreamSlotArray. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxAudioStreamParam StreamParam; 

    __u32 InByteCnt;//!<Audio stream总共收到的byte总数.
    
	struct Ali_DmxLibAudioStrmStatInfo StatInfo;
};



struct Ali_DmxLibAudioLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxAudioStream Streams[ALI_DMX_LIB_AUDIO_STREAM_CNT];
};




struct ALi_DmxPcrStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    __u32 Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxPcrStreamParam StreamParam; 
	
    __u32 InByteCnt;//!<Video stream总共收到的byte总数.

	struct Ali_DmxLibPcrStrmStatInfo StatInfo;
};


struct Ali_DmxLibPcrLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxPcrStream Streams[ALI_DMX_LIB_PCR_STREAM_CNT];
};




struct ALi_DmxTsStream
{
    enum ALI_DMX_STREAM_STATE State;
	
	__u32 DmxId;

    __u32 Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxTsStreamParam StreamParam; 

    __u32 InByteCnt;//!<TS stream总共收到的byte总数.

	struct Ali_DmxLibTsStrmStatInfo StatInfo;
};



struct Ali_DmxLibTsLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxTsStream Streams[ALI_DMX_LIB_TS_STREAM_CNT];
};





struct ALi_DmxTpStream
{
    enum ALI_DMX_STREAM_STATE State;
	
	__u32 DmxId;

    __u32 Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxTpStreamParam StreamParam; 

    __u32 InByteCnt;//!<TS stream总共收到的byte总数.

	struct Ali_DmxLibTpStrmStatInfo StatInfo;
};



struct Ali_DmxLibTpLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxTpStream Streams[ALI_DMX_LIB_TP_STREAM_CNT];
};





struct ALi_DmxTsInRamStream
{
    enum ALI_DMX_STREAM_STATE State;
	
	__u32 DmxId;

    __u32 Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxTsInRamStreamParam StreamParam; 

    __u32 InByteCnt;//!<TS IN RAM Stream总共收到的用户写入的byte总数.

	struct Ali_DmxLibTsInRamStrmStatInfo StatInfo;
};



struct Ali_DmxLibTsInRamLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxTsInRamStream Streams[ALI_DMX_LIB_TS_IN_RAM_STREAM_CNT];
};




struct ALi_DmxSrcCtrlStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    __u32 Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/

    /* Param configed to kernel driver.
     */
    struct Ali_DmxSrcCtrlStreamParam StreamParam; 
};




struct Ali_DmxLibSrcCtlLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

    struct ALi_DmxSrcCtrlStream Streams[ALI_DMX_LIB_SRC_CTRL_STREAM_CNT];
};



struct Ali_DmxDescramKeyStream
{
    enum ALI_DMX_STREAM_STATE State;

	__u32 DmxId;

    __u32 Idx; /* Idex in struct Ali_DmxDescramKeyStream Streams. */

   __u32 CsaDrvHandler;
   	
   //__u16 Pid;

   //__u16 Reserved;

   __u32 	RefCnt;	
   
   struct CSA_KEY Key;

   struct Ali_DmxDescramKeyStreamParam Param;
};



struct Ali_DmxLibDescramLayer
{
    /* Share memory ID for storing ALi_DmxSecStreamSlot array accross
     * processes & threads.
     */
    __s32 ShmId; 

    /* Semaphore ID for protecting ALi_DmxSecStreamSlot array from concurrent 
     * access.
     */
    __s32 SemId; 

    __s32 TotalStreamCnt;

	pCSA_DEV CsaDevPtr;

    struct Ali_DmxDescramKeyStream KeyStreams[ALI_DMX_LIB_DESCRAM_KEY_STREAM_CNT];
};


struct ALi_DmxSubtStream
{
    __s32 Fd; /*  ali dmx linux kernel driver file descriptor.*/
};


struct Ali_DmxLibSubtLayer
{
    __s32 TotalStreamCnt;

    struct ALi_DmxSubtStream Streams[ALI_DMX_LIB_SUBT_STREAM_CNT];
};



// CRC32 lookup table for polynomial 0x04c11db7
__u32 Ali_DmxDvbCrcTable[256] =
{
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};





/* Worker thread for calling callbacks for section slot & channel-filter layer.
 * One thread for each process space.
 */
pthread_t Ali_DmxLibSecSlotThreadId = (pthread_t)0xFFFFFFFF;

struct Ali_DmxStreamPollInfo Ali_DmxLibSecSlotPollList[ALI_DMX_LIB_SEC_SLOT_CNT];
struct ALi_DmxSecSlot *Ali_DmxLibSecSlotList[ALI_DMX_LIB_SEC_SLOT_CNT]; 


/* Ali_DmxSecStreamLayer is a Pointer to share memroy which stores info about 
 * section streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxSecStreamLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibSecLayer *Ali_DmxLibSecLayer = NULL;


/* Ali_DmxLibVideoLayer is a Pointer to share memroy which stores info about 
 * video streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibVideoLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibVideoLayer *Ali_DmxLibVideoLayer = NULL;


/* Ali_DmxLibAudioLayer is a Pointer to share memroy which stores info about 
 * audio streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibAudioLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibAudioLayer *Ali_DmxLibAudioLayer = NULL;


/* Ali_DmxLibTsLayer is a Pointer to share memroy which stores info about 
 * audio streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibTsLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibTsLayer *Ali_DmxLibTsLayer = NULL;


/* Ali_DmxLibPcrLayer is a Pointer to share memroy which stores info about 
 * PCR streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibPcrLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibPcrLayer *Ali_DmxLibPcrLayer = NULL;


/* Ali_DmxLibTsInRamLayer is a Pointer to share memroy which stores info about 
 * PCR streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibTsInRamLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibTsInRamLayer *Ali_DmxLibTsInRamLayer = NULL;


/* Ali_DmxLibSrcCtlLayer is a Pointer to share memroy which stores info about 
 * PCR streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibSrcCtlLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibSrcCtlLayer *Ali_DmxLibSrcCtlLayer = NULL;


/* Ali_DmxLibDescramLayer is a Pointer to share memroy which stores info about 
 * PCR streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibDescramLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibDescramLayer *Ali_DmxLibDescramLayer = NULL;



/* Ali_DmxLibTpLayer is a Pointer to share memroy which stores info about 
 * audio streams, shared by all processes & threads using ali dmx lib.
 * Each processes has it's own Ali_DmxLibTpLayer pointer in it's address 
 * space. All these pointers reference to the same block of share memory.
 */
struct Ali_DmxLibTpLayer *Ali_DmxLibTpLayer = NULL;


struct Ali_DmxLibSubtLayer Ali_DmxLibSubtLayer;


__u32 Ali_DmxDvbCrc32
(
    __u8 *Data,
    __s32 Len
)
{
    __s32 Idx;
	__u32 Crc;
	__u8  TblIdx;
	
    Crc = 0xFFFFFFFF;

    for (Idx = 0; Idx < Len; Idx++)
    {
    	TblIdx = ((Crc >> 24) ^ *Data++) & 0xFF;
    	Crc = (Crc << 8) ^ Ali_DmxDvbCrcTable[TblIdx];
    }

    return(Crc);
}


__u8* Ali_DmxId2Path
(
	enum ALI_DMX_IN_OUT Direction,
    __u32               DmxId
)
{
	ALI_DMX_LIB_PRINT("%s,%d,DmxId:%d\n", __FUNCTION__, __LINE__, DmxId);

    if (ALI_DMX_OUTPUT == Direction)
    {
		switch(DmxId)
		{
		    /* ID 0 and ID 10000 map to same ALI_HWDMX0_OUTPUT_PATH.
		     * Since ID 0~2 may be not enough for CHIPs that have more than 3 HW DMXs, 
		     * so we map HW DMX start form 10000, of cuase the the old ID is still supported.
			*/
			case 0:
			case 10000:
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(ALI_HWDMX0_OUTPUT_PATH);
			}
			break;
		
			case 1:
			case 10001:				
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(ALI_HWDMX1_OUTPUT_PATH);
			}
			break;
		
			case 2:
			case 10002:						
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(ALI_HWDMX2_OUTPUT_PATH);
			}
			break;

			case 10003:						
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(ALI_HWDMX3_OUTPUT_PATH);
			}
			break;			

		    /* ID 3 and ID 20000 map to same ALI_SWDMX0_OUTPUT_PATH.
		     * Since ID 0~2 may be not enough for CHIPs that have more than 3 HW DMXs, 
		     * so we map SW DMX start form 10000, of cuase the the old ID is still supported.
			*/		
			case 3:
			case 20000:				
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(ALI_SWDMX0_OUTPUT_PATH);
			}
			break;
		
			default:
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
				return(NULL);
			}
		} 

        return(NULL);
	}
	else 
	{
    	switch(DmxId)
    	{
    		case 3:
			case 20000:					
    		{
    			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	
    			return(ALI_SWDMX0_INPUT_PATH);
    		}
    		break;
    	
    		case 4:
    		{
    			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	
    			return(ALI_SWDMX1_INPUT_PATH);
    		}
    		break;
    	
    		default:
    		{
    			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	
    			return(NULL);
    		}
    	} 
    	
    	return(NULL);
	}

 	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
 
    return(NULL);
}




__s32 Ali_StreamId2Idx
(
    enum ALI_DMX_STREAM_TYPE StreamType,
    __s32                    StreamId
)
{
    __s32 StreamIdx;

	//ALI_DMX_LIB_PRINT("%s,%d,StreamType:%x,StreamId:%x\n",
		                //__FUNCTION__, __LINE__, StreamType, StreamId);

    switch(StreamType)
    {
        case ALI_DMX_STREAM_TYPE_SEC:
		{
            if ((StreamId < ALI_DMX_STREAM_ID_SEC_START) || 
				(StreamId >= ALI_DMX_STREAM_ID_SEC_END))
            {
                StreamIdx = ALI_ERR_DMX_SEC_STREAM_INVALID_ID;
			}
			else
			{
                StreamIdx = StreamId - ALI_DMX_STREAM_ID_SEC_START;
			}
		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_SLOT:
		{
            if ((StreamId < ALI_DMX_STREAM_ID_SEC_SLOT_START) || 
				(StreamId >= ALI_DMX_STREAM_ID_SEC_SLOT_END))
            {
                StreamIdx = ALI_ERR_DMX_INVALID_ID;
			}
			else
			{
                StreamIdx = StreamId - ALI_DMX_STREAM_ID_SEC_SLOT_START;
			}
		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_CH:
		{
            if ((StreamId < ALI_DMX_STREAM_ID_SEC_CH_START) || 
				(StreamId >= ALI_DMX_STREAM_ID_SEC_CH_END))
            {
                StreamIdx = ALI_ERR_DMX_INVALID_ID;
			}
			else
			{
                StreamIdx = StreamId - ALI_DMX_STREAM_ID_SEC_CH_START;
			}
		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_FLT:
		{
            if ((StreamId < ALI_DMX_STREAM_ID_SEC_FLT_START) || 
				(StreamId >= ALI_DMX_STREAM_ID_SEC_FLT_END))
            {
                StreamIdx = ALI_ERR_DMX_INVALID_ID;
			}
			else
			{
                StreamIdx = StreamId - ALI_DMX_STREAM_ID_SEC_FLT_START;
			}
		}
		break;

    	case ALI_DMX_STREAM_TYPE_TS:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_TS_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_TS_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_TS_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_PES:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_PES_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_PES_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_PES_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_VIDEO:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_VIDEO_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_VIDEO_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_VIDEO_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_AUDIO:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_AUDIO_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_AUDIO_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_AUDIO_START;
    		}
    	}
    	break;
		
    	case ALI_DMX_STREAM_TYPE_PCR:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_PCR_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_PCR_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_PCR_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_TS_IN_RAM:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_IN_TS_RAM_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_IN_TS_RAM_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_IN_TS_RAM_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_SRC_CTRL:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_SRC_CTRL_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_SRC_CTRL_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_SRC_CTRL_START;
    		}
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_DESCRAM_KEY:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_DESCRAM_KEY_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_DESCRAM_KEY_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_DESCRAM_KEY_START;
    		}
    	}
    	break;		

    	case ALI_DMX_STREAM_TYPE_TP:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_TP_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_TP_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_TP_START;
    		}
    	}
    	break;	

    	case ALI_DMX_STREAM_TYPE_SUBT:
    	{
    		if ((StreamId < ALI_DMX_STREAM_ID_SUBT_START) || 
    			(StreamId >= ALI_DMX_STREAM_ID_SUBT_END))
    		{
    			StreamIdx = ALI_ERR_DMX_INVALID_ID;
    		}
    		else
    		{
    			StreamIdx = StreamId - ALI_DMX_STREAM_ID_SUBT_START;
    		}
    	}
    	break;		

        default:
		{
            StreamIdx = ALI_ERR_DMX_INVALID_ID;
		}
		break;

	}

	//ALI_DMX_LIB_PRINT("%s,%d,StreamType:%x,StreamId:%x,StreamIdx:%x\n",
		                //__FUNCTION__, __LINE__, StreamType, StreamId, StreamIdx);

    return(StreamIdx);
}




__s32 Ali_StreamIdx2Id
(
    enum ALI_DMX_STREAM_TYPE StreamType,
    __s32                    StreamIdx
)
{
    __s32 StreamId;

	//ALI_DMX_LIB_PRINT("%s,%d,StreamType:%x,StreamIdx:%x\n",
		                //__FUNCTION__, __LINE__, StreamType, StreamIdx);

    switch(StreamType)
    {
        case ALI_DMX_STREAM_TYPE_SEC:
		{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SEC_START;
		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_SLOT:
		{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SEC_SLOT_START;

		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_CH:
		{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SEC_CH_START;

		}
		break;

        case ALI_DMX_STREAM_TYPE_SEC_FLT:
		{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SEC_FLT_START;
		}
		break;

	
    	case ALI_DMX_STREAM_TYPE_TS:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_TS_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_PES:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_PES_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_VIDEO:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_VIDEO_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_AUDIO:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_AUDIO_START;
    	}
    	break;
		
    	case ALI_DMX_STREAM_TYPE_PCR:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_PCR_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_TS_IN_RAM:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_IN_TS_RAM_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_SRC_CTRL:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SRC_CTRL_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_DESCRAM_KEY:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_DESCRAM_KEY_START;
    	}
    	break;		

    	case ALI_DMX_STREAM_TYPE_TP:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_TP_START;
    	}
    	break;

    	case ALI_DMX_STREAM_TYPE_SUBT:
    	{
            StreamId = StreamIdx + ALI_DMX_STREAM_ID_SUBT_START;
    	}
    	break;		

        default:
		{
            StreamId = ALI_ERR_DMX_INVALID_ID;
		}
		break;
	}

	//ALI_DMX_LIB_PRINT("%s,%d,StreamType:%x,StreamIdx:%x,StreamId:%x\n",
		                //__FUNCTION__, __LINE__, StreamType, StreamIdx, StreamId);

    return(StreamId);
}




__s32 Ali_StreamId2Type
(
    __s32 StreamId
)
{
    //ALI_DMX_LIB_PRINT("%s,%d,StreamId:%x\n", __FUNCTION__, __LINE__, StreamId);
	
	if ((StreamId >= ALI_DMX_STREAM_ID_SEC_START) &&
		(StreamId < ALI_DMX_STREAM_ID_SEC_END))
	{
		return(ALI_DMX_STREAM_TYPE_SEC);
	}

    if ((StreamId >= ALI_DMX_STREAM_ID_SEC_SLOT_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_SEC_SLOT_END))
    {
    	return(ALI_DMX_STREAM_TYPE_SEC_SLOT);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_SEC_CH_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_SEC_CH_END))
    {
    	return(ALI_DMX_STREAM_TYPE_SEC_CH);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_TS_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_TS_END))
    {
    	return(ALI_DMX_STREAM_TYPE_TS);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_SEC_FLT_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_SEC_FLT_END))
    {
    	return(ALI_DMX_STREAM_TYPE_SEC_FLT);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_VIDEO_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_VIDEO_END))
    {
    	return(ALI_DMX_STREAM_TYPE_VIDEO);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_AUDIO_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_AUDIO_END))
    {
    	return(ALI_DMX_STREAM_TYPE_AUDIO);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_PCR_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_PCR_END))
    {
    	return(ALI_DMX_STREAM_TYPE_PCR);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_IN_TS_RAM_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_IN_TS_RAM_END))
    {
    	return(ALI_DMX_STREAM_TYPE_TS_IN_RAM);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_SRC_CTRL_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_SRC_CTRL_END))
    {
    	return(ALI_DMX_STREAM_TYPE_SRC_CTRL);
    }

    if ((StreamId >= ALI_DMX_STREAM_ID_TP_START) &&
    	(StreamId < ALI_DMX_STREAM_ID_TP_END))
    {
    	return(ALI_DMX_STREAM_TYPE_TP);
    }

    return(ALI_ERR_DMX_INVALID_ID);
}






__s32 Ali_DmxSemLock
(
    __s32 SemId
)
{
	#ifdef ADR_ALIDROID
		return OS_AcquireSemaphoreTimeOut(SemId, OSAL_WAIT_FOREVER_TIME);
	#else
    __s32         Ret;
    struct sembuf SemBuf;

    SemBuf.sem_num = 0;
    SemBuf.sem_op = -1; 
    SemBuf.sem_flg = SEM_UNDO;

    //ALI_DMX_LIB_PRINT("L,%d\n", SemId);

    Ret = semop(SemId, &SemBuf, 1);

	if(0 != Ret)
	{
		ALI_DMX_LIB_PRINT("\n %s(): L[%d], SemId[0x%x], Ret[%d] \n", \
		                      __FUNCTION__, __LINE__, SemId, Ret);
    }


    return(Ret);
    #endif    
}



__s32 Ali_DmxSemUnlock
(
    __s32 SemId
)
{
	#ifdef ADR_ALIDROID
		return OS_FreeSemaphore(SemId);
	#else

    __s32         Ret;
    struct sembuf SemBuf;

    SemBuf.sem_num = 0;
    SemBuf.sem_op = 1; 
    SemBuf.sem_flg = SEM_UNDO;

    //ALI_DMX_LIB_PRINT("U,%d\n", SemId);

    Ret = semop(SemId, &SemBuf, 1);

	if(0 != Ret)
	{
		ALI_DMX_LIB_PRINT("\n %s(): L[%d], SemId[0x%x], Ret[%d] \n", \
		                      __FUNCTION__, __LINE__, SemId, Ret);
    }


    return(Ret);
#endif
}




__s32 Ali_DmxSecSemCondLock
(
    void
)
{
    __s32                      Ret;
    pthread_t                  CallerThreadId;
    struct Ali_DmxLibSecLayer *Layer;

    if (NULL == Ali_DmxLibSecLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_OK);
    }

    Ret = 0;

    Layer = Ali_DmxLibSecLayer;

    CallerThreadId = pthread_self();

    //ALI_DMX_LIB_PRINT("%s,%d,CallerThreadId:%x,SemLockThreadId:%x\n", __FUNCTION__, __LINE__, CallerThreadId, Layer->SemLockThreadId);

    /* Lock semphore only if the caller IS NOT the thread who has already
     * locked the semphore to avoid self-dead-lock.
     * return value of pthread_equal():
     * The pthread_equal() function shall return a non-zero value if t1 and t2
     * are equal; otherwise, zero shall be returned.
     * If either t1 or t2 are not valid thread IDs, the behavior is undefined.
     * The pthread_equal() function shall return a non-zero value if t1 and t2
     * are equal; otherwise, zero shall be returned.
     * If either t1 or t2 are not valid thread IDs, the behavior is undefined.
     */
    if (0 == pthread_equal(Layer->SemLockThreadId, CallerThreadId))
    {
        Ret = Ali_DmxSemLock(Layer->SemId);

        Layer->SemLockThreadId = CallerThreadId;

        Layer->SemLockTimes = 1;
    }
    else
    {
        Layer->SemLockTimes++;
    }

    return(Ret);
}




__s32 Ali_DmxSecSemCondUnlock
(
    void
)
{
    __s32                      Ret;
    pthread_t                  CallerThreadId;
    struct Ali_DmxLibSecLayer *Layer;

    if (NULL == Ali_DmxLibSecLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_OK);
    }

    Ret = 0;

    Layer = Ali_DmxLibSecLayer;

    CallerThreadId = pthread_self();

    //ALI_DMX_LIB_PRINT("%s,%d,CallerThreadId:%x,SemLockThreadId:%x\n", __FUNCTION__, __LINE__, CallerThreadId, Layer->SemLockThreadId);

    /* Unock semphore only if the caller IS the thread who has already
     * locked the semphore to avoid self-dead-lock.
     * return value of pthread_equal():
     * The pthread_equal() function shall return a non-zero value if t1 and t2
     * are equal; otherwise, zero shall be returned.
     * If either t1 or t2 are not valid thread IDs, the behavior is undefined.
     * The pthread_equal() function shall return a non-zero value if t1 and t2
     * are equal; otherwise, zero shall be returned.
     * If either t1 or t2 are not valid thread IDs, the behavior is undefined.
     */
    if (0 != pthread_equal(Layer->SemLockThreadId, CallerThreadId))
    {
        if (Layer->SemLockTimes < 1)
        {
            ALI_DMX_LIB_PRINT("%s,%d,SemLockTimes:%d\n", __FUNCTION__, __LINE__, 
                   Layer->SemLockTimes);
        }
        else if (1 == Layer->SemLockTimes)
        {
            Layer->SemLockThreadId = (pthread_t)0xFFFFFFFF;

            Ret = Ali_DmxSemUnlock(Layer->SemId);
        }
        else
        {
            Layer->SemLockTimes--;
        }
    }

    return(Ret);
}

#ifdef ADR_ALIDROID
__u8* Ali_DmxAShmGet(int *ShmId, __s32 Size)
{    
	__u8  *Addr = NULL;
    unsigned int pagesize = sysconf(_SC_PAGE_SIZE); 
    unsigned int ashmem_size = ((Size + pagesize-1) & ~(pagesize-1)); 
	

	if (-1 == *ShmId)
	{
		*ShmId = ashmem_create_region(NULL, ashmem_size);	    
	    if (*ShmId < 0)
	    {	       
	        DMX_LIB_DBG_ERR("[%s,%d], error\n", __FUNCTION__, __LINE__);        

	        return(-1);
	    }

	    Addr = mmap(NULL, ashmem_size, PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_SHARED, *ShmId, 0);    
	    if ((void *)-1 == Addr) 
	    {
	        DMX_LIB_DBG_ERR("[%s,%d], error\n", __FUNCTION__, __LINE__);        
	        close(*ShmId);        
	    } 
    }
    else
    {    	
		ashmem_size = ashmem_get_size_region(*ShmId);
 
	
		Addr = mmap(NULL, ashmem_size, PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_SHARED, *ShmId, 0);

		
	
		if ((void *)-1 == Addr)	
		{
			DMX_LIB_DBG_ERR("[ %s %d ], error\n", __FUNCTION__, __LINE__);
			close(*ShmId); 
			return -1;
		}
    }
	

    return(Addr);
}
#else
__u8* Ali_DmxShmGet
(
    key_t key,
    __s32 Size
)
{
    __s32  ShmId;
    __u8  *Addr;

    ShmId = shmget(key, Size, 0);

    if (ShmId < 0)
    {
        /* Make sure that our section stream layer only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, ShmId);

        return((__u8 *)ShmId);
    }

    Addr = (void *)(shmat(ShmId, (void *)0, 0));

    if ((void *)-1 == Addr) 
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    } 

    return(Addr);
}
#endif

/* Return Counted in milliseconds. 
 */
__s32 Ali_DmxGetTime
(
    void
)
{
    __u32 Time;

    struct timespec TimeSpec;
    
    clock_gettime(CLOCK_REALTIME, &TimeSpec);

    Time = (TimeSpec.tv_sec * 1000) + (TimeSpec.tv_nsec / 1000000);

    return(Time);
}


/* Change linux error code to DMX error code. 
 */
__s32 Ali_DmxErrCode
(
    __s32 LinuxErrCode
)
{
    /* TODO: Need mor work.
     */
    __s32 AliDmxErrCode;

    AliDmxErrCode = LinuxErrCode;

    return(AliDmxErrCode);
}





/* Ali linux dmx lib. */
__s32 Ali_DmxSecStreamOpen
(
    __u32 DmxId 
)
{
    __u8                    *DmxPath;
    __s32                    ShmId;
    struct ALi_DmxSecStream *Stream;
    __s32                    Idx;
    __s32                    Fd;
    __s32                    Ret;
    __u32                    ShmSize;
    __u8                    *Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

	if (NULL == DmxPath)
	{
		//Stream->StatInfo.InvPathCnt++;
	
        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}
	
    /* If first open in current address space.
     */
    if (NULL == Ali_DmxLibSecLayer)
    {
        ShmSize = sizeof(struct Ali_DmxLibSecLayer);

       	#ifdef ADR_ALIDROID
       	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
       	#else       	
        Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
        #endif
    
        /* Fetal error. 
         */
        if ((void *)-1 == Addr) 
        {
            ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer);

            //Ali_DmxLibSecLayerExit();

            return(ALI_ERR_DMX_NOT_INITILAIZED);
        } 

        Ali_DmxLibSecLayer = (struct Ali_DmxLibSecLayer *)Addr;
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    //ALI_DMX_LIB_PRINT("TotalStreamCnt:%d\n", Ali_DmxSecStreamLayer->TotalStreamCnt);

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalStreamCnt; Idx++)
    { 
        //ALI_DMX_LIB_PRINT("status[%d]:%d\n", Idx, Streams[Idx].status);

        Stream = &(Ali_DmxLibSecLayer->Streams[Idx]);

        if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibSecLayer->TotalStreamCnt)
    {
        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_STREAM_EXHAUST);
    }

	memset(&Stream->StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream->StatInfo.IoOpenFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream->Fd = Fd;

	Stream->DmxId = DmxId;

	Stream->InByteCnt = 0;

    Stream->State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC, Idx));
}




__s32 Ali_DmxSecStreamCfg  
(
    __s32                         StreamId, 
    struct Ali_DmxSecStreamParam *StreamParam
)
{ 
    __s32                    Ret;   
    struct ALi_DmxSecStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if ((StreamParam->NeedDiscramble != 0) && 
		(StreamParam->NeedDiscramble != 1))
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->SecMask.MatchLen > ALI_DMX_SEC_MATCH_MAX_LEN)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->Pid > 0x1FFF)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();        

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxSecStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_SEC_STREAM_CFG, &(Stream->StreamParam));  

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d,err:%m\n", __FUNCTION__, __LINE__, errno);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSecSemCondUnlock();  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxSecStreamStart
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
    struct ALi_DmxSecStream *Streams;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();   
		
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);   

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_SEC_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;

        Ali_DmxSecSemCondUnlock();  
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSecSemCondUnlock();  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxSecStreamStop
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
    struct ALi_DmxSecStream *Streams;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    { 
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();        

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_SEC_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;

        Ali_DmxSecSemCondUnlock();  
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxSecStreamClose
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }
	
    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();      

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }


    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);

    Ret = close(Stream->Fd); 

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	} 

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSecSemCondUnlock();    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxSecStreamRead
(
    __s32  StreamId, 
    __u8  *Buf, 
    __u32  BufLen
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
	__s32                    StreamIdx;
	__u32                    Crc;

    //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

#if 0
    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 
#endif

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();   

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    Ret = read(Stream->Fd, Buf, BufLen);

#if 0
    Ali_DmxSecSemCondUnlock(); 
#endif

    if (Ret > 0)
    {
        Stream->InByteCnt += Ret;

		/* Need CRC?
		*/
        if (Stream->StreamParam.SecMask.Flags & ALI_DMX_SEC_MASK_FLAG_CRC_EN)
        {
			Crc = Ali_DmxDvbCrc32(Buf, Ret);
			
			ALI_DMX_LIB_PRINT("%s,%d,pid:%d,len:%d,crc:%x\n",
				              __FUNCTION__, __LINE__, Stream->StreamParam.Pid,
				              Ret, Crc);
			
			if (Crc != 0)
			{
				/* TODO: error statistics.
				*/
				Stream->StatInfo.CrcErrCnt++;

				return(ALI_ERR_DMX_SEC_STREAM_CRC_FAILED);
			}
		}
	}

    //ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

	return(Ali_DmxErrCode(Ret));
}


__s32 Ali_DmxSecStreamInfoGet
(
    __s32                        StreamId, 
	struct Ali_DmxSecStreamInfo *SecStreamInfo
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecStreamInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

	memset(SecStreamInfo, 0, sizeof(struct Ali_DmxSecStreamInfo));

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);
	
	SecStreamInfo->State = Stream->State;

	SecStreamInfo->DmxId = Stream->DmxId;

	SecStreamInfo->Idx = Stream->Idx;
	
    SecStreamInfo->Fd = Stream->Fd;

    SecStreamInfo->InByteCnt = Stream->InByteCnt;

    memcpy(&SecStreamInfo->StreamParam, &Stream->StreamParam,
		   sizeof(struct Ali_DmxSecStreamParam));

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecStreamErrStatGet
(
    __s32                             StreamId, 
	struct Ali_DmxLibSecStrmStatInfo *SecStrmStatInfo
)
{
    __s32                    Ret;
    struct ALi_DmxSecStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == SecStrmStatInfo)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC, StreamId);

    if ((StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Stream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

    memcpy(SecStrmStatInfo, &Stream->StatInfo,
		   sizeof(struct Ali_DmxLibSecStrmStatInfo));

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


void* Ali_DmxLibSecSlotThread
(
    void *Para
)
{
    __s32                            Ret;
    __u32                            Idx;
    __u32                            SlotIdx;
    __u32                            ListIdx;
    __s32                            SecLen;
    __u32                            CurTime;
    __s32                            ErrCode;
    __s32                            ShmId;
    struct Ali_DmxLibSecLayer       *Layer;
    struct ALi_DmxSecSlot           *Slot;
#if 0
    struct Ali_DmxSecStreamPollInfo  PollList[ALI_DMX_LIB_SEC_SLOT_CNT];
    struct ALi_DmxSecSlot           *SlotList[ALI_DMX_LIB_SEC_SLOT_CNT]; 
#else
    struct Ali_DmxStreamPollInfo  *PollList;
    struct ALi_DmxSecSlot		 **SlotList; 
    
    PollList = Ali_DmxLibSecSlotPollList;
    
    SlotList = Ali_DmxLibSecSlotList;
#endif

	Layer = Ali_DmxLibSecLayer;

    for (;;)
    {
        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        
        //Ret = Ali_DmxSemLock(Layer->SemId);
        Ret = Ali_DmxSecSemCondLock();

        if (Ret < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
            return((void*)Ret);
        } 

        CurTime = Ali_DmxGetTime();

        Idx = 0;

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        /* Setup poll list.
         */
        for (SlotIdx = 0; SlotIdx < ALI_DMX_LIB_SEC_SLOT_CNT; SlotIdx++)
        {
            Slot = &(Layer->Slots[SlotIdx]);

            /* Multi-proccess safe, only call callback funtions which belongs
             * to current process address space.
             * In the Linux threading implementations, thread IDs are unique
             * across processes. However, this is not necessarily the case on
             * other implementations, and SUSv3 explicitly notes that an 
             * application can not portably use a thread ID to identify a 
             * thread in another process. 
             */
            if ((ALI_DMX_STREAM_STATE_RUN == Slot->State) &&
                (0 != pthread_equal(Slot->WorkThreadId, 
                 Ali_DmxLibSecSlotThreadId)))
            {
                PollList[Idx].StreamId = Slot->SecStreamId;  

                SlotList[Idx] = Slot;

                Idx++;

                //ALI_DMX_LIB_PRINT("%s,%d,Idx:%d\n", __FUNCTION__, __LINE__, Idx);

                /* Timeout, inform user. 
                 */
                if (CurTime - Slot->StartTime > Slot->SlotParam.Timeout)
                {
                    Ali_DmxSecSemCondUnlock();
					
                    Slot->SlotParam.SecRcvCallback(Slot->SlotParam.CbParam,
                                         ALI_ERR_DMX_SEC_STREAM_TIMEOUT, 
                                         NULL, 
                                         0); 
					
                    Ret = Ali_DmxSecSemCondLock();
					
                    if (Ret < 0)
                    {
                        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
                
                        return((void*)Ret);
                    }
					
                    Slot->StartTime = CurTime;

                    Slot->StatInfo.CbTimeOutCnt++;
                }
            }
        }

        Ali_DmxSecSemCondUnlock();
        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
 
        /* Sleep and re-check if no slot is running.
         */
        if (0 == Idx)
        {
            //Ali_DmxSecSemCondUnlock();

            /* Sleep 50 miliseconds. 
             */
            usleep(50 * 1000);

            //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            continue;
        }

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        /* Poll for incoming sections.
         */
        //Ret = Ali_DmxSecStreamPoll(PollList, Idx, 50);
        Ret = Ali_DmxStreamPoll(PollList, Idx, 50);

        if (0 == Ret)
        {
            /*Ret == 0: timeout fro current poll, do nothing.
             */
            //Ali_DmxSecSemCondUnlock();

            continue;
        }        

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        if (Ret < 0)
        {
            /*Ret < 0: Error, Should never happen in normal situations.
             */
            //Ali_DmxSecSemCondUnlock();

            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            continue;
        }

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        Ret = Ali_DmxSecSemCondLock();
		
        if (Ret < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
            return((void*)Ret);
        } 

        CurTime = Ali_DmxGetTime();

        /* Else Ret > 0, there must be something happened for this poll. 
         * dispatch incoming sections accordingly.
         */
        for (ListIdx = 0; ListIdx < Idx; ListIdx++)
        {
            ErrCode = 0;

            /* 1 means there is data for FD then it could be read.(POLLIN)
			*/
            if (1 == PollList[ListIdx].StreamStatus)
            {
                Slot = SlotList[ListIdx];

                SecLen = Ali_DmxSecStreamRead(Slot->SecStreamId, Slot->SecBuf, 
                                              ALI_DMX_LIB_SEC_SLOT_BUF_SIZE);

                /* Send out incoming sections only if there is no error
                 * happened.
                 */
                if (SecLen > 0)
                {
                    Ali_DmxSecSemCondUnlock();
					
                    Slot->SlotParam.SecRcvCallback(Slot->SlotParam.CbParam, 
                                                   ErrCode, Slot->SecBuf,
                                                   SecLen);
					
                    Ret = Ali_DmxSecSemCondLock();
					
                    if (Ret < 0)
                    {
                        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
                
                        return((void*)Ret);
                    } 
					
					Slot->InByteCnt += SecLen;

                    Slot->StatInfo.CallbackCnt++;
                }
                else
                {
                    /* TODO: error statistics.
                     */
                    Slot->StatInfo.CallbackErrCnt++;

                    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
                }

                /* Reset timeout base.
                 */
                Slot->StartTime = CurTime;
            }
        }

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        //Ali_DmxSemUnlock(Layer->SemId);
        Ali_DmxSecSemCondUnlock();

        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }

    return((void*)Ret);
}









__s32 Ali_DmxSecSlotOpen
(
    __u32 DmxId 
)
{
    __s32                  Ret;
    __s32                  Idx;
    __s32                  SecStreamId;
    struct ALi_DmxSecSlot *Slot;
    __u32                  ShmSize;
    __u8                  *Addr;
    pthread_attr_t         ThreadAttr;
    __s32                  ThreadPolicy;
    struct sched_param     ThreadParam;
    __s32                  rr_max_priority;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    if (NULL == Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }		

    /* If first open in current address space.
     */
    if (NULL == Ali_DmxLibSecLayer)
    {
        ShmSize = sizeof(struct Ali_DmxLibSecLayer);

        #ifdef ADR_ALIDROID
       	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
       	#else     
        Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
    	#endif
        /* Fetal error. 
         */
        if ((void *)-1 == Addr) 
        {
            ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer);

            //Ali_DmxLibSecLayerExit();

            return(ALI_ERR_DMX_NOT_INITILAIZED);
        } 

        Ali_DmxLibSecLayer = (struct Ali_DmxLibSecLayer *)Addr;
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }  

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalSlotCnt; Idx++)
    {
        Slot = &(Ali_DmxLibSecLayer->Slots[Idx]);

        if (ALI_DMX_STREAM_STATE_IDLE == Slot->State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibSecLayer->TotalSlotCnt)
    {
        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_SLOT_EXHAUST);
    }
	
	memset(&Slot->StatInfo, 0, sizeof(Slot->StatInfo));

    Slot->SecBuf = malloc(ALI_DMX_LIB_SEC_SLOT_BUF_SIZE);

    if (NULL == Slot->SecBuf)
    {
		Slot->StatInfo.NoSecBufCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_SLOT_NO_BUF);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Ali_DmxLibSecSlotThreadId:%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecSlotThreadId);

    /* If section work thread in current process address space has not yet 
     * been created, create it.
     */
    if (0 != pthread_equal(Ali_DmxLibSecSlotThreadId, (pthread_t)0xFFFFFFFF))
    {
        ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecSlotThreadId);

        /* create callback thread.
         */
        pthread_attr_init(&ThreadAttr);	
			
        pthread_attr_getschedparam(&ThreadAttr, &ThreadParam);
		
        Ret = pthread_attr_setschedpolicy(&ThreadAttr, SCHED_RR);
		
        if(Ret != 0)
        {
            DMX_LIB_DBG_ERR("Unable to set SCHED_RR policy.\n");
		}
        else
		{
            rr_max_priority = sched_get_priority_max(SCHED_RR);
			
            if(rr_max_priority == -1)
            {
                DMX_LIB_DBG_ERR("Get SCHED_RR max priority fail!\n");
			}
			
            ThreadParam.sched_priority = rr_max_priority;
			
            pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
			
            ALI_DMX_LIB_PRINT("Creating thread at RR/%d\n", ThreadParam.sched_priority);
			
			#ifndef ADR_ALIDROID			
            pthread_attr_setinheritsched(&ThreadAttr, PTHREAD_EXPLICIT_SCHED);
            #endif
        }
		
        Ret = pthread_create(&Ali_DmxLibSecSlotThreadId, NULL, 
                             Ali_DmxLibSecSlotThread, NULL);
    
        if (Ret != 0)
        {
			Slot->StatInfo.ThreadCreatErrCnt++;

            Ali_DmxSecSemCondUnlock();

            pthread_attr_destroy(&ThreadAttr);

            ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer);
    
            return(ALI_ERR_DMX_PTHREAD_CREATE_FAIL);
        }

        pthread_attr_destroy(&ThreadAttr);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    SecStreamId = Ali_DmxSecStreamOpen(DmxId);

    if (SecStreamId < 0)
    {
		Slot->StatInfo.StrmOpenFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(SecStreamId);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	Slot->DmxId = DmxId;

    Slot->SecStreamId = SecStreamId;

	Slot->InByteCnt = 0;

    Slot->State = ALI_DMX_STREAM_STATE_CFG;

    Slot->WorkThreadId = Ali_DmxLibSecSlotThreadId;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d,Idx:%d\n", __FUNCTION__, __LINE__, Idx);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_SLOT, Idx));
}



__s32 Ali_DmxSecSlotCfg  
(
    __s32                       SlotId, 
    struct Ali_DmxSecSlotParam *SecSlotParam
)
{
    __s32                         Ret;   
    struct ALi_DmxSecSlot        *Slot;
    struct Ali_DmxSecStreamParam  StreamParam;
	__s32                         SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecSlotParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (NULL == SecSlotParam->SecRcvCallback)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (SecSlotParam->SecMask.MatchLen > ALI_DMX_SEC_MATCH_MAX_LEN)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PARAM);
	}
	
	if (SecSlotParam->Pid > 0x1FFF)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_SLOT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);

    if ((Slot->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Slot->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Slot->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();       

        return(ALI_ERR_DMX_SEC_SLOT_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Slot->SlotParam), SecSlotParam,
           sizeof(struct Ali_DmxSecSlotParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	memset(&StreamParam, 0, sizeof(struct Ali_DmxSecStreamParam));

    StreamParam.Pid = Slot->SlotParam.Pid;

	StreamParam.NeedDiscramble = Slot->SlotParam.NeedDiscramble;

    memcpy(&(StreamParam.SecMask), &(Slot->SlotParam.SecMask), 
           sizeof(struct Ali_DmxSecMaskInfo));

    Ret = Ali_DmxSecStreamCfg(Slot->SecStreamId, &(StreamParam));

    if (Ret < 0)
    {
		Slot->StatInfo.StrmCfgFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }   

    Slot->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxSecSlotStart
(
    __s32 SlotId
)
{
    __s32                  Ret;
    struct ALi_DmxSecSlot *Slot;
	__s32                  SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_SLOT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);

    if (Slot->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Slot->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();   

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);    

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = Ali_DmxSecStreamStart(Slot->SecStreamId);

    if (Ret < 0)
    {
		Slot->StatInfo.StrmStartFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }   

	Slot->StartTime = Ali_DmxGetTime();

    Slot->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxSecSlotStop
(
    __s32 SlotId
)
{
    __s32                  Ret;
    struct ALi_DmxSecSlot *Slot;
	__s32                  SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_SLOT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);

    if (Slot->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Slot->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();   

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);    

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = Ali_DmxSecStreamStop(Slot->SecStreamId);

    if (Ret < 0)
    {
		Slot->StatInfo.StrmStopFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }   

    Slot->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxSecSlotClose
(
    __s32 SlotId
)
{
    __s32                  Ret;
    struct ALi_DmxSecSlot *Slot;
	__s32                  SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_SLOT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Slot->State)
    {
		Slot->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();   

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);    

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = Ali_DmxSecStreamClose(Slot->SecStreamId);

    if (Ret < 0)
    {
		Slot->StatInfo.StrmCloseFailCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }   

    free(Slot->SecBuf);

    Slot->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecSlotInfoGet
(
    __s32                      SlotId, 
	struct Ali_DmxSecSlotInfo *SecSlotInfo
)
{
    __s32                  Ret;
    struct ALi_DmxSecSlot *Slot;
	__s32                  SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecSlotInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);
	
	memset(SecSlotInfo, 0, sizeof(struct Ali_DmxSecSlotInfo));

    SecSlotInfo->State = Slot->State;
	
	SecSlotInfo->DmxId = Slot->DmxId;//!<Section slot对应的DMX的ID。

    SecSlotInfo->Idx = Slot->Idx;//!<Section slot在内部数组中的INDX。

    SecSlotInfo->SecStreamId = Slot->SecStreamId;//!<Section slot对应的secton stream文件描述符。 

    SecSlotInfo->SecBuf = Slot->SecBuf;//!<Section slot内部使用的缓存buffer的地址。

    SecSlotInfo->StartTime = Slot->StartTime;//!<Section slot最后一次从callback返回时的时间。

	SecSlotInfo->InByteCnt = Slot->InByteCnt;

    memcpy(&SecSlotInfo->SlotParam, &Slot->SlotParam,
		   sizeof(struct Ali_DmxSecSlotParam));

    //pthread_t WorkThreadId;//!<ALI DMX lib内部用以调用callback函数的thread的ID。 

	SecSlotInfo->InByteCnt = Slot->InByteCnt;//!<Section slot总共收到的byte总数。	

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecSlotErrStatGet
(
    __s32                             SlotId, 
	struct Ali_DmxLibSecSlotStatInfo *SecSlotStatInfo
)
{
    __s32                  Ret;
    struct ALi_DmxSecSlot *Slot;
	__s32                  SlotIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecSlotStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    SlotIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_SLOT, SlotId);

    if ((SlotIdx >= Ali_DmxLibSecLayer->TotalSlotCnt) || (SlotIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Slot = &(Ali_DmxLibSecLayer->Slots[SlotIdx]);

    memcpy(SecSlotStatInfo, &Slot->StatInfo,
		   sizeof(struct Ali_DmxLibSecSlotStatInfo));

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_SecFltRcvCallback
(
    __u32  CbParam,
    __s32  ErrCode,
    __u8  *SecBuf,
    __u32  SecLen
)
{
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;

    /* CbParam used as a filter ID.
     */
    Flt = &(Ali_DmxLibSecLayer->Flts[CbParam]);
    
    Ch = &(Ali_DmxLibSecLayer->Chs[Flt->ChIdx]);

    if (Ch->State != ALI_DMX_STREAM_STATE_RUN)
    {
    	Ch->StatInfo.StatErrCnt++;
		
        return(ALI_ERR_DMX_SEC_FLT_OPER_DENIED);
    }   

    Flt->FltParam.SecRcvCallback(Flt->FltParam.CbParam, ErrCode, SecBuf, 
                                 SecLen);

	Flt->InByteCnt += SecLen;

	Ch->InByteCnt += SecLen;

	Flt->StatInfo.CallbackCnt++;

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxSecFltOpen
(
    __s32 ChId 
)
{ 
    __s32                 Ret;   
    __s32                 Idx;   
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;
	__s32                 ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalChCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Check if the channel this filter belongs to is ready for this operation.
     */
    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);
#if 0
    if ((Ch->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Ch->State != ALI_DMX_STREAM_STATE_RUN))
#else
    if (ALI_DMX_STREAM_STATE_IDLE == Ch->State)
#endif
    {
		Ch->StatInfo.StatErrCnt++;
	
        Ali_DmxSecSemCondUnlock();

		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalFltCnt; Idx++)
    {
        Flt = &(Ali_DmxLibSecLayer->Flts[Idx]);

        ALI_DMX_LIB_PRINT("%s,%d,Idx:%d,State:%d\n", __FUNCTION__, __LINE__, Idx, Flt->State);

        if (ALI_DMX_STREAM_STATE_IDLE == Flt->State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibSecLayer->TotalFltCnt)
    {
        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Idx:%d\n", __FUNCTION__, __LINE__, Idx);

	memset(&Flt->StatInfo, 0, sizeof(Flt->StatInfo));

    Ret = Ali_DmxSecSlotOpen(Ch->DmxId);

    if (Ret < 0)
    {
		Flt->StatInfo.SlotOpenFailCnt++;

        Ali_DmxSecSemCondUnlock();

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        /* TODO: return linux erron.
         */
        return(Ret);
    }

    Flt->ChIdx = ChIdx;

    Flt->SecSlotId = Ret;

	Flt->InByteCnt = 0;

    Flt->State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d,(Idx):%d\n", __FUNCTION__, __LINE__, Idx);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_FLT, Idx));
}





__s32 Ali_DmxSecFltCfg  
(
    __s32                      FltId,
    struct Ali_DmxSecFltParam *FltParam
)
{
    __s32                       Ret;   
    struct ALi_DmxSecCh        *Ch;
    struct ALi_DmxSecFlt       *Flt;
    struct Ali_DmxSecSlotParam  SlotParam;
	__s32                       FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == FltParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (NULL == FltParam->SecRcvCallback)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (FltParam->SecMask.MatchLen >= ALI_DMX_SEC_MATCH_MAX_LEN)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_FLT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if ((Flt->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Flt->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Flt->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();  

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);  

        return(ALI_ERR_DMX_SEC_FLT_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Permit this operation only if corresponding channel is not idle.
     */
    Ch = &(Ali_DmxLibSecLayer->Chs[Flt->ChIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Ch->State)
    {
		Ch->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    memcpy(&(Flt->FltParam), FltParam, sizeof(struct Ali_DmxSecFltParam));

    SlotParam.Pid = Ch->ChParam.Pid;

	SlotParam.NeedDiscramble = Ch->ChParam.NeedDiscramble;
	
    SlotParam.CbParam = FltIdx;

    SlotParam.SecRcvCallback = Ali_SecFltRcvCallback;

    SlotParam.Timeout = FltParam->Timeout;

    memcpy(&(SlotParam.SecMask), &(FltParam->SecMask), 
           sizeof(struct Ali_DmxSecMaskInfo));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = Ali_DmxSecSlotCfg(Flt->SecSlotId, &SlotParam);

    if (Ret < 0)
    {
		Flt->StatInfo.SlotCfgFailCnt++;

        Ali_DmxSecSemCondUnlock(); 

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }
    
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Flt->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxSecFltStart
(
    __s32 FltId
)
{
    __s32                 Ret;   
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;
	__s32                 FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_FLT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if (Flt->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Flt->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();  
		
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);  

        return(ALI_ERR_DMX_SEC_FLT_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

#if 0
    /* Permit this operation only if corresponding channel is not idle.
     */
    Ch = &(Ali_DmxLibSecLayer->Chs[Flt->ChIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Ch->State)
    {
        Ali_DmxSecSemCondUnlock(); 

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);   

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }
#endif

    Ret = Ali_DmxSecSlotStart(Flt->SecSlotId);

    if (Ret < 0)
    {
		Flt->StatInfo.SlotStartFailCnt++;

        Ali_DmxSecSemCondUnlock();  

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__); 
 
        return(Ret);
    }

    Flt->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}







__s32 Ali_DmxSecFltStop
(
    __s32 FltId
)
{
    __s32                 Ret;   
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;
	__s32                 FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_FLT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if (Flt->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Flt->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();

        return(ALI_ERR_DMX_SEC_FLT_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
#if 0
    /* Check if the channel this filter belongs to is ready for this operation.
     */
    Ch = &(Ali_DmxLibSecLayer->Chs[Flt->ChIdx]);

    if ((Ch->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Ch->State != ALI_DMX_STREAM_STATE_RUN))
    {
        Ali_DmxSecSemCondUnlock();    

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }
#endif
    Ret = Ali_DmxSecSlotStop(Flt->SecSlotId);

    if (Ret < 0)
    {
        /* TODO: linux erron.
         */
		Flt->StatInfo.SlotStopFailCnt++;

        Ali_DmxSecSemCondUnlock();

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(Ret);
    }

    Flt->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxSecFltClose
(
    __s32 FltId
)
{
    __s32                 Ret;   
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;
	__s32                 FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_FLT_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Flt->State)
    {
		Flt->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();    

        return(ALI_ERR_DMX_SEC_FLT_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
#if 0
    /* Check if the channel this filter belongs to is ready for this operation.
     */
    Ch = &(Ali_DmxLibSecLayer->Chs[Flt->ChIdx]);

    if ((Ch->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Ch->State != ALI_DMX_STREAM_STATE_RUN))
    {
        Ali_DmxSecSemCondUnlock();    

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }
#endif
    Ret = Ali_DmxSecSlotClose(Flt->SecSlotId);

    if (Ret < 0)
    {
		Flt->StatInfo.SlotCloseFailCnt++;

	    Ali_DmxSecSemCondUnlock();

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
        return(Ret);
    }

    Flt->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSecSemCondUnlock();

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecFltInfoGet
(
    __s32                     FltId, 
	struct Ali_DmxSecFltInfo *SecFltInfo
)
{
    __s32                 Ret;
    struct ALi_DmxSecFlt *Flt;
	__s32                 FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecFltInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Flt->State)
    {
		Flt->StatInfo.StatErrCnt++;
	
        Ali_DmxSecSemCondUnlock();      

        return(ALI_ERR_DMX_SEC_SLOT_OPER_DENIED);
    }
	
	memset(SecFltInfo, 0, sizeof(struct Ali_DmxSecFltInfo));
	
    SecFltInfo->State = Flt->State;//!<Section filter的状态。

    SecFltInfo->Idx = Flt->Idx; //!<Section filter在内部数组中的Index。	

    SecFltInfo->ChIdx = Flt->ChIdx;//!<Section filter属于的的Section channel的IDX。

    SecFltInfo->SecSlotId = Flt->SecSlotId;//!<Section filter使用的Section slot的IDX。

    //!<用户配置的Section filter参数。
	memcpy(&SecFltInfo->FltParam, &Flt->FltParam, sizeof(struct Ali_DmxSecFltParam));

    SecFltInfo->InByteCnt = Flt->InByteCnt;//!<Section filter总共收到的byte总数.

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecFltErrStatGet
(
    __s32                            FltId, 
	struct Ali_DmxLibSecFltStatInfo *SecFltStatInfo
)
{
    __s32                 Ret;
    struct ALi_DmxSecFlt *Flt;
	__s32                 FltIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecFltStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    FltIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_FLT, FltId);

    if ((FltIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (FltIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Flt = &(Ali_DmxLibSecLayer->Flts[FltIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Flt->State)
    {
		Flt->StatInfo.StatErrCnt++;
	
        Ali_DmxSecSemCondUnlock();      

        return(ALI_ERR_DMX_SEC_SLOT_OPER_DENIED);
    }
	
	memcpy(SecFltStatInfo, &Flt->StatInfo, sizeof(struct Ali_DmxLibSecFltStatInfo));

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}

__s32 Ali_DmxSecChOpen
(
    __u32 DmxId 
)
{
    __s32                Ret;
    __s32                Idx;
    __s32                SecStreamId;
    __u32                ShmSize;
    struct ALi_DmxSecCh *Ch;
    __u8                *Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    if (NULL == Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    /* If first open in current address space.
     */
    if (NULL == Ali_DmxLibSecLayer)
    {
        ShmSize = sizeof(struct Ali_DmxLibSecLayer);

        #ifdef ADR_ALIDROID
       	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
       	#else     
        Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
    	#endif
    
        /* Fetal error. 
         */
        if ((void *)-1 == Addr) 
        {
            Ali_DmxSecSemCondUnlock();

            ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer);

            //Ali_DmxLibSecLayerExit();

            return(ALI_ERR_DMX_NOT_INITILAIZED);
        } 

        Ali_DmxLibSecLayer = (struct Ali_DmxLibSecLayer *)Addr;
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,TotalChCnt:%d,TotalFltCnt,:%d\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer->TotalChCnt, Ali_DmxLibSecLayer->TotalFltCnt);

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalChCnt; Idx++)
    {
        Ch = &(Ali_DmxLibSecLayer->Chs[Idx]);

        if (ALI_DMX_STREAM_STATE_IDLE == Ch->State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibSecLayer->TotalChCnt)
    {
        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_CH_EXHAUST);
    }

	memset(&Ch->StatInfo, 0, sizeof(Ch->StatInfo));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ch->DmxId = DmxId;

	Ch->InByteCnt = 0;

    Ch->State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d,Idx:%d\n", __FUNCTION__, __LINE__, Idx);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_CH, Idx));
}




__s32 Ali_DmxSecChCfg  
(
    __s32                     ChId, 
    struct Ali_DmxSecChParam *ChParam
)
{
    __s32                 Ret;   
    __s32                 Idx;  
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecCh  *ChCmp;
    struct ALi_DmxSecFlt *Flt;
	__s32                 ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == ChParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (ChParam->Pid > 0x1FFF)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalChCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);

    /* Channel state verification.
	*/
    if ((Ch->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Ch->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Ch->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    /* A PID could only be configured to one channel.
	*/
	
    if(!(ChParam->NeedNoDupPidCheck))
    {
    
        for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalChCnt; Idx++)
        {
            ChCmp = &(Ali_DmxLibSecLayer->Chs[Idx]);
            
            if ((Idx != ChIdx) && 
                ((ALI_DMX_STREAM_STATE_STOP == ChCmp->State) ||
                 (ALI_DMX_STREAM_STATE_RUN == ChCmp->State)))
            {
                if (ChCmp->ChParam.Pid == ChParam->Pid)
                {
                    Ch->StatInfo.DupPidCnt++;
        
                    Ali_DmxSecSemCondUnlock();
                    
                    return(ALI_ERR_DMX_SEC_CH_DUPED_PID);
                }
            }
        }
    }


    /* Permit this operation only if there is no filter linked to this channel.
     */
    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalFltCnt; Idx++)
    {
        Flt = &(Ali_DmxLibSecLayer->Flts[Idx]);

        //if (ALI_DMX_STREAM_STATE_IDLE != Flt->State)
        if ((ALI_DMX_STREAM_STATE_RUN == Flt->State) ||
			(ALI_DMX_STREAM_STATE_STOP == Flt->State))
        {
            if (Flt->ChIdx == ChIdx)
            {
                break;
            }
        }
    }

    if (Idx < Ali_DmxLibSecLayer->TotalFltCnt)
    {
       	Ch->StatInfo.DupFltCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Ch->ChParam), ChParam, sizeof(struct Ali_DmxSecChParam));

    Ch->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}









__s32 Ali_DmxSecChStart
(
    __s32 ChId
)
{
    __s32                Ret;
    struct ALi_DmxSecCh *Ch;
	__s32                ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalChCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);

    if (Ch->State != ALI_DMX_STREAM_STATE_STOP)
    {
       	Ch->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();  

        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);   

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__); 

    Ch->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxSecChStop
(
    __s32 ChId
)
{
    __s32                Ret;
    __s32                SemId;
    struct ALi_DmxSecCh *Ch;
	__s32                ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalChCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ch->State != ALI_DMX_STREAM_STATE_RUN)
    {
       	Ch->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();    

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__); 

    Ch->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxSecChClose
(
    __s32 ChId
)
{
    __s32           Ret;
    __s32           SemId;
    __s32           Idx;  
    struct ALi_DmxSecCh  *Ch;
    struct ALi_DmxSecFlt *Flt;
	__s32                 ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalChCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_CH_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Ch->State)
    {
       	Ch->StatInfo.StatErrCnt++;

        Ali_DmxSecSemCondUnlock();    

        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__); 

    /* Permit this operation only if there is no filter linked to this channel.
     */
    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalFltCnt; Idx++)
    {
        Flt = &(Ali_DmxLibSecLayer->Flts[Idx]);

        if (ALI_DMX_STREAM_STATE_IDLE != Flt->State)
        {
            if (Flt->ChIdx == ChIdx)
            {
                break;
            }
        }
    }

    if (Idx < Ali_DmxLibSecLayer->TotalFltCnt)
    {
       	Ch->StatInfo.DupFltCnt++;

        Ali_DmxSecSemCondUnlock();
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_SEC_CH_OPER_DENIED);
    }

    Ch->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}





__s32 Ali_DmxSecChInfoGet
(
    __s32                    ChId, 
	struct Ali_DmxSecChInfo *SecChInfo
)
{
    __s32                Ret;
    struct ALi_DmxSecCh *Ch;
	__s32                ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecChInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);
	
	memset(SecChInfo, 0, sizeof(struct Ali_DmxSecChInfo));
	
    SecChInfo->State = Ch->State;//!<Section channel的状态。 

    SecChInfo->DmxId = Ch->DmxId;//!<Section channel对应的DMX的ID。

    SecChInfo->Idx = Ch->Idx;//!<Section channel在内部数组中的Index。

	//!<用户配置的Section channel参数。
	memcpy(&SecChInfo->ChParam, &Ch->ChParam, sizeof(struct Ali_DmxSecChParam));

	SecChInfo->InByteCnt = Ch->InByteCnt;//!<Section channel总共收到的byte总数.

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxSecChErrStatGet
(
    __s32                           ChId, 
	struct Ali_DmxLibSecChStatInfo *SecChStatInfo
)
{
    __s32                Ret;
    struct ALi_DmxSecCh *Ch;
	__s32                ChIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == SecChStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibSecLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    ChIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SEC_CH, ChId);

    if ((ChIdx >= Ali_DmxLibSecLayer->TotalFltCnt) || (ChIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ret = Ali_DmxSecSemCondLock();

    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    } 

    Ch = &(Ali_DmxLibSecLayer->Chs[ChIdx]);

	memcpy(SecChStatInfo, &Ch->StatInfo, sizeof(struct Ali_DmxLibSecChStatInfo));

    Ali_DmxSecSemCondUnlock(); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxLibSecLayerExit
(
    void
)
{
	__u8					*Addr;
	struct ALi_DmxSecFlt    *Flt;
	struct ALi_DmxSecCh     *Ch;
	struct ALi_DmxSecSlot   *Slot;
	struct ALi_DmxSecStream *Stream;

	__s32					 Idx;
	__u32					 ShmSize;

	ShmSize = sizeof(struct Ali_DmxLibSecLayer);

	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_SEC_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
    #else     
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
	#endif
	
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSecLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibSecLayer = (struct Ali_DmxLibSecLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibSecLayer->SemId);

    Ali_DmxSecSemCondLock();

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalFltCnt; Idx++)
    { 
		Ali_DmxSecFltClose(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_FLT, Idx));
    }	

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalChCnt; Idx++)
    { 
		Ali_DmxSecChClose(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_CH, Idx));
    }

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalSlotCnt; Idx++)
    { 
		Ali_DmxSecSlotClose(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC_SLOT, Idx));
    }

    for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalStreamCnt; Idx++)
    { 
		Ali_DmxSecStreamClose(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SEC, Idx));
    }

    Ali_DmxSecSemCondUnlock();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);

}



__s32 Ali_DmxLibSecLayerInit
(
    void
)
{
    __s32                Idx;
    __s32                SemId;
    __s32                ShmId;
    __u32                ShmSize;
    __s32                Ret;
    struct Ali_DmxLibSecLayer *Layer;
    struct ALi_DmxSecStream   *Stream;
    struct ALi_DmxSecSlot     *Slot;
    struct ALi_DmxSecCh       *Ch;
    struct ALi_DmxSecFlt      *Flt;
    union semun                Option;
    __u8                      *Addr;
   int tmp = ALI_DMX_LIB_SEC_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_SEC_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibSecLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibSecLayer);
    #ifdef ADR_ALIDROID
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
    #else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))    
    {       
     #ifdef ADR_ALIDROID     	
     	SemId = OS_CreateSemaphore(1);
 		if (SemId < 0)
	    {
	        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
	    }
     #else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured .
     */
    SemId = semget(ALI_DMX_LIB_SEC_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);
    #endif


    Ali_DmxSemLock(SemId);


    /* Init share memory.
     */
    ShmSize = sizeof(struct Ali_DmxLibSecLayer);
	    #ifdef ADR_ALIDROID
	    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
		Layer = (struct Ali_DmxLibSecLayer *)Addr;

	    #else

    ShmId = shmget(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, ShmId);

         Ali_DmxSemUnlock(SemId);
           return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    } 

    Layer = (struct Ali_DmxLibSecLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Layer);

         Ali_DmxSemUnlock(SemId);
           return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Layer);

    memset(Layer, 0, sizeof(struct Ali_DmxLibSecLayer));

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 
    }   
    else
    {
        Layer = (struct Ali_DmxLibSecLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }

    /* Init section stream interface.
     */
    Layer->TotalStreamCnt = ALI_DMX_LIB_SEC_STREAM_CNT;

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

		Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    /* Init section slot interface.
     */
    Layer->TotalSlotCnt = ALI_DMX_LIB_SEC_SLOT_CNT;

    for (Idx = 0; Idx < Layer->TotalSlotCnt; Idx++)
    {
        Slot = &(Layer->Slots[Idx]);

        Slot->State = ALI_DMX_STREAM_STATE_IDLE;

        Slot->Idx = Idx;

        Slot->SecStreamId = -1;
    }   


    /* Init section channel-filter interface.
     */
    Layer->TotalChCnt = ALI_DMX_LIB_SEC_CH_CNT;

    for (Idx = 0; Idx < Layer->TotalChCnt; Idx++)
    {
        Ch = &(Layer->Chs[Idx]);

        Ch->State = ALI_DMX_STREAM_STATE_IDLE;

        Ch->Idx = Idx;
    }   

    Layer->TotalFltCnt = ALI_DMX_LIB_SEC_FLT_CNT;

    for (Idx = 0; Idx < Layer->TotalFltCnt; Idx++)
    {
        Flt = &(Layer->Flts[Idx]);

        Flt->State = ALI_DMX_STREAM_STATE_IDLE;

        Flt->Idx = Idx;

        Flt->ChIdx = 0xFFFFFFFF;
    } 

    Layer->SemLockThreadId = (pthread_t)0xFFFFFFFF;

    Layer->SemLockTimes = 0;
	#ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
    #endif

    Ali_DmxSemUnlock(SemId);
     
    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Layer);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxLibVideoLayerExit
(
    void
)
{
	__u8					  *Addr;
	struct ALi_DmxVideoStream *Stream;
	__s32					   Idx;
	__u32					   ShmSize;
	__s32                      Ret;

	ShmSize = sizeof(struct Ali_DmxLibVideoLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
    #else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
	#endif
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibVideoLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibVideoLayer = (struct Ali_DmxLibVideoLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);	

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibVideoLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibVideoLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}

			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);

}



__s32 Ali_DmxLibVideoLayerInit
(
    void  
)
{
    struct Ali_DmxLibVideoLayer  *Layer;
    struct ALi_DmxVideoStream *Stream;
    __u32                      ShmSize;
    union semun                Option;
    __s32                      SemId;
    __s32                      ShmId;
    __s32                      Idx;
    __u8                        *Addr;
    int tmp = ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY;
 
    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_VIDEO_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibVideoLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibVideoLayer);
    
    #ifdef ADR_ALIDROID
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
    #else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
    #endif    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))     
    {       
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured 
     */
    SemId = semget(ALI_DMX_LIB_VIDEO_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(-1);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibVideoLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
    Layer = (struct Ali_DmxLibVideoLayer *)Addr;    

    #else

    ShmId = shmget(ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        Ali_DmxLibVideoLayerExit();

        Ali_DmxSemUnlock(SemId);

        return(-2);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibVideoLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(-3);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibVideoLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_VIDEO_STREAM_CNT;

    } 
    else
    {
        Layer = (struct Ali_DmxLibVideoLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	#ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
    #endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxVideoStreamOpen
(
    __u32 DmxId 
)
{
    __u8                      *DmxPath;
    struct ALi_DmxVideoStream *Stream;
    __s32                      Idx;
    __s32                      Fd;
    __u32                      ShmSize;
    __u8                      *Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

	if (NULL == DmxPath)
	{
		//Stream[Idx].StatInfo.InvPathCnt++;
	
        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}

    if (NULL == Ali_DmxLibVideoLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibVideoLayer);
    	
    	#ifdef ADR_ALIDROID
	    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
	    #else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_VIDEO_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{    	
    		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibVideoLayer);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibVideoLayer = (struct Ali_DmxLibVideoLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ALI_DMX_LIB_PRINT("TotalStreamCnt:%d\n", Ali_DmxLibVideoLayer->TotalStreamCnt);

    Stream = Ali_DmxLibVideoLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibVideoLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibVideoLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d, Idx = %d, Ali_DmxLibVideoLayer->TotalStreamCnt= %d\n", 
        	__FUNCTION__, __LINE__, Idx, Ali_DmxLibVideoLayer->TotalStreamCnt);
    
        return(ALI_ERR_DMX_VIDEO_STREAM_EXHAUST);
    }

	memset(&Stream[Idx].StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream[Idx].StatInfo.IoOpenFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }	

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

	Stream[Idx].DmxId = DmxId;

	Stream[Idx].InByteCnt = 0;

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_VIDEO, Idx));
}







__s32 Ali_DmxVideoStreamCfg  
(
    __s32                           StreamId, 
    struct Ali_DmxVideoStreamParam *StreamParam
)
{ 
    __s32                      Ret;   
    struct ALi_DmxVideoStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->Pid > 0x1FFF)
	{
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);        

        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxVideoStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_VIDEO_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}








__s32 Ali_DmxVideoStreamStart
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxVideoStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);        

        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_VIDEO_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}








__s32 Ali_DmxVideoStreamStop
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxVideoStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);        

        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_VIDEO_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;
    
        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxVideoStreamClose
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxVideoStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);        

        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);

    Ret = close(Stream->Fd); 

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxVideoStreamInfoGet
(
    __s32                            StreamId, 
	struct Ali_DmxVideoStreamInfo   *VideoInfo
)
{
    __s32                              Ret;
    struct ALi_DmxVideoStream         *Stream;
	__s32                              StreamIdx;
	struct Ali_DmxDrvVideoStrmStatInfo DrvVideoStreamInfo;
	struct Ali_DmxDrvTsFltStatInfo	   TsFltStatInfo;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == VideoInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);
	
	memset(VideoInfo, 0, sizeof(struct Ali_DmxVideoStreamInfo));
	
    VideoInfo->State = Stream->State;//!<Video stream的状态.

	VideoInfo->DmxId = Stream->DmxId;//!<Video stream在内部数组中的Index。	

    VideoInfo->Idx = Stream->Idx; //!<Video stream在内部数组中的Index。	

    VideoInfo->Fd = Stream->Fd;//!<Video stream对应的linux DMX driver的文件描述符。  

	memcpy(&(VideoInfo->StreamParam), &(Stream->StreamParam), sizeof(struct Ali_DmxVideoStreamParam));

#if 0	
    Ret = ioctl(Stream->Fd, ALI_DMX_VIDEO_STREAM_INFO_GET, &DrvVideoStreamInfo);  

    if (Ret < 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    VideoInfo->InByteCnt = DrvVideoStreamInfo.TsInCnt * 188;//!<Video Stream总共收到的byte总数.
#endif
    
    /* TS layer info
	*/
	Ret = ioctl(Stream->Fd, ALI_DMX_VIDEO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("Ret=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

    VideoInfo->TsInCnt = TsFltStatInfo.TsInCnt;
    VideoInfo->TsScrmbCnt = TsFltStatInfo.TsScrmbCnt;
    VideoInfo->TsSyncByteErrCnt = TsFltStatInfo.TsSyncByteErrCnt;
	VideoInfo->TsDupCnt = TsFltStatInfo.TsDupCnt;
	VideoInfo->TsLostCnt = TsFltStatInfo.TsLostCnt;
    VideoInfo->TsErrCnt = TsFltStatInfo.TsErrCnt;

    VideoInfo->InByteCnt = VideoInfo->TsInCnt * 188;//!<Audio Stream总共收到的byte总数.

    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxVideoStreamErrStatGet
(
    __s32                               StreamId, 
	struct Ali_DmxLibVideoStrmStatInfo *VideoStatInfo
)
{
    __s32                              Ret;
    struct ALi_DmxVideoStream         *Stream;
	__s32                              StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == VideoStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibVideoLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_VIDEO, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);

    Stream = &(Ali_DmxLibVideoLayer->Streams[StreamIdx]);
	
	memcpy(VideoStatInfo, &Stream->StatInfo, sizeof(struct Ali_DmxLibVideoStrmStatInfo));
	
    Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxLibAudioLayerExit
(
    void
)
{
	__u8					  *Addr;
	struct ALi_DmxAudioStream *Stream;
	__s32					   Idx;
	__u32					   ShmSize;
	__s32                      Ret;

	ShmSize = sizeof(struct Ali_DmxLibAudioLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
	#endif
	
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibAudioLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibAudioLayer = (struct Ali_DmxLibAudioLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);	

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibAudioLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibAudioLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}

			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxLibAudioLayerInit
(
    void  
)
{
    struct Ali_DmxLibAudioLayer *Layer;
    struct ALi_DmxAudioStream   *Stream;
    __u32                        ShmSize;
    union semun                  Option;
    __s32                        SemId;
    __s32                        ShmId;
    __s32                        Idx;
    __u8                        *Addr;
   int tmp = ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_AUDIO_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibAudioLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibAudioLayer);

	#ifdef ADR_ALIDROID
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
    #else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))     
    {      
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured 
     */
    SemId = semget(ALI_DMX_LIB_AUDIO_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibAudioLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
    Layer = (struct Ali_DmxLibAudioLayer *)Addr;    

    #else

    ShmId = shmget(ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        Ali_DmxLibAudioLayerExit();

        Ali_DmxSemUnlock(SemId);

            return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibAudioLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibAudioLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

        Layer->TotalStreamCnt = ALI_DMX_LIB_AUDIO_STREAM_CNT;
    }   
    else
    {
        Layer = (struct Ali_DmxLibAudioLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	#ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
    #endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxAudioStreamOpen
(
    __u32 DmxId 
)
{
    __u8                      *DmxPath;
    __u8                      *Addr;
    struct ALi_DmxAudioStream *Stream;
    __s32                      Idx;
    __s32                      Fd;
    __u32                      ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);
	
	if (NULL == DmxPath)
	{
		//Stream[Idx].StatInfo.InvPathCnt++;
	
        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}
	
    if (NULL == Ali_DmxLibAudioLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibAudioLayer);
    	
    	#ifdef ADR_ALIDROID
	    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
	    #else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_AUDIO_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{    	
    		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibAudioLayer);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibAudioLayer = (struct Ali_DmxLibAudioLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ALI_DMX_LIB_PRINT("TotalStreamCnt:%d\n", Ali_DmxLibAudioLayer->TotalStreamCnt);

    Stream = Ali_DmxLibAudioLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibAudioLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibAudioLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_AUDIO_STREAM_EXHAUST);
    }

	memset(&Stream[Idx].StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream[Idx].StatInfo.IoOpenFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    
	
    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

	Stream[Idx].DmxId = DmxId;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_AUDIO, Idx));
}







__s32 Ali_DmxAudioStreamCfg  
(
    __s32                           StreamId, 
    struct Ali_DmxAudioStreamParam *StreamParam
)
{ 
    __s32                      Ret;   
    struct ALi_DmxAudioStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->Pid > 0x1FFF)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibAudioLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);        

        return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxAudioStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_AUDIO_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
	
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxAudioStreamStart
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxAudioStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibAudioLayer)
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);        

        return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_AUDIO_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;
    
        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxAudioStreamStop
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxAudioStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d,ID:%d\n", __FUNCTION__, __LINE__, StreamId);

    if (NULL == Ali_DmxLibAudioLayer)
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);        

        return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_AUDIO_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxAudioStreamClose
(
    __s32 StreamId
)
{
    __s32                      Ret;
    struct ALi_DmxAudioStream *Stream;
	__s32                      StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibAudioLayer)
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;
    
        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);        

        return(ALI_ERR_DMX_AUDIO_STREAM_OPER_DENIED);
    }

	ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
	
	Ret = close(Stream->Fd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxAudioStreamInfoGet
(
    __s32                          StreamId, 
	struct Ali_DmxAudioStreamInfo *AudioInfo
)
{
    __s32                              Ret;
    struct ALi_DmxAudioStream         *Stream;
	__s32                              StreamIdx;
	struct Ali_DmxDrvAudioStrmStatInfo DrvAudioStreamInfo;
	struct Ali_DmxDrvTsFltStatInfo	   TsFltStatInfo;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == AudioInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibAudioLayer)
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);
	
	memset(AudioInfo, 0, sizeof(struct Ali_DmxAudioStreamInfo));
	
    AudioInfo->State = Stream->State;//!<Audio stream的状态.

	AudioInfo->DmxId = Stream->DmxId;//!<Audio stream对应的DMX的ID。	

    AudioInfo->Idx = Stream->Idx;//!<Audio stream在内部数组中的Index。

    AudioInfo->Fd = Stream->Fd;//!<Audio stream对应的linux DMX driver的文件描述符。 

    //!<用户配置的Audio stream参数。
	memcpy(&(AudioInfo->StreamParam), &(Stream->StreamParam), sizeof(struct Ali_DmxAudioStreamParam));

#if 0
    Ret = ioctl(Stream->Fd, ALI_DMX_AUDIO_STREAM_INFO_GET, &DrvAudioStreamInfo);  

    if (Ret < 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }
#endif
    
    /* TS layer info
	*/
	Ret = ioctl(Stream->Fd, ALI_DMX_AUDIO_FILTER_INFO_GET, &TsFltStatInfo);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("RET=%d %s,%d,%m\n", Ret, __FUNCTION__, __LINE__, errno);

		return (-1);
	}

    AudioInfo->TsInCnt = TsFltStatInfo.TsInCnt;
    AudioInfo->TsScrmbCnt = TsFltStatInfo.TsScrmbCnt;
    AudioInfo->TsSyncByteErrCnt = TsFltStatInfo.TsSyncByteErrCnt;
	AudioInfo->TsDupCnt = TsFltStatInfo.TsDupCnt;
	AudioInfo->TsLostCnt = TsFltStatInfo.TsLostCnt;
    AudioInfo->TsErrCnt = TsFltStatInfo.TsErrCnt;

    AudioInfo->InByteCnt = AudioInfo->TsInCnt * 188;//!<Audio Stream总共收到的byte总数.
	
    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxAudioStreamErrStatGet
(
    __s32                               StreamId, 
	struct Ali_DmxLibAudioStrmStatInfo *AudioStatInfo
)
{
    __s32                              Ret;
    struct ALi_DmxAudioStream         *Stream;
	__s32                              StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == AudioStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibAudioLayer)
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_AUDIO, StreamId);

    if ((StreamIdx >= Ali_DmxLibAudioLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);

    Stream = &(Ali_DmxLibAudioLayer->Streams[StreamIdx]);

	memcpy(AudioStatInfo, &Stream->StatInfo, sizeof(struct Ali_DmxLibAudioStrmStatInfo));

    Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxPcrStreamOpen
(
    __u32 DmxId 
)
{
    __u8                    *DmxPath;
    __u8                    *Addr;
    struct ALi_DmxPcrStream *Stream;
    __s32                    Idx;
    __s32                    Fd;
    __u32                    ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

	if (NULL == DmxPath)
	{
		//Stream[Idx].StatInfo.InvPathCnt++;
		
        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}
	
    if (NULL == Ali_DmxLibPcrLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibPcrLayer);
    	
    	#ifdef ADR_ALIDROID
	    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
	    #else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{		
    		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibPcrLayer);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibPcrLayer = (struct Ali_DmxLibPcrLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = Ali_DmxLibPcrLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibPcrLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibPcrLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_PCR_STREAM_EXHAUST);
    }

	memset(&Stream[Idx].StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream[Idx].StatInfo.IoOpenFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

	Stream[Idx].DmxId = DmxId;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_PCR, Idx));
}


__s32 Ali_DmxPcrStreamCfg  
(
    __s32                         StreamId, 
    struct Ali_DmxPcrStreamParam *StreamParam
)
{ 
    __s32                    Ret;   
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->Pid > 0x1FFF)
	{
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
		return(ALI_ERR_DMX_INVALID_PID);
	}

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);        

        return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxPcrStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_PCR_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d,FD:%d,Ret:%d,%m\n", __FUNCTION__, __LINE__, Stream->Fd, Ret, errno);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxPcrStreamStart
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;
    
        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);        

        return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_PCR_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxPcrStreamStop
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);        

        return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_PCR_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxPcrStreamClose
(
    __s32 StreamId
)
{
    __s32                    Ret;
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);        

        return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);

    Ret = close(Stream->Fd); 

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxPcrStreamInfoGet
(
    __s32                        StreamId, 
	struct Ali_DmxPcrStreamInfo *PcrStreamInfo
)
{
    __s32                    Ret;
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == PcrStreamInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);
	
	memset(PcrStreamInfo, 0, sizeof(struct Ali_DmxPcrStreamInfo));
	
    PcrStreamInfo->State = Stream->State;//!<Pcr stream的状态.

	PcrStreamInfo->DmxId = Stream->DmxId;//!<Pcr stream对应的DMX的ID。	

    PcrStreamInfo->Idx = Stream->Idx;//!<Pcr stream在内部数组中的Index。	

    PcrStreamInfo->Fd = Stream->Fd;//!<Pcr stream对应的linux DMX driver的文件描述符。 

    //!<用户配置的Pcr stream参数。
	memcpy(&(PcrStreamInfo->StreamParam), &(Stream->StreamParam), sizeof(struct Ali_DmxPcrStreamParam));

    PcrStreamInfo->InByteCnt = Stream->InByteCnt;//!<Pcr Stream总共收到的byte总数.	

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxPcrStreamErrStatGet
(
    __s32                             StreamId, 
	struct Ali_DmxLibPcrStrmStatInfo *PcrStatInfo
)
{
    __s32                    Ret;
    struct ALi_DmxPcrStream *Stream;
	__s32                    StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == PcrStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibPcrLayer)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_PCR, StreamId);

    if ((StreamIdx >= Ali_DmxLibPcrLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    Stream = &(Ali_DmxLibPcrLayer->Streams[StreamIdx]);
	
	memcpy(PcrStatInfo, &Stream->StatInfo, sizeof(struct Ali_DmxLibPcrStrmStatInfo));

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxLibPcrLayerExit
(
    void
)
{
	__u8					*Addr;
	struct ALi_DmxPcrStream *Stream;
	__s32					 Idx;
	__u32					 ShmSize;
	__s32                    Ret;

	ShmSize = sizeof(struct Ali_DmxLibPcrLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_PCR_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
	    Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
	#endif
	
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibPcrLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibPcrLayer = (struct Ali_DmxLibPcrLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);	

    Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibPcrLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibPcrLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}	
			
			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxLibPcrLayerInit
(
    void  
)
{
    struct Ali_DmxLibPcrLayer *Layer;
    struct ALi_DmxPcrStream      *Stream;
    __u32                         ShmSize;
    union semun                   Option;
    __s32                         SemId;
    __s32                         ShmId;
    __s32                         Idx;
    __u8                         *Addr;
    int tmp = ALI_DMX_LIB_PCR_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_PCR_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibPcrLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibPcrLayer);
    
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
	#else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))   
    {       
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
        /* semget returns -1 if the key already exits while IPC_EXCL 
         * is configured 
         */
        SemId = semget(ALI_DMX_LIB_PCR_LAYER_SEM_KEY, 1, 
                       IPC_CREAT | IPC_EXCL | 0x777);
        
        /* Make sure that our section stream layer only be inited once.
         */
        if (SemId < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        
            return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
        }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		   #endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibPcrLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize);
    Layer = (struct Ali_DmxLibPcrLayer *)Addr;    

	#else

    ShmId = shmget(ALI_DMX_LIB_PCR_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it does happened, something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        Ali_DmxLibPcrLayerExit();

        Ali_DmxSemUnlock(SemId);

            return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibPcrLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_PCR_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibPcrLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_PCR_STREAM_CNT;

    }   
    else
    {
        Layer = (struct Ali_DmxLibPcrLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}







__s32 Ali_DmxTsStreamOpen
(
    __u32 DmxId 
)
{
    __u8                   *DmxPath;
    __u8                   *Addr;
    struct ALi_DmxTsStream *Stream;
    __s32                   Idx;
    __s32                   Fd;
    __u32                   ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);	

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

	if (NULL == DmxPath)
	{
		//Stream[Idx].StatInfo.InvPathCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}
	
    if (NULL == Ali_DmxLibTsLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibTsLayer);
    	
    	#ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
		#else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{		
    		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibTsLayer);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibTsLayer = (struct Ali_DmxLibTsLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = Ali_DmxLibTsLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibTsLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibTsLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_TS_STREAM_EXHAUST);
    }

	memset(&Stream[Idx].StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream[Idx].StatInfo.IoOpenFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    
	
    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

	Stream[Idx].DmxId = DmxId;

	Stream[Idx].InByteCnt = 0;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TS, Idx));
}



__s32 Ali_DmxTsStreamCfg  
(
    __s32                        StreamId, 
    struct Ali_DmxTsStreamParam *StreamParam
)
{
    __s32                   Ret;   
	__s32                   Idx;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}	

	if (StreamParam->PidCnt > ALI_DMX_TS_STREAM_MAX_PID_CNT)
	{
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	for(Idx = 0; Idx < StreamParam->PidCnt; Idx++)
	{
        if (StreamParam->PidList[Idx] > 0x1FFF)
        {
	    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

			return(ALI_ERR_DMX_INVALID_PID);
		}

    	if ((StreamParam->NeedDiscramble[Idx] != 0) && 
    		(StreamParam->NeedDiscramble[Idx] != 1))
    	{
            return(ALI_ERR_DMX_INVALID_PARAM);
    	}		
	}

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);        

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxTsStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);

}


__s32 Ali_DmxTsStreamStart
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);        

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}







__s32 Ali_DmxTsStreamStop
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);        

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
	
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}







__s32 Ali_DmxTsStreamClose
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);        

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

	ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
	
	Ret = close(Stream->Fd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxTsStreamRead
(
    __s32 StreamId, 
    __u8 *Buf, 
    __u32 BufLen
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {     
		Stream->StatInfo.StatErrCnt++;

		ALI_DMX_LIB_PRINT("%s,%d,State:%d\n", __FUNCTION__, __LINE__, Stream->State);

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    //Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = read(Stream->Fd, Buf, BufLen);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret > 0)
    {
		Stream->InByteCnt += Ret;
	}

    //Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_DmxErrCode(Ret));
}


__s32 Ali_DmxTsStreamInfoGet
(
    __s32                       StreamId, 
	struct Ali_DmxTsStreamInfo *TsStreamInfo
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == TsStreamInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);
	
	memset(TsStreamInfo, 0, sizeof(struct Ali_DmxTsStreamInfo));
	
    TsStreamInfo->State = Stream->State;//!<TS stream的状态.

	TsStreamInfo->DmxId = Stream->DmxId;//!<TS stream对应的DMX的ID。	

    TsStreamInfo->Idx = Stream->Idx; //!<TS stream在内部数组中的Index。

    TsStreamInfo->Fd = Stream->Fd;//!<TS stream对应的linux DMX driver的文件描述符.

    //!<用户配置的TS stream参数。
	memcpy(&TsStreamInfo->StreamParam, &Stream->StreamParam, sizeof(struct Ali_DmxTsStreamParam));

    TsStreamInfo->InByteCnt = Stream->InByteCnt;//!<TS Stream总共收到的byte总数.	
	
    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxTsStreamErrStatGet
(
    __s32                            StreamId, 
	struct Ali_DmxLibTsStrmStatInfo *TsStatInfo
)
{
    __s32                   Ret;
    struct ALi_DmxTsStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == TsStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibTsLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
	}

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    Stream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);

	memcpy(TsStatInfo, &Stream->StatInfo, sizeof(struct Ali_DmxLibTsStrmStatInfo));
	
    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxLibTsLayerExit
(
    void
)
{
	__u8				   *Addr;
	struct ALi_DmxTsStream *Stream;
	__s32					Idx;
	__u32					ShmSize;
	__s32                   Ret;
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	ShmSize = sizeof(struct Ali_DmxLibTsLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_TS_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
	#endif
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibTsLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibTsLayer = (struct Ali_DmxLibTsLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);

    Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibTsLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibTsLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}

			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxLibTsLayerInit
(
    void  
)
{
    struct Ali_DmxLibTsLayer *Layer;
    struct ALi_DmxTsStream   *Stream;
    __u32                     ShmSize;
    union semun               Option;
    __s32                     SemId;
    __s32                     ShmId;
    __s32                     Idx;
    __u8                     *Addr;
    int tmp = ALI_DMX_LIB_TS_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_TS_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibTsLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibTsLayer);
    
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
	#else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))  
    {       
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured 
     */
    SemId = semget(ALI_DMX_LIB_TS_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibTsLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
    Layer = (struct Ali_DmxLibTsLayer *)Addr;    

	#else

    ShmId = shmget(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        //Ali_DmxLibTsLayerExit();

        Ali_DmxSemUnlock(SemId);

            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibTsLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibTsLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_TS_STREAM_CNT;
    } 
    else
    {
        Layer = (struct Ali_DmxLibTsLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}











__s32 Ali_DmxTpStreamOpen
(
    __u32 DmxId 
)
{
    __u8                   *DmxPath;
    __u8                   *Addr;
    struct ALi_DmxTpStream *Stream;
    __s32                   Idx;
    __s32                   Fd;
    __u32                   ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);

	if (NULL == DmxPath)
	{
		//Stream[Idx].StatInfo.InvPathCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibTpLayer);
    	
    	#ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
		#else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{		
    		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibTpLayer);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibTpLayer = (struct Ali_DmxLibTpLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = Ali_DmxLibTpLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibTpLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibTpLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_TP_STREAM_EXHAUST);
    }

	memset(&Stream[Idx].StatInfo, 0, sizeof(Stream->StatInfo));

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

		Stream[Idx].StatInfo.IoOpenFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    
	
    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

	Stream[Idx].DmxId = DmxId;

	Stream[Idx].InByteCnt = 0;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TP, Idx));
}







__s32 Ali_DmxTpStreamCfg  
(
    __s32                        StreamId, 
    struct Ali_DmxTpStreamParam *StreamParam
)
{
    __s32                   Ret;   
	__s32                   Idx;
    struct ALi_DmxTpStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TP, StreamId);

    if ((StreamIdx >= Ali_DmxLibTpLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTpLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);        

        return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxTpStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TP_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
		Stream->StatInfo.IoCfgFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);

}






__s32 Ali_DmxTpStreamStart
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTpStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TP, StreamId);

    if ((StreamIdx >= Ali_DmxLibTpLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    Stream = &(Ali_DmxLibTpLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);        

        return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_TP_STREAM_START, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStartFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxTpStreamStop
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTpStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TP, StreamId);

    if ((StreamIdx >= Ali_DmxLibTpLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTpLayer->Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);        

        return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TP_STREAM_STOP, 0);  

    if (Ret < 0)
    {
		Stream->StatInfo.IoStopFailCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);
	
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}









__s32 Ali_DmxTpStreamClose
(
    __s32 StreamId
)
{
    __s32                   Ret;
    struct ALi_DmxTpStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TP, StreamId);

    if ((StreamIdx >= Ali_DmxLibTpLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTpLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
		Stream->StatInfo.StatErrCnt++;

        Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);        

        return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
    }

    close(Stream->Fd);  

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxTpStreamRead
(
    __s32 StreamId, 
    __u8 *Buf, 
    __u32 BufLen
)
{
    __s32                   Ret;
    struct ALi_DmxTpStream *Stream;
	__s32                   StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTpLayer)
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TP, StreamId);

    if ((StreamIdx >= Ali_DmxLibTpLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
    	ALI_DMX_LIB_PRINT("%s,%d,StreamIdx:%d\n", __FUNCTION__, __LINE__, StreamIdx);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTpLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {     
		Stream->StatInfo.StatErrCnt++;

		ALI_DMX_LIB_PRINT("%s,%d,State:%d\n", __FUNCTION__, __LINE__, Stream->State);

        return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    //Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d,Fd:%d\n", __FUNCTION__, __LINE__, Stream->Fd);

    Ret = read(Stream->Fd, Buf, BufLen);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret > 0)
    {
		Stream->InByteCnt += Ret;
	}

    //Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId); 

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_DmxErrCode(Ret));
}








__s32 Ali_DmxTpStreamInfoGet
(
    __s32                       StreamId, 
	struct Ali_DmxTsStreamInfo *TsStreamInfo
)
{
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	/* TODO: Implement.
	*/
    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxLibTpLayerExit
(
    void
)
{
	__u8				   *Addr;
	struct ALi_DmxTpStream *Stream;
	__s32					Idx;
	__u32					ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	ShmSize = sizeof(struct Ali_DmxLibTpLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_TP_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
	#endif
	
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibTpLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibTpLayer = (struct Ali_DmxLibTpLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);

    Ali_DmxSemLock(Ali_DmxLibTpLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibTpLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibTpLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			close(Stream->Fd);	
			
			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibTpLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxLibTpLayerInit
(
    void  
)
{
    struct Ali_DmxLibTpLayer *Layer;
    struct ALi_DmxTpStream   *Stream;
    __u32                     ShmSize;
    union semun               Option;
    __s32                     SemId;
    __s32                     ShmId;
    __s32                     Idx;
    __u8                     *Addr;
    int tmp = ALI_DMX_LIB_TP_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_TP_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibTpLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibTpLayer);
    #ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
	#else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))  
    {       
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
        /* semget returns -1 if the key already exits while IPC_EXCL 
         * is configured 
         */
        SemId = semget(ALI_DMX_LIB_TP_LAYER_SEM_KEY, 1, 
                       IPC_CREAT | IPC_EXCL | 0x777);
    
        /* Make sure that our section stream layer only be inited once.
         */
        if (SemId < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
                return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
        }
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        Option.val = 1;
    
        semctl(SemId, 0, SETVAL, Option);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	    #endif
    
        Ali_DmxSemLock(SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        ShmSize = sizeof(struct Ali_DmxLibTpLayer);
    
        /* shmget returns -1 if key already exits when IPC_EXCL is configured.
         */
        #ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize);
        Layer = (struct Ali_DmxLibTpLayer *)Addr;        

		#else

        ShmId = shmget(ALI_DMX_LIB_TP_LAYER_SHM_KEY, ShmSize, 
                       IPC_CREAT | IPC_EXCL | 0x777); 
        
        if (ShmId < 0)
        {
            /* Should never happen. 
             * If it does happen, something must be seriously wrong, close slot 
             * layer at all. 
             */
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
            //Ali_DmxLibTsLayerExit();
    
            Ali_DmxSemUnlock(SemId);
    
            return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
        }
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        Layer = (struct Ali_DmxLibTpLayer *)(shmat(ShmId, (void *)0, 0));
#endif    
        if ((void *)-1 == Layer) 
        {
            Ali_DmxSemUnlock(SemId);
    
            /* Make sure that our section stream layer could only be inited once.
             */
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
            return(ALI_ERR_DMX_TP_STREAM_OPER_DENIED);
        }
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        memset(Layer, 0, sizeof(struct Ali_DmxLibTpLayer));
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        Layer->SemId = SemId;
    
        Layer->ShmId = ShmId; 
    
        Layer->TotalStreamCnt = ALI_DMX_LIB_TP_STREAM_CNT;
    } 
    else
    {
        Layer = (struct Ali_DmxLibTpLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}












__s32 Ali_DmxLibTsInRamLayerExit
(
    void
)
{
	__u8				        *Addr;
	struct ALi_DmxTsInRamStream *Stream;
	__s32					     Idx;
	__u32					     ShmSize;
	__s32                        Ret;
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	ShmSize = sizeof(struct Ali_DmxLibTsInRamLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);
	#endif
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibTsInRamLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibTsInRamLayer = (struct Ali_DmxLibTsInRamLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancel all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Ali_DmxLibTsInRamLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibTsInRamLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}

			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxLibTsInRamLayerInit
(
    void  
)
{ 
    struct Ali_DmxLibTsInRamLayer *Layer;
    struct ALi_DmxTsInRamStream   *Stream;
    __u32                          ShmSize;
    union semun                    Option;
    __s32                          SemId;
    __s32                          ShmId;
    __s32                          Idx;
    __u8                          *Addr;
    int tmp = ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_TS_IN_RAM_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibTsInRamLayerExit();

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibTsInRamLayer);
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);	
	#else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))  
    {       
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
		/* semget returns -1 if the key already exits while IPC_EXCL 
		 * is configured 
		 */
		SemId = semget(ALI_DMX_LIB_TS_IN_RAM_LAYER_SEM_KEY, 1, 
					   IPC_CREAT | IPC_EXCL | 0x777);

        if (SemId < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
        
            return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
        }		

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibTsInRamLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);	
    Layer = (struct Ali_DmxLibTsInRamLayer *)Addr;    

	#else

    ShmId = shmget(ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        //Ali_DmxLibTsLayerExit();

        Ali_DmxSemUnlock(SemId);

			return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibTsInRamLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibTsInRamLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_TS_IN_RAM_STREAM_CNT;
    } 	
    else
    {
        Layer = (struct Ali_DmxLibTsInRamLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxTsInRamStreamOpen
(
    __u32 DmxId 
)
{
    __u8                        *DmxPath;
    __u8                        *Addr;
    struct ALi_DmxTsInRamStream *Stream;
    __s32                        Idx;
    __s32                        Fd;
    __u32                        ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2Path(ALI_DMX_INPUT, DmxId);

	if (NULL == DmxPath)
	{
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibTsInRamLayer);
    	
    	#ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);	
		#else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_IN_RAM_LAYER_SHM_KEY, ShmSize);
    	#endif
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{		
    		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibTsInRamLayer = (struct Ali_DmxLibTsInRamLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = Ali_DmxLibTsInRamLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibTsInRamLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_EXHAUST);
    }

    //Fd = open(DmxPath, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    Fd = open(DmxPath, O_WRONLY | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;
	
	Stream[Idx].DmxId = DmxId;

	Stream[Idx].InByteCnt = 0;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_CFG;

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_TS_IN_RAM, Idx));
}







__s32 Ali_DmxTsInRamStreamCfg  
(
    __s32                             StreamId, 
    struct Ali_DmxTsInRamStreamParam *StreamParam
)
{
    __s32                        Ret;   
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

#if 0
	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}
#endif

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

    if ((Stream->State != ALI_DMX_STREAM_STATE_STOP) &&
        (Stream->State != ALI_DMX_STREAM_STATE_CFG))
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);        

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memcpy(&(Stream->StreamParam), StreamParam, 
           sizeof(struct Ali_DmxTsInRamStreamParam));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_IN_RAM_STREAM_CFG, &(Stream->StreamParam));  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}





__s32 Ali_DmxTsInRamStreamStart
(
    __s32 StreamId
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

    if (Stream->State != ALI_DMX_STREAM_STATE_STOP)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);        

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_IN_RAM_STREAM_START, 0);  

    if (Ret < 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }

    Stream->State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxTsInRamStreamStop
(
    __s32 StreamId
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Stream->State != ALI_DMX_STREAM_STATE_RUN)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);        

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_TS_IN_RAM_STREAM_STOP, 0);  

    if (Ret < 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream->State = ALI_DMX_STREAM_STATE_STOP;

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}








__s32 Ali_DmxTsInRamStreamClose
(
    __s32 StreamId
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);        

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

	ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
	
	Ret = close(Stream->Fd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxTsInRamStreamWrite
(
    __s32 StreamId, 
    __u8 *Buf, 
    __u32 BufLen
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || 
		(StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    //Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);        

        return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
    }

    Ret = write(Stream->Fd, Buf, BufLen);

	if (Ret > 0)
	{
        Stream->InByteCnt += Ret;
	}

    //Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_DmxErrCode(Ret));
}


__s32 Ali_DmxTsInRamStreamInfoGet
(
    __s32                            StreamId, 
	struct Ali_DmxTsInRamStreamInfo *TsInRamStreamInfo
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == TsInRamStreamInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);
	
	memset(TsInRamStreamInfo, 0, sizeof(struct Ali_DmxTsInRamStreamInfo));
	
    TsInRamStreamInfo->State = Stream->State;
	
	TsInRamStreamInfo->DmxId = Stream->DmxId;

    TsInRamStreamInfo->Idx = Stream->Idx; /* Idex in struct ALi_DmxPcrStream Streams. */

    TsInRamStreamInfo->Fd = Stream->Fd; /*  ali dmx linux kernel driver file descriptor.*/

	memcpy(&(TsInRamStreamInfo->StreamParam), &(Stream->StreamParam), sizeof(struct Ali_DmxTsInRamStreamParam));

    TsInRamStreamInfo->InByteCnt = Stream->InByteCnt;//!<TS IN RAM Stream总共收到的用户写入的byte总数.

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 Ali_DmxTsInRamStreamErrStatGet
(
    __s32                                 StreamId, 
	struct Ali_DmxLibTsInRamStrmStatInfo *TsInRamStatInfo
)
{
    __s32                        Ret;
    struct ALi_DmxTsInRamStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == TsInRamStatInfo)
	{
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (NULL == Ali_DmxLibTsInRamLayer)
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_TS_IN_RAM, StreamId);

    if ((StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
	    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);

    Stream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);
	
	memcpy(TsInRamStatInfo, &Stream->StatInfo, sizeof(struct Ali_DmxLibTsInRamStrmStatInfo));

    Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}


__s32 ALi_DmxSrcCtrlStreamOpen
(
    __u32 DmxId 
)
{
    __u8                        *TsiPath;
    __u8                        *Addr;
    struct ALi_DmxSrcCtrlStream *Stream;
    __s32                        Idx;
    __s32                        Fd;
    __u32                        ShmSize;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    if (NULL == Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    if (NULL == Ali_DmxLibSrcCtlLayer)
    {
    	ShmSize = sizeof(struct Ali_DmxLibSrcCtlLayer);
    	
    	#ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);	
		#else
    	Addr = Ali_DmxShmGet(ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
    	#endif
    	
    	/* Fetal error. 
    	 */
    	if ((void *)-1 == Addr) 
    	{		
    		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    	
    		//Ali_DmxLibSecLayerExit();
    	
    		return(ALI_ERR_DMX_NOT_INITILAIZED);
    	} 
    	
    	Ali_DmxLibSrcCtlLayer = (struct Ali_DmxLibSrcCtlLayer *)Addr;
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = Ali_DmxLibSrcCtlLayer->Streams;

    for (Idx = 0; Idx < Ali_DmxLibSrcCtlLayer->TotalStreamCnt; Idx++)
    { 
        if (ALI_DMX_STREAM_STATE_IDLE == Stream[Idx].State)
        {
            break;
        }
    }

    if (Idx >= Ali_DmxLibSrcCtlLayer->TotalStreamCnt)
    {
        Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d, Idx = %d, Ali_DmxLibSrcCtlLayer->TotalStreamCnt = %d\n", 
        	__FUNCTION__, __LINE__, Idx, Ali_DmxLibSrcCtlLayer->TotalStreamCnt);
    
        return(ALI_ERR_DMX_SRC_CTRL_STREAM_EXHAUST);
    }

    TsiPath = "/dev/ali_m36_tsi_0";

    Fd = open(TsiPath, O_RDWR | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        //if (EMFILE == errno)
        //{
            //RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
        //}

        Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);
    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_HW_FLT_EXHAUST);
    }    
	
    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);

    Stream[Idx].Fd = Fd;

	Stream[Idx].DmxId = DmxId & 0xF;

    Stream[Idx].State = ALI_DMX_STREAM_STATE_RUN;

    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SRC_CTRL, Idx));
}




__s32 Ali_DmxSrcCtrlStreamClose
(
    __s32 StreamId
)
{
    __s32                        Ret;
    struct ALi_DmxSrcCtrlStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSrcCtlLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SRC_CTRL, StreamId);

    if ((StreamIdx >= Ali_DmxLibSrcCtlLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibSrcCtlLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == Stream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);        

        return(ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED);
    }

	ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
	
	Ret = close(Stream->Fd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Stream->State = ALI_DMX_STREAM_STATE_IDLE;

    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}





__s32 Ali_DmxSrcCtrlStreamWrite
(
    __s32                             StreamId, 
	struct Ali_DmxSrcCtrlStreamParam *Param
)
{
    __s32                            Ret;
    struct ALi_DmxSrcCtrlStream     *Stream;
    struct ali_tsi_input_set_param   InputPortAttr;
    struct ali_tsi_channel_set_param PathSrcAttr;
    struct ali_tsi_output_set_param  DmxSrcAttr;
	__s32                            StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSrcCtlLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SRC_CTRL, StreamId);

    if ((StreamIdx >= Ali_DmxLibSrcCtlLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibSrcCtlLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);        

        return(ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED);
    }

    InputPortAttr.id = Param->InPutPortId;

    InputPortAttr.attribute = Param->InputPortAttr;

    Ret = ioctl(Stream->Fd, ALI_TSI_INPUT_SET, &InputPortAttr);
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}

    PathSrcAttr.channel_id = Param->InputPathId;

    PathSrcAttr.input_id = Param->InPutPortId;
	
	Ret = ioctl(Stream->Fd, ALI_TSI_CHANNEL_SET, &PathSrcAttr);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}
	

	DmxSrcAttr.output_id = Stream->DmxId;

	DmxSrcAttr.channel_id = Param->InputPathId;

    Ret = ioctl(Stream->Fd, ALI_TSI_OUTPUT_SET, &DmxSrcAttr);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}


    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_DmxErrCode(sizeof(struct Ali_DmxSrcCtrlStreamParam)));
}



__s32 Ali_DmxSrcCtrlStreamRead
(
    __s32                             StreamId, 
	struct Ali_DmxSrcCtrlStreamParam *Param
)
{
    __s32                            Ret;
    struct ALi_DmxSrcCtrlStream     *Stream;
    struct ali_tsi_input_set_param   InputPortAttr;
    struct ali_tsi_channel_set_param PathSrcAttr;
    struct ali_tsi_output_set_param  DmxSrcAttr;
	__s32                            StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSrcCtlLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SRC_CTRL, StreamId);

    if ((StreamIdx >= Ali_DmxLibSrcCtlLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Stream = &(Ali_DmxLibSrcCtlLayer->Streams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != Stream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);        

		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED);
    }
	
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	DmxSrcAttr.output_id = Stream->DmxId;

    Ret = ioctl(Stream->Fd, ALI_TSI_OUTPUT_GET, &DmxSrcAttr);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}

    ALI_DMX_LIB_PRINT("%s,%d,DmxId:%d,PathId:%d\n", __FUNCTION__, __LINE__,
		              DmxSrcAttr.output_id, DmxSrcAttr.channel_id);


    PathSrcAttr.channel_id = DmxSrcAttr.channel_id;
	
	Ret = ioctl(Stream->Fd, ALI_TSI_CHANNEL_GET, &PathSrcAttr);

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}

    ALI_DMX_LIB_PRINT("%s,%d,PathId:%d,PortId:%d\n", __FUNCTION__, __LINE__,
		              PathSrcAttr.channel_id, PathSrcAttr.input_id);


    InputPortAttr.id = PathSrcAttr.input_id;
	
    Ret = ioctl(Stream->Fd, ALI_TSI_INPUT_GET, &InputPortAttr);
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		return(Ret);
	}	

    ALI_DMX_LIB_PRINT("%s,%d,PortId:%d,Attr:%x\n", __FUNCTION__, __LINE__,
		              InputPortAttr.id, InputPortAttr.attribute);	

	Param->DmxId = Stream->DmxId;

	Param->InPutPortId = InputPortAttr.id;
	Param->InputPortAttr =InputPortAttr.attribute;

	Param->InputPathId = PathSrcAttr.channel_id;

    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(Ali_DmxErrCode(sizeof(struct Ali_DmxSrcCtrlStreamParam)));
}





__s32 Ali_DmxSrcCtrlStreamInfoGet
(
    __s32                            StreamId, 
	struct Ali_DmxSrcCtrlStreamInfo *SrcCtrlStreamInfo
)
{
    __s32                        Ret;
    struct ALi_DmxSrcCtrlStream *Stream;
	__s32                        StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibSrcCtlLayer)
    {
        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SRC_CTRL, StreamId);

    if ((StreamIdx >= Ali_DmxLibSrcCtlLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    Stream = &(Ali_DmxLibSrcCtlLayer->Streams[StreamIdx]);
	
	memset(SrcCtrlStreamInfo, 0, sizeof(struct Ali_DmxSrcCtrlStreamInfo));
	
    SrcCtrlStreamInfo->State = Stream->State;//!<source control stream的状态.

	SrcCtrlStreamInfo->DmxId = Stream->DmxId;//!<source control stream对应的DMX的ID。

    SrcCtrlStreamInfo->Idx = Stream->Idx;//!<source control stream在内部数组中的Index。	

    SrcCtrlStreamInfo->Fd = Stream->Fd;//!<source control stream对应的linux DMX driver的文件描述符.

    //!<用户配置的source control stream参数. 
	memcpy(&(SrcCtrlStreamInfo->StreamParam), &(Stream->StreamParam), sizeof(struct Ali_DmxSrcCtrlStreamParam));

    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}










__s32 Ali_DmxLibSrcCtlLayerExit
(
    void  
)
{
	__u8				        *Addr;
	struct ALi_DmxSrcCtrlStream *Stream;
	__s32					     Idx;
	__u32					     ShmSize;
	__s32                        Ret;

	ShmSize = sizeof(struct Ali_DmxLibSrcCtlLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
	#else
	Addr = Ali_DmxShmGet(ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
	#endif
	
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibSrcCtlLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibSrcCtlLayer = (struct Ali_DmxLibSrcCtlLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
    /* Cancle all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);

    Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);

    for (Idx = 0; Idx < Ali_DmxLibSrcCtlLayer->TotalStreamCnt; Idx++)
    { 
		Stream = &(Ali_DmxLibSrcCtlLayer->Streams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != Stream->State)
		{
			ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);
			
			Ret = close(Stream->Fd); 
			
			if (Ret < 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
			}
			
			Stream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}







__s32 Ali_DmxLibSrcCtlLayerInit
(
    void  
)
{ 
    struct Ali_DmxLibSrcCtlLayer *Layer;
    struct ALi_DmxSrcCtrlStream  *Stream;
    __u32                         ShmSize;
    union semun                   Option;
    __s32                         SemId;
    __s32                         ShmId;
    __s32                         Idx;
    __u8                         *Addr;
    int tmp = ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_SRC_CTRL_LAYER_SEM_KEY);

    /* First clean all remaining info.
    */
    Ali_DmxLibSrcCtlLayerExit();

    ShmSize = sizeof(struct Ali_DmxLibSrcCtlLayer);
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
	#else	
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
    #endif
    
    /* Fetal error. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp))  
    {      
    	#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured.
     */
    SemId = semget(ALI_DMX_LIB_SRC_CTRL_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(-1);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibSrcCtlLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
	    #ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize);
	    Layer = (struct Ali_DmxLibSrcCtlLayer *)Addr;    

		#else	

    ShmId = shmget(ALI_DMX_LIB_SRC_CTRL_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        //Ali_DmxLibTsLayerExit();

        Ali_DmxSemUnlock(SemId);

        return(-2);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibSrcCtlLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(-3);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibSrcCtlLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_SRC_CTRL_STREAM_CNT;
    } 
    else
    {
        Layer = (struct Ali_DmxLibSrcCtlLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        Stream = &(Layer->Streams[Idx]);

        Stream->State = ALI_DMX_STREAM_STATE_IDLE;

        Stream->Idx = Idx;

        Stream->Fd = -1;
    }   

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}





__s32 Ali_DmxStreamPoll
(
    struct Ali_DmxStreamPollInfo *PollList, 
    __u32                         PollUnitCnt, 
    __s32                         Timeout
)
{
    __u32                        Idx;
    __s32                        Ret;
    __s32                        Fd;
    __s32                        StreamType;
    struct ALi_DmxSecStream     *SecStream;
    struct ALi_DmxTsStream      *TsStream;
	struct ALi_DmxTsInRamStream *TsInRamStream;
    __s32                        StreamId;
	__s32                        StreamIdx;
    struct pollfd                PollFd[ALI_DMX_LIB_STREAM_POLL_CNT];
    __u8                        *Addr;
    __u32                        ShmSize;
	__u16                        Evnent;

    //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (PollUnitCnt >= ALI_DMX_LIB_STREAM_POLL_CNT)
	{		
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_OPER_DENIED);
	}

    //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < PollUnitCnt; Idx++)
    {
        //ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        PollList[Idx].StreamStatus = 0;

        StreamId = PollList[Idx].StreamId;

		StreamType = Ali_StreamId2Type(StreamId);

		if (StreamType < 0)
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    		return(ALI_ERR_DMX_INVALID_ID);
		}

        StreamIdx = Ali_StreamId2Idx(StreamType, StreamId);	

		if (StreamIdx < 0)
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    		return(ALI_ERR_DMX_INVALID_ID);
		}

        switch(StreamType)
        {
            case ALI_DMX_STREAM_TYPE_SEC:
			{	
				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

				if (NULL == Ali_DmxLibSecLayer)
				{
					ShmSize = sizeof(struct Ali_DmxLibSecLayer);
					
					#ifdef ADR_ALIDROID
					Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
					#else	
					Addr = Ali_DmxShmGet(ALI_DMX_LIB_SEC_LAYER_SHM_KEY, ShmSize);
					#endif
					
					/* Fetal error. 
					 */
					if ((void *)-1 == Addr) 
					{
						ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
						//Ali_DmxLibSecLayerExit();
					
						return(ALI_ERR_DMX_NOT_INITILAIZED);
					} 
					
					Ali_DmxLibSecLayer = (struct Ali_DmxLibSecLayer *)Addr;
				}

				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

				if (StreamIdx >= Ali_DmxLibSecLayer->TotalStreamCnt)
			    {
     				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
					return(ALI_ERR_DMX_INVALID_ID);
			    }

				SecStream = &(Ali_DmxLibSecLayer->Streams[StreamIdx]);

				if (ALI_DMX_STREAM_STATE_RUN != SecStream->State)
				{
					ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
					return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
				}
				
				Fd = SecStream->Fd;

				Evnent = POLLIN;

				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
			}
			break;

    		case ALI_DMX_STREAM_TYPE_TS:
    		{
				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

				if (NULL == Ali_DmxLibTsLayer)
				{
					ShmSize = sizeof(struct Ali_DmxLibTsLayer);
					
					#ifdef ADR_ALIDROID
					Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
					#else	
					Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
					#endif
					
					/* Fetal error. 
					 */
					if ((void *)-1 == Addr) 
					{		
						ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
						//Ali_DmxLibSecLayerExit();
					
						return(ALI_ERR_DMX_NOT_INITILAIZED);
					} 
					
					Ali_DmxLibTsLayer = (struct Ali_DmxLibTsLayer *)Addr;
				}

				if (StreamIdx >= Ali_DmxLibTsLayer->TotalStreamCnt)
			    {
				    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
				    
					return(ALI_ERR_DMX_INVALID_ID);
			    }
			    
	            TsStream = &(Ali_DmxLibTsLayer->Streams[StreamIdx]);
	            
				if (ALI_DMX_STREAM_STATE_RUN != TsStream->State)
				{
					ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
					return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
				}

				Fd = TsStream->Fd;
				
				Evnent = POLLIN;
    		}
    		break;

    		case ALI_DMX_STREAM_TYPE_TS_IN_RAM:
    		{
				//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

				if (NULL == Ali_DmxLibTsInRamLayer)
				{
					ShmSize = sizeof(struct Ali_DmxLibTsInRamLayer);
					
					#ifdef ADR_ALIDROID
					Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
					#else	
					Addr = Ali_DmxShmGet(ALI_DMX_LIB_TS_LAYER_SHM_KEY, ShmSize);
					#endif
					
					/* Fetal error. 
					 */
					if ((void *)-1 == Addr) 
					{		
						ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
						//Ali_DmxLibSecLayerExit();
					
						return(ALI_ERR_DMX_NOT_INITILAIZED);
					} 
					
					Ali_DmxLibTsInRamLayer = (struct Ali_DmxLibTsInRamLayer *)Addr;
				}

				if (StreamIdx >= Ali_DmxLibTsInRamLayer->TotalStreamCnt)
			    {
				    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
				    
					return(ALI_ERR_DMX_INVALID_ID);
			    }

				TsInRamStream = &(Ali_DmxLibTsInRamLayer->Streams[StreamIdx]);

				if (ALI_DMX_STREAM_STATE_RUN != TsInRamStream->State)
				{
					ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
					
					return(ALI_ERR_DMX_TS_IN_RAM_STREAM_OPER_DENIED);
				}
				
				Fd = TsInRamStream->Fd;

				Evnent = POLLOUT;
    		}
    		break;

    		default:
    		{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

				return(ALI_ERR_DMX_INVALID_ID);
    		}
    		break;
		}

        PollFd[Idx].fd = Fd;

        PollFd[Idx].events = Evnent;
    }	

	//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	Ret = poll(PollFd, PollUnitCnt, Timeout);
	
	//ALI_DMX_LIB_PRINT("%s,%d,Ret:0x%x\n", __FUNCTION__, __LINE__, Ret);

	for (Idx = 0; Idx < PollUnitCnt; Idx++)
	{
		PollList[Idx].StreamStatus = Ali_DmxErrCode(PollFd[Idx].revents);
	}

	//ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
	return(Ali_DmxErrCode(Ret));
}


#if 0
__s32 ALi_DmxDevInfoGet
(
    UINT32                 DmxId,
    struct ALi_DmxDevInfo *DmxDevInfo
)
{
    __u8  *DmxPath;
    __s32  Fd;
	__s32  Ret;
	struct ALi_DmxDrvOutputDevInfo DrvOutputDevInfo;

    DmxPath = Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId);
    
    if (NULL == DmxPath)
    {
    	Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
    	return(ALI_ERR_DMX_INVALID_ID);
    }
    
    Fd = open(DmxPath, O_RDONLY | O_CLOEXEC);
    
    if (Fd < 0)
    {
    	//if (EMFILE == errno)
    	//{
    		//RetCode = ALI_ERR_DMX_SEC_STREAM_EXHAUST;
    	//}
        
    	ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Fd);
    
    	return(ALI_ERR_DMX_OPER_DENIED);
    }

    Ret = ioctl(Fd, ALI_DMX_OUTPUT_DEV_CTRL_STREAM_CFG, 0);  

    if (Ret < 0)
    {    
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_OPER_DENIED);
    }    
    
    Ret = ioctl(Fd, ALI_DMX_OUTPUT_DEV_CTRL_STREAM_START, 0);  
    
    if (Ret < 0)
    {	 
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
    	return(ALI_ERR_DMX_OPER_DENIED);
    }  

    Ret = ioctl(Fd, ALI_DMX_OUTPUT_DEV_CTRL_STREAM_DEV_INFO_GET,
		        &DrvOutputDevInfo);  
    
    if (Ret < 0)
    {	 
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
    	return(ALI_ERR_DMX_OPER_DENIED);
    }  	

    DmxDevInfo->OutputDevTsRcvCnt = DrvOutputDevInfo.TsInCnt;

    Ret = ioctl(Fd, ALI_DMX_OUTPUT_DEV_CTRL_STREAM_STOP, 0);  
    
    if (Ret < 0)
    {	 
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
    	return(ALI_ERR_DMX_OPER_DENIED);
    }  	

    close(Fd); 		

	return(0);
}
#endif


__s32 ALi_DmxSysInfoGet
(
	struct ALi_DmxSysInfo *DmxSysInfo
)
{
    __s32 Ret;
	__s32 Idx;
	
    memset(DmxSysInfo, 0, sizeof(struct ALi_DmxSysInfo));

    if (Ali_DmxLibSecLayer != NULL)
    {
        Ret = Ali_DmxSecSemCondLock();
    
        if (Ret < 0)
        {
            ALI_DMX_LIB_PRINT("%s,%d,%d\n", __FUNCTION__, __LINE__, Ret);
    
            return(ALI_ERR_DMX_NOT_INITILAIZED);
        } 

        DmxSysInfo->SecStreamCntTotal = Ali_DmxLibSecLayer->TotalStreamCnt;

		for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalStreamCnt; Idx++)
		{
            if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibSecLayer->Streams[Idx].State)
            {
                DmxSysInfo->SecStreamCntFree++;
			}
		}

        DmxSysInfo->SecSlotCntTotal = Ali_DmxLibSecLayer->TotalSlotCnt;

		for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalSlotCnt; Idx++)
		{
            if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibSecLayer->Slots[Idx].State)
            {
                DmxSysInfo->SecSlotCntFree++;
			}
		}	
		
        DmxSysInfo->SecChCntTotal = Ali_DmxLibSecLayer->TotalChCnt;

		for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalChCnt; Idx++)
		{
            if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibSecLayer->Chs[Idx].State)
            {
                DmxSysInfo->SecChCntFree++;
			}
		}

        DmxSysInfo->SecFltCntTotal = Ali_DmxLibSecLayer->TotalFltCnt;

		for (Idx = 0; Idx < Ali_DmxLibSecLayer->TotalFltCnt; Idx++)
		{
            if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibSecLayer->Flts[Idx].State)
            {
                DmxSysInfo->SecFltCntFree++;
			}
		}

		Ali_DmxSecSemCondUnlock();
	}

	if (Ali_DmxLibVideoLayer != NULL)
    {
		Ali_DmxSemLock(Ali_DmxLibVideoLayer->SemId);
     
        DmxSysInfo->VideoStreamCntTotal = Ali_DmxLibVideoLayer->TotalStreamCnt;

		for (Idx = 0; Idx < Ali_DmxLibVideoLayer->TotalStreamCnt; Idx++)
		{
            if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibVideoLayer->Streams[Idx].State)
            {
                DmxSysInfo->VideoStreamCntFree++;
			}
		}

		Ali_DmxSemUnlock(Ali_DmxLibVideoLayer->SemId);
	}

    if (Ali_DmxLibAudioLayer != NULL)
    {
    	Ali_DmxSemLock(Ali_DmxLibAudioLayer->SemId);
     
    	DmxSysInfo->AudioStreamCntTotal = Ali_DmxLibAudioLayer->TotalStreamCnt;
    
    	for (Idx = 0; Idx < Ali_DmxLibAudioLayer->TotalStreamCnt; Idx++)
    	{
    		if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibAudioLayer->Streams[Idx].State)
    		{
    			DmxSysInfo->AudioStreamCntFree++;
    		}
    	}
    
    	Ali_DmxSemUnlock(Ali_DmxLibAudioLayer->SemId);
    }

    if (Ali_DmxLibPcrLayer != NULL)
    {
    	Ali_DmxSemLock(Ali_DmxLibPcrLayer->SemId);
     
    	DmxSysInfo->PcrStreamCntTotal = Ali_DmxLibPcrLayer->TotalStreamCnt;
    
    	for (Idx = 0; Idx < Ali_DmxLibPcrLayer->TotalStreamCnt; Idx++)
    	{
    		if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibPcrLayer->Streams[Idx].State)
    		{
    			DmxSysInfo->PcrStreamCntFree++;
    		}
    	}
    
    	Ali_DmxSemUnlock(Ali_DmxLibPcrLayer->SemId);
    }

    if (Ali_DmxLibTsLayer != NULL)
    {
    	Ali_DmxSemLock(Ali_DmxLibTsLayer->SemId);
     
    	DmxSysInfo->TsStreamCntTotal = Ali_DmxLibTsLayer->TotalStreamCnt;
    
    	for (Idx = 0; Idx < Ali_DmxLibTsLayer->TotalStreamCnt; Idx++)
    	{
    		if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibTsLayer->Streams[Idx].State)
    		{
    			DmxSysInfo->TsStreamCntFree++;
    		}
    	}
    
    	Ali_DmxSemUnlock(Ali_DmxLibTsLayer->SemId);
    }

    if (Ali_DmxLibTsInRamLayer != NULL)
    {
    	Ali_DmxSemLock(Ali_DmxLibTsInRamLayer->SemId);
     
    	DmxSysInfo->TsInRamStreamCntTotal = Ali_DmxLibTsInRamLayer->TotalStreamCnt;
    
    	for (Idx = 0; Idx < Ali_DmxLibTsInRamLayer->TotalStreamCnt; Idx++)
    	{
    		if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibTsInRamLayer->Streams[Idx].State)
    		{
    			DmxSysInfo->TsInRamStreamCntFree++;
    		}
    	}
    
    	Ali_DmxSemUnlock(Ali_DmxLibTsInRamLayer->SemId);
    }

    if (Ali_DmxLibSrcCtlLayer != NULL)
    {
    	Ali_DmxSemLock(Ali_DmxLibSrcCtlLayer->SemId);
     
    	DmxSysInfo->SrcCtrlStreamCntTotal= Ali_DmxLibSrcCtlLayer->TotalStreamCnt;
    
    	for (Idx = 0; Idx < Ali_DmxLibSrcCtlLayer->TotalStreamCnt; Idx++)
    	{
    		if (ALI_DMX_STREAM_STATE_IDLE == Ali_DmxLibSrcCtlLayer->Streams[Idx].State)
    		{
    			DmxSysInfo->SrcCtrlStreamCntFree++;
    		}
    	}
    
    	Ali_DmxSemUnlock(Ali_DmxLibSrcCtlLayer->SemId);
    }	

    return(ALI_ERR_DMX_OK);
}



static __u8 DmxLibRealTimePrint;

void Ali_DmxLibRealTimePrintSet(__u8 Rt)
{
	DmxLibRealTimePrint = Rt;

	return;
}

int Ali_DmxDscKeyShow(const char *fmt, ...)
{
	va_list args;
	int n;

	if (!DmxLibRealTimePrint)
    {
        return(0);
    }

	va_start(args, fmt);
	n = vprintf(fmt, args);
	va_end(args);

	return n;
}

__s32 Ali_DmxDescramKeyOpen
(
	__u32                                DmxId,
    struct Ali_DmxDescramKeyStreamParam *Param
)
{
	__u8						   *Addr;
	struct Ali_DmxDescramKeyStream *Stream;
	__s32							Idx;
	__s32							Ret;
	__u32							ShmSize;
	KEY_PARAM						CsaParamList;
	struct Ali_DmxDescramKeyStream *KeyStream;
	__s32							SedDmxFd;

	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (NULL == Param)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

    if (Param->Pid > 0x1FFF)
    {
        return(ALI_ERR_DMX_INVALID_PARAM);
	}
	
    /* DmxId check.
     */
    if (NULL == Ali_DmxId2Path(ALI_DMX_OUTPUT, DmxId))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }
	
	if (NULL == Ali_DmxLibDescramLayer)
	{
		ShmSize = sizeof(struct Ali_DmxLibDescramLayer);
		
		#ifdef ADR_ALIDROID
		Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
		#else	
		Addr = Ali_DmxShmGet(ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
		#endif
		
		/* Fetal error. 
		 */
		if ((void *)-1 == Addr) 
		{		
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
			//Ali_DmxLibSecLayerExit();
		
			return(ALI_ERR_DMX_NOT_INITILAIZED);
		} 
		
		Ali_DmxLibDescramLayer = (struct Ali_DmxLibDescramLayer *)Addr;
	}

	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);

	Ret = ALI_ERR_DMX_DESCRAM_KEY_STREAM_EXHAUST;

	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	for (Idx = 0; Idx < Ali_DmxLibDescramLayer->TotalStreamCnt; Idx++)
	{
		KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_RUN == KeyStream->State)
		{
			break;
		}
	}

	if (Idx >= Ali_DmxLibDescramLayer->TotalStreamCnt)
	{
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
    	SedDmxFd = open("/dev/ali_m36_dmx_see_0", O_RDWR | O_CLOEXEC);
    	
    	if (SedDmxFd < 0)
    	{		 
    		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    	}    
    
    	Ret = ioctl(SedDmxFd , ALI_DMX_SEE_CRYPTO_START, 0);
    	
    	if (Ret< 0)
    	{		 
    		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    	}
    
        ALI_DMX_LIB_PRINT("%s,%d,SedDmxFd:%d\n", __FUNCTION__, __LINE__, SedDmxFd);
    
    	Ret = close(SedDmxFd); 
    	
    	if (Ret < 0)
    	{
    		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
    	}
	}

	for (Idx = 0; Idx < Ali_DmxLibDescramLayer->TotalStreamCnt; Idx++)
	{
		KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_RUN == KeyStream->State)
		{
			if (Param->Pid == KeyStream->Param.Pid)
			{
				KeyStream->RefCnt++;

				Ret = Idx;
				
				goto DONE;
			}
		}
	}

	for (Idx = 0; Idx < Ali_DmxLibDescramLayer->TotalStreamCnt; Idx++)
	{
		KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE == KeyStream->State)
		{
    		memset(&CsaParamList, 0, sizeof(KEY_PARAM));
			
			CsaParamList.ctr_counter = NULL;
			CsaParamList.force_mode = 0;
			CsaParamList.handle = 0xFF;
			CsaParamList.init_vector = NULL;
			CsaParamList.key_length = 64;
			CsaParamList.pid_len = 1;
			CsaParamList.pid_list = &(Param->Pid);
			
			CsaParamList.stream_id = 0;
			CsaParamList.p_csa_key_info = (CSA_KEY_PARAM *)(&KeyStream->Key);
			
			Ret = csa_ioctl(Ali_DmxLibDescramLayer->CsaDevPtr, 
							IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&CsaParamList);
			
			if(Ret != 0)
			{
				ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
				
				Ret = ALI_ERR_DMX_DESCRAM_DRV_ERR;
				
				goto DONE;
			}

			KeyStream->RefCnt = 1;

			KeyStream->CsaDrvHandler = CsaParamList.handle;
						
			memcpy(&(KeyStream->Param), Param, 
			       sizeof(struct Ali_DmxDescramKeyStreamParam));
			
			KeyStream->State = ALI_DMX_STREAM_STATE_RUN;
			
			Ret = Idx;
			
			goto DONE;
		}
	}

DONE:
	Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);

	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	if (Ret < 0)
	{
		return(Ret);
	}

	return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_DESCRAM_KEY, Ret));
}




__s32 Ali_DmxDescramKeyClose
(
	__s32 KeyStreamId
)
{
    __s32                           Ret;
	struct Ali_DmxDescramKeyStream *KeyStream;
	__s32                           StreamIdx;
    __s32                           SedDmxFd;
	__s32                           Idx;
	
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibDescramLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_DESCRAM_KEY, KeyStreamId);

    if ((StreamIdx >= Ali_DmxLibDescramLayer->TotalStreamCnt) ||
		(StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }	

    Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_IDLE == KeyStream->State)
    {
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);        

        return(ALI_ERR_DMX_SRC_CTRL_STREAM_OPER_DENIED);
    }

	KeyStream->RefCnt--;

	if (KeyStream->RefCnt > 0)
	{
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);        
	
		return(ALI_ERR_DMX_OK);
	}

    Ret = csa_ioctl(Ali_DmxLibDescramLayer->CsaDevPtr,
		            IO_DELETE_CRYPT_STREAM_CMD, KeyStream->CsaDrvHandler);
    
    if(Ret != 0)
    {
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);   
		
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
		return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
    }

    KeyStream->State = ALI_DMX_STREAM_STATE_IDLE;

	for (Idx = 0; Idx < Ali_DmxLibDescramLayer->TotalStreamCnt; Idx++)
	{
		KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_IDLE != KeyStream->State)
		{
			break;
		}
	}

	if (Idx >= Ali_DmxLibDescramLayer->TotalStreamCnt)
	{
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
    	SedDmxFd = open("/dev/ali_m36_dmx_see_0", O_RDWR | O_CLOEXEC);
    	
    	if (SedDmxFd < 0)
    	{		 
    		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    	}    
    
    	Ret = ioctl(SedDmxFd , ALI_DMX_SEE_CRYPTO_STOP, 0);
    	
    	if (Ret< 0)
    	{		 
    		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    	}
    
        ALI_DMX_LIB_PRINT("%s,%d,SedDmxFd:%d\n", __FUNCTION__, __LINE__, SedDmxFd);
    
    	Ret = close(SedDmxFd); 
    	
    	if (Ret < 0)
    	{
    		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
    	}
	}
	
    Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);    

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}




__s32 Ali_DmxDescramKeySetEven
(
	__u32  KeyStreamId, 
	__u8  *EvenKey
)
{
    __s32                           Ret;
	__s32                           StreamIdx;
	__s32							KeyLen;
	KEY_PARAM                       CsaParamList;
	struct Ali_DmxDescramKeyStream *KeyStream;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibDescramLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_DESCRAM_KEY, KeyStreamId);

    if ((StreamIdx >= Ali_DmxLibDescramLayer->TotalStreamCnt) || 
		(StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);

    KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != KeyStream->State)
    {     
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);    
		
		ALI_DMX_LIB_PRINT("%s,%d,State:%d\n", __FUNCTION__, __LINE__, KeyStream->State);

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	memcpy(KeyStream->Key.EvenKey, EvenKey, 8);

	memset(&CsaParamList, 0, sizeof(KEY_PARAM));
	
	CsaParamList.handle = KeyStream->CsaDrvHandler;
	CsaParamList.pid_list = &(KeyStream->Param.Pid);		
	CsaParamList.p_csa_key_info = (CSA_KEY_PARAM *)(&KeyStream->Key);
	CsaParamList.stream_id = 0 ;
	CsaParamList.pid_len = 1;
	CsaParamList.force_mode = 0;

	Ali_DmxDscKeyShow("Pid:%u,StrmIdx:%d,Even CW:", KeyStream->Param.Pid, StreamIdx);

	KeyLen = 8;
	
	while (KeyLen > 0)
	{
		Ali_DmxDscKeyShow("%02x ", KeyStream->Key.EvenKey[--KeyLen]);
	}

	Ali_DmxDscKeyShow("@%u,", osal_get_tick());
	
	Ret = csa_ioctl(Ali_DmxLibDescramLayer->CsaDevPtr, IO_KEY_INFO_UPDATE_CMD,
	                (UINT32)&CsaParamList);
	                
	if(Ret != 0)
	{
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId); 	

		Ali_DmxDscKeyShow("Fail,%d.\n", Ret);
	
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
		return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
	}

	Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);

	Ali_DmxDscKeyShow("Done.\n");

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxDescramKeySetOdd
(
	__u32  KeyStreamId, 
	__u8  *OddKey
)
{
    __s32                           Ret;
	__s32                           StreamIdx;
	__s32							KeyLen;
	KEY_PARAM                       CsaParamList;
	struct Ali_DmxDescramKeyStream *KeyStream;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibDescramLayer)
    {
        return(ALI_ERR_DMX_NOT_INITILAIZED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_DESCRAM_KEY, KeyStreamId);

    if ((StreamIdx >= Ali_DmxLibDescramLayer->TotalStreamCnt) || 
		(StreamIdx < 0))
    {
        return(ALI_ERR_DMX_INVALID_ID);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);

    KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[StreamIdx]);

    if (ALI_DMX_STREAM_STATE_RUN != KeyStream->State)
    {     
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);    
		
		ALI_DMX_LIB_PRINT("%s,%d,State:%d\n", __FUNCTION__, __LINE__, KeyStream->State);

        return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	memcpy(KeyStream->Key.OddKey, OddKey, 8);
	
	memset(&CsaParamList, 0, sizeof(KEY_PARAM));
	
	CsaParamList.handle = KeyStream->CsaDrvHandler;
	CsaParamList.pid_list = &(KeyStream->Param.Pid);	
	CsaParamList.p_csa_key_info = (CSA_KEY_PARAM *)(&KeyStream->Key);
	CsaParamList.stream_id = 0 ;
	CsaParamList.pid_len = 1;
	CsaParamList.force_mode = 0;

    Ali_DmxDscKeyShow("Pid:%u,StrmIdx:%d,Odd CW:", KeyStream->Param.Pid, StreamIdx);

	KeyLen = 8;
	
	while (KeyLen > 0)
	{
		Ali_DmxDscKeyShow("%02x ", KeyStream->Key.EvenKey[--KeyLen]);
	}

	Ali_DmxDscKeyShow("@%u,", osal_get_tick());
	
	Ret = csa_ioctl(Ali_DmxLibDescramLayer->CsaDevPtr, IO_KEY_INFO_UPDATE_CMD,
	                (UINT32)&CsaParamList);
	                
	if(Ret != 0)
	{
        Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId); 	

		Ali_DmxDscKeyShow("Fail,%d.\n", Ret);
	
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
		return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
	}

	Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);

	Ali_DmxDscKeyShow("Done.\n");

    return(ALI_ERR_DMX_OK);
}





__s32 Ali_DmxDescramLayerExit
(
    void  
)
{
    __s32                           Ret;
	__u8					       *Addr;
	struct Ali_DmxDescramKeyStream *KeyStream;
	__s32					        Idx;
	__u32					        ShmSize;
    __s32                           SedDmxFd;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);	

	ShmSize = sizeof(struct Ali_DmxLibDescramLayer);
	
	#ifdef ADR_ALIDROID
	if (-1 == ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY)
	{
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	}
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
	#else	
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
	#endif
	/* Fetal error. 
	 */
	if ((void *)-1 == Addr) 
	{		
		ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, Ali_DmxLibDescramLayer);
	
		//Ali_DmxLibSecLayerExit();
	
		return(ALI_ERR_DMX_NOT_INITILAIZED);
	} 
	
	Ali_DmxLibDescramLayer = (struct Ali_DmxLibDescramLayer *)Addr;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* Cancel all locked mutex.
	*/
    Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);
	
    Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);

    /* In case we are recovering from APP crush.
	*/
	dsc_api_attach();
	
	Ali_DmxLibDescramLayer->CsaDevPtr = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

    if(NULL == Ali_DmxLibDescramLayer->CsaDevPtr)
    {    
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
		return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
	}	

    /* Close all remaining descramble channel at SEE.
	*/
    for (Idx = 0; Idx < Ali_DmxLibDescramLayer->TotalStreamCnt; Idx++)
    { 
		KeyStream = &(Ali_DmxLibDescramLayer->KeyStreams[Idx]);
		
		if (ALI_DMX_STREAM_STATE_RUN == KeyStream->State)
		{
			Ret = csa_ioctl(Ali_DmxLibDescramLayer->CsaDevPtr,
							IO_DELETE_CRYPT_STREAM_CMD, KeyStream->CsaDrvHandler);
			
			if(Ret != 0)
			{
				Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);   
				
                ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
				
				return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
			}

			KeyStream->State = ALI_DMX_STREAM_STATE_IDLE;
		}
    }

    SedDmxFd = open("/dev/ali_m36_dmx_see_0", O_RDWR | O_CLOEXEC);
    
    if (SedDmxFd < 0)
    {        
        ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    }   
	
    Ret = ioctl(SedDmxFd , ALI_DMX_SEE_CRYPTO_STOP, 0);
    
    if (Ret< 0)
    {        
        ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d,SedDmxFd:%d\n", __FUNCTION__, __LINE__, SedDmxFd);
	
	Ret = close(SedDmxFd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}



__s32 Ali_DmxDescramLayerInit
(
    void  
)
{ 
    struct Ali_DmxLibDescramLayer  *Layer;
	struct Ali_DmxDescramKeyStream *KeyStream;
    __u32                           ShmSize;
    union semun                     Option;
    __s32                           SemId;
    __s32                           ShmId;
    __s32                           Idx;
	__s32                           Ret;
	CSA_INIT_PARAM                  CsaDrvInitParam;
	struct dec_parse_param          SedDmxDscParam;
	__s32                           SedDmxFd;
    __u8                           *Addr;
    int tmp = ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);	

    /* First clean all remaining info.
    */
    Ret = Ali_DmxDescramLayerExit();

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    ShmSize = sizeof(struct Ali_DmxLibDescramLayer);
	#ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
	#else
    Addr = Ali_DmxShmGet(ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
	#endif
    /* If share resouce not inited. 
     */
    if (((void *)-1 == Addr) || (-1 == tmp)) 
    {
    	ALI_DMX_LIB_PRINT("%s,%d,%x\n", __FUNCTION__, __LINE__, ALI_DMX_LIB_DESCRAM_LAYER_SEM_KEY);

		#ifdef ADR_ALIDROID
     		SemId = OS_CreateSemaphore(1);
     		if (SemId < 0)
		    {
		        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

		            return(ALI_ERR_DMX_TS_STREAM_OPER_DENIED);
		    }
     	#else
    /* semget returns -1 if the key already exits while IPC_EXCL 
     * is configured.
     */
    SemId = semget(ALI_DMX_LIB_DESCRAM_LAYER_SEM_KEY, 1, 
                   IPC_CREAT | IPC_EXCL | 0x777);

    /* Make sure that our section stream layer only be inited once.
     */
    if (SemId < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_DESCRAM_KEY_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Option.val = 1;

    semctl(SemId, 0, SETVAL, Option);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		#endif

    Ali_DmxSemLock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    ShmSize = sizeof(struct Ali_DmxLibDescramLayer);

    /* shmget returns -1 if key already exits when IPC_EXCL is configured.
     */
    #ifdef ADR_ALIDROID
	Addr = Ali_DmxAShmGet(&ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize);
    Layer = (struct Ali_DmxLibDescramLayer *)Addr;

	#else

    ShmId = shmget(ALI_DMX_LIB_DESCRAM_LAYER_SHM_KEY, ShmSize, 
                   IPC_CREAT | IPC_EXCL | 0x777); 
    
    if (ShmId < 0)
    {
        /* Should never happen. 
         * If it doeshappen,  something must be seriously wrong, close slot 
         * layer at all. 
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        //Ali_DmxLibTsLayerExit();

        Ali_DmxSemUnlock(SemId);

            return(ALI_ERR_DMX_DESCRAM_KEY_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer = (struct Ali_DmxLibDescramLayer *)(shmat(ShmId, (void *)0, 0));
#endif
    if ((void *)-1 == Layer) 
    {
        Ali_DmxSemUnlock(SemId);

        /* Make sure that our section stream layer could only be inited once.
         */
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

            return(ALI_ERR_DMX_DESCRAM_KEY_STREAM_OPER_DENIED);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    memset(Layer, 0, sizeof(struct Ali_DmxLibDescramLayer));

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Layer->SemId = SemId;

    Layer->ShmId = ShmId; 

    Layer->TotalStreamCnt = ALI_DMX_LIB_DESCRAM_KEY_STREAM_CNT;
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }
    else
    {
    	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        Layer = (struct Ali_DmxLibDescramLayer *)Addr;

        SemId = Layer->SemId;
        
        Ali_DmxSemLock(SemId);
    }
	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

	/* Init CSA command.
	*/
	dsc_api_attach();
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
	Layer->CsaDevPtr = (pCSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

    if(NULL == Layer->CsaDevPtr)
    {
		Ali_DmxSemUnlock(SemId);
    
		ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    
		return(ALI_ERR_DMX_DESCRAM_DRV_ERR);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    CsaDrvInitParam.Dcw[0] = 0;
    CsaDrvInitParam.Dcw[1] = 1;
    CsaDrvInitParam.Dcw[2] = 2;
    CsaDrvInitParam.Dcw[3] = 3;
    CsaDrvInitParam.dma_mode = TS_MODE;
    CsaDrvInitParam.key_from = KEY_FROM_SRAM;
    CsaDrvInitParam.parity_mode = AUTO_PARITY_MODE0;
    CsaDrvInitParam.pes_en = 1;			 /*not used now*/
    CsaDrvInitParam.scramble_control = 0;	/*dont used default CW*/
    CsaDrvInitParam.stream_id = 0;//sourceid;
    CsaDrvInitParam.version = CSA2;//csa_version;
    
    Ret = csa_ioctl(Layer->CsaDevPtr, IO_INIT_CMD, (UINT32)&CsaDrvInitParam);
    
    if(Ret != 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
    }

	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);


	/* Order SEE dmx pass TS pakcet through descrambler.
	*/
	SedDmxDscParam.dec_dev = Layer->CsaDevPtr->priv;
	SedDmxDscParam.type= CSA;

	SedDmxFd = open("/dev/ali_m36_dmx_see_0", O_RDWR | O_CLOEXEC);
	
	if (SedDmxFd < 0)
	{		 
		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
	}    
	
	Ret = ioctl(SedDmxFd, ALI_DMX_SEE_CRYPTO_TYPE_SET, &SedDmxDscParam);
	
	if (Ret < 0)
	{		 
		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
	}

#if 0	
	Ret = ioctl(SedDmxFd , ALI_DMX_SEE_CRYPTO_START, 0);
	
	if (Ret< 0)
	{		 
		ALI_DMX_LIB_PRINT("%s,%d,ret:%d\n", __FUNCTION__, __LINE__, Ret);
	}

    ALI_DMX_LIB_PRINT("%s,%d,SedDmxFd:%d\n", __FUNCTION__, __LINE__, SedDmxFd);
#endif

	Ret = close(SedDmxFd); 
	
	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    /* Init internal key stream slot.
	*/
    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    for (Idx = 0; Idx < Layer->TotalStreamCnt; Idx++)
    {
        KeyStream = &(Layer->KeyStreams[Idx]);

        KeyStream->State = ALI_DMX_STREAM_STATE_IDLE;

        KeyStream->Idx = Idx;

        KeyStream->RefCnt = 0;
    }  
	
	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    #ifdef ADR_ALIDROID
		munmap((void *)Layer, ashmem_get_size_region(Layer->ShmId));
	#else
    	shmdt((void *)Layer);
	#endif

    Ali_DmxSemUnlock(SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}

__s32 Ali_DmxDescramKeyInfoGet
(
	struct Ali_DmxDescramKeyStreamParam  KeyStreamInfo
)
{
	__s32                   StreamIdx = -1;
    __u32					i = 0;
	struct Ali_DmxDescramKeyStream *Stream;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (NULL == Ali_DmxLibDescramLayer)
    {
        return(ALI_ERR_DMX_SEC_STREAM_OPER_DENIED);
    }
  

    Ali_DmxSemLock(Ali_DmxLibDescramLayer->SemId);


	for( i = 0; i < ALI_DMX_LIB_DESCRAM_KEY_STREAM_CNT; i++)
	{
	
        Stream = &(Ali_DmxLibDescramLayer->KeyStreams[i]);
   
		if(KeyStreamInfo.Pid == Stream->Param.Pid)
		{
			StreamIdx = Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_DESCRAM_KEY, i);

            break;
		}  
    }     

	
    Ali_DmxSemUnlock(Ali_DmxLibDescramLayer->SemId);

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(StreamIdx);
}



__u8* Ali_DmxId2PathSubt
(
    __u32 DmxId
)
{
	ALI_DMX_LIB_PRINT("%s,%d,DmxId:%d\n", __FUNCTION__, __LINE__, DmxId);

	switch(DmxId)
	{
	    /* ID 0 and ID 10000 map to same ALI_HWDMX0_OUTPUT_PATH.
	     * Since ID 0~2 may be not enough for CHIPs that have more than 3 HW DMXs, 
	     * so we map HW DMX start form 10000, of cuase the the old ID is still supported.
		*/
		case 0:
		case 10000:
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return("/dev/ali_hwdmx0_subt");
		}
		break;
	
		case 1:
		case 10001:				
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return("/dev/ali_hwdmx1_subt");
		}
		break;
	
		case 2:
		case 10002:						
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return("/dev/ali_hwdmx2_subt");
		}
		break;

		case 10003:						
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return("/dev/ali_hwdmx3_subt");
		}
		break;			

	    /* ID 3 and ID 20000 map to same ALI_SWDMX0_OUTPUT_PATH.
	     * Since ID 0~2 may be not enough for CHIPs that have more than 3 HW DMXs, 
	     * so we map SW DMX start form 10000, of cuase the the old ID is still supported.
		*/		
		case 3:
		case 20000:				
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return("/dev/ali_swdmx0_subt");
		}
		break;
	
		default:
		{
			ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
	
			return(NULL);
		}
	} 

 	ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
 
    return(NULL);
}





__s32 Ali_DmxSubtStreamOpen
(
    __u32 DmxId 
)
{
    __u8                         *DmxPath;
    struct ALi_DmxVideoStream    *Stream;
    __s32                         Ret;
    __s32                         Fd;
    __u32                         ShmSize;
    __u8                         *Addr;
	__s32                         Err;
    struct Ali_DmxSubtStreamInfo  StreamInfo;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    /* DmxId check.
     */
    DmxPath = Ali_DmxId2PathSubt(DmxId);

	if (NULL == DmxPath)
	{
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
	}

    Fd = open(DmxPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

    if (Fd < 0)
    {
        Err = errno;
		
        ALI_DMX_LIB_PRINT("%s,%d,err:%m\n", __FUNCTION__, __LINE__, Err);

        return(Err);
    }  

    Ret = ioctl(Fd, ALI_DMX_SUBT_STREAM_INFO_GET, &StreamInfo);  

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    if (Ret < 0)
    {
        Err = errno;
		
        ALI_DMX_LIB_PRINT("%s,%d,err:%m\n", __FUNCTION__, __LINE__, Err);

        return(Err);
    }	
	
	Ali_DmxLibSubtLayer.Streams[StreamInfo.StreamIdx].Fd = Fd;

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d,StreamIdx:%d,ID:0x%x,Fd:%d\n", __FUNCTION__, __LINE__,
		   Fd, StreamInfo.StreamIdx, Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SUBT, StreamInfo.StreamIdx),
		   Ali_DmxLibSubtLayer.Streams[StreamInfo.StreamIdx].Fd);
	
    return(Ali_StreamIdx2Id(ALI_DMX_STREAM_TYPE_SUBT, StreamInfo.StreamIdx));
}







__s32 Ali_DmxSubtStreamCfg  
(
    __s32                          StreamId, 
    struct Ali_DmxSubtStreamParam *StreamParam
)
{ 
    __s32                     Ret;   
    struct ALi_DmxSubtStream *Stream;
	__s32                     StreamIdx;
	__s32                     Err;

    ALI_DMX_LIB_PRINT("%s,%d,StreamId:0x%x\n", __FUNCTION__, __LINE__, StreamId);

	if (NULL == StreamParam)
	{
        return(ALI_ERR_DMX_INVALID_PARAM);
	}

	if (StreamParam->Pid > 0x1FFF)
	{
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);
		
        return(ALI_ERR_DMX_INVALID_PID);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SUBT, StreamId);

    if ((StreamIdx >= ALI_DMX_LIB_SUBT_STREAM_CNT) || (StreamIdx < 0))
    {
        DMX_LIB_DBG_ERR("%s,%d,StreamId:0x%xStreamIdx:0x%x\n", __FUNCTION__, __LINE__, StreamId, StreamIdx);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Stream = &(Ali_DmxLibSubtLayer.Streams[StreamIdx]);

    ALI_DMX_LIB_PRINT("%s,%d,StreamIdx:%d,Fd:%d\n", __FUNCTION__, __LINE__, StreamIdx, Stream->Fd);

    Ret = ioctl(Stream->Fd, ALI_DMX_SUBT_STREAM_CFG, StreamParam);  

    if (Ret < 0)
    {
        Err = errno;
		
        DMX_LIB_DBG_ERR("%s,%d,err:%m\n", __FUNCTION__, __LINE__, Err);

        return(Err);
    }	

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}








__s32 Ali_DmxSubtStreamStart
(
    __s32 StreamId
)
{
    __s32                     Ret;
    struct ALi_DmxSubtStream *Stream;
	__s32                     StreamIdx;
	__s32                     Err;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SUBT, StreamId);

    if ((StreamIdx >= ALI_DMX_LIB_SUBT_STREAM_CNT) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Stream = &(Ali_DmxLibSubtLayer.Streams[StreamIdx]);

    if (Stream->Fd < 0)
    {
        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
	}

    Ret = ioctl(Stream->Fd, ALI_DMX_SUBT_STREAM_START, 0);  

    if (Ret < 0)
    {
        Err = errno;
		
        ALI_DMX_LIB_PRINT("%s,%d,err:%m\n", __FUNCTION__, __LINE__, Err);

        return(Err);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}








__s32 Ali_DmxSubtStreamStop
(
    __s32 StreamId
)
{
    __s32                     Ret;
    struct ALi_DmxSubtStream *Stream;
	__s32                     StreamIdx;
	__s32                     Err;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SUBT, StreamId);

    if ((StreamIdx >= ALI_DMX_LIB_SUBT_STREAM_CNT) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Stream = &(Ali_DmxLibSubtLayer.Streams[StreamIdx]);

    if (Stream->Fd < 0)
    {
        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
	}

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    Ret = ioctl(Stream->Fd, ALI_DMX_SUBT_STREAM_STOP, 0);  

    if (Ret < 0)
    {
        Err = errno;
		
        ALI_DMX_LIB_PRINT("%s,%d,err:%m\n", __FUNCTION__, __LINE__, Err);

        return(Err);
    }

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    return(ALI_ERR_DMX_OK);
}






__s32 Ali_DmxSubtStreamClose
(
    __s32 StreamId
)
{
    __s32                     Ret;
    struct ALi_DmxSubtStream *Stream;
	__s32                     StreamIdx;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

    StreamIdx = Ali_StreamId2Idx(ALI_DMX_STREAM_TYPE_SUBT, StreamId);

    if ((StreamIdx >= Ali_DmxLibVideoLayer->TotalStreamCnt) || (StreamIdx < 0))
    {
        ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);

        return(ALI_ERR_DMX_INVALID_ID);
    }

    Stream = &(Ali_DmxLibSubtLayer.Streams[StreamIdx]);

    if (Stream->Fd < 0)
    {
        return(ALI_ERR_DMX_VIDEO_STREAM_OPER_DENIED);
	}

    ALI_DMX_LIB_PRINT("%s,%d,FD:%d\n", __FUNCTION__, __LINE__, Stream->Fd);

    Ret = close(Stream->Fd); 

	if (Ret < 0)
	{
		ALI_DMX_LIB_PRINT("%s,%d,errno:%m\n", __FUNCTION__, __LINE__, errno);
	}

    /* Mark this stream as free.
	*/
    Stream->Fd = -1;
	
    return(ALI_ERR_DMX_OK);
}









__s32 Ali_DmxLibSubtLayerInit
(
    void  
)
{ 
    __s32 Idx;
	
    for (Idx = 0; Idx < ALI_DMX_LIB_SUBT_STREAM_CNT; Idx++)
    {
        Ali_DmxLibSubtLayer.Streams[Idx].Fd = -1;
	}

	return(0);
}






__s32 Ali_DmxLibExit
(
    void     
)
{
    __s32 Ret;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);	

    Ret = ALI_ERR_DMX_OK;

    Ret = Ali_DmxLibTsLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }   

    Ret = Ali_DmxLibSecLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibVideoLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibAudioLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibPcrLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibTsInRamLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibSrcCtlLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    Ret = Ali_DmxLibTpLayerExit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ali_DmxLibSubtLayerInit();

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    return(ALI_ERR_DMX_OK); 
}




__s32 Ali_DmxLibInit
(
    void     
)
{
    __s32                  Ret;
	struct dec_parse_param param;
	pCSA_DEV			   pCsaDev;
	__s32				   see_dmx_fd;
	__s32				   ret;

    ALI_DMX_LIB_PRINT("%s,%d\n", __FUNCTION__, __LINE__);	

    Ret = ALI_ERR_DMX_OK;

    Ret = Ali_DmxLibTsLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibSecLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibVideoLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);
	
    Ret = Ali_DmxLibAudioLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibPcrLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibTsInRamLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibSrcCtlLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }   

    ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxDescramLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        //return(Ret);
    }

	ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

    Ret = Ali_DmxLibTpLayerInit();
    if (Ret < 0)
    {
        ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);

        return(Ret);
    }

	//Ali_SignalHandlerRegister(SIGINT, ALI_SIGNAL_DEV_TYPE_DMX, Ali_DmxLibExit, 0);
	dmxdbg_module_register();

	ALI_DMX_LIB_PRINT("%s,%d,Ret:%d\n", __FUNCTION__, __LINE__, Ret);
	
    return(ALI_ERR_DMX_OK); 
}



