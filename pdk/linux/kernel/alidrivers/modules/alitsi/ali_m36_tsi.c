
#include <linux/ioport.h>


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 

#include <linux/fs.h>
#include <linux/types.h>

#include <linux/cdev.h>
#include <linux/device.h>

#include <ali_basic_common.h>

#include <ali_tsi_common.h>

#include <ali_soc_common.h>

#include <linux/io.h>
#include <linux/uaccess.h>

#include <linux/version.h>
#include <ali_reg.h>

#define ALI_TSI_REG_BASE    0x1801A000//0xB801A000

#if 1
#define ALI_TSI_DEBUG(...)  do{}while(0)
#else
#define ALI_TSI_DEBUG printk

#endif

enum ALI_M36_TSI_STATUS
{
    ALI_M36_TSI_STATUS_IDLE,
    ALI_M36_TSI_STATUS_SET
};


struct ali_m36_tsi_dev
{
    char name[16];

    enum ALI_M36_TSI_STATUS status;

    void __iomem *base;/* Base address. */

    dev_t dev_id;

    /* Linux char device for dmx. */
    struct cdev cdev;

    struct mutex io_mutex; 

    int spi_0_1_swaped;
};

int ali_m36_tsi_open(struct inode *inode, struct file *file);

int ali_m36_tsi_release(struct inode *inode, struct file *file);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long ali_m36_tsi_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
#else
int ali_m36_tsi_ioctl(struct inode * inode, struct file * filp, unsigned int cmd, unsigned long arg);
#endif

struct file_operations g_ali_tsi_fops = {
    .owner =    THIS_MODULE,
    //.llseek =   scull_llseek,
    //.read =     dmx_channel_read,
    //.write =    dmx_write,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    .unlocked_ioctl = ali_m36_tsi_ioctl,
#else
    .ioctl =    ali_m36_tsi_ioctl,
#endif  
    .open =     ali_m36_tsi_open,
    .release =  ali_m36_tsi_release,
    //.poll = dmx_channel_poll,
};





struct ali_m36_tsi_dev  g_ali_tsi_dev[1];
struct class           *g_ali_tsi_class;




int ali_m36_tsi_input_set
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    int                            ret;
    struct ali_tsi_input_set_param input_param;
    __u32                          chip_id;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&input_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (1 == tsi->spi_0_1_swaped)
    {
        if (ALI_TSI_INPUT_SPI_0 == input_param.id)
        {
            input_param.id = ALI_TSI_INPUT_SPI_1;
        }

        if (ALI_TSI_INPUT_SPI_1 == input_param.id)
        {
            input_param.id = ALI_TSI_INPUT_SPI_0;
        }
    }

    ret = 0;

    chip_id = ali_sys_ic_get_chip_id();

    if(ALI_S3602F == chip_id )
    {
        switch (input_param.id)
        {
            case ALI_TSI_INPUT_SPI_0:
            {
                iowrite8(input_param.attribute, tsi->base + 0x0);
            }
            break;
        
            case ALI_TSI_INPUT_SPI_1:
            {
                iowrite8(input_param.attribute, tsi->base + 0x1);
            }
            break;
        
            case ALI_TSI_INPUT_TSG:
            {
                iowrite8(input_param.attribute, tsi->base + 0x2);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_0:
            {
                iowrite8(input_param.attribute, tsi->base + 0x4);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_1:
            {
                iowrite8(input_param.attribute, tsi->base + 0x5);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_2:
            {
                iowrite8(input_param.attribute, tsi->base + 0x6);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_3:
            {
                iowrite8(input_param.attribute, tsi->base + 0x7);
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_0:
            {
                iowrite8(input_param.attribute, tsi->base + 0x10);
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_1:
            {
                iowrite8(input_param.attribute, tsi->base + 0x11);
        
                ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
            }
            break;
        
            case ALI_TSI_INPUT_SPI4B_0:
            {
                iowrite8(input_param.attribute, tsi->base + 0x12);
            }
            break;
        
            case ALI_TSI_INPUT_SPI4B_1:
            {
                iowrite8(input_param.attribute, tsi->base + 0x13);
            }
            break;
        
            default:
            {
                ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        
                ret = -EINVAL;
            }
            break;
        }
    }
    else //if( ALI_C3921 == chip_id )        //for M3701C & M3921
    {
        switch (input_param.id)
        {
            case ALI_TSI_INPUT_SPI_0:
            {                
                iowrite8(input_param.attribute, tsi->base + 0x0);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x0));
            }
            break;
        
            case ALI_TSI_INPUT_SPI_1:
            {
                iowrite8(input_param.attribute, tsi->base + 0x1);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x1));
            }
            break;
        
            case ALI_TSI_INPUT_TSG:
            {               
                iowrite8(input_param.attribute, tsi->base + 0x2);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x2));
            }
            break;

            case ALI_TSI_INPUT_SPI_3:
            {
                iowrite8(input_param.attribute, tsi->base + 0x3);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x03));
            }
            break;
        
            case ALI_TSI_INPUT_SSI_0:
            {
                iowrite8(input_param.attribute, tsi->base + 0x4);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_1:
            {              
                iowrite8(input_param.attribute, tsi->base + 0x5);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x5));
            }
            break;

            case ALI_TSI_INPUT_SSI_2:
            {
                iowrite8(input_param.attribute, tsi->base + 0x6);
                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x6));
            }
            break;
        
            case ALI_TSI_INPUT_SSI_3:
            {              
                iowrite8(input_param.attribute, tsi->base + 0x7);
                
                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x7));
            }
            break;

            case ALI_TSI_INPUT_SPI2B_0:
            {               
                iowrite8(input_param.attribute, tsi->base + 0x4);
                
                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x4));
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_1:
            { 
                iowrite8(input_param.attribute, tsi->base + 0x5);

                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x5));
            }
            break;

            case ALI_TSI_INPUT_SPI2B_2:
            { 
                iowrite8(input_param.attribute, tsi->base + 0x6);
                
                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x6));
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_3:
            { 
                iowrite8(input_param.attribute, tsi->base + 0x7);
                
                ALI_TSI_DEBUG("%s:%d: addr[%x], value[%x]\n", 
                            __FUNCTION__, __LINE__, input_param.id,ioread8(tsi->base + 0x7));
            }
            break;
                
            default:
            {
                ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        
                ret = -EINVAL;
            }
            break;
        }     
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    return(ret);
}




