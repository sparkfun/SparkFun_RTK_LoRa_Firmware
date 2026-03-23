#ifndef __HC_TYPEDEF_H__
#define __HC_TYPEDEF_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SYS_STM32

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef char               HC_CHAR;
typedef signed char        HC_INT8;
typedef signed short       HC_INT16;
typedef signed int         HC_INT32;
typedef signed long long   HC_INT64;

typedef unsigned char       HC_UINT8;
typedef unsigned short      HC_UINT16;
typedef unsigned int        HC_UINT32;
typedef unsigned long long  HC_UINT64;

//typedef char               ;
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

//typedef void HC_VOID;
typedef int  HC_BOOL;
#define HC_VOID void


typedef unsigned long HC_ULONG;
typedef double HC_DOUBLE;
typedef float  HC_FLOAT;
typedef long HC_LONG;

#define HC_OK    0
#define HC_ERR  -1

#define HC_TRUE  1
#define HC_FALSE 0

#define HC_IN_PROCESS   1

#define HC_ON   1
#define HC_OFF  0

#define HC_EOF  1

/*HC_OUT, HC_IN, HC_IN_OUT used by function parmeter */
#define HC_OUT
#define HC_IN
#define HC_IN_OUT

/* lw: according to libc stddef.h  */
#ifdef __cplusplus
#define HC_NULL 0
#else
#define HC_NULL ((void *)0)
#endif

#define ARRAY_SIZE(x) ((unsigned int)(sizeof(x)/sizeof((x)[0])))

#define ALIGN_UP(num, size) (((num) + (size - 1)) & (~(size -1)))
#define ALIGN_DOWN(num,size) ((num) & (~(size -1)))


/* convert parmeter x to a string */
#define str(x)  #x

/* convert the value of macro to string  */
#define xstr(x) str(x)

#define debug_printf_hex(head,line_size,addr,size)  { \
													    HC_UINT32 i; \
														printf(head); \
														for(i=0;i<size;i++) \
														{ \
															if(i%line_size==0) \
															{ \
																printf("\r\n"); \
															} \
															printf("%02X ",addr[i]); \
														} \
														printf("\r\n"); \
													} \

#define PRINTF_LEVEL 1
													

extern int debug ;
#define  loge(fmt,args...)  {if(debug == 1){printf("ERROR-%s, %d: ",__FILE__, __LINE__);printf(fmt,##args);}}
#define  logi(fmt,args...)  {if(debug == 1){  printf("INF-%s, %d: ",__FILE__, __LINE__);printf(fmt,##args);}}

#ifndef WIN32
#if(PRINTF_LEVEL>=1)
//#define  loge(fmt,args...)  {printf("ERROR-%s, %d: ",__FILE__, __LINE__);printf(fmt,##args);}
#else
#define  loge(fmt,args...)  {}
#endif

#if(PRINTF_LEVEL>=2)
#define  logi(fmt,args...)  {printf("INF---%s, %d: ",__FILE__, __LINE__);printf(fmt,##args);}
#else
//#define  logi(fmt,args...)  {}
#endif

#if(PRINTF_LEVEL>=3)
#define  logd(fmt,args...)  {printf("DEB---%s, %d: ",__FILE__, __LINE__);printf(fmt,##args);}
#else
#define  logd(fmt,args...)  {}
#endif
#else

#define  loge    printf
#define  logi    printf
#define  logd    printf

#endif

#define usleep(a)  delay_ms((a)/1000)

#endif

