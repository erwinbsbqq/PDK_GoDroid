#ifndef __MEDIA_ALI_PE_H
#define __MEDIA_ALI_PE_H

#if 0
#define	PE_PRF(arg, value...)		printk(arg, ##value)
#else
#define PE_PRF(...)					do{}while(0)
#endif

#define ALI_PE_CACHE_NUM		2

#define ALI_PE_CMD_FLAG_CACHE			0x00000001
#define ALI_PE_CMD_FLAG_IMAGE			0x00000002
#define ALI_PE_CMD_FLAG_MUSIC			0x00000004
#define ALI_PE_CMD_FLAG_VIDEO			0x00000008

#define ALI_PE_CMD_FLAG_WORK			0x00010000

enum ali_pe_status
{
	ALI_PE_INITED,
	ALI_PE_PAUSED,
	ALI_PE_WORKING,
};

struct cache_info
{
	/* cache id */
	int id;
	
	/* busy flag of this cache channel */
	int busy;

	/* wait new data flag */
	int wait_new_data;
	
	/* cache buffer parameters */
	void *buf_start;
	int buf_size;
	int buf_cnt;
	int buf_read;
	int buf_write;

	/* file information */
	long long file_offset;
	long long file_size;

	/* cache cmd and par*/
	unsigned long cmd;
	unsigned long par1;
	unsigned long par2;
	unsigned long par3;
	unsigned long par4;	
};

struct pe_image_info
{
	unsigned long par1;
	unsigned long par2;
};

struct pe_music_info
{
	unsigned long par1;
	unsigned long par2;
};

struct pe_video_info
{
	unsigned long par1;
	unsigned long par2;
};

struct ali_pe_info
{
	/* status of pe module */
	volatile enum ali_pe_status status;

	/* semaphore of pe module */
	struct semaphore semaphore;

	/* semaphore for the rpc operation */
	struct semaphore rpc_sem;
	
	/* transport id to communicate with user space */
	int port_id;

	/* control flag */
	volatile unsigned long flag;
	
	/* wait queue */
	wait_queue_head_t wait_que;

	/* kernel thread for ali pe */
	struct task_struct *thread_pe;
	
	/* pe cache parameters */
	int cache_init;
	int current_cache_idx;
	int cache_pending;
	long long file_size_pending;
	struct cache_info cache[ALI_PE_CACHE_NUM];

	struct pe_image_info image;
	struct pe_music_info music;
	struct pe_video_info video;
};

void ali_pe_cache_mon_routine(struct ali_pe_info *info, enum ali_pe_status status);
int ali_pe_cache_routine(struct ali_pe_info *info);
int ali_pe_cache_write(struct ali_pe_info *info, void *buf , int size);
void ali_pe_cache_init(struct ali_pe_info *info);
void ali_pe_cache_release(struct ali_pe_info *info);

int ali_pe_image_routine(struct ali_pe_info *info);
int ali_pe_image_operation(int API_idx);
void ali_pe_image_init(struct ali_pe_info *info);

int ali_pe_music_routine(struct ali_pe_info *info);
int ali_pe_music_operation(int API_idx);
void ali_pe_music_init(struct ali_pe_info *info);

int ali_pe_video_routine(struct ali_pe_info *info);
int ali_pe_video_operation(int API_idx);
void ali_pe_video_init(struct ali_pe_info *info);
#endif

