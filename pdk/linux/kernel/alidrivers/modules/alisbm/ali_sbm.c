/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ali_sbm.c
 *
 *  Description: ali share buffer memory for cpu & see access
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.08.03       Dylan.Yang     0.1.000     First version Created
 ****************************************************************************/
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ali_transport.h>
#include <ali_sbm_common.h>
#include <linux/version.h>
#include <rpc_hld/ali_rpc_sbm.h>
#include <linux/ali_rpc.h>
#include <ali_cache.h>
#include <ali_shm.h>
#include "ali_sbm.h"
#include "ali_sbm_dbg.h"



static struct semaphore m_sbm_sem;

extern int sbm_see_create(int sbm_idx, int sbm_mode, void *sbm_init); //added by David@2013/06/09
extern int sbm_see_destroy(int sbm_idx, int sbm_mode);



//static volatile struct sbm_desc *sbm_info[SBM_NUM];
//static volatile struct sbm_desc_pkt *sbm_info_pkt[SBM_NUM];
volatile struct sbm_desc *sbm_info[SBM_NUM];
volatile struct sbm_desc_pkt *sbm_info_pkt[SBM_NUM];


int ali_rpc_sbm_lock(int sbm_idx)
{
    struct sbm_config *sbm_cfg = NULL;  
    int lock_mode, id;
        
    if(sbm_info[sbm_idx]) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
    
        id = sbm_ctx->mutex;
     sbm_cfg = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
        lock_mode = sbm_cfg->lock_mode;
    } else if(sbm_info_pkt[sbm_idx]) {
        struct sbm_desc_pkt *sbm_ctx_pkt = (struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
        
        id = sbm_ctx_pkt->mutex;
     sbm_cfg = (struct sbm_config *)__VSTMALI(sbm_ctx_pkt->sbm_cfg);        
        lock_mode = sbm_cfg->lock_mode;
    } else {
        PRF("sbm lock fail\n");
        return -1;
    }

    if(lock_mode == SBM_MUTEX_LOCK) {
        ali_rpc_mutex_lock(id, ALI_RPC_MAX_TIMEOUT);
    } else {
        ali_rpc_spinlock(id);
    }

    return 0;
}

int ali_rpc_sbm_unlock(int sbm_idx)
{
    struct sbm_config *sbm_cfg = NULL;  
    int lock_mode, id;
        
    if(sbm_info[sbm_idx]) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
        
        id = sbm_ctx->mutex;
     sbm_cfg = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
        lock_mode = sbm_cfg->lock_mode;
    } else if(sbm_info_pkt[sbm_idx]) {
        struct sbm_desc_pkt *sbm_ctx_pkt = (struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
        
        id = sbm_ctx_pkt->mutex;
     sbm_cfg = (struct sbm_config *)__VSTMALI(sbm_ctx_pkt->sbm_cfg);        
        lock_mode = sbm_cfg->lock_mode;
    } else {
        PRF("sbm unlock fail\n");
        return -1;
    }
    
    if(lock_mode == SBM_MUTEX_LOCK) {
        ali_rpc_mutex_unlock(id);
    } else {
        ali_rpc_spinunlock(id);
    }

    return 0;
}

