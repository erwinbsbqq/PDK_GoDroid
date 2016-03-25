#include <common.h>
#include <nand.h>
#include "bcb.h"

static struct bootloader_message *bcb=NULL;

struct bootloader_message * get_bcb()
{
	int ret;
	const char *partName = "misc";
	loff_t offset;
	size_t len;
	u_char *bcb_tmp;
	
    if (bcb)
        return bcb;     

	bcb_tmp = malloc(sizeof(struct bootloader_message) + 0x20);
	bcb = (struct device_info *)(((u32)bcb_tmp + (u32)0x1F) & (~(u32)0x1F));

#if defined(CONFIG_NAND_ALI) 
	nand_info_t *nand = &nand_info[nand_curr_device];	

	offset = 1*nand->writesize;
#elif defined(CONFIG_ALI_MMC)
    offset = 0;
#endif

	len = sizeof(struct bootloader_message);

	ret = load_part_ext(partName, bcb, offset, len);
	if (ret < 0)
	{
		free(bcb_tmp);
        bcb = NULL;
		return NULL;
	}

	return bcb;	
}

int get_bootloader_message(struct bootloader_message *out)
{
	struct bootloader_message *bcb = get_bcb();

	if (bcb == NULL)
		return -1;

	memcpy(out, bcb, sizeof(struct bootloader_message));
	
	return 0;
}

