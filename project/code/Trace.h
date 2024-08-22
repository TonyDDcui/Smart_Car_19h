#ifndef __Trace_h
#define __Trace_h

#include "main.h"

#define IMG_BLACK     0X00      //0x00是黑
#define IMG_WHITE     0Xff      //0xff为白


extern uint8 image_01[MT9V03X_H][MT9V03X_W];//储存二值化图像的数组
extern volatile float Err;
extern volatile int Cross_Flag;
extern volatile int Zebra_Stripes_Flag;
extern uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];//储存二值化图像的数组

void image_ceshi(void);
void Transfer_Camera(void);
uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold);
void Get01change_Dajin(void);
void Pixle_Filter(void);
void Longest_White_Column(void);
void Show_Boundry(void);
float Err_Sum(void);

struct ROAD_TYPE
{
        int8 straight;     
        int8 bend;          
        int8 Ramp;         
        int8 Cross;         
        int8 L_Cross;        
        int8 R_Cross;        
        int8 LeftCirque;     
        int8 RightCirque;    
        int8 Fork;           

};


struct STATUS_TYPE 
{
        int8 start_pick;     //开始捡卡片
        int8 stop_pick;      
        int8 start_lay_W;   //开始放置字母小类卡片
        int8 stop_lay_W;    
        int8 start_lay_F;   //开始放置数字大类卡片
        int8 stop_lay_F;     
};

#endif
