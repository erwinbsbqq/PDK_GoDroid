#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <sys/time.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <dvb_audio.h>

#include <linux/ali_stc.h>

#include <hld_cfg.h>
#include <osal/osal.h>
#include <hld/decv/adr_decv.h>
#include <hld/deca/adr_deca_dev.h>
#include <hld/avsync/adr_avsync.h>
#include <hld/snd/adr_snd_dev.h>
#include <hld/ape/adr_ape.h>
#include <hld/bdma/ALiDMA_API.h>
#ifdef ADR_IPC_ENABLE
#include <hld/misc/adr_ipc.h>
#endif

#include <ali_video_common.h>
#include <ali_sbm_common.h>

#include "ape_priv.h"
#include "adr_ape_dbg.h"

//#define DUMP_VIDEO_ES_DATA

#ifdef ADR_IPC_ENABLE	
#define MUTEX_LOCK() adr_ipc_semlock(m_ape_mutex_id)		
#define MUTEX_UNLOCK() adr_ipc_semunlock(m_ape_mutex_id)
#else
#define MUTEX_LOCK() osal_mutex_lock(m_ape_mutex_id, OSAL_WAIT_FOREVER_TIME)		
#define MUTEX_UNLOCK() osal_mutex_unlock(m_ape_mutex_id)
#endif

#ifdef HLD_DBG_ENABLE
static int g_ali_ape_dbg_on = 1;
#else
static int g_ali_ape_dbg_on = 0;
#endif

static int m_ape_video_count = 0;
static int m_ape_video_size = 0;
static int m_ape_video_start = 0;

static int m_ape_audio_size = 0;
static int m_ape_audio_start = 0;
static int m_ape_audio_end = 0;
static int m_ape_audio_count = 0;

static int m_ape_mutex_id = -1;

static unsigned char video_flag[10] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99}; 
static unsigned char audio_flag[10] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0xAA}; 

static int m_ape_audio_buf_init = 0;
static int m_ape_video_buf_init = 0;

#ifdef DUMP_VIDEO_ES_DATA
static FILE *file_handle = NULL;
static char file_path[] = "/mnt/usb/DISK_a1/ape_video_es.dat";
static int dump_data(void *buffer, int size)
{
	int ret = 0;
	unsigned char sc[] = {0x00, 0x00, 0x00, 0x01};
	
	if(file_handle == NULL)
	{
		file_handle = fopen(file_path, "wb+");
		if(file_handle == NULL)
		{
			APE_DEBUG("ape open dump file fail %s\n", file_path);
			return -1;
		}
		else
		{
			APE_DEBUG("ape open dump file done %d\n", file_handle);
		}
	}

	fwrite(sc, 1, 4, file_handle);
	
	ret = fwrite(buffer, 1, size, file_handle);	
	if(ret <= 0)
	{
		APE_DEBUG("ape write dump data fail\n");
	}

	fsync(file_handle);
	
	APE_DEBUG("dump video data %d\n", size);
	
	return ret;
}

static void dump_stop(void)
{
	if(file_handle != 0)
	{
		APE_DEBUG("ape close the fail\n");
		fsync(file_handle);
		fclose(file_handle);
		file_handle = NULL;
	}
}

#endif

#ifdef HLD_APE_DUMP_FILE
static int m_ape_dump_flag = 1;
static char dump_def[] = HLD_APE_DUMP_FILE;
#else
static int m_ape_dump_flag = 0;
static char dump_def[] = "/mnt/sda1/ali_ape_es_001.ts";
#endif

static int m_ape_dump_start = 0;
static FILE *m_ape_dump_file = NULL;

static video_config_info_t m_ape_video_init_info;
static audio_config_info_t m_ape_audio_init_info;

static void *m_video_extra_buf_start = NULL;
static int m_video_extra_buf_size = 0;

static void *m_audio_extra_buf_start = NULL;
static int m_audio_extra_buf_size = 0;

static int m_ape_rgb_output_enable = 0;

static void ape_dump_start(void );
static void ape_dump_stop(void);
static void ape_dump_es(enum APE_BLOCK_TYPE type, void *buf_start, int buf_size)
{
	ape_block_header_t block_header;
	ape_priv_t* ape_priv = ape_get_priv();	
	int ret = 0;
	
	if(m_ape_dump_flag == 0)
		return;

	if(m_ape_dump_start == 0)
	{
		ape_dump_start();

		if(m_ape_dump_start == 1)
		{
			if(ape_priv->v_init_flag)
			{	
				memset((void *)&block_header, 0, sizeof(block_header));
				block_header.type = BLOCK_VIDEO_CODEC_TYPE;
				block_header.len = sizeof(ape_priv->codec_tag);
				ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
				ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
				ret += fwrite((void *)&(ape_priv->codec_tag), 1, block_header.len, m_ape_dump_file);
				if(ret <= 0)
				{
					APE_DEBUG("write video init info fail ret %d\n", ret);
				}
				else
				{
					APE_DEBUG("write video init info done ret %d\n", ret);
				}
				
				memset((void *)&block_header, 0, sizeof(block_header));
				block_header.type = BLOCK_VIDEO_INIT_INFO;
				block_header.len = sizeof(m_ape_video_init_info);
				ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
				ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
				ret += fwrite((void *)&m_ape_video_init_info, 1, block_header.len, m_ape_dump_file);
				if(ret <= 0)
				{
					APE_DEBUG("write video init info fail ret %d\n", ret);
				}
				else
				{
					APE_DEBUG("write video init info done ret %d\n", ret);
				}

				if(m_video_extra_buf_start)
				{					
					memset((void *)&block_header, 0, sizeof(block_header));
					block_header.type = BLOCK_VIDEO_EXTRA_DATA;
					block_header.len = m_video_extra_buf_size;
					ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
					ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
					ret += fwrite((void *)m_video_extra_buf_start, 1, block_header.len, m_ape_dump_file);
					if(ret <= 0)
					{
						APE_DEBUG("write video extra data fail ret %d\n", ret);
					}
					else
					{
						APE_DEBUG("write video extra data done ret %d\n", ret);
					}
				}
			}

			if(ape_priv->a_init_flag)
			{
				if(m_audio_extra_buf_start)
				{					
					memset((void *)&block_header, 0, sizeof(block_header));
					block_header.type = BLOCK_AUDIO_EXTRA_DATA;
					block_header.len = m_audio_extra_buf_size;
					ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
					ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
					ret += fwrite((void *)m_audio_extra_buf_start, 1, block_header.len, m_ape_dump_file);
					if(ret <= 0)
					{
						APE_DEBUG("write audio extra data fail ret %d\n", ret);
					}
					else
					{
						APE_DEBUG("write audio extra data done ret %d\n", ret);
					}
				}
							
				memset((void *)&block_header, 0, sizeof(block_header));
				block_header.type = BLOCK_AUDIO_INIT_INFO;
				block_header.len = sizeof(m_ape_audio_init_info);
				ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
				ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
				ret += fwrite((void *)&m_ape_audio_init_info, 1, block_header.len, m_ape_dump_file);
				if(ret <= 0)
				{
					APE_DEBUG("write audio init info fail ret %d\n", ret);
				}
				else
				{
					APE_DEBUG("write audio init info done ret %d\n", ret);
				}
			}
		}
	}

	if(m_ape_dump_start == 0)
		return;

	memset((void *)&block_header, 0, sizeof(block_header));
	block_header.type = type;
	block_header.len = buf_size;

	ret = fwrite(APE_BLOCK_MAGIC, 1, 8, m_ape_dump_file);
	ret += fwrite((void *)&block_header,1, sizeof(block_header), m_ape_dump_file);
	if(buf_size > 0)
		ret += fwrite(buf_start, 1, buf_size, m_ape_dump_file);
	if(ret < 0)
	{
		APE_LOG("write es data fail ret %d\n", ret);
	}
	else
	{
		APE_DEBUG("es block type %d buf_size %d ret %d\n", type, buf_size, ret);
	}

	fflush(m_ape_dump_file);
}

static void ape_dump_start(void )
{
	ape_priv_t* ape_priv = ape_get_priv();
	FILE *test_file = NULL;
	int i = 0;
	

	if(m_ape_dump_flag)
	{
		if(m_ape_dump_start)
		{
			return;
		}
		
		if(m_ape_dump_file == NULL)
		{
			i = sizeof(dump_def) - 1;
			while(i > 0)
			{
				if(dump_def[i] == '.')
					break;

				i--;
			}

			if((i <= 0) || (dump_def[i] != '.'))
			{
				APE_LOG("ape dump default fail %s i %d\n", dump_def, i);
				
				return;
			}
			
			i--;
			
			while(1)
			{
				test_file = fopen(dump_def, "r");
				if(test_file == NULL)
				{
					break;
				}

                fclose(test_file);
				dump_def[i] += 1;
			}

			m_ape_dump_file = fopen(dump_def, "wb+");
			if(m_ape_dump_file == NULL)
			{
				APE_LOG("ape open dump file fail %s\n", dump_def);
				
				return;
			}
		}

		m_ape_dump_start = 1;
		
		APE_LOG("ape dump start done %s file 0x%08x\n", dump_def, m_ape_dump_file);

		fwrite(APE_MAGIC, 1, 8, m_ape_dump_file);
	}
}

static void ape_dump_stop(void)
{
	if(m_ape_dump_flag)
	{
		if(m_ape_dump_start == 0)
		{
			return;
		}

		if(m_ape_dump_file)
		{
			ape_dump_es(BLOCK_END, NULL, 0);

			fflush(m_ape_dump_file);			
			fclose(m_ape_dump_file);

			m_ape_dump_file = NULL;		
		}

		m_ape_dump_start = 0;
	}
}

void ape_dump_enable(void *file_path)
{
	ape_priv_t* ape_priv = ape_get_priv();
	
	if (NULL == ape_priv)
	{
		APE_LOG( "you should call the open func first!!\n");
		return;
	}
	
	MUTEX_LOCK();
	
	if(m_ape_dump_start == 0)
	{	
		if(m_ape_dump_file)
		{
			fclose(m_ape_dump_file);

			m_ape_dump_file = NULL;
		}
		
		m_ape_dump_file = fopen(file_path, "wb+");
		if(m_ape_dump_file == NULL)
		{
			APE_LOG("ape open dump file fail %s\n", file_path);

			MUTEX_UNLOCK();
			
			return;
		}
		else
		{
			APE_LOG("ape open dump file done %d\n", m_ape_dump_file);
		}

		m_ape_dump_flag = 1;
	}

	MUTEX_UNLOCK();
}

