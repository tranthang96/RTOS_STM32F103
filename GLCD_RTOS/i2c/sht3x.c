#include "FreeRTOS.h"
#include "task.h"

#include "sht3x.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>

//-- Defines ------------------------------------------------------------------
// Generator polynomial for CRC
#define POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001



static inline TickType_t
diff_ticks(TickType_t early,TickType_t later) {

	if ( later >= early )
		return later - early;
	return ~(TickType_t)0 - early + 1 + later;
}

static inline float SHT3X_CalcTemperature(uint16_t rawValue)
{
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / (2^16-1)
  return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

//-----------------------------------------------------------------------------
static inline float SHT3X_CalcHumidity(uint16_t rawValue)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / (2^16-1) * 100
  return 100.0f * (float)rawValue / 65535.0f;
}

//============================================================================
void SHT3X_Init(I2C_Control *dev, uint8_t addresses, uint32_t i2c, uint32_t ticks)
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
	i2c_configure(dev,i2c,ticks,addresses);
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
void SHT3X_transfer(I2C_Control *dev, etCommands command, uint8_t *data, uint8_t *checksum)
{
	// write
	i2c_start_addr(dev, I2C_WRITE);
	i2c_write_16b(dev, command);
	
	//read

	i2c_start_addr(dev, I2C_READ);

	data[0] = i2c_read(dev, false);
	data[1] = i2c_read(dev, false);
	checksum[0] = i2c_read(dev, false);

	data[2] = i2c_read(dev, false);
	data[3] = i2c_read(dev, false);
	checksum[1] = i2c_read(dev, true);

	i2c_send_stop(dev->device);
}
void SHT3X_transfer_Polling(I2C_Control *dev, etCommands command, uint8_t *data, uint8_t *checksum)
{
	// write
	i2c_start_addr(dev, I2C_WRITE);
	i2c_write_16b(dev, command);
	
	//read
	vTaskDelay(pdMS_TO_TICKS(1000));

	i2c_start_addr(dev, I2C_READ);

	data[0] = i2c_read(dev, false);
	data[1] = i2c_read(dev, false);
	checksum[0] = i2c_read(dev, false);

	data[2] = i2c_read(dev, false);
	data[3] = i2c_read(dev, false);
	checksum[1] = i2c_read(dev, true);

	i2c_send_stop(dev->device);
}

uint32_t SHT3x_ReadSerialNumber(I2C_Control *dev)
{	
	uint8_t byte[4];
	uint8_t checksum[1];

	SHT3X_transfer(dev, CMD_READ_SERIALNBR, byte , checksum);
	return (uint32_t) ( byte[0] << 24 | byte[1] << 16 | byte[2] << 8 | byte[3] );
}
void SHT3X_SoftReset(I2C_Control *dev)
{
	i2c_start_addr(dev, I2C_WRITE);
	i2c_write_16b(dev, CMD_SOFT_RESET);

}
void SHT3X_CalcCrc(uint8_t *data, uint8_t *checksum, bool *result)
{
	uint8_t bit;        // bit mask
	uint8_t crc = 0xFF; // calculated checksum
	uint8_t byteCtr;    // byte counter
// Check data 1

	for(byteCtr = 0; byteCtr < 2; byteCtr++)
  	{
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    	{
      	if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      	else           crc = (crc << 1);
    	}
  	}
  	if(crc != checksum[0]) result[0] = false;
  		else               result[0] = true;

// Check data 2
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
  	if(crc != checksum[1]) result[1] = false;
  		else               result[1] = true;
}

bool SHT3X_ReadStatus(I2C_Control *dev, uint16_t *status)
{
	uint8_t data[1];
	uint8_t checksum[0];
	uint8_t bit;        // bit mask
	uint8_t crc = 0xFF; // calculated checksum
	uint8_t byteCtr;    // byte counter

	i2c_start_addr(dev, I2C_WRITE);
	i2c_write_16b(dev, CMD_READ_STATUS);
	
	//read
	i2c_start_addr(dev, I2C_READ);
	data[0] = i2c_read(dev, false);
	data[1] = i2c_read(dev, false);
	checksum[0] = i2c_read(dev, true);
	i2c_send_stop(dev->device);

	*status = (uint16_t) ( data[0] << 8 | data[1] );
	// check crc
	for(byteCtr = 0; byteCtr < 2; byteCtr++)
  	{
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    	{
      	if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      	else           crc = (crc << 1);
    	}
  	}
  	if(crc != checksum[0]) return false;
  		else               return true;
}

void  SHT3X_ClearAllAlertFlags(I2C_Control *dev)
{
	i2c_start_addr(dev, I2C_WRITE);
	i2c_write_16b(dev, CMD_CLEAR_STATUS);
}

bool SHT3X_GetTempAndHumi(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability, etMode mode)
{
	
	switch(mode)
	{
		case MODE_CLKSTRETCH: // get temperature with clock stretching mode
			return SHT3X_GetTempAndHumiClkStretch(dev, temperature, humidity, repeatability);
			break;
		case MODE_POLLING:
			return SHT3X_GetTempAndHumiPolling(dev, temperature, humidity, repeatability);
			break;
		default:
			return false;
	}
}
bool SHT3X_GetTempAndHumiClkStretch(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability)
{
	uint8_t data[4];
	uint8_t checksum[1];
	uint16_t rawValueTem, rawValueHumi;
	bool result[2];

	switch(repeatability)
	{
		case REPEATAB_LOW:
			SHT3X_transfer(dev, CMD_MEAS_CLOCKSTR_L, data , checksum);
			break;
		case REPEATAB_MEDIUM:
			SHT3X_transfer(dev, CMD_MEAS_CLOCKSTR_M, data , checksum);
			break;
		case REPEATAB_HIGH:
			SHT3X_transfer(dev, CMD_MEAS_CLOCKSTR_H, data , checksum);
			break;
		default:
        	return false;
        	break;
	}

	SHT3X_CalcCrc(data, checksum, result);

	if (result[0] & result[1])
	{
		rawValueTem = (uint16_t)(data[0]<<8 | data[1]);
		rawValueHumi = (uint16_t)(data[2]<<8 | data[3]);

		*temperature = SHT3X_CalcTemperature(rawValueTem);
		*humidity = SHT3X_CalcHumidity(rawValueHumi);
		return true;
	}
	else
		return false;
}

bool SHT3X_GetTempAndHumiPolling(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability)
{

	uint8_t data[4];
	uint8_t checksum[1];
	uint16_t rawValueTem, rawValueHumi;
	bool result[2];

	switch(repeatability)
	{
		case REPEATAB_LOW:
			SHT3X_transfer_Polling(dev, CMD_MEAS_POLLING_L, data , checksum);
			break;
		case REPEATAB_MEDIUM:
			SHT3X_transfer_Polling(dev, CMD_MEAS_POLLING_M, data , checksum);
			break;
		case REPEATAB_HIGH:
			SHT3X_transfer_Polling(dev, CMD_MEAS_POLLING_H, data , checksum);
			break;
		default:
        	return false;
        	break;
	}

	SHT3X_CalcCrc(data, checksum, result);

	if (result[0] & result[1])
	{
		rawValueTem = (uint16_t)(data[0]<<8 | data[1]);
		rawValueHumi = (uint16_t)(data[2]<<8 | data[3]);

		*temperature = SHT3X_CalcTemperature(rawValueTem);
		*humidity = SHT3X_CalcHumidity(rawValueHumi);
		return true;
	}
	else
		return false;
}
