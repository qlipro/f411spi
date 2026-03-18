/*
 * servo.h
 *
 *  Created on: Feb 21, 2026
 *      Author: jt
 */
#include "main.h"

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

// 舵机参数
#define SERVO_MIN_ANGLE    -90.0f
#define SERVO_MAX_ANGLE    90.0f

// PWM参数
#define SERVO_PWM_PERIOD   20000  // 20ms = 20000us
#define SERVO_PWM_MIN_US   500    // 0.5ms = 500us (-90°)
#define SERVO_PWM_MID_US   1500   // 1.5ms = 1500us (0°)
#define SERVO_PWM_MAX_US   2500   // 2.5ms = 2500us (90°)

typedef enum{
	servo1 = 0,
	servo2
}servo_id;

// 舵机控制结构体定义
typedef struct {
    float target_angle;     // 目标角度
    float current_angle;    // 当前角度
    float max_step;         // 最大步进角度（每10ms）
    uint32_t last_update;   // 上次更新时间
} ServoControl_t;


void MPU6050_SetServoAngle(servo_id id,float angle);
void MPU6050_SetServoAngleSmooth(servo_id id,float angle);
void MPU6050_SetServoAngleAuto(servo_id id, float target_angle);
void Servo_InitAlphaTable(void);

#endif /* INC_SERVO_H_ */
