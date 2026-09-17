/*
 * motor.c
 *
 *  Created on: 2026年8月24日
 *      Author: Lenovo
 */


#include "motor.h"

#include "main.h"
#include "tim.h"
#include "gpio.h"

/* =========================================================
 * 电机控制参数
 * ========================================================= */

#define CONTROL_DT     0.1f

/* Conversion from one count during the nominal 100 ms control window to m/s. */
#define MOTOR_MPS_PER_CONTROL_COUNT \
    (MOTOR_WHEEL_CIRCUMFERENCE_M / (MOTOR_ENCODER_CPR * CONTROL_DT))

#define BASE_PWM       300

#define PWM_MAX        700

#define M1_ENCODER_SIGN   -1
#define M2_ENCODER_SIGN    1
#define M3_ENCODER_SIGN   -1
#define M4_ENCODER_SIGN    1

/* ==============================
 * 电机控制调试变量
 * ============================== */

/* Motor_ControlStep() 执行次数 */
volatile uint32_t debug_motor_step_count = 0;

/* 当前四个电机目标值 */
volatile float debug_motor_target[4] =
{
    0, 0, 0, 0
};

/* 当前四个编码器计数 */
volatile int16_t debug_motor_count[4] =
{
    0, 0, 0, 0
};

/* 当前四个 PWM 值 */
volatile uint16_t debug_motor_pwm[4] =
{
    0, 0, 0, 0
};

/* Current measured wheel speed, unit: m/s */
volatile float debug_motor_speed_mps[4] =
{
    0.0f, 0.0f, 0.0f, 0.0f
};

/* Current encoder speed, unit: count/s */
volatile float debug_motor_speed_cps[4] =
{
    0.0f, 0.0f, 0.0f, 0.0f
};

/* Actual time between two samples, unit: ms */
volatile uint32_t debug_motor_dt_ms = 0;


/*
 * 这里填写你现在已经调通的目标值
 *
 * 我暂时写290作为示例。
 */
static float motor_target_mps[4] =
{
    0, 0, 0, 0
};

typedef struct
{
    float kp;
    float ki;
    float kd;

    float integral;
    float last_error;

} PID_t;

static PID_t motor_pid[4] =
{
    {6.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     1.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.01f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.0f, 0.0f},
    {6.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     1.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.01f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.0f, 0.0f},
    {6.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     1.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.01f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.0f, 0.0f},
    {6.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     1.0f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.01f / MOTOR_MPS_PER_CONTROL_COUNT,
     0.0f, 0.0f}
};

static int16_t motor_count[4] =
{
    0, 0, 0, 0
};

/* Raw encoder counter from the previous sample */
static uint16_t motor_previous_counter[4] =
{
    0, 0, 0, 0
};

/* Current measured wheel speed, unit: m/s */
static float motor_speed_mps[4] =
{
    0.0f, 0.0f, 0.0f, 0.0f
};

/* System time of the previous sample */
static uint32_t motor_last_sample_tick = 0;


static uint16_t motor_pwm[4] =
{
    BASE_PWM,
    BASE_PWM,
    BASE_PWM,
    BASE_PWM
};

static void Motor_SetPWM(uint8_t motor,
                         uint16_t pwm)
{
    if (pwm > PWM_MAX)
    {
        pwm = PWM_MAX;
    }


    switch (motor)
    {
        case 1:

            __HAL_TIM_SET_COMPARE(
                &htim8,
                TIM_CHANNEL_1,
                pwm);

            break;


        case 2:

            __HAL_TIM_SET_COMPARE(
                &htim8,
                TIM_CHANNEL_2,
                pwm);

            break;


        case 3:

            __HAL_TIM_SET_COMPARE(
                &htim8,
                TIM_CHANNEL_3,
                pwm);

            break;


        case 4:

            __HAL_TIM_SET_COMPARE(
                &htim5,
                TIM_CHANNEL_1,
                pwm);

            break;


        default:
            break;
    }
}

