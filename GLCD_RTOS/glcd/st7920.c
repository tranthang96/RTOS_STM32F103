/*	Connect STM32F103C8T6 to GLCD

	PA5(SPI1_SCK) - EN (SCK)
	PA7(SPI1_MOSI)- R/W(Sserial data input)

	PA6 - RS (CS) ( serial mode: chip select
							1: chip enable
							0: chip disable )

	PB0  - RESET

	PB1 - backlight   (1 :enable , 0 : disable)

	GLCD : PSB -- GND
*/
#include <stdio.h>

#include "st7920.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

//send 2 byte
static void st7920_send(LCD_STRUCT *lcd, uint8_t dat1, uint8_t dat2);
static void st7920_send_command(LCD_STRUCT *lcd, uint8_t cmd);
static void st7920_send_data(LCD_STRUCT *lcd, uint8_t dat);
static void st7920_set_addr(LCD_STRUCT *lcd, uint8_t row, uint8_t col);

static void st7920_send(LCD_STRUCT *lcd, uint8_t dat1, uint8_t dat2)
{	
	uint16_t timeOut = 0xFFFF;
  	spi_send(lcd->spi, dat1);
  	spi_send(lcd->spi, (dat2 & 0xF0));
  	spi_send(lcd->spi, (dat2 << 4));
  	while((SPI_SR(lcd->spi) & SPI_SR_BSY)&&(timeOut--));
}

static void st7920_send_command(LCD_STRUCT *lcd, uint8_t cmd)
{
	st7920_send(lcd, 0xF8, cmd);
}

static void st7920_send_data(LCD_STRUCT *lcd, uint8_t dat)
{
	st7920_send(lcd, 0xFA, dat);
}

static void st7920_set_addr(LCD_STRUCT *lcd, uint8_t row, uint8_t col)
{
	st7920_send_command(lcd, 0x3E);
  	st7920_send_command(lcd, 0x80 | (row & 31));
  	st7920_send_command(lcd, 0x80 | col | ((row & 32) >> 2));
  	vTaskDelay(pdMS_TO_TICKS(50));
}

void st7920_reset(void)
{
	gpio_clear(GPIOB, GPIO0);
	vTaskDelay(pdMS_TO_TICKS(50));
	gpio_set(GPIOB, GPIO0);
}
void st7920_blacklight_on(void)
{
	gpio_set(GPIOB, GPIO1);
}

void st7920_blacklight_off(void)
{
	gpio_clear(GPIOB, GPIO1);
}

void st7920_init(LCD_STRUCT *lcd, uint32_t spi, uint8_t width, uint8_t height)
{
	lcd->spi = spi;
	lcd->width = width;
	lcd->height = height;

	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_SPI1);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5 | GPIO7);

	spi_reset(spi);
	spi_init_master(spi, SPI_CR1_BAUDRATE_FPCLK_DIV_256, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                  SPI_CR1_CPHA_CLK_TRANSITION_2, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

	//spi_set_bidirectional_transmit_only_mode(spi);
	//pi_disable_software_slave_management(SPI1);
	//spi_enable_ss_output(SPI1);
	spi_enable(spi);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO0); // reset
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);	//blacklight
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6); //chip select

	st7920_reset();
	gpio_set(GPIOA,GPIO6);	// enable chip
	vTaskDelay(pdMS_TO_TICKS(10));

	st7920_send_command(lcd, 0x38); //FUNCTION_SET: 8bit interface (DL=1), basic instruction set (RE=0) 0x38
	//printf("OK\n");
	vTaskDelay(pdMS_TO_TICKS(72));	
	st7920_send_command(lcd, 0x08);  //display on, cursor & blink off; 0x08: all off
	//printf("OK1\n");
	vTaskDelay(pdMS_TO_TICKS(72));	
	st7920_send_command(lcd, 0x06); //Entry mode: cursor move to right, DRAM address counter (AC) plus 1, no shift
	//printf("OK2\n");
	vTaskDelay(pdMS_TO_TICKS(72));	
	st7920_send_command(lcd, 0x02); //disable scroll, enable CGRAM address
	//printf("OK3\n");
	vTaskDelay(pdMS_TO_TICKS(100));	
	st7920_send_command(lcd, 0x01); // clear RAM
	//printf("OK4\n");
	vTaskDelay(pdMS_TO_TICKS(2));	
	gpio_clear(GPIOA,GPIO6);		//disable chip
	printf("OK5\n");

}

void st7920_turn_on(LCD_STRUCT *lcd)
{
	st7920_send_command(lcd, DISPLAY_CONTROL | 0x0C);
}

void st7920_turn_off(LCD_STRUCT *lcd)
{
	st7920_send_command(lcd, DISPLAY_CONTROL | 0x08);
}

void st7920_draw_pixel(LCD_STRUCT *lcd, uint8_t x, uint8_t y, enum PixelMode val)
{
	if((x >= lcd->width) || (y >= lcd->height)) return;
	uint16_t cnt;
	cnt = y*16 + x/8;
	if(val == 0) // clear pixel
		lcd->buff[cnt] &= ~(0x80 >> x%8); 
	else if(val == 1) // set pixel
		lcd->buff[cnt] |= (0x80 >> x%8);
	else  // invert pixelss
		lcd->buff[cnt] ^= (0x80 >> x%8);
}
uint8_t st7920_get_pixel(LCD_STRUCT *lcd, uint8_t x, uint8_t y)
{
	if((x >= lcd->width) || (y >= lcd->height)) return 0;
	return lcd->buff[x + (y*16 + x/8)] & (0x80 >> x%8) ? 1 : 0;
}

