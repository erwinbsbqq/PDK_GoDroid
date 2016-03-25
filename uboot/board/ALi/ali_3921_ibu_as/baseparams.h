#ifndef BASEPARAMS_H
#define BASEPARAMS_H

#include <alidefinition/adf_boot.h>

struct SysInfo
{
    char sw_ver[128];	
    unsigned char sw_md5[128];    
    unsigned char sw_private[1024];    
};

struct base_params {
	char magic[16];
	ADF_BOOT_AVINFO avinfo;
	struct SysInfo sysinfo;	
	unsigned char rsv[1024*6];
};

#endif