int ali_rpc_sbm_create(int sbm_idx, struct sbm_config sbm_init)
{    
    struct sbm_desc *sbm_ctx = NULL;
    struct sbm_desc_pkt *sbm_ctx_pkt = NULL;
    struct sbm_config *sbm_cfg; 
    void *ptr = NULL;
    
    if(sbm_idx < SBM_NUM){
        if(sbm_init.wrap_mode == SBM_MODE_NORMAL && sbm_info[sbm_idx] == NULL){
            //sbm_info[sbm_idx] = ali_rpc_malloc_shared_mm(sizeof(struct sbm_desc));
            ptr = SHM_MALLOC(sbm_idx, 0, sizeof(struct sbm_desc));
            sbm_ctx = (struct sbm_desc *)(NONCACHE_ADDR(ptr));
            if(sbm_ctx == NULL){
                PRF("malloc sbm desc fail 1\n"); return -1;
            }
            sbm_info[sbm_idx] = (volatile struct sbm_desc *)sbm_ctx;
			memset(sbm_info[sbm_idx], 0, sizeof(struct sbm_desc));
            ptr = SHM_MALLOC(sbm_idx, 1, sizeof(struct sbm_config));
            sbm_cfg = (struct sbm_config *)(NONCACHE_ADDR(ptr));
            sbm_ctx->sbm_cfg = sbm_info[sbm_idx]->sbm_cfg = (struct sbm_config *)(__VMTSALI(ptr));
            if(sbm_ctx->sbm_cfg == NULL){
                PRF("malloc sbm desc fail\n"); return -1;
            }else{
                sbm_cfg->buffer_addr = sbm_init.buffer_addr;
                sbm_cfg->buffer_size = sbm_init.buffer_size;
                sbm_cfg->block_size = sbm_init.block_size;
                sbm_cfg->reserve_size = sbm_init.reserve_size;
                sbm_cfg->wrap_mode = sbm_init.wrap_mode;
                sbm_cfg->lock_mode = sbm_init.lock_mode;
                sbm_ctx->read_pos = 0;
                sbm_ctx->write_pos = 0;
                sbm_ctx->valid_size = 0;
                sbm_ctx->status = SBM_CPU_READY;
                sbm_ctx->mutex = -1;
                if(sbm_init.lock_mode == SBM_MUTEX_LOCK) {
                    sbm_ctx->mutex = SBM_MUTEX_CREATE();
                    if(sbm_ctx->mutex <= 0){
                        PRF("create mutex fail\n"); 
               return -1;
                    }
              PRF("mutex %d id %d\n", (int)sbm_ctx, sbm_ctx->mutex);
                }
                sbm_see_create(sbm_idx, sbm_init.wrap_mode, (void *)__VMTSALI(sbm_ctx));
            }
        }else if(sbm_init.wrap_mode == SBM_MODE_PACKET && sbm_info_pkt[sbm_idx] == NULL){
            //sbm_info_pkt[sbm_idx] = ali_rpc_malloc_shared_mm(sizeof(struct sbm_desc));
            ptr = SHM_MALLOC(sbm_idx, 0, sizeof(struct sbm_desc_pkt));
            sbm_ctx_pkt = (struct sbm_desc_pkt *)(NONCACHE_ADDR(ptr));
            if(sbm_ctx_pkt == NULL){
                PRF("malloc sbm desc fail 3\n"); return -1;
            }
            sbm_info_pkt[sbm_idx] = (volatile struct sbm_desc_pkt *)sbm_ctx_pkt;
			memset(sbm_info_pkt[sbm_idx], 0, sizeof(struct sbm_desc_pkt));			
            ptr = SHM_MALLOC(sbm_idx, 1, sizeof(struct sbm_config));
            sbm_cfg = (struct sbm_config *)(NONCACHE_ADDR(ptr));           
            sbm_ctx_pkt->sbm_cfg = sbm_info_pkt[sbm_idx]->sbm_cfg = (struct sbm_config *)(__VMTSALI(ptr));
            if(sbm_ctx_pkt->sbm_cfg == NULL){
                PRF("malloc sbm desc fail 4\n"); return -1;
            }else{
                sbm_cfg->buffer_addr = sbm_init.buffer_addr;
                sbm_cfg->buffer_size = sbm_init.buffer_size;
                sbm_cfg->block_size = sbm_init.block_size;
                sbm_cfg->reserve_size = sbm_init.reserve_size;
                sbm_cfg->wrap_mode = sbm_init.wrap_mode;
                sbm_cfg->lock_mode = sbm_init.lock_mode;
                sbm_ctx_pkt->read_pos = sbm_init.buffer_addr;
                sbm_ctx_pkt->write_pos = sbm_init.buffer_addr;
                sbm_ctx_pkt->tmp_write_pos = sbm_init.buffer_addr;
                sbm_ctx_pkt->last_read_pos = sbm_init.buffer_addr;
                sbm_ctx_pkt->read_wrap_around = 0;
                sbm_ctx_pkt->write_wrap_around = 0;
                sbm_ctx_pkt->valid_size = 0;
                sbm_ctx_pkt->pkt_num = 0;
                sbm_ctx_pkt->status = SBM_CPU_READY;
                sbm_ctx_pkt->mutex = -1;
                if(sbm_init.lock_mode == SBM_MUTEX_LOCK) {
                    sbm_ctx_pkt->mutex = SBM_MUTEX_CREATE();
                    if(sbm_ctx_pkt->mutex <= 0){
                        PRF("create mutex fail\n"); 
                        return -1;
                    }
                    PRF("mutex %x id %d\n", (int)sbm_ctx_pkt, sbm_ctx_pkt->mutex);
                }
                sbm_see_create(sbm_idx, sbm_init.wrap_mode, (void *)__VMTSALI(sbm_ctx_pkt));
            }
        }else{
            return 0;
        }
    }

    return 0;
}

int ali_rpc_sbm_destroy(int sbm_idx, int sbm_mode)
{
    struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
    struct sbm_desc_pkt *sbm_ctx_pkt = (struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
    struct sbm_config *sbm_cfg = NULL;
    
    if(sbm_idx < SBM_NUM){
        if(sbm_mode == SBM_MODE_NORMAL && sbm_ctx){
        PRF("mutex %d mode %d\n", sbm_ctx->mutex, sbm_ctx->sbm_cfg->lock_mode);
            if(sbm_ctx->mutex > 0){
                SBM_MUTEX_DELETE(sbm_ctx->mutex);
            }
            sbm_see_destroy(sbm_idx, sbm_mode);
            sbm_ctx->status &= ~SBM_CPU_READY;
            sbm_cfg = (struct sbm_config *)CACHE_ADDR(sbm_ctx->sbm_cfg);
            sbm_ctx = (struct sbm_desc *)CACHE_ADDR(sbm_ctx);
            SHM_FREE(sbm_cfg);
            sbm_ctx->sbm_cfg = NULL;
            SHM_FREE(sbm_ctx);
            sbm_ctx = NULL;
            sbm_info[sbm_idx] = NULL;
            //ali_rpc_free_shared_mm((void *)sbm_ctx, sizeof(struct sbm_desc));
        }else if(sbm_mode == SBM_MODE_PACKET && sbm_ctx_pkt){
        PRF("mutex %d mode %d\n", sbm_ctx_pkt->mutex, sbm_ctx_pkt->sbm_cfg->lock_mode);        
            if(sbm_ctx_pkt->mutex > 0){
                SBM_MUTEX_DELETE(sbm_ctx_pkt->mutex);
            }
            sbm_see_destroy(sbm_idx, sbm_mode);
            sbm_ctx_pkt->status &= ~SBM_CPU_READY;
            sbm_cfg = (struct sbm_config *)CACHE_ADDR(sbm_ctx_pkt->sbm_cfg);
            sbm_ctx_pkt = (struct sbm_desc_pkt *)CACHE_ADDR(sbm_ctx_pkt);
            SHM_FREE(sbm_cfg);
            sbm_ctx_pkt->sbm_cfg = NULL;
            SHM_FREE(sbm_ctx_pkt);
            sbm_ctx_pkt = NULL;
            sbm_info_pkt[sbm_idx] = NULL;
            //ali_rpc_free_shared_mm((void *)sbm_ctx_pkt, sizeof(struct sbm_desc_pkt));
        }
    }

    return 0;
}

int ali_rpc_sbm_request_read(int sbm_idx, void **buf_start, int *buf_size)
{
    struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
    struct sbm_config info_fix;
    struct sbm_desc info;
    int read_size = 0;

    if(sbm_ctx == NULL || sbm_idx < 0 || sbm_idx >= SBM_NUM){
        printk("%s,%d,sbm_idx:%d,sbm_ctx:0x%x,invalid param!\n",
            __FUNCTION__, __LINE__, sbm_idx, sbm_ctx);      
        return SBM_REQ_FAIL;
    }

    ali_rpc_sbm_lock(sbm_idx);

    sbm_ctx->request_read_total_cnt++;
        
    if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)){   
        sbm_ctx->request_read_status_err_cnt++;                 
        ali_rpc_sbm_unlock(sbm_idx);
        return SBM_REQ_FAIL;        
    }

    memcpy(&info, sbm_ctx, sizeof(struct sbm_desc));
    memcpy(&info_fix, (void *)__VSTMALI(sbm_ctx->sbm_cfg), sizeof(struct sbm_config));
    
    ali_rpc_sbm_unlock(sbm_idx);

    if(info.valid_size){
        if(info.read_pos < info.write_pos)
            read_size = info.write_pos - info.read_pos;
        else
            read_size = info_fix.buffer_size - info.read_pos;
        
        if(read_size < *buf_size){
            *buf_size = read_size;
        }

        *buf_start = (char *)(info_fix.buffer_addr + info.read_pos);

        sbm_ctx->request_read_ok_cnt++;

        return SBM_REQ_OK;
    }

    sbm_ctx->request_read_empty_cnt++;

    return SBM_REQ_FAIL;
}

