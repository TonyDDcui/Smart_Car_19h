/*
 * Servo_CTRL.h
 *
 *  Created on: 2024Äê3ÔÂ24ÈÕ
 *      Author: CuiZheXiao
 */

#ifndef INCLUDE_SERVO_CTRL_H_
#define INCLUDE_SERVO_CTRL_H_

#include "main.h"

#define Delay_Time 5000

extern pwm_channel_enum channel_list[4];

void Init_Servo(void);
void Set_Pick_Up(void);
void Set_Put_Down(void);
void Set270_pwm(uint8 channel, float Angle_now, float Angle_next);

#endif /* INCLUDE_SERVO_CTRL_H_ */
