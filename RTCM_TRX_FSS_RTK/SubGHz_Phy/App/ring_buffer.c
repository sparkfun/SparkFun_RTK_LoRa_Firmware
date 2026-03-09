#include "ring_buffer.h"
#define MIN(x,y) (((x)<(y))?(x):(y))

u8	cfifo_init(CFIFO* pfifo, u8* startAddr, u32 len)
{
	memset(pfifo, 0, sizeof(CFIFO));
	pfifo->StartAddr = startAddr;
	pfifo->in = 0;
	pfifo->out = 0;
	pfifo->len = len;
	return 0;
}



u32	cfifo_write(CFIFO *pfifo, u8* buff, u32 len)
{
		u32 l;
		u32 offset;
		u32 validsize;

		validsize = pfifo->len - pfifo->in + pfifo->out;
		
		len = MIN(len, validsize);                                       
		offset = (pfifo->in & (pfifo->len - 1));                         /* first put the data starting from fifo->in to buffer end*/
		l = MIN(len, pfifo->len - offset);	
		memcpy(pfifo->StartAddr + offset, buff, l);
		
		if (len>l)
			memcpy(pfifo->StartAddr, buff + l, (len - l));              /* then put the rest (if any) at the beginning of the buffer*/
		pfifo->in += len;
		return len;
}


u32 cfifo_read(CFIFO *pfifo, u8* buff, u32 len)
{
		u32 l;
		u32 offset;
		u32 validsize;
		validsize = pfifo->in - pfifo->out;
		if (!validsize)
			return 0;
		len = MIN(len, validsize);                                        
		offset = (pfifo->out & (pfifo->len - 1));                         /* first get the data from fifo->out until the end of the buffer*/	
		l = MIN(len, pfifo->len - offset);
		memcpy(buff, pfifo->StartAddr + offset, l);
		if (len>l)                                                      	/* then get the rest (if any) from the beginning of the buffer*/
			memcpy(buff + l, pfifo->StartAddr, (len - l));
		pfifo->out += len;
		return len;                              /*      Return NULL pointer to caller            */
}

 void cfifo_reset(CFIFO *pfifo)
{
	 pfifo->in = pfifo->out = 0;
}

u32 cfifo_get_datanum(CFIFO *pfifo)
{
	return pfifo->in - pfifo->out;
}

u32 cfifo_is_empty(CFIFO *pfifo)
{
	return (pfifo->in == pfifo->out);
}

u32 cfifo_is_full(CFIFO *pfifo)
{
	return (pfifo->in - pfifo->out== pfifo->len);
}

