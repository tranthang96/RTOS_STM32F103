#ifndef __MAIN_H__
#define __MAIN_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>
#include "FreeRTOS.h"
#include "task.h"

#include "serial.h"
#include "sht3x.h"

#include "iwdg.h"

#include "rtc.h"

#include <stdio.h>
//void vApplicationIdleHook( void );

#endif