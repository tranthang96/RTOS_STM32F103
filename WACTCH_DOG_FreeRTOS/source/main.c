#include "main.h"


#define POLYNOMIAL  0x131 
uint8_t addr_sht = 0x44;
I2C_Fails fc;   
I2C_Control i2c;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

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


volatile uint32_t ulIdleCycleCount = 0;

void vApplicationIdleHook( void )
{
 ulIdleCycleCount++;
}
// Task 1 
static void SHT_Read(void *args __attribute__((unused)))
{
    float temperature,humidity;
    SHT3X_SoftReset(&i2c);
    vTaskDelay( pdMS_TO_TICKS(1000) );

    for(;;){

/*
    uint8_t data[4];
    uint8_t checksum[2];
    uint16_t rawValueTem, rawValueHumi;
    //bool result[1];

    SHT3X_transfer(&i2c, CMD_MEAS_CLOCKSTR_H, data , checksum);

    //SHT3X_CalcCrc(data, checksum, result);

    rawValueTem = (uint16_t)(data[0]<<8 | data[1]);
    rawValueHumi = (uint16_t)(data[2]<<8 | data[3]);

    temperature = SHT3X_CalcTemperature(rawValueTem);
    humidity = SHT3X_CalcHumidity(rawValueHumi);

    uint8_t bit;        // bit mask
    uint8_t crc = 0xFF; // calculated checksum
    uint8_t byteCtr;    // byte counter
// Check data 1

    crc = 0xFF;

    for(byteCtr = 2; byteCtr < 4; byteCtr++)
    {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
        {
        if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
        else           crc = (crc << 1);
        }
    }

*/
    SHT3X_GetTempAndHumi(&i2c, &temperature, &humidity, REPEATAB_LOW, MODE_CLKSTRETCH);

    printf("nhiet do: %0.2f \n", temperature);  
    printf("do am: %0.2f \n", humidity);  
    vTaskDelay( xDelay250ms );
    
    iwdg_reset();
    }

}


static void vTaskFunction( void *pvParameters )
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
    }
}

int main(void)
{
    init_clock();
    gpio_init();
    init_uart();
    SHT3X_Init(&i2c, addr_sht, I2C1, 1000);
    iwd_init(); // watch dog dem thoi gian 5s. ham dc reset khi goi ham iwdg_reset();

static const char *pcTextForTask1 = "Task 1 is running\r\n";
//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

//xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );
xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 1, NULL );

xTaskCreate(vTaskFunction,"Task 1",100,(void*)pcTextForTask1,1,NULL );

//xTaskCreate(checki2c, "Task 3", 1000, NULL, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();

for(;;);

return 0;
}
