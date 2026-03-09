#ifndef __DRV_UART_H__
#define __DRV_UART_H__
#include "usart.h"
#include <stdarg.h>

//#define UART_DMA_IDLE_MODE  1
typedef int (* drv_uart_read_cb_t)(uint8_t *data, const uint16_t len);

extern int drv_uart_com1_init(void);
extern int drv_uart_com2_init(void);

extern int drv_uart_com1_read_set_cb(const drv_uart_read_cb_t cb);
extern int drv_uart_com1_send(const uint8_t *data, const uint16_t len);
extern int drv_uart_com1_clear_rx(void);

extern int drv_uart_com2_read_set_cb(const drv_uart_read_cb_t cb);
extern int drv_uart_com2_send(const uint8_t *data, const uint32_t len);
extern int drv_uart_com2_clear_rx(void);
extern int drv_printf(const char * format, ...);
extern void com1_send_block(uint8_t *p_data, uint16_t size);
extern void com1_send_DMA(uint8_t *p_data, uint16_t size);
#endif /*__DRV_UART_H__*/