void ape_dbg_enable(int level)
{
#ifdef HLD_DBG_ENABLE
	level = 1;
#endif

	if(level)
		g_ali_ape_dbg_on = 1;
	else
		g_ali_ape_dbg_on = 0;

	ape_set_dbg_mode(g_ali_ape_dbg_on);	
}

int ape_setup_bdma(void *addr,unsigned int size)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int fd;
	fd = open("/dev/mem", O_RDWR);  
	if (fd == -1)  
	{  
		return (-1);  
	}  

	ape_priv->phy_base= (unsigned char *)((unsigned int)addr & 0x1fffffff);
	ape_priv->mmap_size = size;
	ape_priv->mmap_base = mmap(NULL, ape_priv->mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (unsigned int)addr);	
	if(ape_priv->mmap_base != NULL)
		APE_DEBUG("mmap base value 0x%lx,size 0x%x 0x%08x pid %d\n",ape_priv->mmap_base,size,addr,getpid());
	else
		APE_DEBUG("mmap fail!!!\n");

}

int ape_release_bdma()
{
	ape_priv_t* ape_priv = ape_get_priv();

	munmap(ape_priv->mmap_base,ape_priv->mmap_size);

	if(ape_priv->mmap_base != NULL)
		APE_DEBUG("mmap base value !!!\n");
	else
		APE_DEBUG("unmmap fail!!!\n");
}

int ape_bdma_copy(void *user_addr,void *phy_addr,unsigned int copy_len)
{
	ape_priv_t* ape_priv = ape_get_priv();	
	ape_priv->mmap_offset = (unsigned long)phy_addr - (unsigned long)ape_priv->phy_base;

	unsigned char * src_addr = user_addr;
	unsigned char * dst_addr = ape_priv->mmap_base + ape_priv->mmap_offset;
	ALiDMA_memcpy((void *)src_addr,(void *)dst_addr,copy_len,1);
//	APE_DEBUG("phy_base 0x%x phy_addr 0x%x req_size %d,offset %d user_base 0x%0x user_addr 0x%0x pid %d\n",\
//		ape_priv->phy_base,phy_addr,copy_len,ape_priv->mmap_offset,
//		ape_priv->mmap_base,dst_addr,getpid());
}

int ape_set_bdma_mode(int enable)
{
	ape_priv_t* ape_priv = ape_get_priv();
	if(enable)
		ape_priv->bdma_mode = 1;
	else
		ape_priv->bdma_mode = 0;
	APE_DEBUG("%s bdma mode %d\n",__FUNCTION__,ape_priv->bdma_mode);
	return 0;
}

int ape_write_data(int fd, void* addr, int size )
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	struct av_packet pkt_info;
	enum APE_BLOCK_TYPE es_type = 0;
	unsigned int tmp_pts;
	int valid = 0;
        int video_frame_rateX10 = 0;
	int audio_byte_rateX10 = 0;
	
	if (0 == ape_check_fd(fd))
	{
		APE_DEBUG( "the fd is not right, please get the right fd first!! %d\n", fd);
		return -1;
	}
	
	MUTEX_LOCK();

	if(fd == ape_priv->vpkt_data_fd)
	{
		if(!ape_priv->sbm_lock)
		{
			if((ape_priv->sbm_last_size + size + ape_priv->sbm_reserved_size)
				> ape_priv->sbm_total_size)
			{
				APE_DEBUG("video sbm is locked size 0x%08x last 0x%08x total 0x%08x res 0x%08x\n"
					, ape_priv->sbm_last_size, size, ape_priv->sbm_total_size
					, ape_priv->sbm_reserved_size);
				
				ape_priv->sbm_lock = 1;
			}

			ape_priv->sbm_last_size = size;
		}
		else
		{
			APE_DEBUG("drop video data for sbm is locked\n");
		
			MUTEX_UNLOCK();
			
			return size;
		}
	}
	if(ape_priv->bdma_mode == 1)
	{
		struct  sbm_req_buf buf_info;
		memset(&buf_info,0,sizeof(struct sbm_req_buf));

		buf_info.req_size = size;
		ioctl(fd,SBMIO_REQ_BUF_INFO,&buf_info);
		
		if( buf_info.req_size != 0)//req size success
		{
			ret = buf_info.req_size;
			buf_info.phy_addr  = (unsigned char * )((unsigned int)buf_info.phy_addr  & 0x1fffffff);
			ape_bdma_copy(addr,(void *)buf_info.phy_addr,buf_info.req_size);
			ioctl(fd,SBMIO_UPDATE_BUF_INFO,&buf_info);	
		}
		else
		{
			ret = -1;
			APE_DEBUG("kernel req buf fail!!!\n");
		}
	}
	else
	{
#ifdef AUD_MERGE_BLOCK
        audio_merge_info_t *p_aud_merge_info = NULL;
        struct av_packet *p_pkt_info = NULL;
        
        p_aud_merge_info = &(ape_priv->audio_merge_info);
        
        if (ape_priv->audio_merge_mode)
        {            
            //APE_DEBUG("%s,%d, ape_priv->audio_merge_mode = %d\n\n", __FUNCTION__, __LINE__, ape_priv->audio_merge_mode);

            if (fd == ape_priv->apkt_hdr_fd)
            {
                p_pkt_info = (struct av_packet *)addr;
                
                if (0 == p_aud_merge_info->hdr_merge_time)
                {
                    p_aud_merge_info->last_hdr_buf = addr;
                    p_aud_merge_info->last_hdr_len = size;
                    memcpy(&(p_aud_merge_info->last_pkt_info), p_pkt_info, sizeof(struct av_packet));
                }
                
                p_aud_merge_info->hdr_merge_time++;
                p_aud_merge_info->hdr_merge_data_size += p_pkt_info->size;

                //APE_DEBUG("fd = %d, hdr_merge_time = %d, size = %d\n\n", fd, p_aud_merge_info->hdr_merge_time, p_pkt_info->size);
                
                if (p_aud_merge_info->merge_block_cnt == p_aud_merge_info->hdr_merge_time)
                {
                    p_pkt_info = (struct av_packet *)p_aud_merge_info->last_hdr_buf;
                    memcpy(p_pkt_info, &(p_aud_merge_info->last_pkt_info), sizeof(struct av_packet));
                    p_pkt_info->size = p_aud_merge_info->hdr_merge_data_size;

                    //APE_DEBUG("fd = %d, merger size = %d\n\n", fd, p_pkt_info->size);
                    
                    ret = write(ape_priv->apkt_hdr_fd, p_aud_merge_info->last_hdr_buf, p_aud_merge_info->last_hdr_len);

                    p_aud_merge_info->hdr_merge_time = 0;
                    p_aud_merge_info->hdr_merge_data_size = 0;
                }
                else
                {
                	ret=p_pkt_info->size; 
                	//ALOGI("in %s, %d, we  make the invoker know we consumed header size is %d  \n",__FUNCTION__,__LINE__, ret );

                }
            }
            else if (fd == ape_priv->apkt_data_fd)
            {
                p_aud_merge_info->data_merge_time++;

                if ((size > (int)p_aud_merge_info->data_merge_len) || \
                    ((p_aud_merge_info->data_merge_size + size) > p_aud_merge_info->data_merge_len))
                {
                    APE_LOG("%s,%d, make merge buffer bigger!!!\n\n", __FUNCTION__, __LINE__);

                    unsigned int new_data_merge_len = 0;
                    unsigned char *new_data_merge_buf = NULL;

                    new_data_merge_len = p_aud_merge_info->data_merge_size + size;

                    p_aud_merge_info->data_merge_len = new_data_merge_len;

                    new_data_merge_buf = malloc(new_data_merge_len);

                    if (new_data_merge_buf)
                    {
                        memset(new_data_merge_buf, 0, new_data_merge_len);
                        memcpy(new_data_merge_buf, p_aud_merge_info->data_merge_buf, p_aud_merge_info->data_merge_size);
                        p_aud_merge_info->data_merge_buf = new_data_merge_buf;
                        p_aud_merge_info->data_merge_buf_tmp = new_data_merge_buf + p_aud_merge_info->data_merge_size;
                    }
                    else
                    {
                        APE_LOG("%s,%d, malloc %d size error!!!\n\n", __FUNCTION__, __LINE__, new_data_merge_len);
                    }
                }
                
                //APE_DEBUG("fd = %d, data_merge_time = %d, size = %d\n\n", fd, p_aud_merge_info->data_merge_time, size);

                p_aud_merge_info->data_merge_size += size;
                
                if (p_aud_merge_info->data_merge_size <= p_aud_merge_info->data_merge_len)
                {
                    memcpy(p_aud_merge_info->data_merge_buf_tmp, addr, size);
                    p_aud_merge_info->data_merge_buf_tmp += size;
                }
                
                if (p_aud_merge_info->merge_block_cnt == p_aud_merge_info->data_merge_time)
                {
                    //APE_DEBUG("fd = %d, merge size = %d\n\n", fd, p_aud_merge_info->data_merge_size);

                    ret = write(fd, p_aud_merge_info->data_merge_buf, p_aud_merge_info->data_merge_size);
                    
                    p_aud_merge_info->data_merge_time = 0;
                    p_aud_merge_info->data_merge_size = 0;
                    p_aud_merge_info->data_merge_buf_tmp = p_aud_merge_info->data_merge_buf;
                }
                else
                {
                    ret=size; 
                   // ALOGI("in %s, %d, we  make the invoker know we consumed data size is %d  \n",__FUNCTION__,__LINE__, ret );

                }
            }
            else
            {
            	ret = write(fd, addr, size);
            }
        }
        else
        {
            ret = write(fd, addr, size);
        }
#else
    	ret = write(fd, addr, size);

#endif
	}

	if(ret < 0)
	{
		APE_DEBUG("write fd %d fail\n", fd);
	}

	if(m_ape_dump_flag)
	{
		if(fd == ape_priv->vpkt_data_fd)
			es_type = BLOCK_VIDEO_DATA;
		else if(fd == ape_priv->apkt_data_fd)
			es_type = BLOCK_AUDIO_DATA;
		else if(fd == ape_priv->vpkt_hdr_fd)
			es_type = BLOCK_VIDEO_HEADER;
		else if(fd == ape_priv->apkt_hdr_fd)
			es_type = BLOCK_AUDIO_HEADER;

		if(es_type != BLOCK_END)
			ape_dump_es(es_type, addr, size);
	}

	if(g_ali_ape_dbg_on)
	{
		struct av_packet *ppkt = (struct av_packet *)addr;

#if 0
		if (1 == ape_priv->data_dump_flag)
		{
			if (fd == ape_priv->apkt_hdr_fd || fd == ape_priv->vpkt_hdr_fd)
			{
				memcpy(&pkt_info, addr, sizeof(struct av_packet));
				ape_store_pts(fd, pkt_info.pts);
			} 
			else if (fd == ape_priv->apkt_data_fd || fd == ape_priv->vpkt_data_fd)
			{
				tmp_pts = ape_get_pts(fd);
				if (fd == ape_priv->apkt_data_fd)
				{
					fwrite(audio_flag, 1, 10,ape_priv->dump_file);
				}
				else if (fd == ape_priv->vpkt_data_fd)
				{
					fwrite(video_flag, 1, 10, ape_priv->dump_file);
				}
				fwrite(&size, 1, 4, ape_priv->dump_file);
				fwrite(&tmp_pts, 1, 4, ape_priv->dump_file);
				fwrite(addr, 1, size, ape_priv->dump_file);
			}		
		}
		else if (ape_priv->dump_file != NULL)
		{
			fclose(ape_priv->dump_file);
			ape_priv->dump_file = NULL;
		}
#endif

		if(fd == ape_priv->vpkt_hdr_fd)
		{
			m_ape_video_size += ppkt->size;
			m_ape_video_count++;
			video_frame_rateX10 = ape_get_video_frame_rateX10();
			if(m_ape_video_count > video_frame_rateX10)	
			{
				m_ape_video_count = osal_get_tick();
				if(m_ape_video_count - m_ape_video_start > 10000)
				{			
					m_ape_video_start = m_ape_video_count - m_ape_video_start;				
					m_ape_video_start /= 1000;
					
					APE_LOG("video speed req %d (Bps) real %d (Bps) time %d (S)\n"
						, m_ape_video_size / 10, m_ape_video_size / m_ape_video_start
						, m_ape_video_start);

					m_ape_video_start = m_ape_video_count;
					m_ape_video_count = 0;
					m_ape_video_size = 0;
				}
			}		
			//APE_DEBUG("v pts %d\n", (unsigned int)ppkt->pts);
		}
		else if(fd == ape_priv->apkt_hdr_fd)
		{
			m_ape_audio_size += ppkt->size;
			m_ape_audio_count++;
			m_ape_audio_end = osal_get_tick();
			if(m_ape_audio_start == 0)
				m_ape_audio_start = m_ape_audio_end;			
			if((m_ape_audio_end - m_ape_audio_start) > 10000)
			{
				m_ape_audio_start = (m_ape_audio_end - m_ape_audio_start) / 1000;
				audio_byte_rateX10 = ape_get_audio_byte_rateX10();
				
				APE_LOG("audio pkt %d speed req %d (Bps) real %d (Bps) time %d (S)\n"
					, m_ape_audio_count, audio_byte_rateX10 / 10, m_ape_audio_size / m_ape_audio_start
					, m_ape_audio_start);

				m_ape_audio_size = 0;
				m_ape_audio_count = 0;
				m_ape_audio_start = m_ape_audio_end;
			}

			//APE_DEBUG("a pts %d\n", (unsigned int)ppkt->pts);
		}
	}

