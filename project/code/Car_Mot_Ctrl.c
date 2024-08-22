/*
 * Car_Mot_Ctrl.c
 *
 * Created on: 2024��3��14��
 * Author: CuiZheXiao
 */

#include "Car_Mot_Ctrl.h"

#define MOTOR_SPEED_DUTY 0.7
#define MOTOR_DUTY_DIVISOR 1.7

void MOT_PID_CTRL(PID* spd_pid,CAR_PARAM* car_mot,float Speed);

void pid_ctrl(PID *pid, float target_value, CAR_PARAM* car_mot); // ����PID�����ԴﵽĿ��ֵ�������³�������
void pid_ctrl_pos(PID *pid, float target_value, CAR_PARAM* car_mot); // ����Ŀ��λ�ÿ���PID�����������³�������
void pid_ctrl_trace(PID *pid); // ����PID����
void Pid_DeInit(PID* pid); // ����ʼ��PID����
void Set_Motor(CAR_PARAM* car_mot, uint8 motor_dir);

// ����ȫ�ֱ����Կ������ܳ�����Ϊ��־�͵�ǰ�ٶ�
bool Change_x_y_flag = true; // �����Ƿ�ı�x��y����ı�־
int cur_speed = 0; // ��ǰ�ٶ�
bool Pos_trace_flag = false; // λ�ø��ٱ�־

// �ṹ���������
CAR_PARAM cars_mot[4]; 

float Speed[4] = {0.0, 0.0, 0.0, 0.0};

// �����ٶ�PID���������飬����4��PID������ʵ��
PID spd_pids[4];

// ����λ��PID���������飬����4��PID������ʵ��
PID pos_pids[4];

// ���岹��PID���������飬����4��PID������ʵ��
PID compensate_pid[4];

uint8 motor_dir[4] = {GPIO_HIGH, GPIO_HIGH, GPIO_HIGH, GPIO_HIGH};

// �������ĸ��������ֱ����ڴ洢���롢����λ�ø��ٱ�־�͸��ٿ��Ʊ�־��
float Distance = 0.0;
uint8 Dir = 0;
bool Pos_Trace_flag = false;
bool Trace_Ctrl_flag = true;

/*
     * @brief ���ֲ�����ʼ������
     * @param car_mot ���ֲ����ṹ��
     * @param port ���ַ���GPIO�˿�
     * @param ch ����PWMͨ��
*/
void CAR_PARAM_Init(CAR_PARAM* car_mot, gpio_pin_enum port, pwm_channel_enum ch) {
    car_mot->MOTOR_DIR = port;
    car_mot->MOTOR_CH = ch;
    car_mot->CAR_MOT_CYC = 0;
    car_mot->CAR_MOT_V = 0;
    car_mot->MOT_OUTPUT_PLUSE = 0;
    car_mot->Set_Dir = 0;
}

/*
     * @brief ���ֿ��ƺ���
     * @param pid ���ƽṹ��
     * @param target_value Ŀ��ֵ
     * @param car_mot ���ֲ����ṹ��    
*/
void Car_Mov_Ctrl() {
    switch(Dir){
    case 0:
        motor_dir[0] = GPIO_HIGH;
        motor_dir[1] = GPIO_LOW;
        motor_dir[2] = GPIO_HIGH;
        motor_dir[3] = GPIO_LOW;
        break;
    case 1:
        motor_dir[0] = GPIO_HIGH;
        motor_dir[1] = GPIO_HIGH;
        motor_dir[2] = GPIO_HIGH;
        motor_dir[3] = GPIO_HIGH;
        break;
    
    default:
        break;
    }

    if(Trace_Ctrl_flag) Trace_Ctrl();
    //if(Pos_Trace_flag) Pos_Trace();

    for(int i = 0; i < 4; i++){
        MOT_PID_CTRL(&spd_pids[i],&cars_mot[i],Speed[i]);
        Set_Motor(&cars_mot[i], motor_dir[i]);
    }
}

