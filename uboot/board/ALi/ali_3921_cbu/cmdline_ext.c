#include <common.h>
#include "deviceinfo.h"

static char cmdline_ext[512];

void init_cmdline_ext()
{
    memset(cmdline_ext, 0, sizeof(cmdline_ext));
}

static int recovery_bootkey=0;
void set_recovery_bootkey(int key)
{
    recovery_bootkey = key;
}

void set_recoveryboot_cmdline(const char *part_name)
{
	char str[64];

    sprintf(str, "recovery.boot=%s ", part_name);    
    strcat(cmdline_ext, str);

    if (recovery_bootkey == 1) {
        // gpio boot key
        strcat(cmdline_ext, "recovery.triger=reset ");
    } else if (recovery_bootkey > 1) {
        // ir/panel boot key
        strcat(cmdline_ext, "recovery.triger=key ");
    }

#ifdef BOOTLOADER_VERSION
    sprintf(str, "recovery.bootloader=%s ", BOOTLOADER_VERSION);    
    strcat(cmdline_ext, str);
#endif

}

void set_androidboot_cmdline()
{
	BOOTCONFIG *bootconfig;
	unsigned char buf[64];
	char str[64];

	if (get_bootconfig((unsigned char **)&bootconfig) == 0)
		return;
    
	if (bootconfig->bootserailno_enable && get_sn(buf) > 0)
	{
		sprintf(str, "androidboot.serialno=%s ", buf);    
		strcat(cmdline_ext, str);
	}

#ifdef BOOTLOADER_VERSION
	if (bootconfig->bootblverison_enable)
	{
		sprintf(str, "androidboot.bootloader=%s ", BOOTLOADER_VERSION);    
		strcat(cmdline_ext, str);
	}
#endif

	if (bootconfig->bootmode_enable && (bootconfig->status&0x1))
	{
		sprintf(str, "androidboot.mode=factory ");    
		strcat(cmdline_ext, str);
	}

	if (bootconfig->boothwversion_enable && get_hwver(buf)>0)
	{
		sprintf(str, "androidboot.hardware=%s ", buf);    
		strcat(cmdline_ext, str);		
	}
}

char *get_cmdline_ext()
{
	return cmdline_ext;
}

