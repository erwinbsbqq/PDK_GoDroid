/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc_test.c
 *
 *  Description: This file contains all functions to test smart card interface
 *   			Please refer the function callback frequency when coding
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *
 ****************************************************************************/

#include <types.h>
#include <retcode.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <err/errno.h>

#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>



#include <linux/dvb/ali_test.h>

static UINT8  IP_test_flag = 0;

static INT32  test_linux_fd = 0;

/********************************************************************
* Single IP Bandwidth Test *
*********************************************************************/
//Get the Linux Dev handle
void get_ip_bw_test_fd( void )
{
	if(0 == test_linux_fd)
    {
        test_linux_fd = open("/dev/ali_m36_ip_bw_0", O_RDONLY);
    }
}

//Start IP BW test task
void start_ip_bw_task( void )
{
    ioctl( test_linux_fd, ALI_TEST_IP_TASK_START, 0);
}

//Pause IP BW test task
void pause_ip_bw_task( void )
{
    ioctl( test_linux_fd, ALI_TEST_IP_TASK_PAUSE, 0);
}

//Resume IP BW test task
void resume_ip_bw_task( void )
{
    ioctl( test_linux_fd, ALI_TEST_IP_TASK_RESUME, 0);
}

//Stop IP BW test task
void stop_ip_bw_task( void )
{
    ioctl( test_linux_fd, ALI_TEST_IP_TASK_STOP, 0);
}

//Set the IP to be monitored 
void set_ip_bw( struct test_ip_bw_cfg_info *user_ip_cfg)
{
	//Set the IP to be monitored
    ioctl( test_linux_fd, ALI_TEST_IP_BW_CONFIG, (struct test_ip_bw_cfg_info *)&user_ip_cfg);   
}

//Get the specific  IP BW info
void get_ip_bw( struct test_ip_get_bw *user_ip_info)
{
	//Set the IP to be monitored
    ioctl( test_linux_fd, ALI_TEST_IP_BW_GET, (struct test_ip_get_bw *)&user_ip_info);   
}





void single_ip_bw_test(void )
{

    UINT32 cnt = 0;
	struct test_ip_bw_cfg_info user_ip_cfg;
	struct test_ip_get_bw 	user_ip_info;

		
	get_ip_bw_test_fd();
	
    //Watch the two IP with IDX ( 1 and 2 ) 
    user_ip_cfg.time_gap = 20;  //The unit is ms
    user_ip_cfg.ip_mode = 1; //or 2
    user_ip_cfg.ip_enable_flag = 0x00000003;
	set_ip_bw(&user_ip_cfg);
	
    start_ip_bw_task();
    
    while(cnt<100)
    {
		msleep(1000);
    }	
 
    //Reconfig the Single IP to be monitored   
    pause_ip_bw_task();

	//Get the IP BW info
	user_ip_info.ip_mode = 1;
	user_ip_info.ip_chan_mode = 3;
	user_ip_info.ip_idx_flag = 0x00000003;
	get_ip_bw(&user_ip_info);


	//Reset the monitored IP with IDX ( 3 and 4 ) 
    user_ip_cfg.time_gap = 20;  //The unit is ms
    user_ip_cfg.ip_mode = 1; //or 2
    user_ip_cfg.ip_enable_flag = 0x0000000B;
	set_ip_bw(&user_ip_cfg);
	
    resume_ip_bw_task();
	
    while(cnt<100)
    {
		msleep(1000);
    }	

	pause_ip_bw_task();	

	//Get the IP BW info
	user_ip_info.ip_mode = 1;
	user_ip_info.ip_chan_mode = 3;
	user_ip_info.ip_idx_flag = 0x0000000B;
	get_ip_bw(&user_ip_info);
    
    stop_ip_bw_task();

}

void total_ip_bw_test(void )
{

    UINT32 cnt = 0;
	struct test_ip_bw_cfg_info user_ip_cfg;
	struct test_ip_get_bw 	user_ip_info;

		
	get_ip_bw_test_fd();
	
    //Watch the two IP with IDX ( 1 and 2 ) 
    user_ip_cfg.time_gap = 20;  //The unit is ms
    user_ip_cfg.ip_mode = 0; //or 2
	set_ip_bw(&user_ip_cfg);
	
    start_ip_bw_task();
    
    while(cnt<100)
    {
		msleep(1000);
    }	
 
    //Reconfig the Single IP to be monitored   
    pause_ip_bw_task();

	//Get the IP BW info
	user_ip_info.ip_mode = 0;
	user_ip_info.ip_chan_mode = 3;
	get_ip_bw(&user_ip_info);


	//Restart to get total IP BW
    resume_ip_bw_task();
   
    while(cnt<100)
    {
		msleep(1000);
    }	

	pause_ip_bw_task();	

	//Get the IP BW info
	user_ip_info.ip_mode = 0;
	user_ip_info.ip_chan_mode = 3;
	get_ip_bw(&user_ip_info);
	
    stop_ip_bw_task();

}


 