int ali_m36_tsi_input_get
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    int                             ret;
    struct ali_tsi_input_set_param  input_param;
    __u32                           chip_id;
    
    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&input_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = 0;

    chip_id = ali_sys_ic_get_chip_id();

    if(ALI_S3602F == chip_id )
    {
        switch (input_param.id)
        {
            case ALI_TSI_INPUT_SPI_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x0);
            }
            break;
    
            case ALI_TSI_INPUT_SPI_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x1);
            }
            break;
    
            case ALI_TSI_INPUT_TSG:
            {
                input_param.attribute = ioread8(tsi->base + 0x2);
            }
            break;
    
            case ALI_TSI_INPUT_SSI_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x4);
            }
            break;
    
            case ALI_TSI_INPUT_SSI_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x5);
            }
            break;
    
            case ALI_TSI_INPUT_SSI_2:
            {
                input_param.attribute = ioread8(tsi->base + 0x6);
            }
            break;
    
            case ALI_TSI_INPUT_SSI_3:
            {
                input_param.attribute = ioread8(tsi->base + 0x7);
            }
            break;
    
            case ALI_TSI_INPUT_SPI2B_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x10);
            }
            break;
    
            case ALI_TSI_INPUT_SPI2B_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x11);
            }
            break;
    
            case ALI_TSI_INPUT_SPI4B_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x12);
            }
            break;
    
            case ALI_TSI_INPUT_SPI4B_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x13);
            }
            break;
    
            default:
            {
                ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    
                ret = -EINVAL;
            }
            break;
        }

    }
    else
    {
        switch (input_param.id)
        {
            case ALI_TSI_INPUT_SPI_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x0);
            }
            break;
        
            case ALI_TSI_INPUT_SPI_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x1);
            }
            break;
        
            case ALI_TSI_INPUT_TSG:
            {
                input_param.attribute = ioread8(tsi->base + 0x2);
            }
            break;
        
            case ALI_TSI_INPUT_SSI_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x4);

                if(0x20 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;
        
            case ALI_TSI_INPUT_SSI_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x5);

                if(0x20 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;

            case ALI_TSI_INPUT_SSI_2:
            {
                input_param.attribute = ioread8(tsi->base + 0x6);

                if(0x20 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;
        
            case ALI_TSI_INPUT_SSI_3:
            {
                input_param.attribute = ioread8(tsi->base + 0x7);

                if(0x20 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_0:
            {
                input_param.attribute = ioread8(tsi->base + 0x4);

                if(0x40 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_1:
            {
                input_param.attribute = ioread8(tsi->base + 0x5);

                if(0x40 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                } 
            }
            break;

            case ALI_TSI_INPUT_SPI2B_2:
            {
                input_param.attribute = ioread8(tsi->base + 0x6);

                if(0x40 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                }
            }
            break;
        
            case ALI_TSI_INPUT_SPI2B_3:
            {
        
                input_param.attribute = ioread8(tsi->base + 0x7);

                if(0x40 == ((input_param.attribute)&0xE0))
                {
                    input_param.attribute = ((input_param.attribute)&0x1F )|0x80;
                }
                else
                {
                    input_param.attribute = ((input_param.attribute)&0x1F);
                } 
            }
            break;

               
            default:
            {
                ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
        
                ret = -EINVAL;
            }
            break;
        }
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (1 == tsi->spi_0_1_swaped)
    {
        ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

        if (ALI_TSI_INPUT_SPI_0 == input_param.id)
        {
            input_param.id = ALI_TSI_INPUT_SPI_1;
        }

        if (ALI_TSI_INPUT_SPI_1 == input_param.id)
        {
            input_param.id = ALI_TSI_INPUT_SPI_0;
        }
    }

    if (copy_to_user((void __user *)arg, &input_param, _IOC_SIZE(cmd)))
    {
        ALI_TSI_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);     
    }   

    return(ret);
}



int ali_m36_tsi_channel_set
(
    struct ali_m36_tsi_dev  *tsi,
    unsigned int             cmd,
    unsigned long            arg
)
{
    char                             orig;
    char                             ci_link_mode;
    int                              ret;
    struct ali_tsi_channel_set_param ch_param;
    __u32                           chip_id;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&ch_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    if (1 == tsi->spi_0_1_swaped)
    {
        if (ALI_TSI_INPUT_SPI_0 == ch_param.input_id)
        {
            ch_param.input_id = ALI_TSI_INPUT_SPI_1;
        }
        else if (ALI_TSI_INPUT_SPI_1 == ch_param.input_id)
        {
            ch_param.input_id = ALI_TSI_INPUT_SPI_0;
        }    
    }

    ret = 0;
    
    chip_id = ali_sys_ic_get_chip_id();

    if(ALI_S3602F == chip_id )
    {
        switch(ch_param.channel_id)
        {
            /* Set channel 0 data src. */
            case ALI_TSI_CHANNEL_0:
            {
                orig = ioread8(tsi->base + 0x8);
        
                orig &= 0xF0;
        
                orig |= ch_param.input_id;
        
                iowrite8(orig, tsi->base + 0x8);
            }
            break;
        
            /* Set channel 1 data src. */
            case ALI_TSI_CHANNEL_1:
            {
                ci_link_mode = ioread8(tsi->base + 0xE) & 0x1;
        
                orig = ioread8(tsi->base + 0x9);
        
                /* CI chain_mode */
                if (0 == ci_link_mode)
                {
                    orig &= 0xF0;
        
                    orig |= ch_param.input_id;
                }
                /* CI parallel_mode */
                else 
                {
                    orig &= 0x0F;
        
                    orig |= (ch_param.input_id << 4);
                    
                    orig &= 0xF0;
        
                    orig |= 0x06;
        
                }
        
                iowrite8(orig, tsi->base + 0x9);
            }
            break;
        
            default:
            {
                ret = -EINVAL;
            }
            break;
        }
    }
    else
    {
        switch(ch_param.channel_id)
        {
            /* Set channel 0 data src. */
            case ALI_TSI_CHANNEL_0:
            {
                orig = ioread8(tsi->base + 0x8);
        
                orig &= 0xF0;
        
                if(ALI_TSI_INPUT_SPI2B_0 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_0;  //0x04
                }
                else if(ALI_TSI_INPUT_SPI2B_1 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_1; //0x05
                }
                else if(ALI_TSI_INPUT_SPI2B_2 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_2;  //0x0C
                }
                else if(ALI_TSI_INPUT_SPI2B_3 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_3; //0x0D
                }
                else
                    orig |= ch_param.input_id;
        
                iowrite8(orig, tsi->base + 0x8);

                ALI_TSI_DEBUG("%s:%d:TSI_chan_0, addr[0x8],value[%x]\n", \
                                __FUNCTION__,__LINE__,ioread8(tsi->base + 0x8));
                
            }
            break;
        
            /* Set channel 1 data src. */
            case ALI_TSI_CHANNEL_1:
            {
                ci_link_mode = ioread8(tsi->base + 0xE) & 0x1;
        
                orig = ioread8(tsi->base + 0x9);
        
                /* CI chain_mode */
                if (0 == ci_link_mode)
                {
                    orig &= 0xF0;
        
                    if(ALI_TSI_INPUT_SPI2B_0 == ch_param.input_id)
                    {
                        orig |= ALI_TSI_INPUT_SSI_0;  //0x04
                    }
                    else if(ALI_TSI_INPUT_SPI2B_1 == ch_param.input_id)
                    {
                        orig |= ALI_TSI_INPUT_SSI_1;  //0x05
                    }
                    else if(ALI_TSI_INPUT_SPI2B_2 == ch_param.input_id)
                    {
                        orig |= ALI_TSI_INPUT_SSI_2;  //0x0C
                    }
                    else if(ALI_TSI_INPUT_SPI2B_3 == ch_param.input_id)
                    {
                        orig |= ALI_TSI_INPUT_SSI_3;  //0x0D
                    }                        
                    else
                        orig |= ch_param.input_id;
                }
                /* CI parallel_mode */
                else 
                {
                    orig &= 0x0F;

                    if(ALI_TSI_INPUT_SPI2B_0 == ch_param.input_id)
                    {
                        orig |= (ALI_TSI_INPUT_SSI_0 << 4);  //0x40; //(0x04 << 4);
                    }
                    else if(ALI_TSI_INPUT_SPI2B_1 == ch_param.input_id)
                    {
                        orig |= (ALI_TSI_INPUT_SSI_1 << 4);  //0x50; //(0x05 << 4);
                    }
                    else if(ALI_TSI_INPUT_SPI2B_2 == ch_param.input_id)
                    {
                        orig |= (ALI_TSI_INPUT_SSI_2 << 4);  //0xC0; //(0xC0 << 4);
                    }
                    else if(ALI_TSI_INPUT_SPI2B_3 == ch_param.input_id)
                    {
                        orig |= (ALI_TSI_INPUT_SSI_3 << 4);  //0xD0; //(0xD0 << 4);
                    }
                    else
                        orig |= (ch_param.input_id << 4);
                    
                    orig &= 0xF0;
        
                    orig |= 0x06;
        
                }
        
                iowrite8(orig, tsi->base + 0x9);
                
                ALI_TSI_DEBUG("%s:%d: TSI_chan_1,addr[0x9],value[%x]\n", 
                                __FUNCTION__,__LINE__,ioread8(tsi->base + 0x9));
            }
            break;
            /* Set TSI channel 2 data src. */
            case ALI_TSI_CHANNEL_2:
            {
                orig = ioread8(tsi->base + 0x14);
        
                orig &= 0xF0;
        
                if(ALI_TSI_INPUT_SPI2B_0 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_0;  //0x04
                }
                else if(ALI_TSI_INPUT_SPI2B_1 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_1; //0x05
                }
                else if(ALI_TSI_INPUT_SPI2B_2 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_2;  //0x0C
                }
                else if(ALI_TSI_INPUT_SPI2B_3 == ch_param.input_id)
                {
                    orig |= ALI_TSI_INPUT_SSI_3; //0x0D
                }
                else
                    orig |= ch_param.input_id;
        
                iowrite8(orig, tsi->base + 0x14);

                ALI_TSI_DEBUG("%s:%d: TSI_chan_2,addr[0x14],value[%x]\n", 
                                __FUNCTION__,__LINE__,ioread8(tsi->base + 0x14));
            }
            break;

            /* Set TSI channel 3 data src. */
            case ALI_TSI_CHANNEL_3:
            {
                orig = ioread8(tsi->base + 0x14);
        
                orig &= 0x0F;

                if(ALI_TSI_INPUT_SPI2B_0 == ch_param.input_id)
                {
                    orig |= (ALI_TSI_INPUT_SSI_0 << 4);  //0x40; //(0x04 << 4);
                }
                else if(ALI_TSI_INPUT_SPI2B_1 == ch_param.input_id)
                {
                    orig |= (ALI_TSI_INPUT_SSI_1 << 4);  //0x50; //(0x05 << 4);
                }
                else if(ALI_TSI_INPUT_SPI2B_2 == ch_param.input_id)
                {
                    orig |= (ALI_TSI_INPUT_SSI_2 << 4);  //0xC0; //(0xC0 << 4);
                }
                else if(ALI_TSI_INPUT_SPI2B_3 == ch_param.input_id)
                {
                    orig |= (ALI_TSI_INPUT_SSI_3 << 4);  //0xD0; //(0xD0 << 4);
                }
                else
                    orig |= (ch_param.input_id << 4);

                iowrite8(orig, tsi->base + 0x14);

                ALI_TSI_DEBUG("%s:%d:TSI_chan_3, addr[0x14],value[%x]\n", 
                                __FUNCTION__,__LINE__,ioread8(tsi->base + 0x14));
            }
            break;

            default:
            {
                ret = -EINVAL;
            }
            break;
        }
    }

    return(ret);
}







int ali_m36_tsi_channel_get
(
    struct ali_m36_tsi_dev  *tsi,
    unsigned int             cmd,
    unsigned long            arg
)
{
    char                             ci_link_mode;
    int                              ret;
    struct ali_tsi_channel_set_param ch_param;
    __u32                            chip_id;
    __u8                             tag;


    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&ch_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    ret = 0;

    chip_id = ali_sys_ic_get_chip_id();

    if(ALI_S3602F == chip_id )
    {
        switch(ch_param.channel_id)
        {
            /* Get channel 0 data src. */
            case ALI_TSI_CHANNEL_0:
            {
                ch_param.input_id = ioread8(tsi->base + 0x8) & 0x0F;                        
            }
            break;

            /* Get channel 1 data src. */
            case ALI_TSI_CHANNEL_1:
            {
                ci_link_mode = ioread8(tsi->base + 0xE) & 0x1;

                /* CI chain_mode */
                if (0 == ci_link_mode)
                {
                    ch_param.input_id = ioread8(tsi->base + 0x9) & 0x0F;
                }
                /* CI parallel_mode */
                else 
                {
                    ch_param.input_id = (ioread8(tsi->base + 0x9) & 0xF0) >> 4;
                }
            }
            break;

            default:
            {
                ret = -EINVAL;
            }
            break;
        }    
    }
    else
    {
        switch(ch_param.channel_id)
        {
            /* Get TSI channel 0 data src. */
            case ALI_TSI_CHANNEL_0:
            {
                ch_param.input_id = ioread8(tsi->base + 0x8) & 0x0F;

                if(ALI_TSI_INPUT_SSI_0 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x4) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_0;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_0;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_1 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x5) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_1;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_1;
                    else
                        ret = -EINVAL;
                }
                else if(ALI_TSI_INPUT_SSI_2 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x6) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_2;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_2;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_3 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x7) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_3;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_3;
                    else
                        ret = -EINVAL;
                }                       
            }
            break;

            /* Get TSI channel 1 data src. */
            case ALI_TSI_CHANNEL_1:
            {
                ci_link_mode = ioread8(tsi->base + 0xE) & 0x1;

                /* CI chain_mode */
                if (0 == ci_link_mode)
                {
                    ch_param.input_id = ioread8(tsi->base + 0x9) & 0x0F;

                    if(ALI_TSI_INPUT_SSI_0 == ch_param.input_id)
                    {
                        tag = ioread8(tsi->base + 0x4) & 0xE0;
                        
                        if(0x40 == tag )                            
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_0;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_0;
                        else
                            ret = -EINVAL;
                            
                    }
                    else if((ALI_TSI_INPUT_SSI_1 == ch_param.input_id))
                    {
                        tag = ioread8(tsi->base + 0x5) & 0xE0;
                        
                        if(0x40 == tag )
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_1;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_1;
                        else
                            ret = -EINVAL;
                    }
                    else if(ALI_TSI_INPUT_SSI_2 == ch_param.input_id)
                    {
                        tag = ioread8(tsi->base + 0x6) & 0xE0;
                        
                        if(0x40 == tag )                            
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_2;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_2;
                        else
                            ret = -EINVAL;
                            
                    }
                    else if((ALI_TSI_INPUT_SSI_3 == ch_param.input_id))
                    {
                        tag = ioread8(tsi->base + 0x7) & 0xE0;
                        
                        if(0x40 == tag )
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_3;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_3;
                        else
                            ret = -EINVAL;
                    }  
                       
                }
                /* CI parallel_mode */
                else 
                {
                    ch_param.input_id = (ioread8(tsi->base + 0x9) & 0xF0) >> 4;

                    if(ALI_TSI_INPUT_SSI_0 == ch_param.input_id)
                    {
                        tag = ioread8(tsi->base + 0x4) & 0xE0;
                        
                        if(0x40 == tag )                            
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_0;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_0;
                        else
                            ret = -EINVAL;
                            
                    }
                    else if((ALI_TSI_INPUT_SSI_1 == ch_param.input_id))
                    {
                        tag = ioread8(tsi->base + 0x5) & 0xE0;
                        
                        if(0x40 == tag )
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_1;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_1;
                        else
                            ret = -EINVAL;
                    } 
                    else if(ALI_TSI_INPUT_SSI_2 == ch_param.input_id)
                    {
                        tag = ioread8(tsi->base + 0x6) & 0xE0;
                        
                        if(0x40 == tag )                            
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_2;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_2;
                        else
                            ret = -EINVAL;
                            
                    }
                    else if((ALI_TSI_INPUT_SSI_3 == ch_param.input_id))
                    {
                        tag = ioread8(tsi->base + 0x7) & 0xE0;
                        
                        if(0x40 == tag )
                            ch_param.input_id = ALI_TSI_INPUT_SPI2B_3;
                        else if(0x20 == tag )                        
                            ch_param.input_id = ALI_TSI_INPUT_SSI_3;
                        else
                            ret = -EINVAL;
                    } 
                }
            }
            break;

            /* Get TSI channel 2 data src. */
            case ALI_TSI_CHANNEL_2:
            {
                ch_param.input_id = ioread8(tsi->base + 0x14) & 0x0F;

                if(ALI_TSI_INPUT_SSI_0 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x4) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_0;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_0;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_1 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x5) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_1;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_1;
                    else
                        ret = -EINVAL;
                }
                else if(ALI_TSI_INPUT_SSI_2 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x6) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_2;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_2;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_3 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x7) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_3;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_3;
                    else
                        ret = -EINVAL;
                }                       
            }
            break;

            /* Get TSI channel 3 data src. */
            case ALI_TSI_CHANNEL_3:
            {
                ch_param.input_id = (ioread8(tsi->base + 0x14) & 0xF0) >> 4;

                if(ALI_TSI_INPUT_SSI_0 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x4) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_0;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_0;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_1 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x5) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_1;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_1;
                    else
                        ret = -EINVAL;
                }
                else if(ALI_TSI_INPUT_SSI_2 == ch_param.input_id)
                {
                    tag = ioread8(tsi->base + 0x6) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_2;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_2;
                    else
                        ret = -EINVAL;
                        
                }
                else if((ALI_TSI_INPUT_SSI_3 == ch_param.input_id))
                {
                    tag = ioread8(tsi->base + 0x7) & 0xE0;
                    
                    if(0x40 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SPI2B_3;
                    else if(0x20 == tag )                        
                        ch_param.input_id = ALI_TSI_INPUT_SSI_3;
                    else
                        ret = -EINVAL;
                }                       
            }
            break;

            default:
            {
                ret = -EINVAL;
            }
            break;
        }    
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (1 == tsi->spi_0_1_swaped)
    {
        if (ALI_TSI_INPUT_SPI_0 == ch_param.input_id)
        {
            ch_param.input_id = ALI_TSI_INPUT_SPI_1;
        }

        if (ALI_TSI_INPUT_SPI_1 == ch_param.input_id)
        {
            ch_param.input_id = ALI_TSI_INPUT_SPI_0;
        }
    }

    if (copy_to_user((void __user *)arg, &ch_param, _IOC_SIZE(cmd)))
    {
        ALI_TSI_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);     
    }   

    return(ret);
}









