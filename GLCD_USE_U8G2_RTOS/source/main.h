#ifndef __MAIN_H__
#define __MAIN_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
// #include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>

#include <libopencm3/stm32/f1/bkp.h>

#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"

// #include "sht3x.h"

#include "iwdg.h"

// #include "rtc.h"

//#include "u8x8.h"
#include "u8g2.h"


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
// #include <math.h>
void vApplicationIdleHook( void );

#endif