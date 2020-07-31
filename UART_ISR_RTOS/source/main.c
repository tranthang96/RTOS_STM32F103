#include "main.h"
#include <libopencm3/stm32/f1/bkp.h>
#include "queue.h"

#define POLYNOMIAL  0x131 
uint8_t addr_sht = 0x44;
I2C_Fails fc;   
I2C_Control i2c;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

#define mainECHO_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName);

static QueueHandle_t uart_txq;              // TX queue for UART

void
vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName) {
    (void)pxTask;
    (void)pcTaskName;
    for(;;);
}
/*********************************************************************
 * Configure and initialize GPIO Interfaces
 *********************************************************************/

static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
}

static void gpio_init(void)
{
    gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO11|GPIO12);
    gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO10|GPIO11);

}


/*static void vTaskFunction( void *pvParameters )
{

char *pcTaskName;

pcTaskName = ( char * ) pvParameters;
for( ;; )
    {
    gpio_set(GPIOA, GPIO11);
    gpio_set(GPIOA, GPIO12);
    gpio_set(GPIOB, GPIO10);
    gpio_set(GPIOB, GPIO11);
    printf("%s" , pcTaskName );
    printf("%ld \n" , ulIdleCycleCount );
    vTaskDelay( xDelay250ms );
    gpio_clear(GPIOA, GPIO11);
    gpio_clear(GPIOA, GPIO12);
    gpio_clear(GPIOB, GPIO11);
    gpio_clear(GPIOB, GPIO10);
    vTaskDelay( xDelay250ms );
    vTaskDelay( xDelay250ms );
    }
}
*/
static void Time_RTC(void *args)
{
    (void)args;
    float temperature,humidity;
    for(;;){
    //pwr_disable_power_voltage_detect();
    //pwr_enable_wakeup_pin();
    //pwr_disable_backup_domain_write_protect();
    //uint32_t a = BKP_DR1;
    SHT3X_GetTempAndHumi(&i2c, &temperature, &humidity, REPEATAB_LOW, MODE_CLKSTRETCH);
    printf("nhiet do        : %0.2f \n", temperature);  
    printf("do am           : %0.2f \n", humidity);

    printf("Thoi gian la    : %d:%d:%d \n", localTime.hour, localTime.min, localTime.sec);
    printf("Ngay/Thang/Nam  : %d/%d/%d \n", localTime.mday, localTime.month, localTime.year);
    vTaskDelay( xDelay250ms );
    vTaskDelay( xDelay250ms );
    vTaskDelay( xDelay250ms );
    vTaskDelay( xDelay250ms );
    iwdg_reset();
    }
}

static inline void
uart_puts(const char *s) {
    
    for ( ; *s; ++s )
        xQueueSend(uart_txq,s,portMAX_DELAY); /* blocks when queue is full */
}

static void SET_DATA(void *args)
{
    int gc;
    char kbuf[256], ch;
    
    (void)args;

    puts_uart(1,"\n\ruart_task() has begun:\n\r");

    for (;;) {
    //puts_uart(1,"\n\ruart_task() has begun:\n\r");
        if ( (gc = getc_uart_nb(1)) != -1 ) {
            puts_uart(1,"\r\n\nENTER INPUT: ");

            ch = (char)gc;
            if ( ch != '\r' && ch != '\n' ) {
                /* Already received first character */
                kbuf[0] = ch;
                putc_uart(1,ch);
                getline_uart(1,kbuf+1,sizeof kbuf-1);
            } else  {
                /* Read the entire line */
                getline_uart(1,kbuf,sizeof kbuf);
            }

            puts_uart(1,"\r\nReceived input '");
            puts_uart(1,kbuf);
            puts_uart(1,"'\n\r\nResuming prints...\n\r");
        }
        /* Receive char to be TX */
        if ( xQueueReceive(uart_txq,&ch,10) == pdPASS )
            putc_uart(1,ch);
        /* Toggle LED to show signs of life */

    }
}

int main(void)
{
    init_clock();
    gpio_init();
    init_uart();
    SHT3X_Init(&i2c, addr_sht, I2C1, 1000);     
    RTC_Init();
    //iwd_init(); // watch dog dem thoi gian 32s. ham dc reset khi goi ham iwdg_reset();
    uart_txq = xQueueCreate(256,sizeof(char));


//static const char *pcTextForTask1 = "Task 1 is running\r\n";
//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

//xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );
//xTaskCreate(SHT_Read, "Task 1", 1000, NULL, 1, NULL );

xTaskCreate(Time_RTC, "Task 1", 1000, NULL, configMAX_PRIORITIES-1, NULL );
xTaskCreate(SET_DATA, "Task 2", 1000, NULL, configMAX_PRIORITIES-2, NULL );


//xTaskCreate(vTaskFunction,"Task 1",100,(void*)pcTextForTask1,1,NULL );

//xTaskCreate(checki2c, "Task 3", 1000, NULL, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();

for(;;);

return 0;
}
