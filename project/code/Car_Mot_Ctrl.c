/*
 * Car_Mot_Ctrl.c
 *
 * Created on: 2024年3月14日
 * Author: CuiZheXiao
 */

#include "Car_Mot_Ctrl.h"

#define MOTOR_SPEED_DUTY 0.7
#define MOTOR_DUTY_DIVISOR 1.7

void MOT_PID_CTRL(PID* spd_pid,CAR_PARAM* car_mot,float Speed);

void pid_ctrl(PID *pid, float target_value, CAR_PARAM* car_mot); // 控制PID参数以达到目标值，并更新车辆参数
void pid_ctrl_pos(PID *pid, float target_value, CAR_PARAM* car_mot); // 根据目标位置控制PID参数，并更新车辆参数
void pid_ctrl_trace(PID *pid); // 跟踪PID参数
void Pid_DeInit(PID* pid); // 反初始化PID参数
void Set_Motor(CAR_PARAM* car_mot, uint8 motor_dir);

// 定义全局变量以控制智能车的行为标志和当前速度
bool Change_x_y_flag = true; // 控制是否改变x和y方向的标志
int cur_speed = 0; // 当前速度
bool Pos_trace_flag = false; // 位置跟踪标志

// 结构体变量声明
CAR_PARAM cars_mot[4]; 

float Speed[4] = {0.0, 0.0, 0.0, 0.0};

// 定义速度PID控制器数组，包含4个PID控制器实例
PID spd_pids[4];

// 定义位置PID控制器数组，包含4个PID控制器实例
PID pos_pids[4];

// 定义补偿PID控制器数组，包含4个PID控制器实例
PID compensate_pid[4];

uint8 motor_dir[4] = {GPIO_HIGH, GPIO_HIGH, GPIO_HIGH, GPIO_HIGH};

// 定义了四个变量，分别用于存储距离、方向、位置跟踪标志和跟踪控制标志。
float Distance = 0.0;
uint8 Dir = 0;
bool Pos_Trace_flag = false;
bool Trace_Ctrl_flag = true;

/*
     * @brief 车轮参数初始化函数
     * @param car_mot 车轮参数结构体
     * @param port 车轮方向GPIO端口
     * @param ch 车轮PWM通道
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
     * @brief 车轮控制函数
     * @param pid 控制结构体
     * @param target_value 目标值
     * @param car_mot 车轮参数结构体    
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
     * @brief 车轮设置函数
     * @param car_mot 车轮参数结构体
     * @param motor_dir 车轮方向
*/
void Set_Motor(CAR_PARAM* car_mot, uint8 motor_dir) { 
    car_mot->Set_Dir = motor_dir;
    gpio_set_level(car_mot->MOTOR_DIR, car_mot->Set_Dir);
    pwm_set_duty(car_mot->MOTOR_CH, (uint32)(car_mot->MOT_OUTPUT_PLUSE / MOTOR_DUTY_DIVISOR));
}

/*
     * @brief 循迹控制函数
     * @param Err
     * @param normal_speed 正常速度
     * @param limit_speed_min 最小速度限制
     * @param limit_speed_max 最大速度限制
     * @param motor_dir 车轮方向
     * @param cars_mot 车轮参数结构体
*/
void Trace_Ctrl() {
    if(fabs(Err) >= 3.65) //3.65
    {
        for(int i = 0; i < 4; i++){
            pid_ctrl_trace(&compensate_pid[i]);
        }

        if(Err > 0){                                //设置车轮减速组
            compensate_pid[0].Set_Value *= -1.5;
            compensate_pid[1].Set_Value *= -1.5;

            compensate_pid[2].Set_Value *= 2.3;
            compensate_pid[3].Set_Value *= 2.3;
        }
        else{                                        //设置车轮减速组
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
    * @brief 位置跟踪函数
    * @param Distance 距离值
*/

/*
     * @brief 车轮速度控制函数
     * @param spd_pid 速度PID控制结构体
     * @param car_mot 车轮参数结构体
     * @param Speed 车轮速度值
*/
void MOT_PID_CTRL(PID* spd_pid,CAR_PARAM* car_mot,float Speed){
    pid_ctrl(spd_pid,Speed,car_mot);
    car_mot->MOT_OUTPUT_PLUSE = spd_pid->Set_Value;
}

/*
     * @brief PID参数设置函数 
     * @param pid PID控制结构体
     * @param p 比例系数
     * @param i 积分系数
     * @param d 微分系数
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
     * @brief 循迹PID控制函数
     * @param pid 循迹PID控制结构体             
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
     * @brief 速度PID控制函数
     * @param pid 速度PID控制结构体
     * @param target_value 目标值
     * @param car_mot 车轮参数结构体
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
     * @brief 位置PID控制函数
     * @param pid 位置PID控制结构体
     * @param target_value 目标值
     * @param car_mot 车轮参数结构体
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
    * @brief PID参数初始化函数 
    * @param pid PID控制结构体
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
    * @brief 汽车停止函数
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
