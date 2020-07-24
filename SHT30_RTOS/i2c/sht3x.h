#ifndef SHT3X_H
#define SHT3X_H

//-- Includes -----------------------------------------------------------------
#include "i2c.h"
//-- Enumerations -------------------------------------------------------------
// Sensor Commands
typedef enum{
  CMD_READ_SERIALNBR  = 0x3780, // read serial number
  CMD_READ_STATUS     = 0xF32D, // read status register
  CMD_CLEAR_STATUS    = 0x3041, // clear status register
  CMD_HEATER_ENABLE   = 0x306D, // enabled heater
  CMD_HEATER_DISABLE  = 0x3066, // disable heater
  CMD_SOFT_RESET      = 0x30A2, // soft reset
  CMD_MEAS_CLOCKSTR_H = 0x2C06, // measurement: clock stretching, high repeatability
  CMD_MEAS_CLOCKSTR_M = 0x2C0D, // measurement: clock stretching, medium repeatability
  CMD_MEAS_CLOCKSTR_L = 0x2C10, // measurement: clock stretching, low repeatability
  CMD_MEAS_POLLING_H  = 0x2400, // measurement: polling, high repeatability
  CMD_MEAS_POLLING_M  = 0x240B, // measurement: polling, medium repeatability
  CMD_MEAS_POLLING_L  = 0x2416, // measurement: polling, low repeatability
  CMD_MEAS_PERI_05_H  = 0x2032, // measurement: periodic 0.5 mps, high repeatability
  CMD_MEAS_PERI_05_M  = 0x2024, // measurement: periodic 0.5 mps, medium repeatability
  CMD_MEAS_PERI_05_L  = 0x202F, // measurement: periodic 0.5 mps, low repeatability
  CMD_MEAS_PERI_1_H   = 0x2130, // measurement: periodic 1 mps, high repeatability
  CMD_MEAS_PERI_1_M   = 0x2126, // measurement: periodic 1 mps, medium repeatability
  CMD_MEAS_PERI_1_L   = 0x212D, // measurement: periodic 1 mps, low repeatability
  CMD_MEAS_PERI_2_H   = 0x2236, // measurement: periodic 2 mps, high repeatability
  CMD_MEAS_PERI_2_M   = 0x2220, // measurement: periodic 2 mps, medium repeatability
  CMD_MEAS_PERI_2_L   = 0x222B, // measurement: periodic 2 mps, low repeatability
  CMD_MEAS_PERI_4_H   = 0x2334, // measurement: periodic 4 mps, high repeatability
  CMD_MEAS_PERI_4_M   = 0x2322, // measurement: periodic 4 mps, medium repeatability
  CMD_MEAS_PERI_4_L   = 0x2329, // measurement: periodic 4 mps, low repeatability
  CMD_MEAS_PERI_10_H  = 0x2737, // measurement: periodic 10 mps, high repeatability
  CMD_MEAS_PERI_10_M  = 0x2721, // measurement: periodic 10 mps, medium repeatability
  CMD_MEAS_PERI_10_L  = 0x272A, // measurement: periodic 10 mps, low repeatability
  CMD_FETCH_DATA      = 0xE000, // readout measurements for periodic mode
  CMD_R_AL_LIM_LS     = 0xE102, // read alert limits, low set
  CMD_R_AL_LIM_LC     = 0xE109, // read alert limits, low clear
  CMD_R_AL_LIM_HS     = 0xE11F, // read alert limits, high set
  CMD_R_AL_LIM_HC     = 0xE114, // read alert limits, high clear
  CMD_W_AL_LIM_HS     = 0x611D, // write alert limits, high set
  CMD_W_AL_LIM_HC     = 0x6116, // write alert limits, high clear
  CMD_W_AL_LIM_LC     = 0x610B, // write alert limits, low clear
  CMD_W_AL_LIM_LS     = 0x6100, // write alert limits, low set
  CMD_NO_SLEEP        = 0x303E,
}etCommands;

