/*********************************************************************************************************************
* RT1064DVL6A Opensourec Library ����RT1064DVL6A ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
* 
* ���ļ��� RT1064DVL6A ��Դ���һ����
* 
* RT1064DVL6A ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
* 
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
* 
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
* 
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
* 
* �ļ�����          main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          IAR 8.32.4 or MDK 5.33
* ����ƽ̨          RT1064DVL6A
* ��������          https://seekfree.taobao.com/
* 
* �޸ļ�¼
* ����              ����                ��ע
* 2022-09-21        SeekFree            first version
********************************************************************************************************************/

#include "main.h"

// ���µĹ��̻��߹����ƶ���λ�����ִ�����²���
// ��һ�� �ر��������д򿪵��ļ�
// �ڶ��� project->clean  �ȴ��·�����������

bool setup_flag = false;
bool rx_finish = false;
bool Which_Recv = false;
uint32 flash_time = 0;
uint8 wifi_flag=0;
uint8 dap_flag=0;
uint8 tft_flag=0;
uint8 rx_num = 0;

bool speed_step = false;
int speed_step_add = 0;

void Encoder_Dispose(Encoder* encoder,CAR_PARAM* CAR_MOT,KalmanFilter* kalman_filter);




int main(void)
{
    sys_init_set();
    
    while(1)
    {
            if(mt9v03x_finish_flag && camera_init_finish_flag == 0)
    {
        image_ceshi();
        mt9v03x_finish_flag = 0;
    }
    }
}

void Usart_handler(void)
{
    openart_recv_buffer[rx_num] = uart_read_byte(UART_1);
    if(openart_recv_buffer[0] != 0x24) Clear_Buffer(openart_recv_buffer);
    else rx_num++;
    if(rx_num == 6)
    {
        if(openart_recv_buffer[0] != 0x24 || openart_recv_buffer[5] != 0x26) 
        {
            Clear_Buffer(openart_recv_buffer);     //�ж��Ƿ�Ϊ��ȷ��֡ͷ
            rx_num = 0;
        }
        else 
        {
            rx_finish = true;
            uart_rx_interrupt(UART_1, false);
        }
    }
}


void pit_handler (void)
{
    flash_time += 1;
    if(flash_time == 2000) 
    {
        gpio_toggle_level(B9);
        flash_time = 0;
    }

    if(flash_time % 4 == 0 && setup_flag) Car_Mov_Ctrl();
    //if(setup_flag) Car_Mov_Ctrl();
    
    for(int i = 0; i < 4; i++)
    {
        Encoder_Dispose(&Encoder_num[i], &cars_mot[i], &encoder_kalman_filter_num[i]);
    }
}

void pit_handler_1(void)
{
    if(mt9v03x_finish_flag && camera_init_finish_flag == 0)
    {
        image_ceshi();
        mt9v03x_finish_flag = 0;
    }
    if(speed_step == false) speed_step_add++;
    if (speed_step_add == 3) 
    {
        speed_step = true; 
        speed_step_add = 0;
    }
    //if(Cross_Flag == 1 || Zebra_Stripes_Flag == 1)
    //{
        //Car_Stop();
        //setup_flag = false;
    //}
}

void pit_handler_2(void)
{
    if(rx_finish)
    {
        Anal_Rec_Data();
        Restart_Recv();
    }
}

/*FIXME:����ͨ��bug����Ҫ�����޸�*/

// �ͷű�������Դ
void Encoder_Dispose(Encoder* encoder, CAR_PARAM* CAR_MOT, KalmanFilter* kalman_filter)
{  
    // ���浱ǰ�������������
    encoder->pulse_buffer[0] = encoder_get_count(encoder->encoder_num);
    encoder_clear_count(encoder->encoder_num);
    
    // ʹ��������¿������˲���״̬
    KalmanFilter_UpdateWithPulse(kalman_filter,encoder);
    
    // ������ת�٣�rad/s��
    CAR_MOT->CAR_MOT_V = (encoder->pulse_buffer[1]) / (1.024 * 5) * (3 / 7.0);
    
    // ������ת��Ȧ��
    CAR_MOT->CAR_MOT_CYC = (encoder->pulse_buffer[2] / 1024) * (3 / 7.0);
}



