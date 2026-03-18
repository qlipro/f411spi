/*
 * 6050.h
 *
 *  Created on: Jan 24, 2026
 *      Author: jt
 */

#ifndef INC_6050_H_
#define INC_6050_H_

#include "main.h"
//#include "6050cfg.h"

/* --函数声明-- */


HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MPU6050_StartDMARead(void);
void MPU6050_ProcessData(void);

/* 数据获取 */
float MPU6050_GetAngle1(void);
float MPU6050_GetAngle2(void);

/* 格式化输出 - 简单函数，直接返回角度字符串 */
char* MPU6050_FormatAngle(char *buffer, uint32_t size);

/* 状态查询 */
uint8_t MPU6050_IsDataReady(void);
//void MPU6050_ResetDataReady(void);

/*------------------------------------*/

/* 外部变量声明 */
extern volatile uint8_t mpu6050_data_ready_flag;
extern volatile uint8_t mpu_dma_busy;


#endif /* INC_6050_H_ */