// Measurement Repeatability
typedef enum{
  REPEATAB_HIGH,   // high repeatability
  REPEATAB_MEDIUM, // medium repeatability
  REPEATAB_LOW,    // low repeatability
}etRepeatability;

// Measurement Mode
typedef enum{
  MODE_CLKSTRETCH, // clock stretching
  MODE_POLLING,    // polling
}etMode;

typedef enum{
  FREQUENCY_HZ5,  //  0.5 measurements per seconds
  FREQUENCY_1HZ,  //  1.0 measurements per seconds
  FREQUENCY_2HZ,  //  2.0 measurements per seconds
  FREQUENCY_4HZ,  //  4.0 measurements per seconds
  FREQUENCY_10HZ, // 10.0 measurements per seconds
}etFrequency;

//-- Typedefs -----------------------------------------------------------------
// Status-Register
typedef union {
  uint16_t u16;
  struct{
    #ifdef LITTLE_ENDIAN  // bit-order is little endian
    uint16_t  CrcStatus     : 1; // write data checksum status
    uint16_t  CmdStatus     : 1; // command status
    uint16_t  Reserve0      : 2; // reserved
    uint16_t  ResetDetected : 1; // system reset detected
    uint16_t  Reserve1      : 5; // reserved
    uint16_t  T_Alert       : 1; // temperature tracking alert
    uint16_t  RH_Alert      : 1; // humidity tracking alert
    uint16_t  Reserve2      : 1; // reserved
    uint16_t  HeaterStatus  : 1; // heater status
    uint16_t  Reserve3      : 1; // reserved
    uint16_t  AlertPending  : 1; // alert pending status 
    #else                 // bit-order is big endian
    uint16_t  AlertPending  : 1;
    uint16_t  Reserve3      : 1;
    uint16_t  HeaterStatus  : 1;
    uint16_t  Reserve2      : 1;
    uint16_t  RH_Alert      : 1;
    uint16_t  T_Alert       : 1;
    uint16_t  Reserve1      : 5;
    uint16_t  ResetDetected : 1;
    uint16_t  Reserve0      : 2;
    uint16_t  CmdStatus     : 1;
    uint16_t  CrcStatus     : 1;
    #endif
  }bit;
} regStatus;


void SHT3X_Init(I2C_Control *dev, uint8_t addresses, uint32_t i2c, uint32_t ticks);

void i2c_write_16b(I2C_Control *dev, etCommands command);

void SHT3X_transfer(I2C_Control *dev, etCommands command, uint8_t *data, uint8_t *checksum);

void SHT3X_transfer_Polling(I2C_Control *dev, etCommands command, uint8_t *data, uint8_t *checksum); // polling 1000ms

uint32_t SHT3x_ReadSerialNumber(I2C_Control *dev);

void SHT3X_SoftReset(I2C_Control *dev);

void SHT3X_CalcCrc(uint8_t data[], uint8_t checksum[], bool *result);
// result[0] = true , CRC = checksum[0] of data[0], data[1]
// result[1] = true , CRC = checksum[1] of data[2], data[3]
bool SHT3X_ReadStatus(I2C_Control *dev, uint16_t *status);

void  SHT3X_ClearAllAlertFlags(I2C_Control *dev);

bool SHT3X_GetTempAndHumi(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability, etMode mode);
// return true : get data Tem and Humi -->True
// return false: get data Tem or Humi -->False
bool SHT3X_GetTempAndHumiClkStretch(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability);
// return true : get data Tem and Humi -->True
// return false: get data Tem or Humi -->False
bool SHT3X_GetTempAndHumiPolling(I2C_Control *dev, float *temperature, float *humidity, etRepeatability repeatability); // polling 1000ms
// return true : get data Tem and Humi -->True
// return false: get data Tem or Humi -->False
#endif // SHT3X_H