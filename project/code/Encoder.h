#ifndef _Encoder_h_
#define _Encoder_h_

#include "main.h"

typedef struct {
    float x;        // 状态变量
    float p;        // 状态误差协方差
    float q;        // 过程噪声协方差
    float r;        // 测量噪声协方差
    float h;        // 预测函数
} KalmanFilter;

typedef struct {
    int pulse_buffer[3];
    encoder_index_enum encoder_num;
} Encoder;

extern Encoder Encoder1, Encoder2, Encoder3, Encoder4,Encoder_num[4];
extern KalmanFilter encoder_kalman_filter1, encoder_kalman_filter2, encoder_kalman_filter3, encoder_kalman_filter4, encoder_kalman_filter_num[4];

void Encoder_Init(void);
void KalmanFilter_Init(KalmanFilter *kf, float process_noise, float measurement_noise, float measurement_prediction, float initial_value); 
void KalmanFilter_UpdateWithPulse(KalmanFilter *kf, Encoder* encoder);

#endif