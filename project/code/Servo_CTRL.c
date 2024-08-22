#include "Servo_CTRL.h"

pwm_channel_enum channel_list[4];

void Set_pwm_duty(uint8 channel, int duty_now, int duty_next);
// 设置PWM通道的占空比，从当前占空比逐步调整到目标占空比
void Set_pwm_duty(uint8 channel, int duty_now, int duty_next) 
{
    int duty_step = (duty_next > duty_now) ? 1 : -1;
    for (int duty_set = duty_now; duty_set != duty_next; duty_set += duty_step) 
    {
        pwm_set_duty(channel_list[channel], duty_set);
        system_delay_us(Delay_Time); // Delay 5ms
    }
}

void Set270_pwm(uint8 channel, float Angle_now, float Angle_next)
{
  //现在的占空比  接下来的占空比
  int duty_now, duty_next;

  duty_now =  (int)(250+(Angle_now * 2000)/540);
  duty_next = (int)(250+(Angle_next * 2000)/540);
  
  Set_pwm_duty(channel,duty_now,duty_next);
}

void Set180_pwm(uint8 channel,float Angle_now,float Angle_next)
{
  //现在的占空比  接下来的占空比
  int duty_now, duty_next;
  float p_now,p_next;
  p_now =  Angle_now/135;
  p_next =  Angle_next/135; 
  duty_now = (int)((p_now + 0.5)*500);
  duty_next = (int)((p_next + 0.5)*500);
  
  Set_pwm_duty(channel,duty_now,duty_next);
}

void Init_Servo(void)
{
  channel_list[2] = SERVO3_270_PWM;

  Set270_pwm(2,5,50);

  system_delay_ms(1000);
  channel_list[0] = SERVO1_180_PWM;
  channel_list[1] = SERVO1_270_PWM;
  Set180_pwm(0,100,80);//110为底板舵机初始化位置
  Set270_pwm(1,50,230);//20为电磁铁舵机初始化位置
  system_delay_ms(200);
  channel_list[0] = SERVO2_180_PWM;
  channel_list[1] = SERVO2_270_PWM;
  Set180_pwm(0,100,80);//110为底板舵机初始化位置
  Set270_pwm(1,50,230);//20为电磁铁舵机初始化位置
  system_delay_ms(200);
}

void Set_Pick_Up(void)
// 设置拾取动作的函数，调用多个PWM控制函数和延时函数来完成一系列舵机动作，实现物体的拾取操作。
{
  Set270_pwm(1,50,30);
  Set180_pwm(0 , 80 , 185);//下大臂
  system_delay_ms(200);
  Set180_pwm(0 , 185 , 110);//上大臂
  Set270_pwm(1 , 30, 180);//电磁舵机转动，接下来是大臂补偿190、
  system_delay_ms(200);
  Set180_pwm(0 , 110 , 80);//上大臂
  Set270_pwm(1 , 180, 230);//电磁舵机转动，接下来是大臂补偿190、
}

void Set_Put_Down(void)
{       
  //Set180_pwm(0 , 105 , 110);//大臂初始化位置

  Set270_pwm(1 , 15, 20);//电磁舵机初始化位置
  Set270_pwm(1 , 20, 235);//准备吸卡片
    
  Set180_pwm(0 , 110 , 100);//吸
  Set180_pwm(0 , 100 , 110);//补偿
  Set270_pwm(1 , 235, 195);//转动卡片
    
  Set180_pwm(0 , 110 , 120);//移动一定位置
    
  Set270_pwm(1 , 195, 20);
    
  Set180_pwm(0 , 120, 180);//下大臂

  Set180_pwm(0 , 180, 110);//上大臂
  Set270_pwm(1 , 20, 15);
}