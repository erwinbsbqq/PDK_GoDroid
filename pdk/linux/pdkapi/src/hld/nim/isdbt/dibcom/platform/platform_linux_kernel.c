#include <adapter/debug.h>

#include <stdarg.h>

#include <osal/osal.h>
#include <api/libc/time.h>

void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...)
{
	va_list ap;

	if (dbg)
		printf("%-12s: ", dbg->prefix);
	else
		printf("%-12s: ", "DBG");

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	printf("\n");
}

void debug_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf(fmt, ap);
	va_end(ap);
}

uint32_t systime()
{
    return osal_get_tick();
}
