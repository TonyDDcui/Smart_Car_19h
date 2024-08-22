#include "Servo_CTRL.h"

pwm_channel_enum channel_list[4];

void Set_pwm_duty(uint8 channel, int duty_now, int duty_next);
// ����PWMͨ����ռ�ձȣ��ӵ�ǰռ�ձ��𲽵�����Ŀ��ռ�ձ�
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
  //���ڵ�ռ�ձ�  ��������ռ�ձ�
  int duty_now, duty_next;

  duty_now =  (int)(250+(Angle_now * 2000)/540);
  duty_next = (int)(250+(Angle_next * 2000)/540);
  
  Set_pwm_duty(channel,duty_now,duty_next);
}

void Set180_pwm(uint8 channel,float Angle_now,float Angle_next)
{
  //���ڵ�ռ�ձ�  ��������ռ�ձ�
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
  Set180_pwm(0,100,80);//110Ϊ�װ�����ʼ��λ��
  Set270_pwm(1,50,230);//20Ϊ����������ʼ��λ��
  system_delay_ms(200);
  channel_list[0] = SERVO2_180_PWM;
  channel_list[1] = SERVO2_270_PWM;
  Set180_pwm(0,100,80);//110Ϊ�װ�����ʼ��λ��
  Set270_pwm(1,50,230);//20Ϊ����������ʼ��λ��
  system_delay_ms(200);
}

void Set_Pick_Up(void)
// ����ʰȡ�����ĺ��������ö��PWM���ƺ�������ʱ���������һϵ�ж��������ʵ�������ʰȡ������
{
  Set270_pwm(1,50,30);
  Set180_pwm(0 , 80 , 185);//�´��
  system_delay_ms(200);
  Set180_pwm(0 , 185 , 110);//�ϴ��
  Set270_pwm(1 , 30, 180);//��Ŷ��ת�����������Ǵ�۲���190��
  system_delay_ms(200);
  Set180_pwm(0 , 110 , 80);//�ϴ��
  Set270_pwm(1 , 180, 230);//��Ŷ��ת�����������Ǵ�۲���190��
}

void Set_Put_Down(void)
{       
  //Set180_pwm(0 , 105 , 110);//��۳�ʼ��λ��

  Set270_pwm(1 , 15, 20);//��Ŷ����ʼ��λ��
  Set270_pwm(1 , 20, 235);//׼������Ƭ
    
  Set180_pwm(0 , 110 , 100);//��
  Set180_pwm(0 , 100 , 110);//����
  Set270_pwm(1 , 235, 195);//ת����Ƭ
    
  Set180_pwm(0 , 110 , 120);//�ƶ�һ��λ��
    
  Set270_pwm(1 , 195, 20);
    
  Set180_pwm(0 , 120, 180);//�´��

  Set180_pwm(0 , 180, 110);//�ϴ��
  Set270_pwm(1 , 20, 15);
}