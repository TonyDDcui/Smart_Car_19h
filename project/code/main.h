#ifndef _main_h
#define _main_h

#include "zf_common_headfile.h"
#include "math.h"
#include "Car_Mot_Ctrl.h"
#include "sys_init.h"
#include "Servo_CTRL.h"
#include "Encoder.h"  
#include "tft_display.h"      
#include "Usart.h"
#include "string.h"
#include "Trace.h"

extern bool rx_finish;
extern bool setup_flag;
extern uint8 rx_num;
#define DAP_TRANSFER   0   //   开启或关闭dap传输   1开启 0关闭
#define WIFI_TRANSFER  0   //   开启或关闭wifi传输  1开启 0关闭
#define TFT_SHOW       0   //   开启或关闭TFT显示   1开启 0关闭

#define limit_speed_max 20
#define limit_speed_min 0.06
#define normal_speed 2.5

extern bool speed_step;

//汽车电机PWM和DIR引脚配置

#define MOTOR1_DIR               (C7)
#define MOTOR1_PWM               (PWM2_MODULE0_CHA_C6)

#define MOTOR2_DIR               (C9)
#define MOTOR2_PWM               (PWM2_MODULE1_CHA_C8)

#define MOTOR3_DIR               (D3)
#define MOTOR3_PWM               (PWM2_MODULE3_CHA_D2)

#define MOTOR4_DIR               (C11)
#define MOTOR4_PWM               (PWM2_MODULE2_CHA_C10)

//机械臂舵机PWM引脚配置

#define SERVO2_270_PWM               (PWM1_MODULE3_CHA_D0)   //第二组机械臂小臂
#define SERVO3_270_PWM               (PWM1_MODULE3_CHB_D1)   //圆盘旋转舵机
#define SERVO1_270_PWM               (PWM4_MODULE2_CHA_C30)  //第一组机械臂小臂
#define SERVO2_180_PWM               (PWM1_MODULE2_CHB_D17)  //第二组机械臂大臂
#define SERVO1_180_PWM               (PWM1_MODULE2_CHA_D16)  //第一组机械臂大臂

//编码器引脚配置

#define ENCODER_1                       (QTIMER1_ENCODER1)
#define ENCODER_1_LSB                   (QTIMER1_ENCODER1_CH1_C0)
#define ENCODER_1_DIR                   (QTIMER1_ENCODER1_CH2_C1)
                                        
#define ENCODER_2                       (QTIMER1_ENCODER2)
#define ENCODER_2_LSB                   (QTIMER1_ENCODER2_CH1_C2)
#define ENCODER_2_DIR                   (QTIMER1_ENCODER2_CH2_C24)

/*XXX:这个是测试专用，需要后面更改，原C3,C4*/

#define ENCODER_3                       (QTIMER2_ENCODER1)
#define ENCODER_3_LSB                   (QTIMER2_ENCODER1_CH1_C3)
#define ENCODER_3_DIR                   (QTIMER2_ENCODER1_CH2_C4)

#define ENCODER_4                       (QTIMER2_ENCODER2)
#define ENCODER_4_LSB                   (QTIMER2_ENCODER2_CH1_C5)
#define ENCODER_4_DIR                   (QTIMER2_ENCODER2_CH2_C25)

#define TEST_PIN                        (B9)

#define catch_1                         (D26)
#define catch_2                         (D27)

#endif