#ifdef DUMP_VIDEO_ES_DATA
	if(fd == ape_priv->vpkt_data_fd)	
	{		
		dump_data(addr, size);
	}
#endif

	MUTEX_UNLOCK();
	return ret;
}

int ape_read_info(int fd, void* addr, int size )
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	
	if (0 == ape_check_fd(fd))
	{
		APE_DEBUG( "the fd is not right, please get the right fd first!! %d\n", fd);
		return -1;
	}

	if (fd == ape_priv->deca_out_fd)	
	{
		return 0;
	}
	
	MUTEX_LOCK();

	APE_IN();
 
	ret = read(fd, addr, size);
	if(ret < 0)
	{
		APE_DEBUG("read fd %d fail\n", fd);
	}

 	APE_OUT();
	
	MUTEX_UNLOCK();
	
	return ret;
}

int ape_show_valid_buf_size(int fd)
{
	int ret = 0;
	int valid_size = 0;
	if (0 == ape_check_fd(fd))
	{
		APE_DEBUG( "the fd is not right, please get the right fd first!! %d\n", fd);
		return -1;
	}

	MUTEX_LOCK();
	
	//APE_DEBUG( "valid size fd = %d\n", fd);
	ret = ioctl(fd, SBMIO_SHOW_VALID_SIZE, &valid_size);
	if(ret < 0)
	{
		APE_DEBUG( "ape show valid size fail!!\n");
		valid_size = -1;
	}
	
	MUTEX_UNLOCK();
	
	return valid_size;
}

int ape_show_free_buf_size(int fd)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = 0;
	int free_size = 0;
	if (0 == ape_check_fd(fd))
	{
		APE_DEBUG( "the fd is not right, please get the right fd first!! %d\n", fd);
		return -1;
	}

	MUTEX_LOCK();
	
	ret = ioctl(fd, SBMIO_SHOW_FREE_SIZE, &free_size);
	if(ret < 0)
	{
		APE_DEBUG( "ape show free size fail!!\n");
		free_size = -1;
	}
	
	MUTEX_UNLOCK();
	
	return free_size;
}

int ape_show_total_buf_size(int fd)
{
	int ret = 0;
	int total_size = 0;
	if (0 == ape_check_fd(fd))
	{
		APE_DEBUG( "the fd is not right, please get the right fd first!! %d\n", fd);
		return -1;
	}

	MUTEX_LOCK();
	
	ret = ioctl(fd, SBMIO_SHOW_TOTAL_SIZE, &total_size);
	if(ret < 0)
	{
		APE_DEBUG( "ape show total size fail!!\n");
		total_size = -1;
	}

	MUTEX_UNLOCK();
	
	return total_size;
}

void ape_release_av_buf(int type)
{
	ape_priv_t* ape_priv = ape_get_priv();

	MUTEX_LOCK();

	APE_IN();
	
	if (NULL == ape_priv)
	{
		APE_DEBUG( "you should call the open func first!!\n");
		goto EXIT;
	}	

	if((m_ape_video_buf_init == 1) && (type == 0))
	{
		ioctl(ape_priv->vpkt_data_fd, SBMIO_DESTROY_SBM, 0);
			
		ioctl(ape_priv->vpkt_hdr_fd, SBMIO_DESTROY_SBM, 0);
		
		ioctl(ape_priv->decv_out_fd, SBMIO_DESTROY_SBM, 0);
			
		ioctl(ape_priv->disp_in_fd, SBMIO_DESTROY_SBM, 0);

		m_ape_video_buf_init = 0;
	}

	if((m_ape_audio_buf_init == 1) && (type == 1))
	{
		ioctl(ape_priv->apkt_data_fd, SBMIO_DESTROY_SBM, 0);
		
		ioctl(ape_priv->apkt_hdr_fd, SBMIO_DESTROY_SBM, 0);
				
		ioctl(ape_priv->deca_out_fd, SBMIO_DESTROY_SBM, 0);
		
		ioctl(ape_priv->snd_in_fd, SBMIO_DESTROY_SBM, 0);

		m_ape_audio_buf_init = 0;
	}
	
EXIT:

	APE_OUT();
	
	MUTEX_UNLOCK();
	return;	
}

