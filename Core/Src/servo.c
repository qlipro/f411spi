/*
 * servo.c
 *
 *  Created on: Feb 21, 2026
 *      Author: jt
 */

#include "servo.h"
#include "tim.h"
#include <math.h>

// 预计算PWM映射表（减少运行时计算）
// -90° -> 50 (0.5ms)
//  0° -> 150 (1.5ms)
// 90° -> 250 (2.5ms)
#define PWM_MIN 50
#define PWM_MID 150
#define PWM_MAX 250
#define ANGLE_CHANGE_THRESHOLD 5.0f  // 角度变化阈值（10度）
#define FILTER_ALPHA_MIN 0.1f          // 最小滤波系数（最平滑）
#define FILTER_ALPHA_MAX 0.8f          // 最大滤波系数（最快逼近）

// 查表相关定义
#define ERROR_TABLE_SIZE 51             // 0-5度，每0.1度一个点
#define ERROR_MAX 5.0f                  // 最大误差5度
#define ERROR_STEP (ERROR_MAX / (ERROR_TABLE_SIZE - 1))  // 0.1度


// 为每个舵机创建独立的结构体实例（在.c文件中定义）
static ServoControl_t servo1_ctrl = {
    .target_angle = 0.0f,
    .current_angle = 0.0f,
    .max_step = 3.0f,
    .last_update = 0
};

static ServoControl_t servo2_ctrl = {
    .target_angle = 0.0f,
    .current_angle = 0.0f,
    .max_step = 3.0f,
    .last_update = 0
};

// 预计算的滤波系数表（避免运行时计算expf）
static float alpha_table[ERROR_TABLE_SIZE];

// 快速映射函数 - 使用整数运算代替浮点
static inline uint32_t AngleToPWM(float angle)
{
    // 限制角度范围
    if (angle < -90.0f) angle = -90.0f;
    if (angle > 90.0f) angle = 90.0f;

    // 线性映射：使用整数运算提高速度
    // PWM = (angle + 90) * (200/180) + 50
    // 200/180 = 1.6667，用整数近似：10/9 = 1.1111
    int32_t pwm = (uint32_t)(angle * 1.1111f + 150.0f);

    return (uint32_t)pwm;
}

/**
 * @brief 初始化滤波系数表（在系统启动时调用一次）
 */
void Servo_InitAlphaTable(void)
{
    for (int i = 0; i < ERROR_TABLE_SIZE; i++) {
        float error = i * ERROR_STEP;  // 0, 0.1, 0.2, ... 5.0

        // 使用简化公式计算 alpha = 1 - exp(-error/2)
        // 对于小误差，可以用近似公式
        if (error < 0.1f) {
            alpha_table[i] = error / 2.0f;  // 一阶泰勒展开
        } else {
            alpha_table[i] = 1.0f - expf(-error / 2.0f);
        }

        // 限制范围
        if (alpha_table[i] > FILTER_ALPHA_MAX)
            alpha_table[i] = FILTER_ALPHA_MAX;
        if (alpha_table[i] < FILTER_ALPHA_MIN)
            alpha_table[i] = FILTER_ALPHA_MIN;
    }
}

/**
 * @brief 从查表获取滤波系数
 */
static inline float GetAlphaFromTable(float error)
{
    // 限制误差范围
    if (error <= 0.0f) return FILTER_ALPHA_MIN;
    if (error >= ERROR_MAX) return FILTER_ALPHA_MAX;

    // 计算表索引
    float index_f = error / ERROR_STEP;
    int index = (int)index_f;

    // 线性插值，使过渡更平滑
    float frac = index_f - index;

    if (index >= ERROR_TABLE_SIZE - 1) {
        return alpha_table[ERROR_TABLE_SIZE - 1];
    }

    // 线性插值
    return alpha_table[index] * (1.0f - frac) + alpha_table[index + 1] * frac;
}

// 设置舵机角度
void MPU6050_SetServoAngle(servo_id id,float angle)
{
	 // 直接计算PWM值
	    uint32_t compare = AngleToPWM(angle);

	    // 直接设置比较值（最快的方式）
	    __IO uint32_t* ccr_registers[] = {
	    		&(TIM4->CCR3),//servo1,索引0
				&(TIM4->CCR4),//servo2,索引1
	    };
	    *ccr_registers[id] = compare;
}

// 平滑设置舵机角度
void MPU6050_SetServoAngleSmooth(servo_id id,float target_angle)
{
	// 获取对应舵机的控制结构体
	    ServoControl_t *servo;
	    if (id == servo1) {
	        servo = &servo1_ctrl;
	    } else {
	        servo = &servo2_ctrl;
	    }

	// 更新目标角度
		servo->target_angle = target_angle;
    // 限制目标角度范围
		if (servo->target_angle < -90.0f) servo->target_angle = -90.0f;
		if (servo->target_angle > 90.0f) servo->target_angle = 90.0f;
    uint32_t now = HAL_GetTick();
    float dt = (now - servo->last_update);


if (dt > 8.0f) {  // 至少8ms更新一次
        // 计算角度差
        float angle_diff = servo->target_angle - servo->current_angle;

    // 3. 死区设置（忽略微小变化）
    #define DEAD_ZONE 0.2f  // 0.2°以内的变化忽略

    if (fabs(angle_diff) < DEAD_ZONE) {
        // 在死区内，不更新舵机位置
        servo->last_update = now;
        return;
    }
        // 限制最大变化率
        float max_change = servo->max_step * (dt / 10.0f);  // 按时间缩放
        if (angle_diff > max_change) {
            angle_diff = max_change;
        } else if (angle_diff < -max_change) {
            angle_diff = -max_change;
        }

        // 更新当前角度
        servo->current_angle += angle_diff;

        // 设置舵机

        uint32_t compare_value =  AngleToPWM(servo->current_angle);

        // 设置对应舵机的比较值
		__IO uint32_t* ccr_registers[] = {
		   &(TIM4->CCR3),  // servo1, 索引0
		   &(TIM4->CCR4),  // servo2, 索引1
		};
		*ccr_registers[id] = compare_value;

        servo->last_update = now;
    }
}

