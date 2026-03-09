#ifndef __RING_BUFFER
#define __RING_BUFFER

#include "hc_type.h"
typedef struct __CFIFO
{
	u32  len;
	u32	out;
	u32	in;
	u8*	StartAddr;
}CFIFO;
// len must be 2^n
u8	cfifo_init(CFIFO* pfifo, u8* startAddr, u32 len);
u32 cfifo_read(CFIFO *pfifo, u8* buff, u32 len);
u32	cfifo_write(CFIFO *pfifo, u8* buff, u32 len);
void cfifo_reset(CFIFO *pfifo);
u32 cfifo_get_datanum(CFIFO *pfifo);
u32 cfifo_is_empty(CFIFO *pfifo);
u32 cfifo_is_full(CFIFO *pfifo);

#endif
