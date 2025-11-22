/**
  ******************************************************************************
  * @file    sensor_io.c
  * @brief   Minimal sensor IO layer for I2C sensors.
  ******************************************************************************
  */
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

void SENSOR_IO_Init(void)
{
  /* I2C is already initialized in main; nothing else required */
}

void SENSOR_IO_DeInit(void)
{
  /* Not used */
}

void SENSOR_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  (void)HAL_I2C_Mem_Write(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, HAL_MAX_DELAY);
}

uint8_t SENSOR_IO_Read(uint8_t Addr, uint8_t Reg)
{
  uint8_t val = 0;
  (void)HAL_I2C_Mem_Read(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);
  return val;
}

uint16_t SENSOR_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
  if (HAL_I2C_Mem_Read(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, HAL_MAX_DELAY) != HAL_OK)
  {
    return 1;
  }
  return 0;
}

void SENSOR_IO_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
  (void)HAL_I2C_Mem_Write(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length, HAL_MAX_DELAY);
}
