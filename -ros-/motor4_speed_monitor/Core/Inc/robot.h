/*
 * robot.h
 *
 *  Created on: 2026年8月28日
 *      Author: Lenovo
 */

#ifndef INC_ROBOT_H_
#define INC_ROBOT_H_
void Robot_Forward(float speed);
void Robot_Backward(float speed);
void Robot_Stop(void);
void Robot_Left(float speed);
void Robot_Right(float speed);

void Robot_TurnLeft(float speed);
void Robot_TurnRight(float speed);
void Robot_SetVelocity(float vx,
                       float vy,
                       float wz);

#endif /* INC_ROBOT_H_ */