int ali_m36_tsi_output_set
(
    struct ali_m36_tsi_dev  *tsi,
    unsigned int             cmd,
    unsigned long            arg
)
{
    char                             orig;
    int                              ret;
    struct ali_tsi_output_set_param  output_param;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&output_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    ret = 0;

    switch(output_param.output_id)
    {
        case ALI_TSI_OUTPUT_DMX_0:
        {
            orig = ioread8(tsi->base + 0xB);

            if (ALI_TSI_CHANNEL_0 == output_param.channel_id)
            {
                orig &= 0xFE;
            }

            else if (ALI_TSI_CHANNEL_1 == output_param.channel_id)
            {
                orig |= 0x01;
            }

            iowrite8(orig, tsi->base + 0xB);
        }
        break;

        case ALI_TSI_OUTPUT_DMX_1:
        {
            orig = ioread8(tsi->base + 0xB);

            if (ALI_TSI_CHANNEL_0 == output_param.channel_id)
            {
                orig &= 0xFD;
            }

            else if (ALI_TSI_CHANNEL_1 == output_param.channel_id)
            {
                orig |= 0x2;
            }

            iowrite8(orig, tsi->base + 0xB);
        }
        break;

        case ALI_TSI_OUTPUT_DMX_2:
        {
            orig = ioread8(tsi->base + 0xB);

            if (ALI_TSI_CHANNEL_0 == output_param.channel_id)
            {
                orig &= 0xF3;
            }
            else if (ALI_TSI_CHANNEL_1 == output_param.channel_id)
            {
                orig &= 0xF3;
                orig |= 0x04; 
            }
            else if (ALI_TSI_CHANNEL_2 == output_param.channel_id)
            {
                orig &= 0xF3;
                orig |= 0x08;  
            }

            iowrite8(orig, tsi->base + 0xB);
        }
        break;

        case ALI_TSI_OUTPUT_DMX_3:
        {
            orig = ioread8(tsi->base + 0xB);

            if (ALI_TSI_CHANNEL_0 == output_param.channel_id)
            {
                orig &= 0x3F;
            }
            else if (ALI_TSI_CHANNEL_1 == output_param.channel_id)
            {
                orig &= 0x3F;
                orig |= 0x40; 
            }
            else if (ALI_TSI_CHANNEL_3 == output_param.channel_id)
            {
                orig &= 0x3F;
                orig |= 0x80; 
            }

            iowrite8(orig, tsi->base + 0xB);
        }
        break;

        default:
        {
            ret = -EINVAL;
        }
        break;
    }

    return(ret);
}