void ali_rpc_sbm_update_read(int sbm_idx, int update_size)
{
    struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);  
    struct sbm_config *sbm_fix ;

    if(sbm_ctx == NULL || sbm_idx < 0 || sbm_idx >= SBM_NUM){
        printk("%s,%d,sbm_idx:%d,sbm_ctx:0x%x,invalid param!\n",
            __FUNCTION__, __LINE__, sbm_idx, sbm_ctx);              
        return;
    }
    sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
    if(sbm_fix == NULL){
        printk("%s,%d,sbm_fix:0x%x empty!\n", __FUNCTION__, __LINE__, sbm_fix);             
        return;
    }
    ali_rpc_sbm_lock(sbm_idx);  

    sbm_ctx->update_read_total_cnt++;
    
    if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)){   
        sbm_ctx->update_read_status_err_cnt++;          
        ali_rpc_sbm_unlock(sbm_idx);
        return;     
    }

    sbm_ctx->read_pos += update_size;
    while(sbm_ctx->read_pos >= sbm_fix->buffer_size){
        sbm_ctx->read_pos -= sbm_fix->buffer_size;
    }

    sbm_ctx->valid_size -= update_size;

    sbm_ctx->update_read_ok_cnt++;
    
    ali_rpc_sbm_unlock(sbm_idx);    
}

int ali_rpc_sbm_request_write(int sbm_idx, void **buf_start, int *buf_size)
{   
    unsigned int buffer_end = 0, end_addr = 0;
    int ret = SBM_REQ_OK, free_size = 0, req_size = *buf_size, write_size = 0;
    
    if(sbm_idx < 0 || sbm_idx >= SBM_NUM) {
        printk("%s,%d,sbm_idx:%d out of range!\n", __FUNCTION__, __LINE__, sbm_idx);
        return SBM_REQ_FAIL;
    }

    ali_rpc_sbm_lock(sbm_idx);

    if(sbm_info[sbm_idx]) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)sbm_info[sbm_idx];
        struct sbm_config *sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);

        sbm_ctx->request_write_total_cnt++;

        if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)) {  
            sbm_ctx->request_write_status_err_cnt++;            
            ali_rpc_sbm_unlock(sbm_idx);
            return SBM_REQ_FAIL;        
        }

        free_size = sbm_fix->buffer_size - sbm_ctx->valid_size;
        if(free_size > sbm_fix->reserve_size ) {
            if(sbm_ctx->write_pos >= sbm_ctx->read_pos) {
                write_size = sbm_fix->buffer_size - sbm_ctx->write_pos;
            } else {
                write_size = sbm_ctx->read_pos - sbm_ctx->write_pos - sbm_fix->reserve_size;
            }
            
            if(write_size > sbm_fix->block_size) {
                write_size = sbm_fix->block_size;
            }
            if(req_size < write_size) {
                write_size = req_size;
            }

            *buf_start = (char *)(sbm_fix->buffer_addr + sbm_ctx->write_pos);
            *buf_size = write_size;
            sbm_ctx->request_write_ok_cnt++;
        } else {
            sbm_ctx->request_write_full_cnt++;  
            ret = SBM_REQ_BUSY;
        }
    } else if (sbm_info_pkt[sbm_idx]) {
        struct sbm_desc_pkt *sbm_ctx = (struct sbm_desc_pkt *)sbm_info_pkt[sbm_idx];
        struct sbm_config *sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);

        sbm_ctx->request_write_total_cnt++;
        
        if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)) {  
            sbm_ctx->request_write_status_err_cnt++;                
            ali_rpc_sbm_unlock(sbm_idx);
            return SBM_REQ_FAIL;        
        }
        
        buffer_end = sbm_fix->buffer_addr + sbm_fix->buffer_size;

        if(!sbm_ctx->write_wrap_around) {
            end_addr = sbm_ctx->read_wrap_around ? sbm_ctx->last_read_pos : buffer_end;
            if(sbm_ctx->write_pos + req_size <= end_addr) {
               *buf_start = (void *)sbm_ctx->write_pos;
               sbm_ctx->request_write_ok_cnt++;
            } else {
                if(!sbm_ctx->read_wrap_around) {
                    if(sbm_fix->buffer_addr + req_size + sbm_fix->reserve_size <= sbm_ctx->last_read_pos) {
                        *buf_start = (void *)sbm_fix->buffer_addr;
                        sbm_ctx->request_write_ok_cnt++;
                    } else {
                        sbm_ctx->request_write_full_cnt++;  
                        ret = SBM_REQ_BUSY;
                    }
                } else {
                    sbm_ctx->request_write_full_cnt++;  
                    ret = SBM_REQ_BUSY;
                }
            }
        } else {
            if(sbm_ctx->tmp_write_pos + req_size + sbm_fix->reserve_size <= sbm_ctx->last_read_pos) {
                *buf_start = (void *)sbm_ctx->tmp_write_pos;
                sbm_ctx->request_write_ok_cnt++;
            } else {
                ret = SBM_REQ_BUSY;
                sbm_ctx->request_write_full_cnt++;              
            }
        }   
    } else {
        printk("%s,%d,sbm_idx:%d empty!\n", __FUNCTION__, __LINE__, sbm_idx);    
        ret = SBM_REQ_FAIL;
    }

    ali_rpc_sbm_unlock(sbm_idx);
    
    return ret;
}

