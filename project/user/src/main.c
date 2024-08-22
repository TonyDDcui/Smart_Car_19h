/*********************************************************************************************************************
* RT1064DVL6A Opensourec Library 即（RT1064DVL6A 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
* 
* 本文件是 RT1064DVL6A 开源库的一部分
* 
* RT1064DVL6A 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
* 
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
* 
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
* 
* 文件名称          main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 8.32.4 or MDK 5.33
* 适用平台          RT1064DVL6A
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2022-09-21        SeekFree            first version
********************************************************************************************************************/

#include "main.h"

// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

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
            Clear_Buffer(openart_recv_buffer);     //判断是否为正确的帧头
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

/*FIXME:存在通道bug，需要考虑修复*/

// 释放编码器资源
void Encoder_Dispose(Encoder* encoder, CAR_PARAM* CAR_MOT, KalmanFilter* kalman_filter)
{  
    // 保存当前脉冲计数并清零
    encoder->pulse_buffer[0] = encoder_get_count(encoder->encoder_num);
    encoder_clear_count(encoder->encoder_num);
    
    // 使用脉冲更新卡尔曼滤波器状态
    KalmanFilter_UpdateWithPulse(kalman_filter,encoder);
    
    // 计算电机转速（rad/s）
    CAR_MOT->CAR_MOT_V = (encoder->pulse_buffer[1]) / (1.024 * 5) * (3 / 7.0);
    
    // 计算电机转动圈数
    CAR_MOT->CAR_MOT_CYC = (encoder->pulse_buffer[2] / 1024) * (3 / 7.0);
}



