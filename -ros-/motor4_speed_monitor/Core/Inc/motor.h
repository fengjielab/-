/*
 * motor.h
 *
 *  Created on: 2026年8月24日
 *      Author: Lenovo
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include <stdint.h>


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
 * 修改四个电机的目标count
 * ============================== */
void Motor_SetTarget(uint8_t motor, float target);


/* 获取目标count */

float Motor_GetTarget(uint8_t motor);

/* 获取某个电机本周期count */
int16_t Motor_GetCount(uint8_t motor);


/* 获取某个电机当前PWM */
uint16_t Motor_GetPWM(uint8_t motor);


/* 停止所有电机 */
void Motor_StopAll(void);


#endif /* INC_MOTOR_H_ */
