#include "main.h"
#include <libopencm3/stm32/f1/bkp.h>
#include "queue.h"

    int8_t hourSet = 6;                // hour 0 - 23
    int8_t minSet = 0;                 // min 0 - 59 

#define POLYNOMIAL  0x131 
uint8_t addr_sht = 0x44;
I2C_Fails fc;   
I2C_Control i2c;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

#define mainECHO_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )

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
    gpio_set_mode(GPIOA,GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,GPIO8);
}


static void vTaskFunction( void *pvParameters )
{

char *pcTaskName;

pcTaskName = ( char * ) pvParameters;
for( ;; )
    {
/*    gpio_set(GPIOA, GPIO11);
    gpio_set(GPIOA, GPIO12);
    gpio_set(GPIOB, GPIO10);
    gpio_set(GPIOB, GPIO11);*/
    printf("%s" , pcTaskName );
   // printf("%ld \n" , ulIdleCycleCount );
    vTaskDelay( xDelay250ms );
/*    gpio_clear(GPIOA, GPIO11);
    gpio_clear(GPIOA, GPIO12);
    gpio_clear(GPIOB, GPIO11);
    gpio_clear(GPIOB, GPIO10);*/
    vTaskDelay( xDelay250ms );
    vTaskDelay( xDelay250ms );
    }
}

static void Time_RTC(void *args)
{
    (void)args;
    float temperature,humidity;
    for(;;){

    if(gpio_get(GPIOA, GPIO8)); //gpio_get(GPIOA, GPIO0)

    else{

        localTime.year = 2020;
        localTime.month = 8;
        localTime.mday = 1;
        localTime.hour = 6-7;
        localTime.min = 0;
        localTime.sec = 0;
        pwr_disable_backup_domain_write_protect();
        RTC_Sync(&localTime, +7);
        pwr_enable_backup_domain_write_protect();
    }
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

static void TUOICAY(void *args)
{   // 5 phut tuoi cay
    (void)args;
    for (;;)
    {
        if(localTime.hour == hourSet)
        {
            if(localTime.min == minSet)
            {
            gpio_clear(GPIOA, GPIO11|GPIO12);
            gpio_clear(GPIOB, GPIO11|GPIO10);
        //gpio_clear(GPIOB, GPIO10);
            vTaskDelay( xDelay250ms );
            vTaskDelay( xDelay250ms );
            printf("%s \n" , "Tuoi cay" );
            }
            else{
            gpio_set(GPIOA, GPIO11);
            gpio_set(GPIOA, GPIO12);
            gpio_set(GPIOB, GPIO10);
            gpio_set(GPIOB, GPIO11);
        }

        }
        /* code */
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


static const char *pcTextForTask1 = "Task 1 is running\r\n";
//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

//xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );
//xTaskCreate(SHT_Read, "Task 1", 1000, NULL, 1, NULL );

xTaskCreate(Time_RTC, "Task 1", 1000, NULL, configMAX_PRIORITIES-1, NULL );
xTaskCreate(TUOICAY, "Task 2", 1000, NULL, configMAX_PRIORITIES-2, NULL );


xTaskCreate(vTaskFunction,"Task 3",100,(void*)pcTextForTask1,configMAX_PRIORITIES ,NULL );

//xTaskCreate(checki2c, "Task 3", 1000, NULL, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();

for(;;);

return 0;
}
