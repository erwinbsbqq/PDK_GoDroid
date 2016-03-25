#if   (!defined(CONFIG_ALI_CHIP_M3921))   

#include <linux/uaccess.h> //for IBU ioctl
#include <linux/random.h>//for IBU
/*-----------------------------------------------------------------------------------------*/
uint8_t ali_nand_write_sys_info(struct mtd_info *mtd, uint8_t *sysbuf, loff_t to, loff_t len);
uint8_t ali_nand_get_sys_info(struct mtd_info *mtd, uint8_t *sysbuf, loff_t from, loff_t len);
uint8_t ali_nand_get_allkeys_info(struct mtd_info *mtd, uint8_t *keybuf);
uint8_t ali_nand_write_allkeys_info(struct mtd_info *mtd, uint8_t *keybuf);
uint8_t ali_nand_get_state_mach_var_info(struct mtd_info *mtd, uint8_t *varbuf, loff_t len);
uint8_t ali_nand_write_state_mach_var_info(struct mtd_info *mtd, uint8_t *varbuf, loff_t len);
//uint8_t ali_nand_get_pmi_to_buf(struct mtd_info *mtd, uint8_t *pmibuf);
size_t ali_nand_get_img_size(struct mtd_info *mtd, u_int img_no, size_t *retStart, size_t *retLenth);
int ali_nand_get_img_bin(struct mtd_info *mtd, size_t img_Start, size_t img_Lenth, u_char *img_buf );
int ali_nand_ioctl(struct mtd_info *mtd, u_int cmd, u_long arg);
int ali_nand_compare_partitons(struct mtd_info *mtd, u_int compare_part_no_1, u_int compare_part_no_2);
int ali_nand_replace_partitons(struct mtd_info *mtd, u_int Replace_part, u_int BeReplaced_part);
int ali_nand_compare_partitons_with_shafile(struct mtd_info *mtd, u_int compare_part_no_1, u_int compare_part_no_2, size_t len, char *path);
int ali_nand_offsettoPart(struct mtd_info *mtd, loff_t offset);
unsigned int ali_nand_read_nandstas(struct mtd_info *mtd);
unsigned int ali_nand_check_nandstas(struct mtd_info *mtd, uint8_t part);
int ali_nand_wr_nandstas_to_nand(struct mtd_info *mtd, uint8_t part);
int ali_nand_reset_nandstas(struct mtd_info *mtd, u_int reset_part_no_1, u_int reset_part_no_2);

#endif


//for IBU                                                                                  
#ifdef CONFIG_ALI_AS                                                                       
	static u8 ali_nand_pmi_hash_gen(struct mtd_info *mtd, uint8_t *blk_buf,uint8_t* out_buf);
	static u32 ali_nand_pmi_verify(struct mtd_info *mtd, uint8_t *PMIbuf, u32 ofset);        
#endif    

int ali_nand_ioctl(struct mtd_info *mtd, u_int cmd, u_long arg)       
{                                                                     
#if (!defined(CONFIG_ALI_CHIP_M3921))                                 
                                                                                 
 	int ret_code = 0;
    switch (cmd)
 	{
        case MEMGETPMI:
        case ALI_UPDATE_MTDINFO:
        case MEMWRITEPMI:
        case MEMGETSYSINFO:      
        case MEMWRITESYSINFO:
        case MEMGETALLKEYS:
        case MEMWRITEALLKEYS:
        case MEMGETCHECKPART:
        case MEMREPLACEPART:
        case MEMCOMPAREWITHSHAFILE:
        case MEMRESETNANDSTATS:
	 	default:
 		ret_code = -ENOIOCTLCMD;
 		break;
 	}
 	return ret_code; 	
#else
	return 0;
#endif
 }
EXPORT_SYMBOL(ali_nand_ioctl);


///////////////////////////////////////////////////////////////////////////////
uint8_t ali_nand_write_sys_info(struct mtd_info *mtd, uint8_t *sysbuf, loff_t to, loff_t len)
{
    return 0;
}

