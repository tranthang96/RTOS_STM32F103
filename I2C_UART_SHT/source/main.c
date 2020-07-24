#include "main.h"
#include <stdio.h>

I2C_Fails fc;   
I2C_Control i2c;
uint8_t addr = 0x44;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_init(void)
{
    gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO11);
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
 /* The string to print out is passed in via the parameter. Cast this to a
 character pointer. */
pcTaskName = ( char * ) pvParameters;
 /* As per most tasks, this task is implemented in an infinite loop. */
for( ;; )
    {
    gpio_set(GPIOA, GPIO11);
    /* Print out the name of this task AND the number of times ulIdleCycleCount
    has been incremented. */
    uart_printf("%s" , pcTaskName );
    uart_printf("%d \n" , ulIdleCycleCount );
    /* Delay for a period of 250 milliseconds. */
    vTaskDelay( xDelay250ms );

    gpio_clear(GPIOA, GPIO11);

    vTaskDelay( xDelay250ms );
    uart_printf("I2C Fail code %d\n %s \n",fc,i2c_error(fc));

    if ( (fc = setjmp(i2c_exception)) != I2C_Ok ) 
        {
        uart_printf("I2C Fail code %d\n %s \n",fc,i2c_error(fc));
        }
    }
}

static void SHT_Read(void *args __attribute__((unused)))
{
    uint8_t data[10];
    uint32_t serialNumber;

    float temperature;
    float humidity;

    for(;;)
    {
    if ( (fc = setjmp(i2c_exception)) != I2C_Ok ) 
        {
        uart_printf("I2C Fail code %d\n %s \n",fc,i2c_error(fc));
        break;
        }
    //uart_printf("data %d \n", data[0]);
    //i2c_start_addr(&i2c, addr, I2C_READ);

    SHT3X_transfer(&i2c, addr, CMD_MEAS_CLOCKSTR_H , data, 6);

    serialNumber = (uint32_t) ( data[0] << 24 | data[1] << 16 | data[3] << 8 | data[4] );
    uart_printf("serialNumber %d \n", serialNumber);
   // uart_printf("serialNumber1 %d \n", SHT3x_ReadSerialNumber(&i2c,addr));
    uint16_t rawValue = (uint16_t) ((data[0] << 8) | data[1]);
    uart_printf("serialNumber %d \n", rawValue);

    float nhietdo = (float)(175.0f * (float)rawValue / 65535.0f - 45.0f);
    uart_printf("nhiet do %f \n", nhietdo );

    SHT3X_GetTempAndHumi(&i2c, addr, &temperature, &humidity, REPEATAB_HIGH, MODE_CLKSTRETCH);
    uart_printf("nhiet do %.2f \n", temperature);
    uart_printf("do am %d \n", humidity);
    vTaskDelay( xDelay250ms );
    }
}

int main (void)
{   
    int i;
    init_clock();
    gpio_init();
    init_uart();
    SHT3X_Init(&i2c, I2C1, 1000);

static const char *pcTextForTask1 = "Task 1 is running\r\n";
//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );

xTaskCreate(vTaskFunction,"Task 1",100,(void*)pcTextForTask1,1,NULL );

//xTaskCreate(checki2c, "Task 3", 1000, NULL, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();

for( ;; );
}
