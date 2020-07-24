#ifndef __UART_H__
#define __UART_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "miniprintf.h"

void init_uart(void);
void uart_puts(char *string);
void uart_putln(char *string);

int uart_printf(const char *format,...);

#endif /*__UART_H_*/