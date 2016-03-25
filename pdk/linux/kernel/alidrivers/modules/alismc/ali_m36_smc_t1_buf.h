
#include "ali_m36_smartcard.h"

typedef struct t1_buf 
{
	UINT8*		base;
	UINT32		head, tail, size;
	UINT32		overrun;
} t1_buf_t;

void 	t1_buf_init(t1_buf_t *, void *, UINT32);
void 	t1_buf_set(t1_buf_t *, void *, UINT32);
INT32 	t1_buf_get(t1_buf_t *, void *, UINT32);
INT32 	t1_buf_put(t1_buf_t *, const void *, UINT32);
INT32 	t1_buf_putc(t1_buf_t *, INT32  );
UINT32 	t1_buf_avail(t1_buf_t * );
void *	t1_buf_head(t1_buf_t * );

