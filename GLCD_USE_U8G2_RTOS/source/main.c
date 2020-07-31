#include "main.h"



#define LCD_RST_1 gpio_set(GPIOB, GPIO0);
#define LCD_RST_0 gpio_clear(GPIOB, GPIO0);

#define LCD_RS_1 gpio_set(GPIOA, GPIO6);
#define LCD_RS_0 gpio_clear(GPIOA, GPIO6);

#define LCD_SCLK_1 gpio_set(GPIOA, GPIO5);
#define LCD_SCLK_0 gpio_clear(GPIOA, GPIO5);

#define LCD_SID_1 gpio_set(GPIOA, GPIO7);
#define LCD_SID_0 gpio_clear(GPIOA, GPIO7);

#define LCD_BLACKLIGHT_ON gpio_set(GPIOB, GPIO1);
#define LCD_BLACKLIGHT_OFF gpio_clear(GPIOB, GPIO1);


#define POLYNOMIAL  0x131 
// uint8_t addr_sht = 0x44;
// I2C_Fails fc;   
// I2C_Control i2c;


static u8g2_t u8g2;

const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
static void init_clock(void);
static void gpio_init(void);


uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr);

static void Time_RTC(void *args __attribute__((unused)))
{
    float temperature, humidity;
    //SHT3X_Init(&i2c, addr_sht, I2C1, 1000);     
    
    //st7920_init(&lcd1, SPI1, 128, 64);
    LCD_BLACKLIGHT_ON;
    u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8g2_gpio_and_delay_stm32); // init u8g2 structure
    printf("OK\n");
    u8g2_InitDisplay(&u8g2);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1);
    u8g2_SetFontDirection(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_vietnamese2);
  //u8g2_DrawStr(&u8g2,  0, 24, "tiếng việt");
    u8g2_DrawUTF8(&u8g2, 58, 18, "Trời mưa");
    u8g2_DrawFrame(&u8g2, 56, 0, 70, 25);
    u8g2_DrawFrame(&u8g2, 56, 28, 70, 25);
  // u8g2_DrawStr(&u8g2,  0, 50, "i'm nhantt");
  // u8g2_SetFont(&u8g2, u8g2_font_u8glib_4_tf);
  // u8g2_DrawStr(&u8g2,  0, 60, "2019-05-20");
  //u8g2_DrawCircle(&u8g2, 64, 40, 10, U8G2_DRAW_ALL);
    u8g2_SetFont(&u8g2, u8g2_font_open_iconic_weather_6x_t);
    u8g2_DrawGlyph(&u8g2, 2, 50, 67);
    u8g2_DrawFrame(&u8g2, 0, 0, 55, 63);
    u8g2_SendBuffer(&u8g2);
    printf("OK Done\n");


    for(;;)
    {
 /*   SHT3X_transfer(&i2c, CMD_MEAS_CLOCKSTR_H, data , checksum);
    rawValueTem = (uint16_t)(data[0]<<8 | data[1]);
    rawValueHumi = (uint16_t)(data[2]<<8 | data[3]);

    temperature= 175.0f * (float)rawValueTem / 65535.0f - 45.0f;
    humidity= 100.0f * (float)rawValueHumi / 65535.0f;*/
/*    SHT3X_GetTempAndHumi(&i2c, &temperature, &humidity, REPEATAB_HIGH, MODE_CLKSTRETCH);

    printf("nhiet do        : %0.2f \n", temperature);  
    printf("do am           : %0.2f \n", humidity);

    printf("Thoi gian la    : %d:%d:%d \n", localTime.hour, localTime.min, localTime.sec);
    printf("Ngay/Thang/Nam  : %d/%d/%d \n", localTime.mday, localTime.month, localTime.year);
*/
/////////////////
      u8g2_DrawUTF8(&u8g2, 58, 18, "Trời mưa");
      u8g2_SendBuffer(&u8g2);
      printf("LCD Refresh \n");
      //gpio_toggle(GPIOC, GPIO13);

    vTaskDelay( pdMS_TO_TICKS(1000) );
    iwdg_reset();
    }
}
int main(void)
{
    init_clock();
    gpio_init();
    init_uart();
    //RTC_Init();
    iwd_init(); // watch dog dem thoi gian 32s. ham dc reset khi goi ham iwdg_reset();


//static const char *pcTextForTask2 = "Task 2 is running\r\n";

/* Create one of the two tasks. */

//xTaskCreate(SHT_Read, "Task 2", 1000, NULL, 2, NULL );
//xTaskCreate(SHT_Read, "Task 1", 1000, NULL, 1, NULL );

xTaskCreate(Time_RTC, "Task 2", 2000, NULL, 1, NULL );


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
    //rcc_periph_clock_enable(RCC_SPI1);
}

static void gpio_init(void)
{
    gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO11|GPIO12);
    gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO10|GPIO11|GPIO1);

}

uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      rcc_periph_clock_enable(RCC_GPIOB);
      rcc_periph_clock_enable(RCC_GPIOA);
      gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6|GPIO5|GPIO7);
      gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO0);
      gpio_set(GPIOA, GPIO6|GPIO5|GPIO7);
      gpio_set(GPIOB, GPIO0);
      break;
    case U8X8_MSG_DELAY_MILLI:
      vTaskDelay(arg_int);
      break;
    case U8X8_MSG_DELAY_10MICRO:
      vTaskDelay(1);
      break;
    case U8X8_MSG_DELAY_100NANO:
      __asm__("nop");
      break;
    case U8X8_MSG_GPIO_SPI_CLOCK:
      if(arg_int)
      {
        LCD_SCLK_1;
      }
      else
      {
        LCD_SCLK_0;
      }
      break;
    case U8X8_MSG_GPIO_SPI_DATA:
      if(arg_int)
      {
        LCD_SID_1;
      }
      else 
      {
        LCD_SID_0;
      }
      break;
    case U8X8_MSG_GPIO_CS1:
      if(arg_int)
      {
        LCD_RS_1;
      }
      else
      {
        LCD_RS_0;
      }
      break;
    case U8X8_MSG_GPIO_DC:
      break;
    case U8X8_MSG_GPIO_RESET:
      if(arg_int)
      {
        LCD_RST_1;
      }
      else
      {
        LCD_RST_0;
      }
      break;
    default:
      return 0;
  }
  return 1;
}
