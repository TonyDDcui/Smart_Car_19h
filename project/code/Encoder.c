#include "Encoder.h"

#define ENCODER_COUNT 4

// ����������Ϳ������˲�����ȫ�ֱ���
KalmanFilter encoder_kalman_filter1, encoder_kalman_filter2, encoder_kalman_filter3, encoder_kalman_filter4,encoder_kalman_filter_num[4]={0};
Encoder Encoder1, Encoder2, Encoder3, Encoder4,Encoder_num[4]={0};
// ���������������ͨ����ö�ٱ���
encoder_index_enum encoder_num[4]= {ENCODER_1, ENCODER_2, ENCODER_3, ENCODER_4};
encoder_channel1_enum encoder_channel1[4]= {ENCODER_1_LSB, ENCODER_2_LSB, ENCODER_3_LSB, ENCODER_4_LSB};
encoder_channel2_enum encoder_channel2[4]= {ENCODER_1_DIR, ENCODER_2_DIR, ENCODER_3_DIR, ENCODER_4_DIR};

void Encoder_Param_Init(Encoder* encoder, encoder_index_enum encoder_num);

// ��ʼ��������ģ��
void Encoder_Init(void)
{
    for(int i=0;i<4;i++)
    {
        Encoder_Param_Init(&Encoder_num[i],encoder_num[i]); // ��ʼ������������
        encoder_dir_init(encoder_num[i], encoder_channel1[i], encoder_channel2[i]); // ��ʼ��������
        KalmanFilter_Init(&encoder_kalman_filter_num[i], 0.01, 0.01, 1.0, 0.0);
    }
}


/* ��ʼ���������˲������� */
void KalmanFilter_Init(KalmanFilter *kf, float process_noise, float measurement_noise, float measurement_prediction, float initial_value) 
{
    kf->x = initial_value;
    kf->p = 1.0;
    kf->q = process_noise;
    kf->r = measurement_noise;
    kf->h = measurement_prediction;
}

// ʹ��������¿������˲���
void KalmanFilter_UpdateWithPulse(KalmanFilter *kf, Encoder* encoder) 
{   
    float s = kf->p + kf->r;
    float k = kf->p / s;

    kf->p = (1 - k * kf->h) * kf->q;
    kf->p += kf->q;
    kf->x += k * (encoder->pulse_buffer[0] - kf->h * kf->x);
    encoder->pulse_buffer[1] = (int)kf->x;
    encoder->pulse_buffer[2] += encoder->pulse_buffer[1];
}

/* ��ʼ�������������ĺ���
 ����:
   encoder - ָ��Ҫ��ʼ���ı������ṹ��ָ��
   encoder_num - �������ı��
*/
void Encoder_Param_Init(Encoder* encoder, encoder_index_enum encoder_num)
{
    encoder->pulse_buffer[0] = 0;
    encoder->pulse_buffer[1] = 0;
    encoder->pulse_buffer[2] = 0;
    encoder->encoder_num = encoder_num;
}
