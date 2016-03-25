#include <sys_define.h>
#include <retcode.h>
#include <basic_types.h>
#include <ticks.h>

static unsigned int ticks =0 ,
					ms_ticks = 0,
					us_ticks = 0 ;
static inline unsigned int read_tsc()
{
	unsigned tick;
	asm volatile ("mfc0 %0, $9" : "=r"(tick));
	return tick;
}

static inline unsigned int init_ticks()
{
	ms_ticks = (sys_ic_get_cpu_clock()*1000000 / 2000);
	us_ticks = 1000 * ms_ticks;
}

static inline void start_ticks()
{
	ticks = read_tsc();
}

static inline unsigned int end_ticks()
{
	ticks = read_tsc()- ticks;
	ticks =  ticks/ms_ticks;
}

/*
* @ size : 			bytes
* @ return value: 	KB/S
*/
static inline unsigned int caculate_s( unsigned int size)
{
	return (size * 1000)/(ticks * 1024);
}



#define MS_TICKS 	ms_ticks
#define US_TICKS	us_ticks


