#include "uart.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
//uint8_t data_uart = 'A';
void init_uart(void)
{
    //clock for GPIO port A:
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);

    /* ==================================
    //STM32F103C8T6 
        RX = PA9
        TX = PA10
        CTS = PA11 (not used)
        RTS = PA12 (not used)
        Baud = 115200
    */
    nvic_enable_irq(NVIC_USART1_IRQ);

    // GPIO_USART1_TX on GPIO port A for tx, rx
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);
    
    usart_set_baudrate(USART1,115200);
    usart_set_databits(USART1,8);
    usart_set_stopbits(USART1,USART_STOPBITS_1);
    usart_set_mode(USART1,USART_MODE_TX);
    usart_set_parity(USART1,USART_MODE_TX_RX);

    usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);

    /* Enable USART1 Receive interrupt. */

    //usart_enable_rx_interrup(USART1);

    USART_CR1(USART1) |= USART_CR1_RXNEIE;

    // finally enable the uart
    usart_enable(USART1);
}