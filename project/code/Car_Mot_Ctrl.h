/*
 * Car_Mot_Ctrl.h
 *
 *  Created on: 2024��3��14��
 *      Author: CuiZheXiao
 */

#ifndef INCLUDE_CAR_MOT_CTRL_H_
#define INCLUDE_CAR_MOT_CTRL_H_

#include "main.h"
#include "math.h"

#define D (uint8)0U
#define R (uint8)1U
#define V (uint8)20U

typedef struct
{
	//�����ز���
	float CAR_MOT_V;
	float CAR_MOT_CYC;
	float MOT_OUTPUT_PLUSE;
    uint8 Set_Dir;

	//�����ڲ���
	pwm_channel_enum MOTOR_CH;
	gpio_pin_enum MOTOR_DIR;
}CAR_PARAM;

typedef struct
{
    float Set_Value;  //��ʵֵ
    float Set_Value_Last;  //��һ����ʵֵ
    float Count_Value;  //�ۼ�ֵ
    float Target_Point;    //����ֵ
    float K_p;
    float K_i;
    float K_d;
    float Last_Error;
    float Last_2_Error;
    float Error_Last;    //��һ��error
}PID;

extern CAR_PARAM cars_mot[4];

extern PID spd_pids[4];
extern PID pos_pids[4];
extern PID compensate_pid[4];

extern float Distance;
extern uint8 Dir;
extern float Speed[4];

extern bool Pos_trace_flag;               //λ�ø��ٱ�־λ                             
extern bool Trace_Ctrl_flag;              //���ٿ��Ʊ�־λ

void Car_Mov_Ctrl(void);
bool Car_Stop(void);
void CAR_PARAM_Init(CAR_PARAM* CAR_MOT,gpio_pin_enum PORT,pwm_channel_enum CH);
void Set_PID_Param(PID* pid,float p,float i,float d);      //����PID����
void Trace_Ctrl(void);
bool Pos_Trace(void);

#endif /* INCLUDE_CAR_MOT_CTRL_H_ */