//uint8_t ali_nand_get_pmi_to_buf(struct mtd_info *mtd, uint8_t *pmibuf)
//{
// 	return 0;
//}

uint8_t ali_nand_get_state_mach_var_info(struct mtd_info *mtd, uint8_t *varbuf, loff_t len)
{
	return 0;
}

uint8_t ali_nand_get_allkeys_info(struct mtd_info *mtd, uint8_t *keybuf)
{
	return 0;
}


uint8_t ali_nand_get_sys_info(struct mtd_info *mtd, uint8_t *sysbuf, loff_t from, loff_t len)
{
	return 0;
}

uint8_t ali_nand_write_state_mach_var_info(struct mtd_info *mtd, uint8_t *varbuf, loff_t len)
{
    return 0;
}

uint8_t ali_nand_write_allkeys_info(struct mtd_info *mtd, uint8_t *keybuf)
{
    return 0;
}


/* decrypt the pmi hash, calucate pmi hash and then verify them 
* ofset  pmi offset by bytes 
*
*/

#ifdef CONFIG_ALI_AS	
static u8 en_sw_uk[0x40];
#define MAX_UK_LEN 0x1000
#define SYS_UK_MAGIC        0x4F53554B      /* sys_uk magic number */
static u32 pmi_verify_pass = 0;
static u32 ali_nand_pmi_verify(struct mtd_info *mtd, uint8_t *PMIbuf, u32 from)
{
   return 0;
}

/*
* generate pmi from blk_buf and calcluate hash ,then encrypt hash result to out_buf
@blk_buf : input pmi block or page buffer
@out_buf : encrypt out_put buffer, it must lager than 32+4 bytes
*
*/
static u8 ali_nand_pmi_hash_gen(struct mtd_info *mtd, uint8_t *blk_buf,uint8_t* out_buf)
{
    return 0;
}

/*
 * To get the system universal key from U-Boot param
 * \@ param format:
 * \@             0x********,0x****
 * \@             addr(8 hex always),size
 */
static int __init ali_nand_sys_uk_setup(char *str)
{
    return 1;
}

__setup("sys_uk=", ali_nand_sys_uk_setup);

#endif

#if   (!defined(CONFIG_ALI_CHIP_M3921))
/**********************************************************/
//for IBU


//#define SHA_DBG_PRINTK ALI_NAND_DEBUG
#define SHA_DBG_PRINTK(...)  do{}while(0)
#define ALINAND_INVALID_SHA_DEV     0xff

//if the partition has been writen already, no need to do write ops
//return val: 0 No need to write
//       val: 1 must write
unsigned int ali_nand_check_nandstas(struct mtd_info *mtd, uint8_t part)
{
    return 0;  
}

unsigned int ali_nand_read_nandstas(struct mtd_info *mtd)
{
    return 0;
}

int ali_nand_reset_nandstas(struct mtd_info *mtd, u_int reset_part_no_1, u_int reset_part_no_2)
{
    return 0;
}

int ali_nand_wr_nandstas_to_nand(struct mtd_info *mtd, uint8_t part)
{
    return 0;
}
#endif

int ali_nand_compare_partitons_with_shafile(struct mtd_info *mtd, u_int compare_part_no_1, u_int compare_part_no_2, size_t len, char *path)
{
	return 0;
}
int ali_nand_compare_partitons(struct mtd_info *mtd, u_int compare_part_no_1, u_int compare_part_no_2)
{
    return 0;
}

int ali_nand_replace_partitons(struct mtd_info *mtd, u_int Replace_part, u_int BeReplaced_part)
{
    return 0;
}
EXPORT_SYMBOL(ali_nand_replace_partitons);

size_t ali_nand_get_img_size(struct mtd_info *mtd, u_int img_no, size_t *retStart, size_t *retLenth)
{
    return 0;
}
EXPORT_SYMBOL(ali_nand_get_img_size);
    
int ali_nand_get_img_bin(struct mtd_info *mtd, size_t img_Start, size_t img_Lenth, u_char *img_buf )
{
    return 0;
}
EXPORT_SYMBOL(ali_nand_get_img_bin);