void st7920_refresh(LCD_STRUCT *lcd)
{
	uint8_t row;
	uint8_t col;
	uint8_t *ptr;
	gpio_set(GPIOA, GPIO6);
	ptr = lcd->buff;
	for(row = 0; row < 64; row++)
	{
		st7920_set_addr(lcd, row, 0);
		for(col = 0; col < 8; col++)
		{
			st7920_send_data(lcd, *ptr++);
			st7920_send_data(lcd, *ptr++);
			vTaskDelay(pdMS_TO_TICKS(10));	
		}
	}
	gpio_clear(GPIOA, GPIO6);
}

void st7920_clear_screen(LCD_STRUCT *lcd)
{
	memset(lcd->buff, 0x00, sizeof(lcd->buff));
}

void st7920_draw_box(LCD_STRUCT *lcd, int x1, int y1, int x2, int y2, bool set)
{
	int x, y;
	for(x = x1; x <= x2; x++)
		for(y = y1; y <= y2; y++)
			set ? st7920_draw_pixel(lcd, x, y, PixelSet) : st7920_draw_pixel(lcd, x, y, PixelClear);
}

void st7920_draw_line(LCD_STRUCT *lcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	uint8_t dy, dx;
	uint8_t addx, addy;
	int16_t p, diff, i;
	if(x1 >= x0)
	{
		dx = x1 - x0;
		addx = 1;
	} else {
		dx = x0 - x1;
		addx = -1;
	}
	if(y1 >= y0)
	{
		dy = y1 - y0;
		addy = 1;
	} else {
		dy = y0 - y1;
		addy = -1;
	}
	if(dx >= dy)
	{
		dy *=2;
		p = dy - dx;
		diff = p - dx;
		for(i = 0; i <= dx; ++i)
		{
			lcd->buff[x0/8 + (y0*16)] |= (0x80 >> x0%8);
			if(p < 0)
			{
				p += dy;
				x0 += addx;
			} else {
				p += diff;
				x0 += addx;
				y0 += addy;
			}
		}
	} else {
		dx *= 2;
		p = dx - dy;
		diff = p - dy;
		for(i = 0; i <= dy; ++i)
		{
			lcd->buff[x0/8 + (y0*16)] |= (0x80 >> x0%8);
			if(p < 0)
			{
				p += dx;
				y0 += addy;
			} else {
				p += diff;
				x0 += addx;
				y0 += addy;
			}
		}
	}
}

void st7920_draw_8_pixel(LCD_STRUCT *lcd, uint8_t xc, uint8_t yc, uint8_t x, uint8_t y)
{
	st7920_draw_pixel(lcd, x + xc, y + yc, PixelSet);
	st7920_draw_pixel(lcd, -x + xc, y + yc, PixelSet);
	st7920_draw_pixel(lcd, x + xc, -y + yc, PixelSet);
	st7920_draw_pixel(lcd, -x + xc, -y + yc, PixelSet);
	st7920_draw_pixel(lcd, y + xc, x + yc, PixelSet);
	st7920_draw_pixel(lcd, -y + xc, x + yc, PixelSet);
	st7920_draw_pixel(lcd, y + xc, -x + yc, PixelSet);
	st7920_draw_pixel(lcd, -y + xc, -x + yc, PixelSet);
}

void st7920_draw_circle_midpoint(LCD_STRUCT *lcd, int xc, int yc, int r)
{
	int x = 0; int y = r;
	int f = 1 - r;
	st7920_draw_8_pixel(lcd, xc, yc, x, y);
	while (x < y)
	{
		if(f < 0) f += (x<<1) + 3;
		else
		{
			y--;
			f += ((x-y) << 1) + 5;
		}
		x++;
		st7920_draw_8_pixel(lcd, xc, yc, x, y);
	}
}

void st7920_draw_circle(LCD_STRUCT *lcd, uint8_t x, uint8_t y, uint8_t radius)
{
	int16_t a, b, p;
	a = 0;
	b = radius;
	p = 1 - radius;
	do {
		// lcd->buff[(x/8 + a) + ((y+b)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 + b) + ((y+a)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 - a) + ((y+b)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 - b) + ((y+a)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 + a) + ((y+b)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 + b) + ((y-a)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 - a) + ((y-b)*16)] |= (0x80 >> x%8);
		// lcd->buff[(x/8 - b) + ((y-a)*16)] |= (0x80 >> x%8);
		st7920_draw_8_pixel(lcd, x, y, a, b);
			// st7920_draw_pixel(lcd, a + x, b + y, PixelSet);
			// st7920_draw_pixel(lcd, -a + x, b + y, PixelSet);
			// st7920_draw_pixel(lcd, a + x, -b + y, PixelSet);
			// st7920_draw_pixel(lcd, -a + x, -b + y, PixelSet);
			// st7920_draw_pixel(lcd, b + x, b + y, PixelSet);
			// st7920_draw_pixel(lcd, -b + x, b + y, PixelSet);
			// st7920_draw_pixel(lcd, b + x, -b + y, PixelSet);
			// st7920_draw_pixel(lcd, -b + x, -b + y, PixelSet);
		if(p < 0)
			p += 3 + 2*a++;
		else
			p += 5 + 2*(a++ -b--);
	} while(a <= b);
}
