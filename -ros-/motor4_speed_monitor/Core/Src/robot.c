/*
 * robot.c
 *
 *  Created on: 2026年8月28日
 *      Author: Lenovo
 */


#include "robot.h"
#include "motor.h"

void Robot_SetVelocity(float vx,
                       float vy,
                       float wz)
{
    float m1;
    float m2;
    float m3;
    float m4;

    m1 = vx + vy - wz;
    m2 = vx - vy + wz;
    m3 = vx - vy - wz;
    m4 = vx + vy + wz;

    Motor_SetTarget(1, m1);
    Motor_SetTarget(2, m2);
    Motor_SetTarget(3, m3);
    Motor_SetTarget(4, m4);
}

void Robot_Forward(float speed)
{
    Robot_SetVelocity(speed, 0, 0);
}

void Robot_Backward(float speed)
{
    Robot_SetVelocity(-speed, 0, 0);
}

void Robot_Left(float speed)
{
    Robot_SetVelocity(0, speed, 0);
}

void Robot_Right(float speed)
{
    Robot_SetVelocity(0, -speed, 0);
}

void Robot_TurnLeft(float speed)
{
    Robot_SetVelocity(0, 0, speed);
}

void Robot_TurnRight(float speed)
{
    Robot_SetVelocity(0, 0, -speed);
}

void Robot_Stop(void)
{
    Motor_StopAll();
}
