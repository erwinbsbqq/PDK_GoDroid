
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/syscalls.h>

#include <linux/debugfs.h>
#include "ali_alsa_dbg.h"

static struct dentry *ali_alsa_dbg_debugfs_root;

static uint32_t playback_dump_en = 0;
static uint32_t capture_dump_en = 0;
static uint32_t i2so_see2main_dump_en = 0;
static uint32_t i2sirx_see2main_dump_en = 0;
static uint32_t i2sirx_i2so_mix_dump_en = 0;
static uint32_t i2sirx_i2so_mix_out_dump_en = 0;
static uint32_t i2sirx_i2so_mix_drop_ms = 700;
static uint32_t i2sirx_i2so_mix_align_ms = 200;

/* For now, dump file format:PCM,signed 16 bit, little endian, 48000hz,mono.
*/
int32_t ali_alsa_dump_data
(
    char *file_path,
    unsigned char *data, 
    const int len
)
{
    //printk("%s,%d,len:%d,data:0x%x,file:%s\n", __FUNCTION__, __LINE__, len, data, file_path); 

    #if 1
    if (0 == len)
    {
        return(0);
    }
    #endif

    int fd = -1;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    //printk("%s,%d\n", __FUNCTION__, __LINE__);    
    
    //fd = sys_open("/data/data/ali_alsa_mix_dump.pcm", O_RDWR | O_CREAT|O_APPEND, 0777);
    fd = sys_open(file_path, O_RDWR | O_CREAT|O_APPEND, 0777);
    if ( fd >= 0)
    {
        //printk("%s,%d\n", __FUNCTION__, __LINE__);        
        //printk("KernelWriteFileCapture,lenData:%d!\n", lenData);
        sys_write(fd, (unsigned char*)data, (__s32)len);
    }
    else 
    {
        printk("KernelWriteFileCapture error,fd :%d!\n", fd);
        return -1;    
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);    
    
    sys_close(fd);

    set_fs(old_fs);

    #if 0
    ali_alsa_mix_dump_cnt += len;

    if (0 == (ali_alsa_mix_dump_cnt % (800 * 1024)))
    {
        printk("%s,%d,ali_alsa_capture_dump_cnt:%d\n",
            __FUNCTION__, __LINE__, ali_alsa_mix_dump_cnt);
    }

    //printk("%s,%d\n", __FUNCTION__, __LINE__);    
    #endif
	
    return 0;
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2sirx_i2so_mix_out_dump_en_get
(
    void 
)
{
    return(i2sirx_i2so_mix_out_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2sirx_i2so_mix_dump_en_get
(
    void 
)
{
    return(i2sirx_i2so_mix_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2so_see2main_dump_en_get
(
    void 
)
{
    return(i2so_see2main_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2sirx_see2main_dump_en_get
(
    void 
)
{
    return(i2sirx_see2main_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_capture_dump_en_get
(
    void 
)
{
    return(capture_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_playback_dump_en_get
(
    void 
)
{
    return(playback_dump_en);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2sirx_i2so_mix_drop_ms_get
(
    void 
)
{
    return(i2sirx_i2so_mix_drop_ms);
}


/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_i2sirx_i2so_mix_align_ms_get
(
    void 
)
{
    return(i2sirx_i2so_mix_align_ms);
}



/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_dbg_exit
(
    void 
)
{
    debugfs_remove_recursive(ali_alsa_dbg_debugfs_root);
    ali_alsa_dbg_debugfs_root = NULL;
    capture_dump_en = 0;
	playback_dump_en = 0;
    i2so_see2main_dump_en = 0;
    i2sirx_see2main_dump_en = 0;	
    return(0);
}

/*****************************************************************************/
/**
* TODO: Description of this function
* - TODO: Description 1
* - TODO: Description 2
* - TODO: Description 3
* - TODO: Description 4
*
*
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
* @param :TODO: Description of this param.
*        
* @return
* - TODO: Description of this return value.
* - TODO: Description of this return value.
*
******************************************************************************/
int32_t ali_alsa_dbg_init
(
    void 
)
{
    struct dentry *fs_entry;

    printk("%s,%d,Go\n", __FUNCTION__, __LINE__);
    
    ali_alsa_dbg_debugfs_root = debugfs_create_dir("ali_alsa", NULL);
    
    if (!ali_alsa_dbg_debugfs_root)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        return(-ENOENT);
    }

    /* For ali ALSA playback data dump.
	*/
    fs_entry = debugfs_create_u32("playback_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &playback_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }	

    /* For ali ALSA capture data dump.
	*/
    fs_entry = debugfs_create_u32("capture_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &capture_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    /* For ali ALSA i2so see to main data dump.
	*/
    fs_entry = debugfs_create_u32("i2so_see2main_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &i2so_see2main_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }	

    /* For ali ALSA i2sirx see to main data dump.
	*/
    fs_entry = debugfs_create_u32("i2sirx_see2main_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &i2sirx_see2main_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    /* For ali ALSA i2sirx_i2so_mix data dump.
	*/
    fs_entry = debugfs_create_u32("i2sirx_i2so_mix_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &i2sirx_i2so_mix_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }	

    /* For ali ALSA i2sirx_i2so_mix_out data dump.
	*/
    fs_entry = debugfs_create_u32("i2sirx_i2so_mix_out_dump_en", 0644,
        ali_alsa_dbg_debugfs_root, &i2sirx_i2so_mix_out_dump_en);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    /* For ali ALSA i2sirx_i2so_mix_drop_ms(couted in millisecond).
	*/
    fs_entry = debugfs_create_u32("i2sirx_i2so_mix_drop_ms", 0644,
        ali_alsa_dbg_debugfs_root, &i2sirx_i2so_mix_drop_ms);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }	

    /* For ali ALSA i2sirx_i2so_mix_drop_ms(couted in millisecond).
	*/
    fs_entry = debugfs_create_u32("i2sirx_i2so_mix_align_ms", 0644,
        ali_alsa_dbg_debugfs_root, &i2sirx_i2so_mix_align_ms);
	
    if (!fs_entry)
    {
        printk("%s,%d\n", __FUNCTION__, __LINE__);
        goto Fail;  
    }

    printk("%s,%d,succeed.\n", __FUNCTION__, __LINE__);
    return(0);

Fail:
    printk("%s,%d,failed\n", __FUNCTION__, __LINE__);
    ali_alsa_dbg_exit();
    return(-ENOENT);    
}