int ali_m36_tsi_output_get
(
    struct ali_m36_tsi_dev  *tsi,
    unsigned int             cmd,
    unsigned long            arg
)
{
    int                              ret;
    struct ali_tsi_output_set_param  output_param;
    __u8                             value = 0;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&output_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    ret = 0;

    switch(output_param.output_id)
    {
        case ALI_TSI_OUTPUT_DMX_0:
        {
            output_param.channel_id = ioread8(tsi->base + 0xB) & 0x1;
        }
        break;

        case ALI_TSI_OUTPUT_DMX_1:
        {
            output_param.channel_id = (ioread8(tsi->base + 0xB) & 0x2) >> 1;
        }
        break;
    
        case ALI_TSI_OUTPUT_DMX_2:
        {    
            value = (ioread8(tsi->base + 0xB) & 0x0C)>>2;
    
            if(0 == value)
                output_param.channel_id = ALI_TSI_CHANNEL_0;
            else if(1 == value)
                output_param.channel_id = ALI_TSI_CHANNEL_1;
            else if( 2 == value )
                output_param.channel_id = ALI_TSI_CHANNEL_2;           
        }
        break;
    
        case ALI_TSI_OUTPUT_DMX_3:
        {    
            value = (ioread8(tsi->base + 0xB) & 0xC0)>>6;
    
            if(0 == value)
                output_param.channel_id = ALI_TSI_CHANNEL_0;
            else if(1 == value)
                output_param.channel_id = ALI_TSI_CHANNEL_1;
            else if( 2 == value )
                output_param.channel_id = ALI_TSI_CHANNEL_3;           
        }
        break;
    
        default:
        {
            ret = -EINVAL;
        }
        break;
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (copy_to_user((void __user *)arg, &output_param, _IOC_SIZE(cmd)))
    {
        ALI_TSI_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);     
    }   

    return(ret);
}