int ape_malloc_av_buf(void)
{
	struct sbm_config sbm_info;
	unsigned int  run_addr, run_size;
	int ret = 1;
	ape_priv_t* ape_priv = ape_get_priv();

	MUTEX_LOCK();

	APE_IN();
 	
	if (NULL == ape_priv)
	{
		APE_DEBUG( "you should call the open func first!!\n");
		ret = -1;
		goto EXIT;
	}	
		
	if((m_ape_video_buf_init == 1)
		&& (m_ape_audio_buf_init == 1))
	{
		APE_DEBUG( "ape av buffer is already inited\n\n");
		goto EXIT;		
	}
	soc_get_media_buf(&run_addr, &run_size);	
	run_addr &= 0x1FFFFFFF;	
	APE_DEBUG( "sbm use pe mem 0x%x 0x%x\n", run_addr, run_size);
	
	
	memset(&sbm_info, 0, sizeof(sbm_info));

	if(run_size < 0xc00000)
	{
		int total_size = 0;
		int extend_buf = 0;

		if(run_size >= 0x800000)
			extend_buf = 1;
		
		if((m_ape_video_buf_init == 0)
			|| (m_ape_audio_buf_init == 0))
		{
			if(run_size < 0x200000)
			{
				APE_DEBUG("Get decv run buffer fail\n");
				ret = -1;
				goto EXIT;
			}

			if(extend_buf == 1)
			{
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 0x600000;
				sbm_info.block_size = 0x20000;
				sbm_info.reserve_size = 256 * 1024;

				ape_priv->sbm_total_size = sbm_info.buffer_size;
				ape_priv->sbm_reserved_size = sbm_info.reserve_size;
				
				sbm_info.wrap_mode = SBM_MODE_PACKET;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;
				ret = ioctl(ape_priv->vpkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail,vpkt_data_fd = %d\n", ape_priv->vpkt_data_fd);
					ret = -1;
					//goto EXIT;
				}
                if(ape_priv->bdma_mode == 1)
                {
                    ape_setup_bdma((void *)sbm_info.buffer_addr ,sbm_info.buffer_size );
                }
				APE_DEBUG("vpk data fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_data_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);		

				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct av_packet);
				sbm_info.reserve_size = 20 * sizeof(struct av_packet);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->vpkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("vpk hdr fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_hdr_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct av_frame);
				sbm_info.reserve_size = 10 * sizeof(struct av_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->decv_out_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("decv out fd %d buf %x size %x reserve %x\n", ape_priv->decv_out_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct av_frame);
				sbm_info.reserve_size = 10 * sizeof(struct av_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->disp_in_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("disp in fd %d buf %x size %x reserve %x\n", ape_priv->disp_in_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);

				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 0x50000 * 4;
				sbm_info.block_size = 0x400;
				sbm_info.reserve_size = 20 * 1024 * 4;
				sbm_info.wrap_mode = SBM_MODE_PACKET;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->apkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}
				if(ape_priv->bdma_mode == 1)
                {
                    ape_setup_bdma((void *)sbm_info.buffer_addr ,sbm_info.buffer_size );
                }
				APE_DEBUG("apkt data fd %d buf %x size %x reserve %x\n", ape_priv->apkt_data_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct av_packet);
				sbm_info.reserve_size = 20 * sizeof(struct av_packet);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->apkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("apkt hdr fd %d buf %x size %x reserve %x\n", ape_priv->apkt_hdr_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct audio_frame);
				sbm_info.reserve_size = 20 * sizeof(struct audio_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->deca_out_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}		

				APE_DEBUG("deca out fd %d buf %x size %x reserve %x\n", ape_priv->deca_out_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct audio_frame);
				sbm_info.reserve_size = 10 * sizeof(struct audio_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->snd_in_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("snd in fd %d buf %x size %x reserve %x\n", ape_priv->snd_in_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
			}
			else
			{
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 0x200000;
				sbm_info.block_size = 0x20000;
				sbm_info.reserve_size = 256 * 1024;

				ape_priv->sbm_total_size = sbm_info.buffer_size;
				ape_priv->sbm_reserved_size = sbm_info.reserve_size;
				
				sbm_info.wrap_mode = SBM_MODE_PACKET;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;
				ret = ioctl(ape_priv->vpkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail,vpkt_data_fd = %d\n", ape_priv->vpkt_data_fd);
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("vpk data fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_data_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);		

				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct av_packet);
				sbm_info.reserve_size = 20 * sizeof(struct av_packet);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->vpkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("vpk hdr fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_hdr_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct av_frame);
				sbm_info.reserve_size = 10 * sizeof(struct av_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->decv_out_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("decv out fd %d buf %x size %x reserve %x\n", ape_priv->decv_out_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct av_frame);
				sbm_info.reserve_size = 10 * sizeof(struct av_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->disp_in_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("disp in fd %d buf %x size %x reserve %x\n", ape_priv->disp_in_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);

				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 0x50000;
				sbm_info.block_size = 0x400;
				sbm_info.reserve_size = 20 * 1024;
				sbm_info.wrap_mode = SBM_MODE_PACKET;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->apkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}
				
				APE_DEBUG("apkt data fd %d buf %x size %x reserve %x\n", ape_priv->apkt_data_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct av_packet);
				sbm_info.reserve_size = 20 * sizeof(struct av_packet);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->apkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("apkt hdr fd %d buf %x size %x reserve %x\n", ape_priv->apkt_hdr_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 2048 * sizeof(struct audio_frame);
				sbm_info.reserve_size = 20 * sizeof(struct audio_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->deca_out_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}		

				APE_DEBUG("deca out fd %d buf %x size %x reserve %x\n", ape_priv->deca_out_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);
				
				sbm_info.buffer_addr = run_addr;
				sbm_info.buffer_size = 64 * sizeof(struct audio_frame);
				sbm_info.reserve_size = 10 * sizeof(struct audio_frame);
				sbm_info.wrap_mode = SBM_MODE_NORMAL;
				sbm_info.lock_mode = SBM_SPIN_LOCK;
				run_addr += sbm_info.buffer_size;
				run_size -= sbm_info.buffer_size;
				total_size += sbm_info.buffer_size;		
				ret = ioctl(ape_priv->snd_in_fd, SBMIO_CREATE_SBM, &sbm_info);
				if(ret < 0)
				{
				    APE_DEBUG( "decore create sbm fail\n");
					ret = -1;
					//goto EXIT;
				}

				APE_DEBUG("snd in fd %d buf %x size %x reserve %x\n", ape_priv->snd_in_fd, sbm_info.buffer_addr
					, sbm_info.buffer_size, sbm_info.reserve_size);			
			}
			ape_priv->pcm_buf_addr = run_addr;
			ape_priv->pcm_buf_size = run_size;	
			total_size += ape_priv->pcm_buf_size;
			APE_DEBUG("pcum buf %x size %x\n", run_addr, run_size);

			m_ape_video_buf_init = 1;
			m_ape_audio_buf_init = 1;
		
			APE_DEBUG("ape total memory size %x\n", total_size);			
		}
	}
	else
	{
		if((m_ape_video_buf_init == 0)
			|| (m_ape_audio_buf_init == 0))
		{
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 0x700000;
			sbm_info.block_size = 0x20000;
			sbm_info.reserve_size = 128*1024;

			ape_priv->sbm_total_size = sbm_info.buffer_size;
			ape_priv->sbm_reserved_size = sbm_info.reserve_size;
				
			sbm_info.wrap_mode = SBM_MODE_PACKET;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->vpkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail,vpkt_data_fd = %d\n", ape_priv->vpkt_data_fd);
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("vpk data fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_data_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			  
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 4096*sizeof(struct av_packet);
			sbm_info.reserve_size = 10*sizeof(struct av_packet);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->vpkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("vpk hdr fd %d buf %x size %x reserve %x\n", ape_priv->vpkt_hdr_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 128*sizeof(struct av_frame);
			sbm_info.reserve_size = 10*sizeof(struct av_frame);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->decv_out_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("decv out fd %d buf %x size %x reserve %x\n", ape_priv->decv_out_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 128*sizeof(struct av_frame);
			sbm_info.reserve_size = 10*sizeof(struct av_frame);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->disp_in_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("disp in fd %d buf %x size %x reserve %x\n", ape_priv->disp_in_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 0x200000;
			sbm_info.block_size = 0x20000;
			sbm_info.reserve_size = 128*1024;
			sbm_info.wrap_mode = SBM_MODE_PACKET;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->apkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("apkt data fd %d buf %x size %x reserve %x\n", ape_priv->apkt_data_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 4096*sizeof(struct av_packet);
			sbm_info.reserve_size = 10*sizeof(struct av_packet);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->apkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("apkt hdr fd %d buf %x size %x reserve %x\n", ape_priv->apkt_hdr_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 4096*sizeof(struct audio_frame);
			sbm_info.reserve_size = 10*sizeof(struct audio_frame);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->deca_out_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("deca out fd %d buf %x size %x reserve %x\n", ape_priv->deca_out_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			sbm_info.buffer_addr = run_addr;
			sbm_info.buffer_size = 128*sizeof(struct audio_frame);
			sbm_info.reserve_size = 10*sizeof(struct audio_frame);
			sbm_info.wrap_mode = SBM_MODE_NORMAL;
			sbm_info.lock_mode = SBM_SPIN_LOCK;
			run_addr += sbm_info.buffer_size;
			run_size -= sbm_info.buffer_size;
			ret = ioctl(ape_priv->snd_in_fd, SBMIO_CREATE_SBM, &sbm_info);
			if(ret < 0)
			{
			    APE_DEBUG( "decore create sbm fail\n");
				ret = -1;
				//goto EXIT;
			}

			APE_DEBUG("snd in fd %d buf %x size %x reserve %x\n", ape_priv->snd_in_fd, sbm_info.buffer_addr
				, sbm_info.buffer_size, sbm_info.reserve_size);
			
			ape_priv->pcm_buf_addr = run_addr;
			ape_priv->pcm_buf_size = 0x200000;
			APE_DEBUG("pcum buf %x size %x\n", run_addr, run_size);

			m_ape_video_buf_init = 1;
			m_ape_audio_buf_init = 1;
		}		
	}

	if (ape_vdec_sbm_reset() < 0)
	{
		APE_DEBUG("retset vdec sbm fail\n");
		
		ret = -1;
		goto EXIT;
	}

	ape_priv->sbm_lock = 0;
	ape_priv->sbm_last_size = 0;
		
	if (ape_adec_sbm_reset() < 0)
	{
		APE_DEBUG("retset vdec sbm fail\n");
		
		ret = -1;
		goto EXIT;
	}
	
EXIT:

	APE_OUT();
	
	MUTEX_UNLOCK();
	return ret;
}

int ape_get_clock_fd(enum BUFFER_FORMAT buf_format)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int clock_fd = -1;

	MUTEX_LOCK();
	
	if (CLOCK0 == buf_format)
	{
	 	clock_fd = ape_priv->clock0_fd;
	}
	else if (CLOCK1== buf_format)
	{
		clock_fd = ape_priv->clock1_fd;
	}
	else
	{
		APE_DEBUG( "clock format %d not support!!\n", buf_format);
	}

	MUTEX_UNLOCK();

	return clock_fd;
}

int ape_get_buf_fd(enum BUFFER_FORMAT buf_format)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int buf_fd = -1;

	MUTEX_LOCK();
	
	if (VPKT_HDR == buf_format)
	{
	 	buf_fd = ape_priv->vpkt_hdr_fd;
	}
	else if (VPKT_DATA== buf_format)
	{
		buf_fd = ape_priv->vpkt_data_fd;
	}
	else if (DECV_OUT == buf_format)
	{
		buf_fd = ape_priv->decv_out_fd;
	}
	else if (DISP_IN == buf_format)
	{
		buf_fd = ape_priv->disp_in_fd;
	}
	else if (APKT_HDR == buf_format)
	{
		buf_fd = ape_priv->apkt_hdr_fd;
	}
	else if (APKT_DATA == buf_format)
	{
		buf_fd = ape_priv->apkt_data_fd;
	}
	else if (DECA_OUT == buf_format)
	{
		buf_fd = ape_priv->deca_out_fd;
	}
	else if (SND_IN == buf_format)
	{
		buf_fd = ape_priv->snd_in_fd;
	}
	else
	{
		APE_DEBUG( "mem format %d not support!!\n", buf_format);
	}

	MUTEX_UNLOCK();

	return buf_fd;
}

/***************************************************************************************

								video related function

****************************************************************************************/

int ape_vdec_get_status(video_status_t *video_status)
{
	ape_priv_t* ape_priv = ape_get_priv();
	struct vdec_decore_status decore_status;
	struct VDec_StatusInfo io_status;
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();
	APE_IN();
	
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &decore_status, NULL);
	if (ret < 0)
	{
		APE_DEBUG( "ape get video status1 error!!\n");
		ret = -1;
		goto EXIT;
	}
	
	if (mpg2 == ape_priv->codec_tag || h264 == ape_priv->codec_tag)
	{
		ret = ape_get_io_status(&io_status);
		if (ret < 0)
		{
			APE_DEBUG( "ape get video status2 error!!\n");
			ret = -1;
			goto EXIT;
		}
	}
	
	video_status->first_header_got = decore_status.first_header_got;
	video_status->frame_rate = decore_status.frame_rate;
	video_status->pic_height = decore_status.pic_height;
	video_status->pic_width = decore_status.pic_width;
	video_status->decoder_feature = decore_status.decoder_feature;
	video_status->decode_error = decore_status.decode_error;
	video_status->buffer_size = decore_status.buffer_size;
	video_status->buffer_used = decore_status.buffer_used;
	video_status->top_field_first = decore_status.top_field_first;
	video_status->first_pic_showed = decore_status.first_pic_showed;
    if (mpg2 == ape_priv->codec_tag || h264 == ape_priv->codec_tag)
    {
		video_status->video_is_support = io_status.is_support;
    }
	video_status->frames_decoded = decore_status.frames_decoded;
	video_status->frames_displayed = decore_status.frames_displayed;
    video_status->frame_last_pts = decore_status.frame_last_pts;

	if((ape_priv->sbm_lock) && (!video_status->decode_error))
	{
		video_status->decode_error |= VDEC_ERR_NOMEMORY;
	}

	if(video_status->decode_error)
	{
		APE_DEBUG("first header %d\n", video_status->first_header_got);
		APE_DEBUG("frame rate %d\n", video_status->frame_rate);
		APE_DEBUG("pic width %d\n", video_status->pic_width);
		APE_DEBUG("pic height %d\n", video_status->pic_height);
		APE_DEBUG("dec err 0x%08x\n", video_status->decode_error);
		APE_DEBUG("buf size 0x%08x\n", video_status->buffer_size);
		APE_DEBUG("buf used 0x%08x\n", video_status->buffer_used);
		APE_DEBUG("top field first %d\n", video_status->top_field_first);
		APE_DEBUG("first pic showed %d\n", video_status->first_pic_showed);
		APE_DEBUG("video is supported %d\n", video_status->video_is_support);
	}

EXIT:	
	APE_OUT();
	MUTEX_UNLOCK();	
	return ret;
}

int ape_vdec_extra_data(unsigned char * extradata, unsigned int * extradata_size)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();

	APE_IN();	

	if(m_video_extra_buf_start)
	{
		free(m_video_extra_buf_start);
		
		m_video_extra_buf_start = NULL;
	}

	m_video_extra_buf_size = *extradata_size;
	if(m_video_extra_buf_size > 0)
	{
		m_video_extra_buf_start = malloc(m_video_extra_buf_size);
		if(m_video_extra_buf_start == NULL)
		{
			APE_DEBUG("malloc video extra buffer fail size %d\n", m_video_extra_buf_size);
		}
		else
		{
			memcpy(m_video_extra_buf_start, extradata, m_video_extra_buf_size);

			APE_DEBUG("backup extra data size %d\n", m_video_extra_buf_size);

			if(m_ape_dump_start)
				ape_dump_es(BLOCK_VIDEO_EXTRA_DATA, m_video_extra_buf_start, m_video_extra_buf_size);
		}
	}
	
	APE_DEBUG( "ape decode extra data!!\n");
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_EXTRA_DADA, extradata, extradata_size);
	if(ret < 0)
	{
		APE_DEBUG("ret %d\n", ret);
	}
	
	 APE_OUT();

	MUTEX_UNLOCK();
	
	return ret;
}

int ape_video_buffer_clear(void)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}

	MUTEX_LOCK();

	APE_IN();
	 
	ret = ape_vdec_sbm_reset();
	if(ret < 0)
	{
		APE_DEBUG("ret %d\n", ret);
	}	

	ape_priv->sbm_lock = 0;
	ape_priv->sbm_last_size = 0;
	
	APE_OUT();

	MUTEX_UNLOCK();

	return ret;
}

int ape_vdec_reset(unsigned int codec_tag, reset_info_t* reset_info)
{
	struct OutputFrmManager output_frm_info;
	struct vdec_decore_status vdec_status;
	int flush_flag = 1, pause_decode = 0, ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	int time_out = 0;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}

	MUTEX_LOCK();

	APE_IN();
	 
	if(ape_priv->bdma_mode == 1)
		 ALiDMAReset();
	
	if (codec_tag != ape_priv->codec_tag)
	{
		APE_DEBUG( "the reset codec_tag must same as the init one!!\n");
		ret = -1;
		goto EXIT;
	}

	pause_decode = 1;
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_PAUSE_DECODE, &pause_decode, &pause_decode);
	if(ret < 0)
	{
		APE_DEBUG( "decore pause fail\n");
		ret = -1;
		//goto EXIT;
	}
	
	if (codec_tag == h264 || codec_tag == mpg2)
	{
		do{
			memset(&vdec_status, 0, sizeof(struct vdec_decore_status));
			ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &vdec_status, &vdec_status);
			if(ret != 0)
			{
				APE_DEBUG( "decore get status fail\n");
				ret = -1;
				//goto EXIT;
			}
			osal_task_sleep(100);
			time_out += 100;
			if(time_out > 4500)
			{
				APE_LOG("time out!\n");
			    break;
			}
		}while(vdec_status.decode_status == VDEC_ON_GOING);
	}
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_SW_RESET, &flush_flag, &output_frm_info);
	if(ret < 0)
	{
		APE_DEBUG( "decore flush fail: %d\n",ret);
		ret = -1;
		//goto EXIT;
	}

	if (ape_vdec_sbm_reset() < 0)
	{
		APE_DEBUG( "vdec sbm reset fail: %d\n",ret);
		ret = -1;
		//goto EXIT;
	}

	ape_priv->sbm_lock = 0;
	ape_priv->sbm_last_size = 0;

	{
		int max, min;
		int video_timescale_num = 1000;
		int video_timescale_den = 1;

		video_timescale_num = ape_get_video_timescale_num();
		video_timescale_den = ape_get_video_timescale_den();
		max = reset_info->qmax * video_timescale_den / video_timescale_num;
		min = reset_info->qmin * video_timescale_den / video_timescale_num;
		
		ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_SYNC_THRESHOLD, &max, &min);
		if(ret < 0)
		{
			APE_DEBUG( "decore set sync threshold fail\n");
			ret = -1;
			//goto EXIT;
		}
	}

	pause_decode = 0;
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_PAUSE_DECODE, &pause_decode, &pause_decode);
	if(ret < 0)
	{
		APE_DEBUG( "decore resume fail\n");
		ret = -1;
		goto EXIT;
	}
	ret = 0;

EXIT:
	APE_OUT();
	MUTEX_UNLOCK();
	return ret;
}

int ape_vdec_avsync_config(avsync_config_info_t* info)
{
	int ret = -1;
	ape_priv_t* ape_priv = ape_get_priv();

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();	

	ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_SYNC_MODE, &info->av_sync_mode, &info->av_sync_unit);
	if(ret != 0)
	{
		APE_DEBUG( "decore set sync mode fail: %d\n",ret);
		ret = -1;
		//goto EXIT;
	}

	{
		int max, min;
		int video_timescale_num = 1000;
		int video_timescale_den = 1;		

		video_timescale_num = ape_get_video_timescale_num();
		video_timescale_den = ape_get_video_timescale_den();
		max = info->hold_threshold * video_timescale_den / video_timescale_num;
		min = info->hold_threshold * video_timescale_den / video_timescale_num;
		
		ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_SYNC_THRESHOLD, &max, &min);
		if(ret < 0)
		{
			APE_DEBUG( "decore set sync threshold fail\n");
			ret = -1;
			//goto EXIT;
		}
	}

EXIT:
	MUTEX_UNLOCK();
	return 0;
}