/*
     * @brief �������ú���
     * @param car_mot ���ֲ����ṹ��
     * @param motor_dir ���ַ���
*/
void Set_Motor(CAR_PARAM* car_mot, uint8 motor_dir) { 
    car_mot->Set_Dir = motor_dir;
    gpio_set_level(car_mot->MOTOR_DIR, car_mot->Set_Dir);
    pwm_set_duty(car_mot->MOTOR_CH, (uint32)(car_mot->MOT_OUTPUT_PLUSE / MOTOR_DUTY_DIVISOR));
}

/*
     * @brief ѭ�����ƺ���
     * @param Err
     * @param normal_speed �����ٶ�
     * @param limit_speed_min ��С�ٶ�����
     * @param limit_speed_max ����ٶ�����
     * @param motor_dir ���ַ���
     * @param cars_mot ���ֲ����ṹ��
*/
void Trace_Ctrl() {
    if(fabs(Err) >= 3.65) //3.65
    {
        for(int i = 0; i < 4; i++){
            pid_ctrl_trace(&compensate_pid[i]);
        }

        if(Err > 0){                                //���ó��ּ�����
            compensate_pid[0].Set_Value *= -1.5;
            compensate_pid[1].Set_Value *= -1.5;

            compensate_pid[2].Set_Value *= 2.3;
            compensate_pid[3].Set_Value *= 2.3;
        }
        else{                                        //���ó��ּ�����
            compensate_pid[2].Set_Value *= -1.5;
            compensate_pid[3].Set_Value *= -1.5;

            compensate_pid[0].Set_Value *= 2.3;
            compensate_pid[1].Set_Value *= 2.3;
        }
    }
    else
    {
        for(int i = 0; i < 4; i++){
            compensate_pid[i].Set_Value = 0;
        }
    }

    for(int i = 0; i < 4; i++){
        /*
        if(cur_speed < normal_speed && speed_step) 
        {
            cur_speed+=0.1; 
            speed_step = false;
        }
        */
        Speed[i] = normal_speed + compensate_pid[i].Set_Value;
        if(Speed[i] < limit_speed_min) 
        {
            Speed[i] = limit_speed_min;
        }
        if(Speed[i] > limit_speed_max) Speed[i] = limit_speed_max;
    }
    Err = 0;
}

/*  
    * @brief λ�ø��ٺ���
    * @param Distance ����ֵ
*/

/*
     * @brief �����ٶȿ��ƺ���
     * @param spd_pid �ٶ�PID���ƽṹ��
     * @param car_mot ���ֲ����ṹ��
     * @param Speed �����ٶ�ֵ
*/
void MOT_PID_CTRL(PID* spd_pid,CAR_PARAM* car_mot,float Speed){
    pid_ctrl(spd_pid,Speed,car_mot);
    car_mot->MOT_OUTPUT_PLUSE = spd_pid->Set_Value;
}

/*
     * @brief PID�������ú��� 
     * @param pid PID���ƽṹ��
     * @param p ����ϵ��
     * @param i ����ϵ��
     * @param d ΢��ϵ��
*/
void Set_PID_Param(PID* pid, float p, float i, float d) {
    pid->K_p = p;
    pid->K_i = i;
    pid->K_d = d;
    pid->Last_Error = 0;
    pid->Set_Value_Last = 0;
    pid->Set_Value = 0;
    pid->Last_Error = 0;
    pid->Last_2_Error = 0;
}

/*
     * @brief ѭ��PID���ƺ���
     * @param pid ѭ��PID���ƽṹ��             
*/
void pid_ctrl_trace(PID *pid) {
    float error = fabs(Err/120);
    float count_value = pid->K_p * error + pid->K_i * (error + pid->Error_Last) + pid->K_d * (error - pid->Error_Last);
    pid->Error_Last = error;

    //pid->Set_Value_Last += MOTOR_SPEED_DUTY * count_value;
    //pid->Set_Value = fmax(pid->Set_Value_Last, -pid->Set_Value_Last);
    pid->Set_Value = count_value;
}