/* 根据变化值，判断舵机使用直接设置角度还是平滑设置角度
 * 角度变化大时，希望它可以直接设置为指定角度；变化小时，希望它能稳定平滑变化角度
 * 通过一阶低通滤波来实现角度变化判断，防止角度变化速度变化时，出现角度回弹的噪声
 * 一阶滤波是将直接更新的角度值与在一定范围内变化的角度值以一定权重分配再相加，作为实际的更新值
 * 根据角度变化值大小动态改变两者的权重，可以更快地实现状态之间的转换
 */
void MPU6050_SetServoAngleAuto(servo_id id, float target_angle)
{


    static float last_target_angle1 = 0.0f;
    static float last_target_angle2 = 0.0f;

    static float filtered_angle1 = 0.0f;
	static float filtered_angle2 = 0.0f;
	static uint32_t last_big_change_time1 = 0;
	static uint32_t last_big_change_time2 = 0;

    // 确保表已初始化（可以在main中调用Servo_InitAlphaTable）
    static uint8_t table_initialized = 0;
    if (!table_initialized) {
        Servo_InitAlphaTable();
        table_initialized = 1;
    }

	float *filtered = (id == servo1) ? &filtered_angle1 : &filtered_angle2;
	float *last_target = (id == servo1) ? &last_target_angle1 : &last_target_angle2;
	uint32_t *last_big_time = (id == servo1) ? &last_big_change_time1 : &last_big_change_time2;

	uint32_t now = HAL_GetTick();

	// 1. 计算当前误差
	    float error = fabs(target_angle - *filtered);

	// 动态计算滤波系数（误差越大，系数越大，逼近越快）
		float dynamic_alpha=
//			FILTER_ALPHA_MIN + (FILTER_ALPHA_MAX - FILTER_ALPHA_MIN) * (error - 0.5f) / (5.0f - 0.5f)
//			1.0f - expf(-error / 2.0f)
			GetAlphaFromTable(error)
							;
		if (dynamic_alpha > FILTER_ALPHA_MAX) {
			// 大误差：快速逼近
			dynamic_alpha = FILTER_ALPHA_MAX;
		}
		if (dynamic_alpha < FILTER_ALPHA_MIN) {
			// 小误差：平滑滤波
			dynamic_alpha = FILTER_ALPHA_MIN;
		}

//	// 1. 一阶低通滤波（减少角度噪声）
//	    if (*last_target == 0.0f && servo1_ctrl.last_update == 0) {
//	        // 首次运行，直接使用目标值
//	        *filtered = target_angle;
//	    } else {
//	        // 正常滤波
//	        *filtered = FILTER_ALPHA * target_angle + (1 - FILTER_ALPHA) * (*filtered);
//	    }

	// 一阶低通滤波（使用动态系数）
		if (*last_target == 0.0f && now < 100) {
			// 首次运行，直接使用目标值
			*filtered = target_angle;
		} else {
			// 动态滤波
			*filtered = dynamic_alpha * target_angle + (1 - dynamic_alpha) * (*filtered);
		}

			// 计算角度变化量
//    float angle_change = fabs(target_angle - *last_target);
	    float angle_change = fabs(*filtered - *last_target);

	    // 获取舵机控制结构体
	        ServoControl_t *servo = (id == servo1) ? &servo1_ctrl : &servo2_ctrl;

		// 4. 判断是否大角度变化
    if (angle_change > ANGLE_CHANGE_THRESHOLD) {
		// 大角度变化：记录时间，直接控制

//        MPU6050_SetServoAngle(id, target_angle);
//
//        // 同步更新平滑控制器的当前角度
//        servo->current_angle = target_angle;
//        servo->target_angle = target_angle;

		*last_big_time = now;

		// 直接设置角度（快速响应）
		MPU6050_SetServoAngle(id, *filtered);

		servo->current_angle = *filtered;
		servo->target_angle = *filtered;
		*last_target = *filtered;
		servo->last_update = now;
		return;
    	}

    // 大变化后的200ms内也使用直接控制（保持连贯性）
	if (now - *last_big_time < 200) {
		// 调用直接控制函数
		MPU6050_SetServoAngle(id, *filtered);

		// 同步更新舵机控制结构体
		ServoControl_t *servo = (id == servo1) ? &servo1_ctrl : &servo2_ctrl;
		servo->current_angle = *filtered;
		servo->target_angle = *filtered;

		*last_target = *filtered;
		servo->last_update = now;
		return;
		}

//    } else {

        // 变化小于10度：平滑控制（减少抖动）

//        MPU6050_SetServoAngleSmooth(id, target_angle);
	MPU6050_SetServoAngleSmooth(id, *filtered);

//    }

    // 更新上次目标角度
//    *last_target = target_angle;
    *last_target = *filtered;
}