int ape_video_rotate(enum VDecRotationAngle frame_angle)
{
	int ret = -1;
    int time_out = 0;
    struct ali_video_rpc_pars rpc_pars;	
	ape_priv_t* ape_priv = ape_get_priv();
    struct vdec_decore_status decore_status;

	if(NULL == ape_priv)
	{
		APE_DEBUG( "ape is not open!!\n");
		return -1;
	}
	
	memset((void *)&rpc_pars, 0, sizeof(struct ali_video_rpc_pars));

	ape_priv->frame_angle = frame_angle;
	ape_priv->rotate_flag = 1;
    APE_LOG("video rotate, frame angle %d\n", frame_angle);

    if(0 == ape_priv->v_init_flag)
    {
        APE_DEBUG( "the decoder is not init!!\n");
        return 0;
    }
	
	MUTEX_LOCK();
	
	ret = ape_video_decore_ioctl(ape_priv, VDEC_ROTATE_FRAME, &(ape_priv->frame_angle), NULL);
	if(ret != 0)
	{
		APE_LOG( "decore rotate frame fail: %d\n",ret);
		ret = -1;
		goto EXIT;
	}
    else
    {
        /* h264 supports set frame angle */
        if(ape_priv->codec_tag == h264)
        {
            while(time_out <= 1000)
            {
                memset(&decore_status, 0, sizeof(decore_status));
            	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &decore_status, NULL);
            	if(ret != 0)
            	{
            		APE_DEBUG( "ape get video status error!!\n");
            		ret = -1;
            		goto EXIT;
            	}
                else
                {
                    APE_DEBUG("cur frame angle %d\n", decore_status.frame_angle);
                    if((decore_status.frame_angle == ape_priv->frame_angle)
                       || (decore_status.frame_angle == VDEC_ANGLE_MAX))
                    {
                        break;
                    }

                    osal_task_sleep(100);
                    time_out += 100;
                }
            }
        }
    }

EXIT:
	MUTEX_UNLOCK();
	return ret;
}

int ape_video_zoom(int x, int y, int width, int  height)
{
	int ret = -1;
    int time_out = 0;
    int preview = 0;
	struct ali_video_rpc_pars rpc_pars;	
	ape_priv_t* ape_priv = ape_get_priv();
    struct vdec_decore_status decore_status;

	if(NULL == ape_priv)
	{
		APE_DEBUG( "ape is not open!!\n");
		return -1;
	}
	
	memset((void *)&rpc_pars, 0, sizeof(struct ali_video_rpc_pars));

    if((width <= 360) || (height <= 1440))
    {
        preview = 1;
    }

	ape_priv->src_rect.x = 0;
	ape_priv->src_rect.y = 0;
	ape_priv->src_rect.w = 720;
	ape_priv->src_rect.h = 2880;

	ape_priv->dst_rect.x = x;
	ape_priv->dst_rect.y = y;
	ape_priv->dst_rect.w = width;
	ape_priv->dst_rect.h = height;

	APE_DEBUG("vpo zoom src<%ld %ld %ld %ld> => dest<%ld %ld %ld %ld>\n", 
		ape_priv->src_rect.x, ape_priv->src_rect.y, \
		ape_priv->src_rect.w, ape_priv->src_rect.h,\
		ape_priv->dst_rect.x, ape_priv->dst_rect.y, \
		ape_priv->dst_rect.w, ape_priv->dst_rect.h);
	
	ape_priv->zoom_flag = 1;

    if(0 == ape_priv->v_init_flag)
    {
        APE_DEBUG( "the decoder is not init!!\n");
        return 0;
    }
	
	MUTEX_LOCK();
	
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_DISPLAY_RECT, \
		&(ape_priv->src_rect), &(ape_priv->dst_rect));
	if(ret != 0)
	{
		APE_DEBUG( "decore set display rect fail: %d\n",ret);
		ret = -1;
		goto EXIT;
	}
    else
    {
        /* h264 supports full screen and preview switch smoothly */
        if(ape_priv->codec_tag == h264)
        {
            while(time_out <= 1000)
            {
                memset(&decore_status, 0, sizeof(decore_status));
            	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &decore_status, NULL);
            	if(ret != 0)
            	{
            		APE_DEBUG( "ape get video status error!!\n");
            		ret = -1;
            		goto EXIT;
            	}
                else
                {
                    APE_DEBUG("cur status %d %d\n", preview, decore_status.output_mode);
                    if(preview)
                    {
                        if((decore_status.output_mode == PREVIEW_MODE)
                           || (decore_status.output_mode == RESERVE_MODE))
                        {
                            APE_DEBUG("switch to preview\n");
                            break;
                        }
                    }
                    else
                    {
                        if((decore_status.output_mode == MP_MODE)
                           || (decore_status.output_mode == RESERVE_MODE))
                        {
                            APE_DEBUG("switch to full screen\n");
                            break;
                        }
                    }

                    osal_task_sleep(100);
                    time_out += 100;
                }
            }
        }
    }

