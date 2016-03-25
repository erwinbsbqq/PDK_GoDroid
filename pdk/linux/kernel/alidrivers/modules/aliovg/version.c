#include "compile.h"
#include <linux/utsrelease.h>

static char *mod_version(void) __attribute__((used));

static char *mod_version(void)
{
	return("\n\n" "<INFO>\nDATE/TIME="MOD_COMPILE_TIME
			"\nPIC=" MOD_COMPILE_BY "\nHOST=" MOD_COMPILE_HOST
			"\nCOMMIT=" MOD_COMMIT 
			"\nLINUX=" UTS_RELEASE
			"\n</INFO>\n");
}

#ifdef __FINAL_ENABLE_PROC__
void todo_print_ver(void)
{
}
#endif
