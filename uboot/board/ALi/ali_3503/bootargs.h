#ifndef BOOTARGS_H
#define BOOTARGS_H
 
struct boot_args
{
	char magic[16];	// bootargs
	int registerinfo_size;
	unsigned char registerinfo[1024*8-12-16-4];		
	int cmdline_size;
	char cmdline[1024*4-4];
	int cmdline_size_rcv;
	char cmdline_rcv[1024*4-4];
	int meminfo_size;	
	unsigned char meminfo[1024*4-4];	
	int meminfo_size_rcv;	
	unsigned char meminfo_rcv[1024*4-4];
	int tveinfo_size;		
	unsigned char tveinfo[1024*28-4];	
};

#endif // BOOTARGS_H