EXIT:
	MUTEX_UNLOCK();
	return ret;
}

int ape_vdec_pause(int pause_decode,int pause_output)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();

 	APE_IN();
 	
	APE_DEBUG("pause dec %d output %d\n", pause_decode, pause_output);
	
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_PAUSE_DECODE, &pause_decode, &pause_output);
	if(ret < 0)
	{
		APE_DEBUG("ret %d\n", ret);
	}	

	APE_OUT();

	MUTEX_UNLOCK();
	
	return ret;
}

int ape_vdec_trickseek( unsigned int status)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}

	MUTEX_LOCK();

 	APE_IN();
	if ( status == 1 )
		ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_FF_FB, NULL, NULL);
	else
		ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_NORMAL_PLAY, NULL, NULL);
	
	if(ret < 0)
	{
		APE_DEBUG("ret %d\n", ret);
	}	

	APE_OUT();

	MUTEX_UNLOCK();
	
	return ret;
	
}
int ape_vdec_reg_cb(struct vdec_io_reg_callback_para  *pPara)
{
	ape_priv_t* ape_priv = ape_get_priv();
	struct vdec_device *vdec_dev;
	int ret = -1;

	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}

	MUTEX_LOCK();

 	APE_IN();

	vdec_dev = (struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV);
	ret = vdec_io_control(vdec_dev, VDEC_IO_REG_CALLBACK, (UINT32)pPara);
	if(ret < 0)
	{
		APE_DEBUG("ret %d\n", ret);
	}	

	APE_OUT();

	MUTEX_UNLOCK();
	
	return ret;

}

int ape_vdec_get_trick_status()
{
	ape_priv_t* ape_priv = ape_get_priv();
	int status = 0;

	MUTEX_LOCK();
	ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_TRICK_FRAME, NULL, &status);
	MUTEX_UNLOCK();	

	return status;
}

void ape_video_rotation_dview()
{
	MUTEX_UNLOCK();
	ape_video_zoom(0, 0, 720, 2880);
	MUTEX_LOCK();
}


int ape_vdec_init(unsigned int codec_tag, video_config_info_t* info)
{
	int ret = 0;
	int pkt_sbm_idx, frm_out_sbm_idx, display_sbm_idx;
	ape_priv_t* ape_priv = ape_get_priv();
	
	if (NULL == ape_priv)
	{
		APE_DEBUG( "you should call the open func first!!\n");
		ret = -1;
		return -1;
	}

	ape_malloc_av_buf();
	
	MUTEX_LOCK();

 	APE_IN();
	if(ape_priv->bdma_mode == 1)
		ALiDMAOpen();

	if(ape_priv->v_init_flag == 1)
	{
		MUTEX_UNLOCK();

		APE_OUT();
		APE_DEBUG("ape vdec is already inited\n");
		return 0;
	}

	if(m_ape_dump_flag)
	{
		ape_dump_start();

		if(m_ape_dump_start)
		{
			ape_dump_es(BLOCK_VIDEO_CODEC_TYPE, (void *)&codec_tag, sizeof(codec_tag));
			ape_dump_es(BLOCK_VIDEO_INIT_INFO, info, sizeof(*info));
		}
	}	
	
	memcpy((void *)&m_ape_video_init_info, info, sizeof(*info));

	ret = ape_vdec_sbm_reset();
	if(ret < 0 )
	{
		APE_DEBUG( "clear video buffer fail\n");
		ret = -1;
		goto EXIT;
	}

	ape_priv->sbm_lock = 0;
	ape_priv->sbm_last_size = 0;
		
	if (!ape_check_codec_support(codec_tag))
	{
		ape_priv->codec_tag = codec_tag;
	}
	else
	{
		APE_DEBUG( "the codec_tag is not support, please check it!!\n");
		ret = -1;
		goto EXIT;
	}
	
	ret = ape_video_select_decoder(ape_priv, info->preview);
	
	if(ape_priv->pavsync_dev != NULL && RET_SUCCESS != avsync_ioctl(ape_priv->pavsync_dev, AVSYNC_IO_UNREG_CALLBACK, 0))
	{
		APE_DEBUG( "unregister avysnc callback fail\n");
		ret = -1;
		goto EXIT;
	}

	if(m_ape_rgb_output_enable == 1)
	{
		ape_video_enable_rgb_output();
		
		m_ape_rgb_output_enable = 0;
	}
	
	ret = ape_video_start(ape_priv);
	if(ret < 0 ){
		APE_DEBUG( "decore start video fail\n");
		ret = -1;
		goto EXIT;
	}
	
	ret = ape_config_video_init_par(info);
	if(ret < 0 )
	{
		APE_DEBUG( "decore config init para fali!!\n");
		ret = -1;
		goto EXIT;
	}
	
	pkt_sbm_idx = (VPKT_HDR_SBM_IDX<<16)|VPKT_DATA_SBM_IDX;
	frm_out_sbm_idx = DECV_OUT_SBM_IDX;
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_VIDEO_SBM_BUF, &pkt_sbm_idx, &frm_out_sbm_idx);
	if(ret != 0)
	{
		APE_DEBUG( "decore set video sbm fail\n");
		ret = -1;
		goto EXIT;
	}

	if (ape_priv->av_output_mode)
	{
    		display_sbm_idx = DECV_OUT_SBM_IDX;
	}
	else
	{
		display_sbm_idx = DISP_IN_SBM_IDX;
	}

	ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_DISPLAY_SBM_BUF, &display_sbm_idx, &display_sbm_idx);
	if(ret != 0)
	{
		APE_DEBUG( "decore set display sbm fail\n");
		ret = -1;
		goto EXIT;
	}

    if(ape_priv->rotate_flag)
    {
		ret = ape_video_decore_ioctl(ape_priv, VDEC_ROTATE_FRAME, &(ape_priv->frame_angle), NULL);
		if(ret != 0)
		{
			APE_DEBUG( "decore rotate frame fail: %d\n",ret);
			ret = -1;
			//goto EXIT;
		}	
    }

	if (ape_priv->zoom_flag)
	{
		ret = ape_video_decore_ioctl(ape_priv, VDEC_CFG_DISPLAY_RECT, \
		    &(ape_priv->src_rect), &(ape_priv->dst_rect));
		if(ret != 0)
		{
			APE_DEBUG( "decore set display rect fail: %d\n",ret);
			ret = -1;
			goto EXIT;
		}	
	}	
    
	ape_priv->v_init_flag = 1;	
	ret = 0;

	if(g_ali_ape_dbg_on)
	{
		m_ape_video_size = 0;
		m_ape_video_start = 0;
		m_ape_video_count = 0;
	}	

	if ( ape_priv->vid_rot_en )
		ape_video_rotation_dview();

EXIT:	
	if(ret != 0 && ape_priv->pavsync_dev != NULL)
		avsync_ioctl(ape_priv->pavsync_dev, AVSYNC_IO_REG_CALLBACK, 0);

	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);
	
 	APE_OUT();	
	MUTEX_UNLOCK();
	return ret;
}

int ape_vdec_release(unsigned int codec_tag)
{
	int ret;
	int pause_decode = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	
	if ((NULL == ape_priv) || (0 == ape_priv->v_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();

	APE_IN();
	if(ape_priv->bdma_mode == 1)
		ALiDMAReset();

#if 0	 
	if (codec_tag != ape_priv->codec_tag)
	{
		APE_DEBUG( "the release codec_tag must same as the init one!!\n");
		ret = -1;
		goto EXIT;
	}
#else
	codec_tag = ape_priv->codec_tag;
#endif

	ape_video_stop(ape_priv, FALSE, FALSE);

	if ((codec_tag == mpg2) || (codec_tag == h264))
	{
		ape_priv->codec_tag = mpg2;
		ape_video_select_decoder(ape_priv, 0);
	}
	
	ape_priv->codec_tag = codec_tag;
	ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_RELEASE, &pause_decode, &pause_decode);
	
	ape_priv->zoom_flag  = 0;
	ape_priv->src_rect.x = 0;
	ape_priv->src_rect.y = 0;
	ape_priv->src_rect.w = 720;
	ape_priv->src_rect.h = 2880;
	ape_priv->dst_rect.x = 0;
	ape_priv->dst_rect.y = 0;
	ape_priv->dst_rect.w = 720;
	ape_priv->dst_rect.h = 2880;

	ape_priv->v_init_flag = 0;

#ifdef DUMP_VIDEO_ES_DATA
	dump_stop();
#endif

	if(m_ape_dump_flag)
	{
		ape_dump_stop();
	}

EXIT:
	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);


	if(m_video_extra_buf_start)
	{
		free(m_video_extra_buf_start);
		
		m_video_extra_buf_start = NULL;
	}
		
 	APE_OUT();
	
	MUTEX_UNLOCK();	
	return ret;
}

