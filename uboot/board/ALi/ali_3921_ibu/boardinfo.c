#include <common.h>
#include <alidefinition/adf_boot.h>


void dump_print(char *str, unsigned char *p, unsigned int size)
{
	int i, j;
	printf("%s\n",str);

	for (i=0;i<size/32;i++)
	{
		for (j=0;j<16;j++)
			printf("0x%02x ",p[i*16+j]);
		printf("\n");
	}

	for (i=i*16; i<size; i++)
		printf("0x%02x ",p[i]);

	printf("\n");
}

int set_boardinfo(int isRecovery)
{
	unsigned int size;
	unsigned char *p;
	
	ADF_BOOT_BOARD_INFO *boardinfo = (ADF_BOOT_BOARD_INFO *)(PRE_DEFINED_ADF_BOOT_START);

	size = get_hdmiinfo(&p);
	memcpy((u_char*)&boardinfo->hdcp, p, size);

	size = get_avinfo(&p);
	memcpy((u_char*)&boardinfo->avinfo, p, size);

	size = get_mac(&p);
	memcpy((u_char*)&boardinfo->macinfo, p, size);

	size = get_memmap(&p,isRecovery);
	memcpy((u_char*)&boardinfo->memmap_info, p, size);

	//dump_print("memmap:",p, 32);

	size = get_tveinfo(&p);
	memcpy((u_char*)&boardinfo->tve_info, p, size);

	size = get_registerinfo(&p);
	memcpy((u_char*)&boardinfo->reg_info, p, size);
	
	return 0;
}