static void Motor_SetDirection(uint8_t motor,
                               int8_t direction)
{
    switch (motor)
    {
        case 1:

            if (direction > 0)
            {
                HAL_GPIO_WritePin(
                    M1_IN1_GPIO_Port,
                    M1_IN1_Pin,
                    GPIO_PIN_SET);

                HAL_GPIO_WritePin(
                    M1_IN2_GPIO_Port,
                    M1_IN2_Pin,
                    GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(
                    M1_IN1_GPIO_Port,
                    M1_IN1_Pin,
                    GPIO_PIN_RESET);

                HAL_GPIO_WritePin(
                    M1_IN2_GPIO_Port,
                    M1_IN2_Pin,
                    GPIO_PIN_SET);
            }

            break;


        case 2:

            if (direction > 0)
            {
                HAL_GPIO_WritePin(
                    M2_IN1_GPIO_Port,
                    M2_IN1_Pin,
                    GPIO_PIN_RESET);

                HAL_GPIO_WritePin(
                    M2_IN2_GPIO_Port,
                    M2_IN2_Pin,
                    GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(
                    M2_IN1_GPIO_Port,
                    M2_IN1_Pin,
                    GPIO_PIN_SET);

                HAL_GPIO_WritePin(
                    M2_IN2_GPIO_Port,
                    M2_IN2_Pin,
                    GPIO_PIN_RESET);
            }

            break;


        case 3:

            if (direction > 0)
            {
                HAL_GPIO_WritePin(
                    M3_IN1_GPIO_Port,
                    M3_IN1_Pin,
                    GPIO_PIN_SET);

                HAL_GPIO_WritePin(
                    M3_IN2_GPIO_Port,
                    M3_IN2_Pin,
                    GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(
                    M3_IN1_GPIO_Port,
                    M3_IN1_Pin,
                    GPIO_PIN_RESET);

                HAL_GPIO_WritePin(
                    M3_IN2_GPIO_Port,
                    M3_IN2_Pin,
                    GPIO_PIN_SET);
            }

            break;


        case 4:

            if (direction > 0)
            {
                HAL_GPIO_WritePin(
                    M4_IN1_GPIO_Port,
                    M4_IN1_Pin,
                    GPIO_PIN_RESET);

                HAL_GPIO_WritePin(
                    M4_IN2_GPIO_Port,
                    M4_IN2_Pin,
                    GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(
                    M4_IN1_GPIO_Port,
                    M4_IN1_Pin,
                    GPIO_PIN_SET);

                HAL_GPIO_WritePin(
                    M4_IN2_GPIO_Port,
                    M4_IN2_Pin,
                    GPIO_PIN_RESET);
            }

            break;


        default:
            break;
    }
}

static float PID_Update(PID_t *pid,
                        float target,
                        float actual)
{
    float error;
    float derivative;
    float correction;
    float output;


    /* 当前误差 */
    error = target - actual;


    /* I */
    pid->integral +=
        error * CONTROL_DT;


    if (pid->integral > 100.0f)
    {
        pid->integral = 100.0f;
    }

    if (pid->integral < -100.0f)
    {
        pid->integral = -100.0f;
    }


    /* D */
    derivative =
        (error - pid->last_error)
        /
        CONTROL_DT;


    /* PID输出修正量 */
    correction =
          pid->kp * error
        + pid->ki * pid->integral
        + pid->kd * derivative;


    pid->last_error = error;


    /* 基础PWM + PID修正 */
    output =
        BASE_PWM + correction;


    if (output > PWM_MAX)
    {
        output = PWM_MAX;
    }

    if (output < 0.0f)
    {
        output = 0.0f;
    }


    return output;
}

void Motor_Init(void)
{
    motor_previous_counter[0] = 0;
    motor_previous_counter[1] = 0;
    motor_previous_counter[2] = 0;
    motor_previous_counter[3] = 0;

    motor_speed_mps[0] = 0.0f;
    motor_speed_mps[1] = 0.0f;
    motor_speed_mps[2] = 0.0f;
    motor_speed_mps[3] = 0.0f;

    motor_last_sample_tick = HAL_GetTick();

    /* ==========================
     * 启动4个编码器
     * ========================== */

    HAL_TIM_Encoder_Start(
        &htim2,
        TIM_CHANNEL_ALL);

    HAL_TIM_Encoder_Start(
        &htim3,
        TIM_CHANNEL_ALL);

    HAL_TIM_Encoder_Start(
        &htim4,
        TIM_CHANNEL_ALL);

    HAL_TIM_Encoder_Start(
        &htim1,
        TIM_CHANNEL_ALL);

    /* Clear counters after encoder peripherals are started. */
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SET_COUNTER(&htim1, 0);


    /* 清零 */



    /* ==========================
     * 启动PWM
     * ========================== */

    HAL_TIM_PWM_Start(
        &htim8,
        TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(
        &htim8,
        TIM_CHANNEL_2);

    HAL_TIM_PWM_Start(
        &htim8,
        TIM_CHANNEL_3);

    HAL_TIM_PWM_Start(
        &htim5,
        TIM_CHANNEL_1);


    /* 设置方向 */

    Motor_SetDirection(1, 1);
    Motor_SetDirection(2, 1);
    Motor_SetDirection(3, 1);
    Motor_SetDirection(4, 1);


    /* 初始PWM */

    motor_pwm[0] = 0;
    motor_pwm[1] = 0;
    motor_pwm[2] = 0;
    motor_pwm[3] = 0;

    Motor_SetPWM(1, 0);
    Motor_SetPWM(2, 0);
    Motor_SetPWM(3, 0);
    Motor_SetPWM(4, 0);
}

void Motor_ControlStep(void)
{
    uint16_t current_counter[4];
    int16_t delta_counter[4];
    uint32_t current_tick;
    uint32_t elapsed_ms;
    float dt_s;

    debug_motor_step_count++;

    current_tick = HAL_GetTick();
    elapsed_ms = current_tick - motor_last_sample_tick;

    /* Do not calculate twice at the same system tick. */
    if (elapsed_ms == 0U)
    {
        return;
    }

    motor_last_sample_tick = current_tick;
    debug_motor_dt_ms = elapsed_ms;
    dt_s = (float)elapsed_ms / 1000.0f;


    /* ==========================
     * 读取四个编码器
     * ========================== */

    /* Read continuously running encoder counters. */
    current_counter[0] =
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim2);

    current_counter[1] =
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);

    current_counter[2] =
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

    current_counter[3] =
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);

    /* Current counter - previous counter; handles one 16-bit wrap. */
    for (int i = 0; i < 4; i++)
    {
        delta_counter[i] =
            (int16_t)(
                (uint16_t)(
                    current_counter[i]
                    - motor_previous_counter[i]
                )
            );

        motor_previous_counter[i] = current_counter[i];
    }


    /*
     * 当前阶段只比较速度大小
     */

    motor_count[0] = M1_ENCODER_SIGN * delta_counter[0];
    motor_count[1] = M2_ENCODER_SIGN * delta_counter[1];
    motor_count[2] = M3_ENCODER_SIGN * delta_counter[2];
    motor_count[3] = M4_ENCODER_SIGN * delta_counter[3];

    /* Convert encoder delta to count/s and then to m/s. */
    for (int i = 0; i < 4; i++)
    {
        debug_motor_count[i] = motor_count[i];

        debug_motor_speed_cps[i] =
            (float)motor_count[i] / dt_s;

        motor_speed_mps[i] =
            debug_motor_speed_cps[i]
            * MOTOR_WHEEL_CIRCUMFERENCE_M
            / MOTOR_ENCODER_CPR;

        debug_motor_speed_mps[i] = motor_speed_mps[i];
    }

    /* 保存调试观察数据 */
debug_motor_count[0] = motor_count[0];
debug_motor_count[1] = motor_count[1];
debug_motor_count[2] = motor_count[2];
debug_motor_count[3] = motor_count[3];

debug_motor_target[0] = motor_target_mps[0];
debug_motor_target[1] = motor_target_mps[1];
debug_motor_target[2] = motor_target_mps[2];
debug_motor_target[3] = motor_target_mps[3];

    /* ==========================
     * 为下一个100ms重新计数
     * ========================== */



    /* ==========================
     * 四路PID
     * ========================== */

    for (int i = 0; i < 4; i++)
    {
        float target;
        float target_abs;
        float actual_abs;


        target = motor_target_mps[i];


        /* =================================
         * 情况1：目标为0
         * → 停止电机
         * ================================= */

        if (target == 0.0f)
        {
            Motor_SetPWM(i + 1, 0);

            motor_pwm[i] = 0;
            debug_motor_pwm[i] = motor_pwm[i];

            /* 停止时清除PID历史 */
            motor_pid[i].integral = 0.0f;
            motor_pid[i].last_error = 0.0f;

            continue;
        }


        /* =================================
         * 情况2：根据目标正负决定方向
         * ================================= */

        if (target > 0.0f)
        {
            Motor_SetDirection(i + 1, 1);

            target_abs = target;
        }
        else
        {
            Motor_SetDirection(i + 1, -1);

            target_abs = -target;
        }


        /* =================================
         * PID只比较速度大小
         * ================================= */

        actual_abs = motor_speed_mps[i];

        if (actual_abs < 0.0f)
        {
            actual_abs = -actual_abs;
        }


        motor_pwm[i] =
            (uint16_t)PID_Update(
                &motor_pid[i],
                target_abs,
                actual_abs);


        Motor_SetPWM(
            i + 1,
            motor_pwm[i]);
            debug_motor_pwm[i] = motor_pwm[i];
    }
}

void Motor_SetTarget(uint8_t motor, float target)
{
    Motor_SetTargetMps(motor, target);
}

void Motor_SetTargetMps(uint8_t motor, float target_mps)
{
    if (motor < 1 || motor > 4)
    {
        return;
    }

    motor_target_mps[motor - 1] = target_mps;
}

float Motor_GetTarget(uint8_t motor)
{
    if (motor < 1 || motor > 4)
    {
        return 0.0f;
    }

    return motor_target_mps[motor - 1];
}

int16_t Motor_GetCount(uint8_t motor)
{
    if (motor < 1 || motor > 4)
    {
        return 0;
    }

    return motor_count[motor - 1];
}

float Motor_GetSpeedMps(uint8_t motor)
{
    if (motor < 1 || motor > 4)
    {
        return 0.0f;
    }

    return motor_speed_mps[motor - 1];
}

uint16_t Motor_GetPWM(uint8_t motor)
{
    if (motor < 1 || motor > 4)
    {
        return 0;
    }

    return motor_pwm[motor - 1];
}

void Motor_StopAll(void)
{
    for (int i = 0; i < 4; i++)
    {
        motor_target_mps[i] = 0.0f;

        motor_pwm[i] = 0;

        motor_pid[i].integral = 0.0f;
        motor_pid[i].last_error = 0.0f;

        Motor_SetPWM(i + 1, 0);
    }
}


