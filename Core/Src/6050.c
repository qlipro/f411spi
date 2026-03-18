/*
 * 6050.c
 *
 *  Created on: Jan 24, 2026
 *      Author: jt
 */

#include "6050.h"
#include "6050cfg.h"
#include <math.h>
#include <stdio.h>
#include "i2c.h"

static I2C_HandleTypeDef *mpu6050_hi2c = NULL;
static uint8_t rx_buffer[14] = {0};           // DMA接收缓冲区
static float current_angle1 = 0.0f;
static float current_angle2 = 0.0f;
static float last_angle1 = 0.0f;
static float last_angle2 = 0.0f;
volatile uint8_t mpu6050_data_ready_flag = 0;  // 数据就绪标志
volatile uint8_t mpu_dma_busy = 0;     // DMA忙标志


static HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(mpu6050_hi2c, MPU6050_ADDR << 1, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}



static HAL_StatusTypeDef MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
    return HAL_I2C_Mem_Read(mpu6050_hi2c, MPU6050_ADDR << 1, reg,
                            I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}

/**
 * @brief 初始化MPU6050 - 配置最大量程
 */
HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *i2c)
{
    uint8_t who_am_i = 0;
    uint8_t retry = 0;
    mpu6050_hi2c = i2c;

    if (HAL_I2C_IsDeviceReady(i2c, MPU6050_ADDR << 1, 2, 100) != HAL_OK)
      {
        return HAL_ERROR;  // 设备没响应，直接返回错误
      }

//    // 多次尝试读取WHO_AM_I
//	for(retry = 0; retry < 5; retry++) {
//	   MPU6050_ReadReg(0x75, &who_am_i);
//	   if(who_am_i == 0x68 || who_am_i == 0x71 || who_am_i == 0x73) {
//		   break;  // 6050或9250都接受
//	   }
//	   HAL_Delay(10);
//	}
//	if(retry >= 5) {
//	   return HAL_ERROR;  // 确实找不到设备
//	}

	// 如果是9250，记录一下（可选）
	if(who_am_i == 0x71 || who_am_i == 0x73) {
	   // 是MPU9250
	}

    // 唤醒设备
    MPU6050_WriteReg(0x6B, 0x00);        // PWR_MGMT_1
    HAL_Delay(10);

    // 配置采样率
    MPU6050_WriteReg(0x19, 0x09);        // SMPLRT_DIV

    // 配置滤波器
    MPU6050_WriteReg(0x1A, 0x03);        // CONFIG

    // 陀螺仪最大量程 ±2000 °/s
    MPU6050_WriteReg(0x1B, 0x18);        // GYRO_CONFIG

    // 加速度计最大量程 ±16g
    MPU6050_WriteReg(0x1C, 0x18);        // ACCEL_CONFIG

    MPU6050_WriteReg(0x37, 0x00);        // INT_PIN_CFG
       MPU6050_WriteReg(0x6A, 0x00);        // USER_CTRL

    return HAL_OK;
}


/**
 * @brief 启动DMA读取
 */
HAL_StatusTypeDef MPU6050_StartDMARead(void)
{
    if(mpu_dma_busy) {
        return HAL_BUSY;
    }

    mpu_dma_busy = 1;

    /* 从ACCEL_XOUT_H开始连续读取14个字节 */
    return HAL_I2C_Mem_Read_DMA(mpu6050_hi2c,
                                MPU6050_ADDR << 1,
                                MPU6050_ACCEL_XOUT_H,
                                I2C_MEMADD_SIZE_8BIT,
                                rx_buffer,
                                14);
}

void MPU6050_ProcessData(void)
{
    if (!mpu6050_data_ready_flag) return;

    // 解析原始数据
    int16_t accel_x = (rx_buffer[0] << 8) | rx_buffer[1];
    int16_t accel_y = (rx_buffer[2] << 8) | rx_buffer[3];
    int16_t accel_z = (rx_buffer[4] << 8) | rx_buffer[5];
    int16_t gyro_x  = (rx_buffer[8] << 8) | rx_buffer[9];
    int16_t gyro_y  = (rx_buffer[10] << 8) | rx_buffer[11];

    // 转换为物理量
    float ax = (float)accel_x / ACCEL_SENSITIVITY;
    float ay = (float)accel_y / ACCEL_SENSITIVITY;
    float az = (float)accel_z / ACCEL_SENSITIVITY;
    float gx = (float)gyro_x / GYRO_SENSITIVITY;
    float gy = (float)gyro_y / GYRO_SENSITIVITY;

    // 加速度计角度
    float angle_a1 = -atan2f(ax, az) * RAD_TO_DEG;
    float angle_a2 = -atan2f(ay, az) * RAD_TO_DEG;
    // 陀螺仪积分
    float angle_g1 = last_angle1 + gy * GYRO_INTEGRAL_SCALE;
    float angle_g2 = last_angle2 + gx * GYRO_INTEGRAL_SCALE;
    // 互补滤波
    current_angle1 = COMPLEMENTARY_ALPHA_A * angle_a1 +
                    COMPLEMENTARY_ALPHA_G * angle_g1;
    current_angle2 = COMPLEMENTARY_ALPHA_A * angle_a2 +
                        COMPLEMENTARY_ALPHA_G * angle_g2;
    last_angle1 = current_angle1;
    last_angle2 = current_angle2;
    // 清除标志
    mpu6050_data_ready_flag = 0;
    mpu_dma_busy = 0;
}

/**
 * @brief 格式化角度字符串 - 只在需要显示时调用
 */
char* MPU6050_FormatAngle(char *buffer, uint32_t size)
{
    if(buffer && size > 0) {
        /* 只格式化角度，不处理其他数据 */
        snprintf(buffer, size, "R %+03.2f P %+03.2f ", current_angle1,current_angle2);
    }
    return buffer;
}

/**
 * @brief 获取角度值
 */
float MPU6050_GetAngle1(void)
{
    return current_angle1;
}

float MPU6050_GetAngle2(void)
{
    return current_angle2;
}

/**
 * @brief 检查数据是否就绪
 */
uint8_t MPU6050_IsDataReady(void)
{
    return mpu6050_data_ready_flag;
}

///**
// * @brief 复位数据就绪标志
// */
//void MPU6050_ResetDataReady(void)
//{
//	mpu6050_data_ready_flag = 0;
//    dma_busy = 0;
//}