void ali_rpc_sbm_update_write(int sbm_idx, int update_size)
{
    struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);  
    struct sbm_config *sbm_fix;

    if(sbm_ctx == NULL || sbm_idx < 0 || sbm_idx >= SBM_NUM){
        printk("%s,%d,sbm_idx:%d,sbm_ctx:0x%x,invalid param!\n",
            __FUNCTION__, __LINE__, sbm_idx, sbm_ctx);              
        return;
    }
    sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
    if(sbm_fix == NULL){
        printk("%s,%d,sbm_fix:0x%x empty!\n", __FUNCTION__, __LINE__, sbm_fix);         
        return;
    }
    
    ali_rpc_sbm_lock(sbm_idx);

    sbm_ctx->update_write_total_cnt++;
        
    if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)){   
        sbm_ctx->update_write_status_err_cnt++;         
        ali_rpc_sbm_unlock(sbm_idx);
        return;     
    }

    sbm_ctx->write_pos += update_size;
    while(sbm_ctx->write_pos >= sbm_fix->buffer_size){
        sbm_ctx->write_pos -= sbm_fix->buffer_size;
    }

    sbm_ctx->valid_size += update_size;

    sbm_ctx->update_write_ok_cnt++;
    
    ali_rpc_sbm_unlock(sbm_idx);
}

