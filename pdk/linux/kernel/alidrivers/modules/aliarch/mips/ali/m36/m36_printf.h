#ifndef __SOCDIAG_H
#define __SOCDIAG_H

#ifdef R4K_MERGING_PSEMU
#include <r4kos_etc.h>
#define soc_printf psr_fprintf
//#define __NDEBUG
#else
void soc_printf(const char *,...);
void soc_MessageBox(const char *,...);
#endif

#ifdef __NDEBUG
#define ASSERT(e)		((void)0)
#else

#ifdef  __cplusplus
extern "C" {
#endif

void wj_assert(int type, const char * file, int line, const char * exp);

#ifdef  __cplusplus
}
#endif

#define ASSERT(e)       ((e) ? (void)0 : wj_assert(1, __FILE__, __LINE__, #e))

#endif	//__NDEBUG


#endif	//__SOCDIAG_H

