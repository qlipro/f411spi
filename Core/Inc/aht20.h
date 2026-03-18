#ifndef __DHT20_H__
#define __DHT20_H__

#include "i2c.h"
#include "main.h"

// 初始化AHT20
HAL_StatusTypeDef AHT20_Init(I2C_HandleTypeDef *i2c);

// 获取温度和湿度
void AHT20_Read(float *Temperature, float *Humidity);

#endif
