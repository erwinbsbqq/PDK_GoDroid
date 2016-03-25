#include <sys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#include <ali_watchdog.h>
#include <bus/dog/dog.h>
//corei#include <api/libc/printf.h>
//#include <api/libc/string.h>
#include <basic_types.h>  //corei

#define WATCHDOG_DEV "/dev/watchdog"
///////////////////////////////////////////////
// watch dog interface.
static int wdt_fd = -1;

// watchdog reboot
void api_watchdog_reboot()
{
    UINT32 wdt_time;

    if (wdt_fd <=0 )    
    {
        wdt_fd = open(WATCHDOG_DEV, O_RDWR);
        if (wdt_fd <= 0)
            return;
    }

    wdt_time = 10; //10us
    int reboot_sec = 0;
	
	/* the following line is just for one IC BUG, so it should be removed after this bug fixed
	 * BUG: watchdog reboot could not work alougth it send the reboot signal but the hardware doesn't reset 
	 *      the pin for NOR Flash(NOR & NAND share pin)
	 */
    ioctl(wdt_fd,  WDIOC_WDT_REBOOT, &reboot_sec);//reboot from NOR
    
    ioctl(wdt_fd , WDIOC_SETTIMEOUT, &wdt_time);
    sleep(10);
    close(wdt_fd);
    wdt_fd = -1;

}

//Set watch dog timeout.
void api_dog_mode_set(UINT32 id, UINT32 mode, UINT32 duration_us, void (*callback)(UINT32))
{
    UINT32 wdt_time;
    if (wdt_fd <=0 )    
    {
        wdt_fd = open(WATCHDOG_DEV, O_RDWR);
        if (wdt_fd <= 0)
            return;
    }

    wdt_time = duration_us;
    ioctl(wdt_fd , WDIOC_SETTIMEOUT, &wdt_time);
}

//Get the watch dog left timeout
UINT32 api_dog_get_time(UINT32 id)
{
    UINT32 wdt_timeleft;
    if (wdt_fd <=0 )    
    {
        wdt_fd = open(WATCHDOG_DEV, O_RDWR);
        if (wdt_fd <= 0)
            return 0xffffffff;
    }

    ioctl(wdt_fd,WDIOC_GETTIMELEFT, &wdt_timeleft);
    return wdt_timeleft;
}

//feed dog
void api_dog_set_time(UINT32 id, UINT32 us)
{
    UINT32 wdt_alive;
    if (wdt_fd <=0 )    
    {
        wdt_fd = open(WATCHDOG_DEV, O_RDWR);
        if (wdt_fd <= 0)
            return;
    }
    wdt_alive = 0;
    ioctl(wdt_fd, WDIOC_KEEPALIVE, &wdt_alive); 
}


void api_dog_stop(UINT32 id)
{
    UINT32 wdt_options_disenable;
    if (wdt_fd <=0 )    
    {
        wdt_fd = open(WATCHDOG_DEV, O_RDWR);
        if (wdt_fd <= 0)
            return;
    }

    wdt_options_disenable = WDIOS_DISABLECARD;
    ioctl(wdt_fd,WDIOC_SETOPTIONS, &wdt_options_disenable);
    
}

void api_dog_start(UINT32 id)
{
    UINT32 wdt_options_enable;
    if (wdt_fd <=0 )    
    {
        wdt_fd = open("/dev/watchdog", O_RDWR);
        if (wdt_fd <= 0)
            return;
    }

    wdt_options_enable = WDIOS_ENABLECARD;
    ioctl(wdt_fd,WDIOC_SETOPTIONS, &wdt_options_enable);
    
}
void api_dog_pause(UINT32 id, int en)
{}

void api_dog_close()
{
    if (wdt_fd > 0 )    
        close(wdt_fd);

    wdt_fd = -1;
}