int ali_m36_tsi_ci_link_mode_set
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    char                      orig;
    int                       ret;
    enum ali_tsi_ci_link_mode link_mode;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&link_mode, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    if (ALI_TSI_CI_LINK_CHAIN == link_mode)
    {
        /* Enable CI chian mode.*/
        orig = ioread8(tsi->base + 0xE);

        orig &= 0xFE;

        iowrite8(orig, tsi->base + 0xE);

        /* Change CI B src to chain mode. */
        orig = ioread8(tsi->base + 0x9);

        orig = (orig & 0xF0) >> 4;

        iowrite8(orig, tsi->base + 0x9);
    }
    else if (ALI_TSI_CI_LINK_PARALLEL == link_mode)
    {
        /* Enable CI parallel mode.*/
        orig = ioread8(tsi->base + 0xE);

        orig |= 0x01;

        iowrite8(orig, tsi->base + 0xE);

        /* Change CI B src to parallel mode. */
        orig = ioread8(tsi->base + 0x9);

        orig = (orig & 0x0F) << 4;

        orig |= 0x06;

        iowrite8(orig, tsi->base + 0x9);
    }
    else
    {
        ret = -EINVAL;
    }

    return(ret);
}






int ali_m36_tsi_ci_link_mode_get
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    unsigned int              reg;
    enum ali_tsi_ci_link_mode link_mode;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    reg = ioread8(tsi->base + 0xE) & 0x1;

    if (0 == reg)
    {
        link_mode = ALI_TSI_CI_LINK_CHAIN;
    }
    else
    {
        link_mode = ALI_TSI_CI_LINK_PARALLEL;
    }

    if (copy_to_user((void __user *)arg, &link_mode, _IOC_SIZE(cmd)))
    {
        ALI_TSI_DEBUG("%s,%d\n", __FUNCTION__, __LINE__);

        return(-EFAULT);     
    }   

    return(0);
}