int ape_adec_init(audio_config_info_t* config_info, int reset)
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();
	
	if (NULL == ape_priv)
	{
		APE_DEBUG( "the decoder is not opened!!\n");
		return -1;
	}

	ape_malloc_av_buf();
	
	MUTEX_LOCK();

 	APE_IN();
 
 	if(ape_priv->bdma_mode == 1)
	 	ALiDMAOpen();
	
	if(ape_priv->a_init_flag == 1)
	{
		MUTEX_UNLOCK();

		 APE_OUT();
		APE_DEBUG("ape adec is already inited\n");
		return 0;
	}

	ret = ape_adec_sbm_reset();
	if(ret < 0 ){
		MUTEX_UNLOCK();

		APE_OUT();
		
		APE_DEBUG( "clear audio buffer fail\n");
		return -1;
	}

	if(m_ape_dump_flag)
	{
		ape_dump_start();

		if(m_ape_dump_start)
		{
			if(config_info->extradata != NULL)
			{
				ape_dump_es(BLOCK_AUDIO_EXTRA_DATA, config_info->extradata, config_info->extradata_size);
			}
			ape_dump_es(BLOCK_AUDIO_INIT_INFO, config_info, sizeof(*config_info));
		}
	}

	memcpy((void *)&m_ape_audio_init_info, config_info, sizeof(*config_info));
	if(config_info->extradata != NULL)
	{
		if(m_audio_extra_buf_start)
		{
			free(m_audio_extra_buf_start);

			m_audio_extra_buf_start = NULL;
		}
		
		m_audio_extra_buf_size = config_info->extradata_size;
		if(m_audio_extra_buf_size > 0)
		{
			m_audio_extra_buf_start = malloc(m_audio_extra_buf_size);
			if(m_audio_extra_buf_start == NULL)
			{
				APE_DEBUG("malloc audio extra buffer fail size %d\n", m_audio_extra_buf_size);
			}
			else
			{
				memcpy(m_audio_extra_buf_start, config_info->extradata, m_audio_extra_buf_size);

				APE_DEBUG("backup extra data size %d\n", m_audio_extra_buf_size);
			}
		}		
	}
	
	ret = ape_config_audio_para(config_info, &reset);
	if(ret == 0)
	{
		if(ape_priv->pavsync_dev != NULL)
			avsync_ioctl(ape_priv->pavsync_dev, AVSYNC_IO_UNREG_CALLBACK, 0);
		ape_priv->a_init_flag = 1;
	}
	else
	{
		APE_DEBUG("%s ret %d\n", __FUNCTION__, ret);
		ret = -1;
	}
	
	if(g_ali_ape_dbg_on)
	{
		m_ape_audio_size = 0;
		m_ape_audio_start = 0;
		m_ape_audio_end = 0;
		m_ape_audio_count = 0;
	}

	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);
	
	MUTEX_UNLOCK();
	
	APE_OUT();
		
	return ret;
}

int ape_audio_release()
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = 0;

	if ((NULL == ape_priv) || (0 == ape_priv->a_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();
	
 	APE_IN();
		
	if(ape_priv->bdma_mode == 1)		
		ALiDMAReset();
	
	ret = ape_audio_decore_ioctl(ape_priv, DECA_DECORE_RLS, NULL, NULL);
	ape_priv->a_init_flag = 0;

	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);
	
 	APE_OUT();

	if(m_ape_dump_flag)
	{
		ape_dump_stop();
	}

	if(m_audio_extra_buf_start)
	{
		free(m_audio_extra_buf_start);

		m_audio_extra_buf_start = NULL;
	}
		
	MUTEX_UNLOCK();

	return ret;
}

int ape_audio_pause(int pause_decode,int pause_output)
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = 0;

	if ((NULL == ape_priv) || (0 == ape_priv->a_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();
	
 	APE_IN();	

	
	APE_DEBUG("pause dec %d output %d\n", pause_decode, pause_output);
	
	ret = ape_audio_decore_ioctl(ape_priv, DECA_DECORE_PAUSE_DECODE, &pause_decode, &pause_output);
	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);	
	
 	APE_OUT();	

	MUTEX_UNLOCK();

	return ret;
}

int ape_audio_buffer_clear()
{
	ape_priv_t* ape_priv = ape_get_priv();
	int ret = 0;

	if ((NULL == ape_priv) || (0 == ape_priv->a_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return -1;
	}
	
	MUTEX_LOCK();
	
 	APE_IN();
	
	ret = ape_adec_sbm_reset();
	if(ret < 0)
		APE_DEBUG("ret %d\n", ret);	

 	APE_OUT();	

	MUTEX_UNLOCK();
	
	return ret;
}

void ape_audio_reset()
{
	int pause_decode;
	ape_priv_t* ape_priv = ape_get_priv();	

	if ((NULL == ape_priv) || (0 == ape_priv->a_init_flag))
	{
		APE_DEBUG( "the decoder is not init!!\n");
		return;
	}
	
	MUTEX_LOCK();

 	APE_IN();
	
	if(ape_priv->bdma_mode == 1)
		ALiDMAReset();
	
	pause_decode = 1;
	ape_audio_decore_ioctl(ape_priv, DECA_DECORE_PAUSE_DECODE, &pause_decode, &pause_decode);
	ape_audio_decore_ioctl(ape_priv, DECA_DECORE_FLUSH, NULL, NULL);
	ape_adec_sbm_reset();
	pause_decode = 0;
	ape_audio_decore_ioctl(ape_priv, DECA_DECORE_PAUSE_DECODE, &pause_decode, &pause_decode);
	
 	APE_OUT();
	
	MUTEX_UNLOCK();
}

/***************************************************************************************

								clock related function

****************************************************************************************/

int ape_set_clock_divisor(int clock_fd, unsigned int divisor)
{
#if 1
	return -1;
#else
	int ret = 0;

	MUTEX_LOCK();
	
	ret = ioctl(clock_fd, STCIO_SET_DIVISOR, divisor);

	MUTEX_UNLOCK();

	return ret;
#endif	
}

int ape_set_clock_valid(int clock_fd, unsigned int valid)
{
#if 1
	return -1;
#else
	int ret = 0;

	MUTEX_LOCK();
	
	ret = ioctl(clock_fd, STCIO_SET_VALID, valid);

	MUTEX_UNLOCK();

	return ret;	
#endif
}

int ape_clock_pause(int clock_fd, unsigned int flag)
{
#if 1
	return -1;
#else
	int ret = 0;

	MUTEX_LOCK();
	
	ret = ioctl(clock_fd, STCIO_PAUSE_STC, flag);

	MUTEX_UNLOCK();

	return ret;	
#endif	
}

int ape_write_clock(int clock_fd, unsigned int value)
{
#if 1
	return -1;
#else
	int ret = 0;

	MUTEX_LOCK();
	
	ret = write(clock_fd, &value, sizeof(unsigned int));

	MUTEX_UNLOCK();

	return ret;		
#endif	
}

unsigned int ape_read_clock(int clock_fd)
{
	ape_priv_t* ape_priv = ape_get_priv();	
	unsigned int value = 0;
	int ret = 0;
	
	MUTEX_LOCK();
	
	if(ape_priv->pavsync_dev != NULL)
	{
		avsync_ioctl(ape_priv->pavsync_dev, AVSYNC_IO_GET_CURRENT_PLAY_PTS, (UINT32)&value);
	}	

	MUTEX_UNLOCK();	

#if 0	
#if 1
	ret = ape_audio_decore_ioctl(ape_priv, DECA_DECORE_GET_CUR_TIME, (void *)&value, NULL);
#else
	ret = read(clock_fd, &value, sizeof(unsigned int));
#endif	

	MUTEX_UNLOCK();
#endif	
	return value;
}

void ape_av_output_mode(int flag)
{
	ape_priv_t* ape_priv = ape_get_priv();

	APE_IN();
	if(NULL == ape_priv)
	{
	    return;
	}
	if (1 == flag)
		ape_priv->av_output_mode = 1;
	else
		ape_priv->av_output_mode = 0; 

	APE_DEBUG("output flag %d\n", flag);

	APE_OUT();
}

unsigned char ape_get_boot_media_flag()
{
	unsigned char boot_media_flag = 0;
	int video_fd = open("/dev/ali_video0", O_RDWR);
	
	if(video_fd <= 0) 
	{
		APE_DEBUG( "open video fail\n");
		return 0;
	}

	APE_IN();
	
	ioctl(video_fd, ALIVIDEOIO_GET_BOOTMEDIA_INFO, &boot_media_flag);
	APE_DEBUG("ape get boot_media flag:%d\n", boot_media_flag);
	close(video_fd);

	APE_OUT();
	return boot_media_flag;
}

void ape_video_rotation_enable(unsigned char flag, unsigned int angle)
{
	ape_priv_t* ape_priv = ape_get_priv();

	APE_IN();
	
	ape_priv->vid_rot_en = flag;
	ape_priv->vid_rot_angle = angle;

	APE_DEBUG("rotation flag %d\n", flag);

	APE_OUT();
}

static struct av_frame ape_last_av_frame_info; 
int ape_read_av_frame(struct ape_av_frame *frame)
{
	ape_priv_t* ape_priv = ape_get_priv();
	struct av_frame frame_info; 
	int decv_info_size, dis_info_size;
	struct vdec_decore_status vdec_status;
	UINT32 width = 0, height = 0;

	memset(&vdec_status, 0, sizeof(struct vdec_decore_status));	
	ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &vdec_status, &vdec_status);
	width = vdec_status.pic_width;
	ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &vdec_status, &vdec_status);
	height = vdec_status.pic_height;
	if ( width != 0 && height != 0 )
	{
		ape_video_get_disp_decv_info_size(&dis_info_size, &decv_info_size);
		if(decv_info_size)
		{
			ape_video_read_avframe(&frame_info);
			frame->width = frame_info.width;
			frame->height = frame_info.height;
			frame->data[0] = frame_info.data[0];
			frame->data[1] = frame_info.data[0];

			memcpy(&ape_last_av_frame_info, &frame_info, sizeof(frame_info));

			return 1;
		}
	}
	
	return 0;
}

int ape_delete_last_av_frame(void)
{
	ape_video_write_avframe(&ape_last_av_frame_info);
}

int ape_enable_rgb_output(void)
{
	m_ape_rgb_output_enable = 1;
}

