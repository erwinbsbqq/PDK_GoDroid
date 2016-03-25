
#include <fcntl.h>
#include <sys/ioctl.h>
#include <hld_cfg.h>

#include <ali_pm_common.h>


#define ALI_PM_PRINTF(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(STANDBY, fmt, ##args); \
			} while(0)

struct pm_param ali_pm_param ={0,0,0} ;
struct pm_key ali_pm_key = {0,0x10effb04,0x60df708f,0x10efeb14,0x00ff00ff,0x00ff807f,0x00ff40bf,0x10ef9b64,0x10efbb44};

void set_ali_fast_standby_ir_power_key(unsigned long * ir_power_key)
{
	int pm_handle;
	
	pm_handle = open("/dev/ali_pm", O_RDWR|O_NONBLOCK|O_CLOEXEC);
	if (pm_handle < 0)
	{
		ALI_PM_PRINTF("open /dev/ali_pm fail\n");
	}
		
	ALI_PM_PRINTF("set_ali_fast_standby_ir_power_key:0x%lx ,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\n", \
			ir_power_key[0],ir_power_key[1],ir_power_key[2],ir_power_key[3],ir_power_key[4],ir_power_key[5],ir_power_key[6],ir_power_key[7]);
	
	ali_pm_key.ir_power[0] = ir_power_key[0];
	ali_pm_key.ir_power[1] = ir_power_key[1];
	ali_pm_key.ir_power[2] = ir_power_key[2];
	ali_pm_key.ir_power[3] = ir_power_key[3];
	ali_pm_key.ir_power[4] = ir_power_key[4];
	ali_pm_key.ir_power[5] = ir_power_key[5];
	ali_pm_key.ir_power[6] = ir_power_key[6];
	ali_pm_key.ir_power[7] = ir_power_key[7];
	
	ioctl(pm_handle, PM_CMD_SET_RESUME_KEY, &ali_pm_key);
	close(pm_handle);
	return;
}

void set_ali_fast_standby_timeout(unsigned long timout)
{
	int pm_handle;
	
	pm_handle = open("/dev/ali_pm", O_RDWR|O_NONBLOCK|O_CLOEXEC);
	if (pm_handle < 0)
	{
		ALI_PM_PRINTF("open /dev/ali_pm fail\n");
	}
	
	ALI_PM_PRINTF("set_ali_fast_standby_timeout: 0x%ld\n", timout);
	ali_pm_param.timeout = timout;	// sec,  0: never timeout
	
	ioctl(pm_handle, PM_CMD_SET_STANDBY_PARAM, &ali_pm_param);
	close(pm_handle);
	
	return;
}
 
void set_ali_fast_standby_reboot(unsigned long reboot)
{
	int pm_handle;
	
	pm_handle = open("/dev/ali_pm", O_RDWR|O_NONBLOCK|O_CLOEXEC);
	if (pm_handle < 0)
	{
		ALI_PM_PRINTF("open /dev/ali_pm fail\n");
	}
	
	ALI_PM_PRINTF("set_ali_fast_standby_reboot: 0x%lx\n", reboot);
	ali_pm_param.reboot = reboot;	//set 1 to reboot after resume
	ioctl(pm_handle, PM_CMD_SET_STANDBY_PARAM, &ali_pm_param);
	close(pm_handle);
	return;
}

void enter_ali_fast_standby(void)
{
	system("echo mem > /sys/power/state");
	return;
}

