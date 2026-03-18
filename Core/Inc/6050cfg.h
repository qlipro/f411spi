/*
 * 6050cfg.h
 *
 *  Created on: Feb 20, 2026
 *      Author: jt
 */

#ifndef INC_6050CFG_H_
#define INC_6050CFG_H_

/* MPU6050 I2C地址 */
#define MPU6050_ADDR         0x68  // AD0接地时
// #define MPU6050_ADDR       0x69  // AD0接VCC时

/* MPU6050寄存器地址 */
#define MPU6050_ACCEL_XOUT_H  0x3B
#define MPU6050_PWR_MGMT_1    0x6B
#define MPU6050_WHO_AM_I      0x75



/* 固定使用最大量程的灵敏度 */
#define ACCEL_SENSITIVITY     2048.0f  // ±16g
#define GYRO_SENSITIVITY      16.4f    // ±2000 °/s

/* 预计算的常数 - 减少运行时计算 */
#define RAD_TO_DEG             57.29578f      // 180/π
#define GYRO_INTEGRAL_SCALE    (2000.0f / 32768.0f * 0.01f)     // 2000/32768 * 0.1
#define COMPLEMENTARY_ALPHA_A  0.8f
#define COMPLEMENTARY_ALPHA_G  0.2f

/* 字符串缓冲区大小 */
#define MPU6050_ANGLE_STR_SIZE     16   // 只够显示角度值

#endif /* INC_6050CFG_H_ */
