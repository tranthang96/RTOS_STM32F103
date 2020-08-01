# RTOS_STM32F103
Template_stm32f103c8t6_freertos
# UART test sensor sht30

	MCU ----send--- > PC
	
# SHT 30 ----  I2C 
	
	use I2C1 stm32f103c8t6
	
	PB6 -- I2C_SCL
	PB7 -- I2C_SDA
	
    Remap value to default


# I2C initialization steps

Clock : 	
		enable RCC_I2C1
				GPIOB

Configure I2C :
 			- disable peripheral I2C1
 			- reset I2C1 
 			- Clear Stop : I2C_CR1(I2C) &= I2C_CR1_STOP
 			- i2c set standard mode (up to 100KHz)
 			- set clock: I2C_CR2_FREQ_36MHZ
 			- set trise 				// 1000 ns
 			- set dutycycle 			
 			- set ccr
 			- i2c_peripheral_enable (I2C1)