#if 0
static void ape_rotation_task(UINT32 upara1,UINT32 upara2)
{
	ape_priv_t* ape_priv = ape_get_priv();
	UINT32 width = 0, height = 0;
	struct vdec_decore_status vdec_status;
	struct av_frame decv_frame_info, disp_frame_info; 
	struct rotation_rect rot_rect;
	int decv_info_size;
	struct rotation_updata_info UpdateInfo;
	unsigned int addr, data;
	int ret;
	struct rotation_rect dst_addr;

	memset(&vdec_status, 0, sizeof(struct vdec_decore_status));
	while (1)
	{
		 if ( ape_priv->vid_rot_en )
		 {
	 		ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &vdec_status, &vdec_status);
			width = vdec_status.pic_width;

	 		ape_video_decore_ioctl(ape_priv, VDEC_CMD_GET_STATUS, &vdec_status, &vdec_status);
			height = vdec_status.pic_height;

			if ( width != 0 && height != 0 )
			{
				memset(&decv_frame_info, 0, sizeof(struct av_frame));
				memset(&disp_frame_info, 0, sizeof(struct av_frame));
				memset(&rot_rect, 0, sizeof(struct rotation_rect));
				//ape_video_decore_ioctl(ape_priv, VDEC_CMD_ROTATION_MEM, NULL, NULL);
				ape_video_get_disp_decv_info_size(&(UpdateInfo.disp_info_size), &decv_info_size);
				if ( decv_info_size )
				{				
					dst_addr.src_y_addr = decv_frame_info.data[0];
					dst_addr.src_c_addr = decv_frame_info.data[1];
					ape_video_decore_ioctl(ape_priv, VDEC_CMD_ROTATION_UPDATA_FRMBUF, &dst_addr, &dst_addr);
				       if ( dst_addr.dst_y_addr != 0 && dst_addr.dst_c_addr != 0 )
				       {
						ape_video_read_avframe(&decv_frame_info);
						rot_rect.width = ( decv_frame_info.width >> 5 ) << 5;
						rot_rect.height = ( decv_frame_info.height >> 5 ) << 5;
						rot_rect.src_y_addr = decv_frame_info.data[0];
						rot_rect.src_c_addr = decv_frame_info.data[1];
						rot_rect.angle = ape_priv->vid_rot_angle;
						rot_rect.dst_y_addr = dst_addr.dst_y_addr;
						rot_rect.dst_c_addr = dst_addr.dst_c_addr;
						
						ret = ape_video_decore_ioctl(ape_priv, VDEC_CMD_ROTATION, &rot_rect, NULL);
						if ( ret == 0 )
						{
							memcpy(&disp_frame_info, &decv_frame_info, sizeof(struct av_frame));
							disp_frame_info.data[0] = rot_rect.dst_y_addr;
							disp_frame_info.data[1] = rot_rect.dst_c_addr;
							if ( rot_rect.angle == 90 || rot_rect.angle == 270 )
							{
								disp_frame_info.width = rot_rect.height;
								disp_frame_info.height = rot_rect.width;
								disp_frame_info.format = 1;
							}
							else if ( rot_rect.angle == 180 )
							{
								disp_frame_info.width = rot_rect.width;
								disp_frame_info.height = rot_rect.height;
								disp_frame_info.format = 0;
							}
							ape_video_write_avframe(&disp_frame_info);
							ape_video_decore_ioctl(ape_priv, VDEC_CMD_ROTATION_SET_FRMBUF, &rot_rect, &rot_rect);
						}
				       }			   
				}
			}

		 }

		 osal_task_sleep(10);
	}
}
#endif

int ape_open(void)
{
	int ret = 0;
	ape_priv_t* ape_priv = ape_get_priv();

	avsync_attach();
	avsync_open((struct avsync_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_AVSYNC));
	
	if (NULL == ape_priv)
	{
		ape_priv = ape_malloc_priv();
		if (NULL == ape_priv)
			return -1;
		memset(ape_priv, 0, sizeof(ape_priv_t));
		
		APE_DEBUG("ape malloc priv done!!\n");

#ifdef AUD_MERGE_BLOCK
        //APE_DEBUG("%s,%d, ape_priv = 0x%08x\n\n", __FUNCTION__, __LINE__, ape_priv);

        audio_merge_info_t *p_aud_merge_info = NULL;

        p_aud_merge_info = &(ape_priv->audio_merge_info);
        memset(p_aud_merge_info, 0, sizeof(audio_merge_info_t));

        p_aud_merge_info->merge_block_cnt = MERGE_BLOCK_COUNT;

        p_aud_merge_info->data_merge_len = MERGE_DATA_LENGTH; 

        if (NULL == p_aud_merge_info->data_merge_buf)
        {
            p_aud_merge_info->data_merge_buf = (unsigned char *)malloc(p_aud_merge_info->data_merge_len);

            if (NULL == p_aud_merge_info->data_merge_buf)
            {
                APE_LOG("\nmalloc data_merge_len: %d data_merge_buffer failed!\n\n", \
                       p_aud_merge_info->data_merge_len);
            }
            else
            {
                p_aud_merge_info->data_merge_buf_tmp = p_aud_merge_info->data_merge_buf;
            }

            #if 0
            APE_DEBUG("%s,%d, data_merge_len = %d, data_merge_buf = 0x%08x, data_merge_buf_tmp = 0x%08x\n\n", __FUNCTION__, __LINE__, \
                   p_aud_merge_info->data_merge_len, \
                   p_aud_merge_info->data_merge_buf, \
                   p_aud_merge_info->data_merge_buf_tmp);
            #endif
        }
        else
        {
            APE_LOG("data_merge_buf has been malloced! addr = 0x%08x\n\n", p_aud_merge_info->data_merge_buf);
        }

        ape_priv->audio_merge_mode = 0;
#endif

#ifdef ADR_IPC_ENABLE
		m_ape_mutex_id = adr_ipc_semget(ADR_IPC_APE, 0, 1);
		if(m_ape_mutex_id < 0)
		{
			APE_DEBUG("ape create mutex fail\n");
			return -1;
		}		
#else		
		m_ape_mutex_id = osal_mutex_create();
		if(m_ape_mutex_id == OSAL_INVALID_ID)
		{
			APE_DEBUG("ape create mutex fail\n");
			return -1;
		}
#endif		
		ape_priv->pavsync_dev = (struct avsync_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_AVSYNC);
	}
	
	if (0 == ape_priv->fd_open_flag)
	{
		ape_fd_open();
		APE_DEBUG("audio_data=%d,audio_head=%d,video_header=%d,video_data=%d", ape_priv->apkt_hdr_fd, 
																				ape_priv->apkt_data_fd, 
																				ape_priv->vpkt_hdr_fd, 
																				ape_priv->vpkt_data_fd);
		ape_priv->fd_open_flag = 1;//support open many times in a proc
	}
	
#if 0
	ret = ape_malloc_av_buf();
	if (-1 == ret)
	{
		APE_DEBUG( "ape malloc sbm buffer error!!\n");
	}
#endif

#ifdef HLD_DBG_ENABLE
	g_ali_ape_dbg_on  = 1;
#else
	if(ape_get_dbg_mode())
		g_ali_ape_dbg_on  = 1;
	else
		g_ali_ape_dbg_on  = 0;
#endif

	apedbg_register();

#if 0	
	/* create a task to for the dongle rotation */		
	{
		OSAL_T_CTSK t_ctsk;

		t_ctsk.stksz = 0x1000;
		t_ctsk.quantum = 15;
		t_ctsk.itskpri = OSAL_PRI_NORMAL;
		t_ctsk.para1 = 0;
		t_ctsk.para2 = 0;
		t_ctsk.name[0] = 'A';
		t_ctsk.name[1] = 'P';
		t_ctsk.name[2] = 'E';
		t_ctsk.task = (FP)ape_rotation_task;
		ape_priv->task_id = osal_task_create(&t_ctsk);
		if(ape_priv->task_id == OSAL_INVALID_ID)
		{
			APE_DEBUG("ape create the rotation task fail\n");
			return -1;
		}
		APE_DEBUG("ape create the rotation task %d done\n", ape_priv->task_id);

		osal_task_sleep(5);
	}	

	ape_priv->vid_rot_en = 0;
#endif


#if 0	
	if (0)
	{
		ape_av_output_mode(0);
		ape_video_rotation_enable(1, 90);
	}
#endif

 	APE_OUT();	
		
	APE_DEBUG("ape_open done task pid %d\n", getpid());

	return ret;
}

int ape_close(void)
{
	ape_priv_t* ape_priv = ape_get_priv();

	if (NULL == ape_priv)
	{
		APE_DEBUG( "it is already closed!!\n");

		return 0;
	}

	ape_vdec_release(0);
	ape_audio_release();
		
#ifdef ADR_IPC_ENABLE
	MUTEX_LOCK();
#endif

	if (1 == ape_priv->fd_open_flag)
	{
		ape_fd_close();
		ape_priv->fd_open_flag = 0;//support open many times in a proc

		m_ape_video_buf_init = 0;
		m_ape_audio_buf_init = 0;

        #if 0
		osal_task_delete(ape_priv->task_id);
        #endif
	}
	
#ifdef ADR_IPC_ENABLE

#else	
	osal_mutex_delete(m_ape_mutex_id);
#endif

#ifdef AUD_MERGE_BLOCK
    audio_merge_info_t *p_aud_merge_info = NULL;

    p_aud_merge_info = &(ape_priv->audio_merge_info);    

    if (p_aud_merge_info->data_merge_buf != NULL)
    {
        free(p_aud_merge_info->data_merge_buf);
        p_aud_merge_info->data_merge_buf = NULL;
    }
    
    ape_priv->audio_merge_mode = 0;
#endif

	ape_free_priv();

#ifdef ADR_IPC_ENABLE
	MUTEX_UNLOCK();
#endif

	APE_DEBUG("ape_close done task pid %d\n", getpid());

	return 0;
}


int ape_set_audio_merge_mode(unsigned int mode)
{
    int ret = 0;
    
	ape_priv_t* ape_priv = ape_get_priv();

#ifdef AUD_MERGE_BLOCK

    ape_priv->audio_merge_mode = mode;
  
#endif

    return ret;
}

int ape_set_merge_count(unsigned int merge_block_cnt)
{
    int ret = 0;
    
	ape_priv_t *ape_priv = ape_get_priv();

#ifdef AUD_MERGE_BLOCK

    audio_merge_info_t *p_aud_merge_info = NULL;

    p_aud_merge_info = &(ape_priv->audio_merge_info);

    p_aud_merge_info->merge_block_cnt = merge_block_cnt;
    
#endif

    return ret;
}

int ape_get_dbg_on()
{
	return g_ali_ape_dbg_on;
}

void ape_stop_bootmedia()
{
	#include <ali_pe_common.h>

	int pe_fd = 0;
	int ret = -1;

	pe_fd = open("/dev/ali_pe0",O_RDWR|O_CLOEXEC);

	struct ali_pe_rpc_pars pars;

	pars.type = RPC_VIDEO ;
	pars.API_ID = RPC_VIDEO_ENGINE_STOP;
	pars.arg_num = 0;
	ret = ioctl(pe_fd,ALIPEIO_MISC_RPC_OPERATION,&pars);
	if(ret != 0)
	{
		APE_LOG("stop bootmedia fail ret %d??\n",ret);
	}
	APE_LOG("stop bootmedia success, ret %d\n",ret);
    	
}

