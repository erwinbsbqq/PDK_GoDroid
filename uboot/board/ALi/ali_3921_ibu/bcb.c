#include <common.h>
#include <nand.h>
#include "bcb.h"

static struct bootloader_message *bcb=NULL;

struct bootloader_message * get_bcb()
{

#if defined(CONFIG_NAND_ALI) 

	int ret;
	const char *partName = "misc";
	nand_info_t *nand = &nand_info[nand_curr_device];	
	u_int offset;
	u_int len;
	
	if (bcb)
		return bcb;		
	
	bcb = (struct device_info*)malloc(sizeof(struct bootloader_message));

	offset = 1*nand->writesize;
	len = sizeof(struct bootloader_message);
	ret = load_part_ext(partName, (u_char *)bcb, offset, len);
	if (ret < 0)
	{
		free(bcb);
		return NULL;
	}
#endif

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

