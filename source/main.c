#include "main.h"

static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

static void gpio_init(void)
{
    gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO11);
}


static inline void
uart_putc(char ch) {
    usart_send_blocking(USART1,ch);
}

static int
uart_printf(const char *format,...) {
    va_list args;
    int rc;

    va_start(args,format);
    rc = mini_vprintf_cooked(uart_putc,format,args);
    va_end(args);
    return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/* Declare a variable that will be incremented by the hook function. */
volatile uint32_t ulIdleCycleCount = 0;
/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
void vApplicationIdleHook( void )
{
 /* This hook function does nothing but increment a counter. */
 ulIdleCycleCount++;
}
////////////////////////////////////////////////

static void vTaskFunction( void *pvParameters )
{
char *pcTaskName;
const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
 /* The string to print out is passed in via the parameter. Cast this to a
 character pointer. */
 pcTaskName = ( char * ) pvParameters;
 /* As per most tasks, this task is implemented in an infinite loop. */
 for( ;; )
 {
 /* Print out the name of this task AND the number of times ulIdleCycleCount
 has been incremented. */
 uart_printf("%s" , pcTaskName );
 uart_printf("%d \n" , ulIdleCycleCount );
 /* Delay for a period of 250 milliseconds. */
 vTaskDelay( xDelay250ms );
 }
}


int main (void)
{   
    int i;
    init_clock();
    gpio_init();
    init_uart();

static const char *pcTextForTask1 = "Task 1 is running\r\n";
static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */
xTaskCreate(
vTaskFunction,
"Task 1",
1000,
(void*)pcTextForTask1,
1,
NULL );
/* Pointer to the function that
implements the task. */
/* Text name for the task. This is to
facilitate debugging only. */
/* Stack depth - small microcontrollers
will use much less stack than this. */
/* Pass the text to be printed into the
task using the task parameter. */
/* This task will run at priority 1. */
/* The task handle is not used in this
example. */
/* Create the other task in exactly the same way. Note this time that multiple
tasks are being created from the SAME task implementation (vTaskFunction). Only
the value passed in the parameter is different. Two instances of the same
task are being created. */
xTaskCreate( vTaskFunction, "Task 2", 1000, (void*)pcTextForTask2, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();
/* If all is well then main() will never reach here as the scheduler will
now be running the tasks. If main() does reach here then it is likely that
there was insufficient heap memory available for the idle task to be created.
Chapter 2 provides more information on heap memory management. */
for( ;; );

/*
    while (true) {

        uart_putln("led on");
        gpio_set(GPIOA, GPIO11);
            for (i = 0; i < 1500000; i++)   // Wait a bit. 
            __asm__("nop");
        uart_putln("LED off");
        gpio_clear(GPIOA, GPIO11);
            for (i = 0; i < 1500000; i++)   // Wait a bit.
            __asm__("nop");
    }
    return 0;
*/

}