int ali_rpc_sbm_write_pkt(int sbm_idx, const char *buf_start, size_t buf_size)
{
    struct sbm_desc_pkt *sbm_ctx = ( struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
    struct sbm_config *sbm_fix;
    unsigned int buffer_end = 0, end_addr = 0;

    if(sbm_ctx == NULL || sbm_idx < 0 || sbm_idx >= SBM_NUM){
        printk("%s,%d,sbm_idx:%d,sbm_ctx:0x%x,invalid param!\n",
            __FUNCTION__, __LINE__, sbm_idx, sbm_ctx);          
        return SBM_REQ_FAIL;
    }
    sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
    if(sbm_fix == NULL){
        printk("%s,%d,sbm_fix:0x%x empty!\n", __FUNCTION__, __LINE__, sbm_fix);         
        return SBM_REQ_FAIL;
    }
    ali_rpc_sbm_lock(sbm_idx);

    sbm_ctx->pkt_write_total_cnt++;

    if(sbm_ctx->status != (SBM_CPU_READY|SBM_SEE_READY)){   
        sbm_ctx->pkt_write_status_err_cnt++;            
        ali_rpc_sbm_unlock(sbm_idx);
        return SBM_REQ_FAIL;        
    }
    
    buffer_end = sbm_fix->buffer_addr + sbm_fix->buffer_size;

    if(!sbm_ctx->write_wrap_around){
        end_addr = sbm_ctx->read_wrap_around ? sbm_ctx->last_read_pos : buffer_end;
        if(sbm_ctx->write_pos + buf_size <= end_addr){
           if(copy_from_user((char *)__VSTMALI(sbm_ctx->write_pos), buf_start, buf_size)){
               sbm_ctx->pkt_write_status_copy_from_user_err_cnt++;
               //return -EFAULT;
           }
#if defined(CONFIG_ALI_CHIP_M3921)      

#else          
           __CACHE_FLUSH_ALI(__VSTMALI(sbm_ctx->write_pos), buf_size);
#endif
           sbm_ctx->write_pos += buf_size;
        }else{
            if(!sbm_ctx->read_wrap_around){
                if(sbm_fix->buffer_addr + buf_size + sbm_fix->reserve_size <= sbm_ctx->last_read_pos){
                    if(copy_from_user((char *)__VSTMALI(sbm_fix->buffer_addr), buf_start, buf_size)){
                        sbm_ctx->pkt_write_status_copy_from_user_err_cnt++;
                      //  return -EFAULT;
                    }
#if defined(CONFIG_ALI_CHIP_M3921)      

#else                   
                    __CACHE_FLUSH_ALI(__VSTMALI(sbm_fix->buffer_addr), buf_size);
#endif
                    sbm_ctx->tmp_write_pos = sbm_fix->buffer_addr + buf_size;
                    sbm_ctx->write_wrap_around = 1;
                }else{
                    sbm_ctx->pkt_write_full_cnt++;
                    ali_rpc_sbm_unlock(sbm_idx);
                    return SBM_REQ_BUSY;
                }
            }else{
                sbm_ctx->pkt_write_full_cnt++;
                ali_rpc_sbm_unlock(sbm_idx);
                return SBM_REQ_BUSY;
            }
        }
    }else{
        if(sbm_ctx->tmp_write_pos + buf_size + sbm_fix->reserve_size <= sbm_ctx->last_read_pos){
            if(copy_from_user((void *)__VSTMALI(sbm_ctx->tmp_write_pos), (void *)buf_start, buf_size))
           {
           	//return -EFAULT;
            	}
#if defined(CONFIG_ALI_CHIP_M3921)      

#else           
            __CACHE_FLUSH_ALI(__VSTMALI(sbm_ctx->tmp_write_pos), buf_size);
#endif
            sbm_ctx->tmp_write_pos += buf_size;
        }else{
            sbm_ctx->pkt_write_full_cnt++;
            ali_rpc_sbm_unlock(sbm_idx);
            return SBM_REQ_BUSY;
        }
    }   

    sbm_ctx->valid_size += buf_size;
    sbm_ctx->pkt_num++;

    sbm_ctx->pkt_write_ok_cnt++;
    
    ali_rpc_sbm_unlock(sbm_idx);

    return SBM_REQ_OK;
}

int ali_rpc_sbm_reset(int sbm_idx, int sbm_mode)
{
    if(sbm_idx < 0 || sbm_idx >= SBM_NUM) {
        return -1;
    }

    if(sbm_mode == SBM_MODE_NORMAL) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
        if(sbm_ctx) {
            ali_rpc_sbm_lock(sbm_idx);
            sbm_ctx->read_pos = 0;
            sbm_ctx->write_pos = 0;
            sbm_ctx->valid_size = 0;
            ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    } else {
        struct sbm_desc_pkt *sbm_ctx = (struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
        if(sbm_ctx) {
            struct sbm_config *sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
            ali_rpc_sbm_lock(sbm_idx);
            sbm_ctx->read_pos = sbm_fix->buffer_addr;
            sbm_ctx->write_pos = sbm_fix->buffer_addr;
            sbm_ctx->tmp_write_pos = sbm_fix->buffer_addr;
            sbm_ctx->last_read_pos = sbm_fix->buffer_addr;
            sbm_ctx->read_wrap_around = 0;
            sbm_ctx->write_wrap_around = 0;
            sbm_ctx->valid_size = 0;
            sbm_ctx->pkt_num = 0;
            ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    }

    return 0;
}

int ali_rpc_sbm_show_valid_size(int sbm_idx, int sbm_mode, unsigned int *valid_size)
{
    if(sbm_idx < 0 || sbm_idx >= SBM_NUM) {
        return -1;
    }
        
    if(sbm_mode == SBM_MODE_NORMAL) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
        if(sbm_ctx) {
            //ali_rpc_sbm_lock(sbm_idx);    
            *valid_size = sbm_ctx->valid_size;
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    } else {
        struct sbm_desc_pkt *sbm_ctx = (struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
        if(sbm_ctx) {
            //ali_rpc_sbm_lock(sbm_idx);    
            *valid_size = (sbm_ctx->pkt_num == 0) ? 0 : sbm_ctx->valid_size;
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    }

    return 0;
}

int ali_rpc_sbm_show_free_size(int sbm_idx, int sbm_mode, unsigned int *free_size)
{
    struct sbm_config *sbm_fix = NULL;

    if(sbm_idx < 0 || sbm_idx >= SBM_NUM) {
        return -1;
    }

    if(sbm_mode == SBM_MODE_NORMAL) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)(sbm_info[sbm_idx]);
        if(sbm_ctx) {
            sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
            //ali_rpc_sbm_lock(sbm_idx);    
            *free_size = sbm_fix->buffer_size - sbm_ctx->valid_size;
            if(*free_size < sbm_fix->reserve_size) {
                *free_size  = 0;
            }else{
                *free_size -=  sbm_fix->reserve_size;
            }
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    } else {
        struct sbm_desc_pkt *sbm_ctx = ( struct sbm_desc_pkt *)(sbm_info_pkt[sbm_idx]);
        if(sbm_ctx) {
            sbm_fix = (struct sbm_config *)__VSTMALI(sbm_ctx->sbm_cfg);
            //ali_rpc_sbm_lock(sbm_idx);
            *free_size = sbm_fix->buffer_size - sbm_ctx->valid_size;
            if(*free_size < sbm_fix->reserve_size) {
                *free_size = 0;
            }else{
                *free_size -=  sbm_fix->reserve_size;
            }
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    }

    return 0;
}

int ali_rpc_sbm_show_pkt_num(int sbm_idx, int sbm_mode, unsigned int *pkt_num)
{
    if(sbm_idx < 0 || sbm_idx >= SBM_NUM) {
        return -1;
    }
        
    if(sbm_mode == SBM_MODE_NORMAL) {
        struct sbm_desc *sbm_ctx = (struct sbm_desc *)sbm_info[sbm_idx];
        if(sbm_ctx) {
            //ali_rpc_sbm_lock(sbm_idx);    
            *pkt_num = 0;
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    } else {
        struct sbm_desc_pkt *sbm_ctx = (struct sbm_desc_pkt *)sbm_info_pkt[sbm_idx];
        if(sbm_ctx) {
            //ali_rpc_sbm_lock(sbm_idx);    
            *pkt_num = sbm_ctx->pkt_num;
            //ali_rpc_sbm_unlock(sbm_idx);
        } else {
            return -1;
        }
    }

    return 0;
}

static int ali_sbm_open(struct inode *inode, struct file *file)
{
    struct sbm_dev *sbm_devp;

	ali_sbm_api_show("%s,%d,go\n", __FUNCTION__, __LINE__);

    down(&m_sbm_sem);
    
    /* Get the per-device structure that contains this cdev */
    sbm_devp = container_of(inode->i_cdev, struct sbm_dev, cdev);

    /* Easy access to sbm_devp from rest of the entry points */
    file->private_data = sbm_devp;

    if(sbm_devp->open_count == 0){
        /* Initialize some fields */
        sbm_devp->status = 0;
    }

    sbm_devp->open_count++;

    up(&m_sbm_sem); 

	ali_sbm_api_show("%s,%d,done.name:%s,sbm_number:%d,status:%d,open_count:%d\n",
		__FUNCTION__, __LINE__, sbm_devp->name, sbm_devp->sbm_number,
		sbm_devp->status, sbm_devp->open_count);	

    return 0;
}

extern void ali_decv_rpc_release(void);
extern void ali_deca_rpc_release(void);

static int sbm_release(struct inode *inode, struct file *file)
{
    struct sbm_dev *sbm_devp = file->private_data;

	ali_sbm_api_show("%s,%d,go\n", __FUNCTION__, __LINE__);

    down(&m_sbm_sem);

    sbm_devp->open_count--;

    if(sbm_devp->open_count == 0)
	{
        ali_decv_rpc_release();
        ali_deca_rpc_release();
        
        if(sbm_devp->status != 0)
        {
            ali_rpc_sbm_destroy(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode);
            sbm_devp->status = 0;
        }
    }
    else if(sbm_devp->open_count < 0)
    {
        ali_sbm_api_show("sbm %d open count fail %d\n", sbm_devp->sbm_number, sbm_devp->open_count);
    }
    
    up(&m_sbm_sem);

	ali_sbm_api_show("%s,%d,done.name:%s,sbm_number:%d,status:%d,open_count:%d\n",
		__FUNCTION__, __LINE__, sbm_devp->name, sbm_devp->sbm_number,
		sbm_devp->status, sbm_devp->open_count);	
      
    return 0;
}

static ssize_t sbm_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    struct sbm_dev *sbm_devp = file->private_data;
    int ret = 0, req_size = count, read_size = 0;
    char *req_buf = NULL;

	#if 0
    ali_sbm_api_show("%s,%d,go.name:%s,status:%d,open_count:%d,count\n",
    	__FUNCTION__, __LINE__, sbm_devp->name, sbm_devp->status, 
    	sbm_devp->open_count, count);		
    #endif
	
    if(down_interruptible(&m_sbm_sem))
    {
       printk("%s : ali sbm down sem fail\n", __FUNCTION__);
       return -1;    
    }

    if(sbm_devp->open_count <= 0)
    {
        up(&m_sbm_sem);
        printk("sbm not opened\n");
        return -1;
    }
    
    ret = ali_rpc_sbm_request_read(sbm_devp->sbm_number, (void **)&req_buf, &req_size);
    if(ret == SBM_REQ_OK)
	{
        req_buf = (char *)__VSTMALI(req_buf);
#if defined(CONFIG_ALI_CHIP_M3921)

#else
        __CACHE_INV_ALI((unsigned long)req_buf, req_size);
#endif
        if(copy_to_user(buf, req_buf, req_size))
        {
            printk("%s,%d,copy_to_user() failed!\n", __FUNCTION__, __LINE__); 
		    return -EFAULT;			  
		}
		
        ali_rpc_sbm_update_read(sbm_devp->sbm_number, req_size);

        /* for buffer wrap around, continue read the left data */
        count -= req_size;
        read_size += req_size;
        if(count)
		{
            buf += req_size;
            req_buf = NULL;
            req_size = count;
            ret = ali_rpc_sbm_request_read(sbm_devp->sbm_number, (void **)&req_buf, &req_size);
            if(ret == SBM_REQ_OK){
            req_buf = (char *)__VSTMALI(req_buf);
#if defined(CONFIG_ALI_CHIP_M3921)      

#else
                __CACHE_INV_ALI((unsigned long)req_buf, req_size);
#endif
                if(copy_to_user(buf, req_buf, req_size))
                    return -EFAULT;
                ali_rpc_sbm_update_read(sbm_devp->sbm_number, req_size);
                read_size += req_size;
            }
        }
    }

    up(&m_sbm_sem); 
    return read_size;
}

static ssize_t sbm_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    struct sbm_dev *sbm_devp = file->private_data;
    // struct sbm_config *sbm_fix = &sbm_devp->sbm_cfg;
    int ret = 0, req_size = count, write_count = 0;
    char *req_buf = NULL;

	ali_sbm_stats_write_go_cnt(sbm_devp, 1);

    if(down_interruptible(&m_sbm_sem))
    {
       printk("%s : ali sbm down sem fail\n", __FUNCTION__);
	   ali_sbm_stats_write_mutex_fail_cnt(sbm_devp, 1);
       return -1;    
    }
	
    if((sbm_devp->open_count <= 0) || (sbm_devp->status == 0))
    {
    	ali_sbm_stats_write_ill_status_cnt(sbm_devp, 1);
        up(&m_sbm_sem);
        printk("sbm not opened\n");
        return -1;
    }
       
    if(sbm_devp->sbm_cfg.wrap_mode == SBM_MODE_NORMAL)
	{
        do
		{
            ret = ali_rpc_sbm_request_write(sbm_devp->sbm_number, (void **)&req_buf, &req_size);
            if(ret == SBM_REQ_OK && req_size > 0)
			{
                if(copy_from_user((void*)__VSTMALI(req_buf), (void*)buf, req_size))
                {
                    printk("%s,%d,copy_from_user() failed!\n", __FUNCTION__, __LINE__); 
                    return -EFAULT;
                }
                #if defined(CONFIG_ALI_CHIP_M3921)      
                #else                 
                __CACHE_FLUSH_ALI((unsigned long)__VSTMALI(req_buf), req_size);
                #endif
                ali_rpc_sbm_update_write(sbm_devp->sbm_number, req_size);
                
                buf += req_size;
                write_count += req_size;
                count -= req_size;
                req_size = count;
                sbm_devp->is_full = 0;
            }
            else
            {
                sbm_devp->is_full = (ret == SBM_REQ_BUSY) ? 1 : 0;                      
                break;
            }
        }while(count > 0);
    }
	else
    {
        if(count > 0)
		{
            ret = ali_rpc_sbm_write_pkt(sbm_devp->sbm_number, buf, count);
            if(ret == SBM_REQ_OK)
			{
                write_count = count;
                sbm_devp->is_full = 0;
            }
			else
			{
                sbm_devp->is_full = (ret == SBM_REQ_BUSY) ? 1 : 0;
            }
        }
    }
    
    up(&m_sbm_sem);

    ali_sbm_stats_write_ok_cnt(sbm_devp, 1);
	
    return write_count;
}

static int sbm_req_buf_info(struct sbm_dev *sbm_devp,struct sbm_req_buf *info)
{
    int ret;
    ret = ali_rpc_sbm_request_write(sbm_devp->sbm_number, (void **)&info->phy_addr, &info->req_size);
    if(ret != SBM_REQ_OK)
    {
        //printk("<0>""req buf fail!!\n");
        info->req_size = 0;;
    }

    return info->req_size;
    
}

static int sbm_update_buf_info(struct sbm_dev *sbm_devp, struct sbm_req_buf *info)
{
    ali_rpc_sbm_update_write(sbm_devp->sbm_number, info->req_size);
    return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long sbm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int sbm_ioctl(struct inode *node, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    struct sbm_dev *sbm_devp = file->private_data;
    int ret = 0;

    if ((cmd != SBMIO_SHOW_VALID_SIZE) && (cmd != SBMIO_SHOW_FREE_SIZE) &&
		(cmd != SBMIO_SHOW_TOTAL_SIZE))
    {
    	ali_sbm_api_show("%s,%d,done.name:%s,status:%d,cmd:0x%x\n",
    		__FUNCTION__, __LINE__, sbm_devp->name, sbm_devp->status, cmd);	
	}

    down(&m_sbm_sem);

    if(sbm_devp->open_count <= 0)
    {
        up(&m_sbm_sem);

        ali_sbm_api_show("sbm dont' be opened\n");
        return -1;
    }   

    switch(cmd){
        case SBMIO_CREATE_SBM:
        {
            struct sbm_config sbm_info;
            
            if(sbm_devp->status != 0){
                ali_sbm_api_show("sbm %d has been created before\n", sbm_devp->sbm_number); 
                ret = -1;
                goto EXIT;
            }
            
            if(copy_from_user((void *)&sbm_info, (void *)arg, sizeof(sbm_info)))
                        return -EFAULT;
            sbm_devp->sbm_cfg.buffer_size = sbm_info.buffer_size;
            sbm_devp->sbm_cfg.block_size = sbm_info.block_size;
            sbm_devp->sbm_cfg.reserve_size = sbm_info.reserve_size;
            sbm_devp->sbm_cfg.wrap_mode = sbm_info.wrap_mode;
            if(sbm_info.buffer_addr == 0){
#if 1
                ali_sbm_api_show("smb buffer is NULL\n");
                ret = -1;
                goto EXIT;
#else
                sbm_devp->sbm_cfg.buffer_addr = (unsigned int)kmalloc(sbm_info.buffer_size, GFP_KERNEL);
                if(!sbm_devp->sbm_cfg.buffer_addr){
                SBM_PRF("Kmalloc sbm fail\n"); return -1;
                }
#endif              
                sbm_info.buffer_addr = sbm_devp->sbm_cfg.buffer_addr;
            }else{
                sbm_info.buffer_addr = (unsigned int)__CACHE_ADDR_ALI_SEE(sbm_info.buffer_addr);
                sbm_devp->sbm_cfg.buffer_addr = sbm_info.buffer_addr;
            }
            ret = ali_rpc_sbm_create(sbm_devp->sbm_number, sbm_info);
            if(ret < 0){
                ali_sbm_api_show("create sbm fail %d\n", sbm_devp->sbm_number); 
                ret = -1;
                goto EXIT;
            }
            
            ali_sbm_api_show("%s : create sbm done %d\n", __FUNCTION__, sbm_devp->sbm_number);
            sbm_devp->status = 1;
            break;
        }

        case SBMIO_RESET_SBM:
        {
            ret = ali_rpc_sbm_reset(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode);
            break;
        }

        case SBMIO_SHOW_VALID_SIZE:
        {
            unsigned int valid_size = 0;
            ret = ali_rpc_sbm_show_valid_size(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode, &valid_size);
            if(copy_to_user((void *)arg, (void *)&valid_size, sizeof(int)))
                return -EFAULT;
            break;
        }

        case SBMIO_SHOW_FREE_SIZE:
        {
            unsigned int free_size = 0;
            ret = ali_rpc_sbm_show_free_size(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode, &free_size);
            if(copy_to_user((void *)arg, (void *)&free_size, sizeof(int)))
                return -EFAULT;
            break;
        }

        case SBMIO_SHOW_TOTAL_SIZE:
        {
            unsigned int total_size = sbm_devp->sbm_cfg.buffer_size;
            if(copy_to_user((void *)arg, (void *)&total_size, sizeof(int)))
                return -EFAULT;
            break;
        }
    case SBMIO_DESTROY_SBM:
    {
        ali_decv_rpc_release();
        ali_deca_rpc_release();

        if(sbm_devp->status != 0)
        {
            ali_rpc_sbm_destroy(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode);     
            sbm_devp->status = 0;
        }
        
        ali_sbm_api_show("%s : release sbm %d \n", __FUNCTION__, sbm_devp->sbm_number);
        break;
    }
        case SBMIO_REQ_BUF_INFO:
    {   
        struct sbm_req_buf *buf_info = (struct sbm_req_buf *)arg;
        ret = sbm_req_buf_info(sbm_devp,buf_info);
        break;
    }
    case SBMIO_UPDATE_BUF_INFO:
    {
        struct sbm_req_buf *buf_info = (struct sbm_req_buf *)arg;
        ret = sbm_update_buf_info(sbm_devp,buf_info);
        break;
    }

        case SBMIO_SHOW_PKT_NUM:
        {
            unsigned int pkt_num = 0;
            ret = ali_rpc_sbm_show_pkt_num(sbm_devp->sbm_number, sbm_devp->sbm_cfg.wrap_mode, &pkt_num);
            if(copy_to_user((void *)arg, (void *)&pkt_num, sizeof(int)))
                return -EFAULT;
            break;
        }

        case SBMIO_REQUEST_WRITE:
        {
            struct sbm_buf write_buffer;
            memset(&write_buffer, 0, sizeof(struct sbm_buf));
            if(copy_from_user((void *)&write_buffer, (void *)arg, sizeof(struct sbm_buf)))
                return -EFAULT;
            ret = ali_rpc_sbm_request_write(sbm_devp->sbm_number, (void**)(&(write_buffer.buf_addr)), (int*)(&(write_buffer.buf_size)));
            write_buffer.buf_addr = (char*)((unsigned int)(write_buffer.buf_addr) & 0xFFFFFFF);
            if(copy_to_user((void *)arg, (void *)&write_buffer, sizeof(struct sbm_buf)))
                return -EFAULT;
            if(ret == SBM_REQ_BUSY)  ret = -1;
            break;
        }

        case SBMIO_IS_FULL:
        {
            int is_full = sbm_devp->is_full;
            if(copy_to_user((void *)arg, (void *)&is_full, sizeof(int)))
                return -EFAULT;
            break;
        } 
        default:
            break;
    }

EXIT:
    up(&m_sbm_sem);
    if ((cmd != SBMIO_SHOW_VALID_SIZE) && (cmd != SBMIO_SHOW_FREE_SIZE) &&
		(cmd != SBMIO_SHOW_TOTAL_SIZE))
    {
	    ali_sbm_api_show("%s,%d,done.ret:%d\n", __FUNCTION__, __LINE__, ret);
    }
	
    return ret;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations sbm_fops = {
    .owner    =   THIS_MODULE,     /* Owner */
    .open     =   ali_sbm_open,        /* Open method */
    .release  =   sbm_release,     /* Release method */
    .read     =   sbm_read,        /* Read method */
    .write    =   sbm_write,       /* Write method */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = sbm_ioctl,
#else
    .ioctl    =   sbm_ioctl,       /* Ioctl method */
#endif    
};

#define DEVICE_NAME                "ali_sbm"

static dev_t sbm_dev_number;       /* Allotted device number */
static struct class *sbm_class;    /* Tie with the device model */
static struct device *sbm_device;
//static struct sbm_dev *sbm_priv[SBM_NUM];
struct sbm_dev *sbm_priv[SBM_NUM];




static int __init sbm_init(void)
{
    int i, ret;

    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&sbm_dev_number, 0,
                            SBM_NUM, DEVICE_NAME) < 0) {
        SBM_PRF(KERN_DEBUG "Can't register device\n"); return -1;
    }

    /* Populate sysfs entries */
    sbm_class = class_create(THIS_MODULE, DEVICE_NAME);
  
    for (i=0; i<SBM_NUM; i++) {
        /* Allocate memory for the per-device structure */
        sbm_priv[i] = kmalloc(sizeof(struct sbm_dev), GFP_KERNEL);
        if (!sbm_priv[i]) {
            SBM_PRF("Bad Kmalloc\n"); return -ENOMEM;
        }
        memset(sbm_priv[i], 0, sizeof(struct sbm_dev));
    
        sprintf(sbm_priv[i]->name, "ali_sbm%d", i);
    
        /* Fill in the sbm number to correlate this device
           with the corresponding sbm */
        sbm_priv[i]->sbm_number = i;

        sbm_priv[i]->status = 0;
    
        /* Connect the file operations with the cdev */
        cdev_init(&sbm_priv[i]->cdev, &sbm_fops);
        sbm_priv[i]->cdev.owner = THIS_MODULE;
    
        /* Connect the major/minor number to the cdev */
        ret = cdev_add(&sbm_priv[i]->cdev, (sbm_dev_number + i), 1);
        if (ret) {
            SBM_PRF("Bad cdev\n");
            return ret;
        }
        
        sbm_device = device_create(sbm_class, NULL, MKDEV(MAJOR(sbm_dev_number), i), 
                                   NULL, "ali_sbm%d", i);
        if(sbm_device == NULL){
            SBM_PRF("sbm create device fail\n");
            return 1;
        }

        sbm_info[i] = NULL;
        sbm_info_pkt[i] = NULL;
    }

#if defined(CONFIG_ALI_CHIP_M3921)
    {
        int info_size = sizeof(struct sbm_desc_pkt) + sizeof(struct sbm_config);
        int i = 0;
            
        memset((void *)m_info_addr, 0, sizeof(m_info_addr));
        m_info_addr[0] = ali_rpc_malloc_shared_mm(info_size * SBM_NUM);
        if(m_info_addr[0] == NULL)
        {
            printk("%s : get the rpc share memory fail\n", __FUNCTION__);
        }
        
        for(i = 1;i < SBM_NUM;i++)
        {
            m_info_addr[i] = m_info_addr[i - 1] + info_size;
        }
    }
#endif

    //init_MUTEX(&m_sbm_sem);
    sema_init(&m_sbm_sem, 1);

    ali_sbm_dbg_init();
    
    return 0;
}

/* Driver Exit */
void __exit sbm_exit(void)
{
    int i;
  
    /* Release the major number */
    unregister_chrdev_region(sbm_dev_number, SBM_NUM);

    for (i=0; i<SBM_NUM; i++) {
        device_destroy(sbm_class, MKDEV(MAJOR(sbm_dev_number), i));
        /* Remove the cdev */
        cdev_del(&sbm_priv[i]->cdev);
        kfree(sbm_priv[i]);
    }
    
    /* Destroy cmos_class */
    class_destroy(sbm_class);
    
    return;
}

module_init(sbm_init);
module_exit(sbm_exit);
 
MODULE_AUTHOR("ALi (Zhuhai) Corporation");
MODULE_DESCRIPTION("ali share buffer memory driver");
MODULE_LICENSE("GPL");
 
