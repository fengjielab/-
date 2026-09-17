/*
 * motor.h
 *
 *  Created on: 2026年8月24日
 *      Author: Lenovo
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include <stdint.h>
/* 速度换算参数 */
#define MOTOR_ENCODER_CPR              26400.0f
#define MOTOR_WHEEL_DIAMETER_M         0.095f
#define MOTOR_WHEEL_CIRCUMFERENCE_M    (3.14159265359f * MOTOR_WHEEL_DIAMETER_M)

/* 电机控制任务周期 */
#define MOTOR_CONTROL_PERIOD_MS        10U

/* ==============================
 * 电机模块初始化
 * ============================== */
void Motor_Init(void);


/* ==============================
 * 完成一次电机闭环控制
 *
 * 读取编码器
 * → PID
 * → 更新PWM
 * ============================== */
void Motor_ControlStep(void);


/* ==============================
 * 修改单个电机的目标轮速，单位：m/s
 * ============================== */
void Motor_SetTarget(uint8_t motor, float target);

/* 设置目标轮速，单位：m/s */
void Motor_SetTargetMps(uint8_t motor, float target_mps);

/* 获取实际轮速，单位：m/s */
float Motor_GetSpeedMps(uint8_t motor);
/* 获取目标轮速，单位：m/s */

float Motor_GetTarget(uint8_t motor);

/* 获取某个电机本周期编码器增量 */
int16_t Motor_GetCount(uint8_t motor);


/* 获取某个电机当前PWM */
uint16_t Motor_GetPWM(uint8_t motor);


/* 停止所有电机 */
void Motor_StopAll(void);


#endif /* INC_MOTOR_H_ */