/*
     * @brief �ٶ�PID���ƺ���
     * @param pid �ٶ�PID���ƽṹ��
     * @param target_value Ŀ��ֵ
     * @param car_mot ���ֲ����ṹ��
*/ 
void pid_ctrl(PID *pid, float target_value, CAR_PARAM* car_mot) {
    pid->Target_Point = fabs(target_value);
    float error = pid->Target_Point - fabs(car_mot->CAR_MOT_V);
    float count_value = pid->K_p * error + pid->K_i * (error + pid->Error_Last) + pid->K_d * (error - pid->Error_Last);
    pid->Error_Last = error;

    pid->Set_Value_Last += MOTOR_SPEED_DUTY * count_value;
    pid->Set_Value = fmax(pid->Set_Value_Last, -pid->Set_Value_Last);
}

/*
     * @brief λ��PID���ƺ���
     * @param pid λ��PID���ƽṹ��
     * @param target_value Ŀ��ֵ
     * @param car_mot ���ֲ����ṹ��
*/ 
void pid_ctrl_pos(PID *pid, float target_value, CAR_PARAM* car_mot) {
    pid->Target_Point = target_value;
    float current_error = pid->Target_Point - fabs(car_mot->CAR_MOT_CYC);
    float error_diff = current_error - pid->Last_Error;
    float last_error_diff = pid->Last_Error - pid->Last_2_Error;

    pid->Count_Value = pid->K_p * error_diff + pid->K_i * current_error + pid->K_d * (error_diff - last_error_diff);

    pid->Last_2_Error = pid->Last_Error;
    pid->Last_Error = current_error;

    if (fabs(current_error) <= 0.15) Pid_DeInit(pid);
    else if (current_error < -0.15) car_mot->Set_Dir = ~car_mot->Set_Dir;
}

/*
    * @brief PID������ʼ������ 
    * @param pid PID���ƽṹ��
*/
void Pid_DeInit(PID* pid) {
    pid->Error_Last = 0;
    pid->Last_Error = 0;
    pid->Last_2_Error = 0;
    pid->Target_Point = 0;
    pid->Set_Value = 0;
    pid->Set_Value_Last = 0;
    pid->Count_Value = 0;
}

/*
    * @brief ����ֹͣ����
*/
bool Car_Stop()
{
    for(int i = 0; i < 4; i++)
    {
        setup_flag = false;
        pwm_set_duty(cars_mot[i].MOTOR_CH,0);
        Speed[i] = 0;
        Pid_DeInit(&spd_pids[i]);
        Pid_DeInit(&pos_pids[i]);
        Pid_DeInit(&compensate_pid[i]);
    }
    return false;
}

bool Pos_Trace() {
    if(Change_x_y_flag){
        for(int i = 0; i < 4; i++){
            pid_ctrl_pos(&pos_pids[i],x_error, &cars_mot[i]);
            Speed[i] = normal_speed+pos_pids[i].Count_Value;
        }
        if(abs(x_error) < 5)
        {
            for(int i = 0; i < 4; i++) Pid_DeInit(&pos_pids[i]);
            Dir = 0;
            return 1;
        }
        Change_x_y_flag = false;
    }
    else
    {
        Dir = 1;
        for(int i = 0; i < 4; i++){
            pid_ctrl_pos(&pos_pids[i],y_error, &cars_mot[i]);
            Speed[i] = normal_speed+pos_pids[i].Count_Value;
        }
        if(abs(y_error) < 5)
        {
            for(int i = 0; i < 4; i++) Pid_DeInit(&pos_pids[i]);
            Dir = 0;
            return 1;
        }
        Change_x_y_flag = true;
    }
}
