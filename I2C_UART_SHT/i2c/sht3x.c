#include "FreeRTOS.h"
#include "task.h"
#include "sht3x.h"

//-- Defines ------------------------------------------------------------------
// Generator polynomial for CRC
#define POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001



static inline TickType_t
diff_ticks(TickType_t early,TickType_t later) {

	if ( later >= early )
		return later - early;
	return ~(TickType_t)0 - early + 1 + later;
}
static inline short SHT3X_CalcTemperature(uint16_t rawValue)
{
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / (2^16-1)
  return 175.0f * (short)rawValue / 65535.0f - 45.0f;
}

//-----------------------------------------------------------------------------
static inline short SHT3X_CalcHumidity(uint16_t rawValue)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / (2^16-1) * 100
  return 100.0f * (short)rawValue / 65535.0f;
}

//=============================================================================
void SHT3X_Init(I2C_Control *dev, uint32_t i2c, uint32_t ticks)
{
	rcc_periph_clock_enable(RCC_GPIOB);	// I2C
	rcc_periph_clock_enable(RCC_GPIOC);	// LED
	rcc_periph_clock_enable(RCC_AFIO);	// EXTI
	rcc_periph_clock_enable(RCC_I2C1);	// I2C

	gpio_set_mode(GPIOB,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
		GPIO6|GPIO7);			// I2C
	gpio_set(GPIOB,GPIO6|GPIO7);		// Idle high
	     
	// AFIO_MAPR_I2C1_REMAP=0, PB6+PB7
	gpio_primary_remap(0,0);

	// configure i2c1 
	i2c_configure(dev,i2c,ticks);
}

void i2c_write_16b(I2C_Control *dev, etCommands command)
{
	uint8_t data1, data2;
	data1	= (uint8_t) (command >> 8);
	data2	= (uint8_t) (command & 0xFF);

	i2c_write(dev, data1);
	i2c_write(dev, data2);

	i2c_send_stop(dev->device);
}


void SHT3X_transfer(I2C_Control *dev, uint8_t addr, etCommands command, uint8_t *res, size_t n)
{
	i2c_start_addr(dev, addr, I2C_WRITE);
/*
	uint8_t data[1];
	data[0] = (uint8_t) (command >>8);
	data[1] = (uint8_t) (command & 0xFF);
// Write command sht3x
	for (int i = 0; i < 2; i++)
	{
		// code 
		i2c_write(dev, data[i]);
	}

	uint8_t data1, data2;
	data1	= (uint8_t) (command >> 8);
	data2	= (uint8_t) (command & 0xFF);

	i2c_write(dev, data1);
	i2c_write(dev, data2);

	i2c_send_stop(dev->device);
*/
	i2c_write_16b(dev, command);
// Read data sht3x 
	i2c_start_addr(dev, addr, I2C_READ);

	TickType_t t0;

	for (uint8_t i = 0; i < n; ++i) 
	{	
		t0 = systicks();
		if (i == n - 1) 
		{
			i2c_disable_ack(dev->device);
		}

		while ( !(I2C_SR1(dev->device) & I2C_SR1_RxNE) ) 
		{
		if ( diff_ticks(t0,systicks()) > dev->timeout )
			longjmp(i2c_exception,I2C_Read_Timeout);
		taskYIELD();
		}
	res[i] = i2c_get_data(dev->device);
	}
	i2c_send_stop(dev->device);
}

uint32_t SHT3x_ReadSerialNumber(I2C_Control *dev, uint8_t addr)
{	
	uint8_t data[5];
	SHT3X_transfer(dev, addr, CMD_READ_SERIALNBR , data, 6);
	return (uint32_t) ( data[0] << 24 | data[1] << 16 | data[3] << 8 | data[4] );
}

uint16_t SHT3X_ReadStatus(I2C_Control *dev, uint8_t addr)
{
	uint8_t data[2];
	SHT3X_transfer(dev, addr, CMD_READ_STATUS , data, 3);
	return (uint16_t) (data[0] << 8 | data[1]);
}

void SHT3X_ClearAllAlertFlags(I2C_Control *dev, uint8_t addr)
{
	i2c_start_addr(dev, addr, I2C_WRITE);

	uint8_t data1, data2;
	data1	= (uint8_t) (CMD_CLEAR_STATUS >> 8);
	data2	= (uint8_t) (CMD_CLEAR_STATUS & 0xFF);

	i2c_write(dev, data1);
	i2c_write(dev, data2);

	i2c_send_stop(dev->device);
}

void SHT3X_GetTempAndHumi(I2C_Control *dev,uint8_t addr, float *temperature, float *humidity, etRepeatability repeatability, etMode mode)
{
	switch (mode)
	{
		case MODE_CLKSTRETCH: // get temperature with clock stretching mode
			SHT3X_GetTempAndHumiClkStretch(dev, addr, temperature, humidity, repeatability);
		 	break;
   		 case MODE_POLLING:    // get temperature with polling mode
 			SHT3X_GetTempAndHumiPolling(dev, addr, temperature, humidity, repeatability);
      		break;
    	default:             
      		break;
	}
}

void SHT3X_GetTempAndHumiClkStretch(I2C_Control *dev, uint8_t addr, float *temperature, float *humidity, etRepeatability repeatability)
{
	uint16_t rawValueTemp; // temperature raw value from sensor
	uint16_t rawValueHumi; // humidity raw value from sensor

	i2c_start_addr(dev, addr, I2C_WRITE);

	switch (repeatability)
	{
		case REPEATAB_LOW:
			i2c_write_16b(dev, CMD_MEAS_CLOCKSTR_L);
			break;
	    case REPEATAB_MEDIUM:
        	i2c_write_16b(dev, CMD_MEAS_CLOCKSTR_M);
        	break;
      	case REPEATAB_HIGH:
        	i2c_write_16b(dev, CMD_MEAS_CLOCKSTR_H);
        	break;
      default:
        	break;
	}

	i2c_start_addr(dev, addr, I2C_READ);
	TickType_t t0;
	uint8_t data[5];
	for (uint8_t i = 0; i < 6; i++) 
	{	
		t0 = systicks();
		if (i == 5) 
		{
			i2c_disable_ack(dev->device);
		}

		while ( !(I2C_SR1(dev->device) & I2C_SR1_RxNE) ) 
		{
		if ( diff_ticks(t0,systicks()) > dev->timeout )
			longjmp(i2c_exception,I2C_Read_Timeout);
		taskYIELD();
		}
	data[i] = i2c_get_data(dev->device);
	}
	i2c_send_stop(dev->device);

// calculation
rawValueTemp = (uint16_t) ((data[0]<<8)| data[1]);
rawValueHumi = (uint16_t) ((data[0]<<8)| data[1]);
//
*temperature = SHT3X_CalcTemperature(rawValueTemp);
*humidity = SHT3X_CalcHumidity(rawValueHumi);

}

void SHT3X_GetTempAndHumiPolling(I2C_Control *dev, uint8_t addr, float *temperature, float *humidity, etRepeatability repeatability)
{

}