int ali_m36_tsi_ci_bypass_set
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    char                               orig;
    int                                ret;
    struct ali_tsi_ci_bypass_set_param bypass_param;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ret = copy_from_user(&bypass_param, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    if (ALI_TSI_CI_0 == bypass_param.ci_id)
    {
        orig = ioread8(tsi->base + 0x8);

        if (1 == bypass_param.is_bypass)
        {
            orig |= 0x80;
        }
        else
        {
            orig &= 0x7F;
        }

        iowrite8(orig, tsi->base + 0x8);
    }
    else if (ALI_TSI_CI_1 == bypass_param.ci_id)
    {
        orig = ioread8(tsi->base + 0xC);

        if (1 == bypass_param.is_bypass)
        {
            orig |= 0x80;
        }
        else
        {
            orig &= 0x7F;
        }

        iowrite8(orig, tsi->base + 0xC);
    }    
    else
    {
        ret = -EINVAL;
    }

    return(ret);
}


int ali_m36_tsi_spi_0_1_swap
(
    struct ali_m36_tsi_dev *tsi,
    unsigned int            cmd,
    unsigned long           arg
)
{
    int ret;
    int spi_0_1_swap_en;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    if (tsi->status != ALI_M36_TSI_STATUS_SET)
    {
        return(-EPERM);        
    }

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = copy_from_user(&spi_0_1_swap_en, (void __user *)arg, _IOC_SIZE(cmd));

    if (0 != ret)
    {
        ALI_TSI_DEBUG("copy_from_user() failed, ret:%d\n", ret);

        return(-EFAULT);
    }

    if(spi_0_1_swap_en)
    {
        tsi->spi_0_1_swaped = 1;
    }
    else
    {
        tsi->spi_0_1_swaped = 0;
    }    

    return(0);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
long ali_m36_tsi_ioctl
(
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#else
int ali_m36_tsi_ioctl
(
    struct inode *inode,
    struct file  *filp,
    unsigned int  cmd,
    unsigned long arg
)
#endif
{
    int                 ret;
    struct ali_m36_tsi_dev *tsi;

    ALI_TSI_DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    ret = 0;

    tsi = filp->private_data;

    switch(cmd)
    {
        case ALI_TSI_CHANNEL_SET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_channel_set(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_INPUT_SET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_input_set(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_OUTPUT_SET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_output_set(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_CI_LINK_MODE_SET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_ci_link_mode_set(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_CI_BYPASS_SET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_ci_bypass_set(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_INPUT_GET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_input_get(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_CHANNEL_GET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_channel_get(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_OUTPUT_GET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_output_get(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_CI_LINK_MODE_GET:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_ci_link_mode_get(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        case ALI_TSI_CI_SPI_0_1_SWAP:
        {
            if (mutex_lock_interruptible(&tsi->io_mutex))
            {
                return(-ERESTARTSYS);
            }

            ret = ali_m36_tsi_spi_0_1_swap(tsi, cmd, arg);

            mutex_unlock(&tsi->io_mutex);
        }
        break;

        default:
        {
            ret = -ENOTTY;
        }
        break;
    }

    return(ret);
}



int ali_m36_tsi_release
(
    struct inode *inode,
    struct file  *file
)
{
    int                 ret;
    struct ali_m36_tsi_dev *tsi;

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    tsi = file->private_data;

    //if (tsi->status == ALI_M36_TSI_STATUS_IDLE)
    //{
    //    return(-EPERM);        
    //}

    ret = 0;

    if (mutex_lock_interruptible(&tsi->io_mutex))
    {
        return(-ERESTARTSYS);
    }

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    tsi->status = ALI_M36_TSI_STATUS_IDLE;

    file->private_data = NULL;

    mutex_unlock(&tsi->io_mutex);

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(ret);
}





int ali_m36_tsi_open
(
    struct inode *inode,
    struct file  *file
)
{
    int                 ret;
    struct ali_m36_tsi_dev *tsi;

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    tsi = container_of(inode->i_cdev, struct ali_m36_tsi_dev, cdev);

    ret = 0;

    if (tsi->status != ALI_M36_TSI_STATUS_IDLE)
    {
        ret = (-EMFILE);
    }

    if (mutex_lock_interruptible(&tsi->io_mutex))
    {
        return(-ERESTARTSYS);
    }

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    file->private_data = tsi;

    tsi->status = ALI_M36_TSI_STATUS_SET;

    mutex_unlock(&tsi->io_mutex);

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(ret);
}




#if 0
#include <mach/ali-s3921.h>

#define ALI_REGS_PHYS_BASE              PHYS_SYSTEM 
#define ALI_REGS_VIRT_BASE              VIRT_SYSTEM 

#define ALI_PHY2VIRT(x) ((x) - (ALI_REGS_PHYS_BASE) + (ALI_REGS_VIRT_BASE))
#endif

#define ALI_PHY2VIRT(x) ((x) - (ALI_REGS_PHYS_BASE) + (ALI_REGS_VIRT_BASE))

static int __init ali_m36_tsi_init
(
    void
)
{
    int                     result;
    struct device          *clsdev;
    struct ali_m36_tsi_dev *tsi;
    __u32                   chip_id;

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);


    tsi = &(g_ali_tsi_dev[0]);

    memset(tsi, 0, sizeof(struct ali_m36_tsi_dev));

    mutex_init(&tsi->io_mutex);

#if 0
    if (!request_mem_region(__pa((void *)ALI_TSI_REG_BASE), 0x18, "ali_tsi")) 
    {
        printk("short: can't get I/O mem address 0x%x\n", ALI_TSI_REG_BASE);

        return(-ENODEV);
    }

    /* Also, ioremap it */
    tsi->base = ioremap(__pa((void *)ALI_TSI_REG_BASE), 0x18);
    /* Hmm... we should check the return value */
#else
    //tsi->base = (void __iomem *)ALI_TSI_REG_BASE;
    //tsi->base = (void __iomem *)(ALI_PHY2VIRT(ALI_TSI_REG_BASE)); 
    //tsi->base = (void __iomem *)(virt_to_phys(ALI_TSI_REG_BASE));
    //ALI_TSI_DEBUG("%s,%d,ALI_TSI_REG_BASE:%x\n", __FUNCTION__, __LINE__, ALI_TSI_REG_BASE);
    //tsi->base = ALI_PHY2VIRT(ALI_TSI_REG_BASE);
    //ALI_TSI_DEBUG("%s,%d,tsi->base:%x\n", __FUNCTION__, __LINE__, tsi->base);
    tsi->base = (void __iomem *)(ALI_PHY2VIRT(ALI_TSI_REG_BASE));
    ALI_TSI_DEBUG("%s,%d,tsi->base:%x\n", __FUNCTION__, __LINE__, (__u32)tsi->base);

    /* Disalbe all externel error pin input for C3921.
    */
    chip_id = ali_sys_ic_get_chip_id();

    if( ALI_C3921 == chip_id )
    {            
        iowrite8(0xF2, tsi->base + 0x18);                       

        ALI_TSI_DEBUG("%s:%d,addr[0x18], value[%x]\n", \
                    __FUNCTION__, __LINE__, ioread8(tsi->base + 0x18));                     
    }   
#endif
    
    result = alloc_chrdev_region(&tsi->dev_id, 0, 1, "ali_m36_tsi_0");

    if (result < 0) 
    {
        ALI_TSI_DEBUG("%s, alloc_chrdev_region() failed.\n", __FUNCTION__);

        goto fail;
    }

    ALI_TSI_DEBUG("%s, dev_id:%d.\n", __FUNCTION__, tsi->dev_id);

    cdev_init(&(tsi->cdev), &g_ali_tsi_fops);

    tsi->cdev.owner = THIS_MODULE;

    result = cdev_add(&tsi->cdev, tsi->dev_id, 1);

    /* Fail gracefully if need be. */
    if (result)
    {
        ALI_TSI_DEBUG("cdev_add() failed, result:%d\n", result);

        goto fail;
    }

    g_ali_tsi_class = class_create(THIS_MODULE, "ali_tsi_class");

    if (IS_ERR(g_ali_tsi_class))
    {
        result = PTR_ERR(g_ali_tsi_class);

        goto fail;
    }

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);


    clsdev = device_create(g_ali_tsi_class, NULL, tsi->dev_id, 
                           tsi, "ali_m36_tsi_0");

    if (IS_ERR(clsdev))
    {
        ALI_TSI_DEBUG(KERN_ERR "device_create() failed!\n");

        result = PTR_ERR(clsdev);

        goto fail;
    }

    ALI_TSI_DEBUG("%s, %d.\n", __FUNCTION__, __LINE__);

    return(0);

fail:
    return(-1);

}



static void __exit ali_m36_tsi_exit(void)
{
    printk("%s\n", __FUNCTION__);
}

module_init(ali_m36_tsi_init);
//device_initcall_sync(ali_m36_tsi_init);
module_exit(ali_m36_tsi_exit);
























