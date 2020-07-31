#include "main.h"

#define POLYNOMIAL  0x131 
uint8_t addr_sht = 0x44;
I2C_Fails fc;   
I2C_Control i2c;

LCD_STRUCT lcd1;

UG_GUI gui;

int X2 = 0;
int Y2 = 0;
int X3 = 0;
int Y3 = 0;
float angulo = 0;
int posicao = 0;
int posicaoh = 0;
int temperatura =0;
int min_temp = 500;
int max_temp = -500;

int ScreenWith = 128;
int ScreenWithC = 96;
int ScreenHeight = 64;
int ScreenHeightC = 32;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
static void init_clock(void);
static void gpio_init(void);

static void Time_RTC(void *args __attribute__((unused)))
{
    float temperature, humidity;
    st7920_blacklight_on();
    //SHT3X_Init(&i2c, addr_sht, I2C1, 1000);     
    i2c_peripheral_disable(I2C1);
    
    st7920_init(&lcd1, SPI1, 128, 64);

    UG_DrawLine(ScreenWithC, ScreenHeightC, X2, Y2, 1);
    printf("Abc\n");
    for(;;)
    {
    //spi_disable(SPI1);

 /*   SHT3X_transfer(&i2c, CMD_MEAS_CLOCKSTR_H, data , checksum);
    rawValueTem = (uint16_t)(data[0]<<8 | data[1]);
    rawValueHumi = (uint16_t)(data[2]<<8 | data[3]);

    temperature= 175.0f * (float)rawValueTem / 65535.0f - 45.0f;
    humidity= 100.0f * (float)rawValueHumi / 65535.0f;*/
    st7920_refresh(&lcd1);
/*    SHT3X_GetTempAndHumi(&i2c, &temperature, &humidity, REPEATAB_HIGH, MODE_CLKSTRETCH);

    printf("nhiet do        : %0.2f \n", temperature);  
    printf("do am           : %0.2f \n", humidity);

    printf("Thoi gian la    : %d:%d:%d \n", localTime.hour, localTime.min, localTime.sec);
    printf("Ngay/Thang/Nam  : %d/%d/%d \n", localTime.mday, localTime.month, localTime.year);*/
    vTaskDelay( pdMS_TO_TICKS(1000) );
    printf("OK done\n");
    iwdg_reset();
    }
}
int main(void)
{
    init_clock();
    gpio_init();
    init_uart();
    //RTC_Init();
   i2c_peripheral_disable(I2C1);
    //st7920_init(&lcd1, SPI1, 128, 64);
    //st7920_clear_screen(&lcd1);

    iwd_init(); // watch dog dem thoi gian 32s. ham dc reset khi goi ham iwdg_reset();

//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

//xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );
//xTaskCreate(SHT_Read, "Task 1", 1000, NULL, 1, NULL );

xTaskCreate(Time_RTC, "Task 2", 2000, NULL, 2, NULL );


//xTaskCreate(checki2c, "Task 3", 1000, NULL, 2, NULL );
/* Start the scheduler so the tasks start executing. */
vTaskStartScheduler();

for(;;);

return 0;
}

static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_SPI1);
}

static void gpio_init(void)
{
    gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO11|GPIO12);
    gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO10|GPIO11|GPIO1);

}