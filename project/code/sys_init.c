/*
 * sys_init.c
 *
 *  Created on: 2024��3��14��
 *      Author: CuiZheXiao
 */

#include "sys_init.h"

uint8 camera_init_finish_flag = 1;

void Peripheral_Init(void);                                                                 //���ͨ����ʼ��
void CAR_INIT(void);
void SERVO_PARAM_INIT(void);
void Mot_Pid_Param_Set(void);
void App_Init(void);

void sys_init_set(void)
{
	clock_init(SYSTEM_CLOCK_600M);       // ����ɾ��
	debug_init();                        // ���Զ˿ڳ�ʼ��
   
	Peripheral_Init();
	interrupt_global_enable(0);
	App_Init();
}

void App_Init(void)
{
	//������е�۳�ʼλ�õ�������ʼ�����ڴ�
	//Distance = 10;
    Dir = D;
	Trace_Ctrl_flag = true;
	Mot_Pid_Param_Set();
	Init_Servo();
	//system_delay_ms(10000);
	Send_Task(0);
	setup_flag = true;
}

void Peripheral_Init(void)                                                          
{
	//ϵͳ�������в���
	gpio_init(TEST_PIN,GPO,GPIO_LOW,GPO_PUSH_PULL);
	gpio_init(catch_1,GPO,GPIO_LOW,GPO_PUSH_PULL);
	gpio_init(catch_2,GPO,GPIO_LOW,GPO_PUSH_PULL);

	//��ʱ����ʼ��
	pit_ms_init(PIT_CH0,5);//5
	//pit_us_init(PIT_CH0,500);
	pit_ms_init(PIT_CH1,9);
	//pit_us_init(PIT_CH2,1);

	pit_enable(PIT_CH0);
	pit_enable(PIT_CH1); 
	//pit_enable(PIT_CH2);

	//��������ʼ��
   	Encoder_Init();

	//�������������ʼ��
   	CAR_INIT();

	camera_init_finish_flag = mt9v03x_init();

	//�����ʼ��
	SERVO_PARAM_INIT();

	//Openart_mini��ʼ��
	Usart_Init();
}


void CAR_INIT(void)                                                       //�����ز�����ʼ��
{	
   //�������DIR��ʼ��
	gpio_init(MOTOR1_DIR,GPO,GPIO_HIGH,GPO_PUSH_PULL);
   	gpio_init(MOTOR2_DIR,GPO,GPIO_HIGH,GPO_PUSH_PULL);
   	gpio_init(MOTOR3_DIR,GPO,GPIO_HIGH,GPO_PUSH_PULL);
   	gpio_init(MOTOR4_DIR,GPO,GPIO_HIGH,GPO_PUSH_PULL);

	pwm_init(MOTOR1_PWM,17000,0);
   	pwm_init(MOTOR2_PWM,17000,0);
   	pwm_init(MOTOR3_PWM,17000,0);
   	pwm_init(MOTOR4_PWM,17000,0);

	CAR_PARAM_Init(&cars_mot[0],MOTOR1_DIR,MOTOR1_PWM);

	CAR_PARAM_Init(&cars_mot[1],MOTOR2_DIR,MOTOR2_PWM);

	CAR_PARAM_Init(&cars_mot[2],MOTOR3_DIR,MOTOR3_PWM);

	CAR_PARAM_Init(&cars_mot[3],MOTOR4_DIR,MOTOR4_PWM);
}

void SERVO_PARAM_INIT(void)                                                        //�����ز�����ʼ��
{
    pwm_init(SERVO2_270_PWM,50,0);
   	pwm_init(SERVO3_270_PWM,50,0);
   	pwm_init(SERVO1_270_PWM,50,0);
	pwm_init(SERVO2_180_PWM,50,0);
	pwm_init(SERVO1_180_PWM,50,0);
}

void Mot_Pid_Param_Set(void)
{
  /*
  	Set_PID_Param(&spd_pids[0],63.5,15,47.2);
	Set_PID_Param(&spd_pids[1],73.5,10,27.2);
	Set_PID_Param(&spd_pids[2],78.5,6.58,27.2);
	Set_PID_Param(&spd_pids[3],78.5,6.58,27.2);
	*/
  
	Set_PID_Param(&spd_pids[0],63.5,15,47.2);
	Set_PID_Param(&spd_pids[1],73.5,10,27.2);
	Set_PID_Param(&spd_pids[2],63.5,10,37.2);
	Set_PID_Param(&spd_pids[3],66.5,14.58,37.2);

/*
	Set_PID_Param(&SPD_PID1,73.5,6.58,37.2);
	Set_PID_Param(&SPD_PID2,63.5,10,27.2);
	Set_PID_Param(&SPD_PID3,63.5,10,37.2);
	Set_PID_Param(&SPD_PID4,63.5,6.58,37.2);
	*/

	Set_PID_Param(&pos_pids[0],1,0,0);
	Set_PID_Param(&pos_pids[1],1,0,2);
	Set_PID_Param(&pos_pids[2],1,0,2);
	Set_PID_Param(&pos_pids[3],1,0,2);

	Set_PID_Param(&compensate_pid[0],10,0,0);
	Set_PID_Param(&compensate_pid[1],10,0,0);
	Set_PID_Param(&compensate_pid[2],10,0,0);
	Set_PID_Param(&compensate_pid[3],10,0,0);
/*
	Set_PID_Param(&compensate_pid[0],0.971,0,0);
	Set_PID_Param(&compensate_pid[1],0.971,0,0);
	Set_PID_Param(&compensate_pid[2],0.971,0,0);
	Set_PID_Param(&compensate_pid[3],0.971,0,0);

	*/
}
