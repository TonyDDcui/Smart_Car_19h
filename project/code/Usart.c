#include "Usart.h"

uint8  openart_recv_buffer[6];
int x_error=0;
int y_error=0;
uint8 send_task_num=0;
bool Sended_flag=false;
uint8 Picked_Cards_1 = 0, Picked_Cards_2 = 0, Picked_Cards_3 = 0;
uint8 Put_Down_Cards = 0;

/*	@brief 发送任务数组
	0：寻找卡片
	1：数字识别
	2：字母识别
	3：卡片识别
	4：休眠
*/
uint8 Send_task[10][5] = {
	{0x24,0x02,0x01,0xA0,0x26},{0x24,0x02,0x02,0xA0,0x26},
	{0x24,0x02,0x01,0xB0,0x26},{0x24,0x02,0x02,0xB0,0x26},
	{0x24,0x02,0x01,0xC0,0x26},{0x24,0x02,0x02,0xC0,0x26},
	{0x24,0x02,0x01,0xD0,0x26},{0x24,0x02,0x02,0xD0,0x26},
	{0x24,0x02,0x01,0xE0,0x26},{0x24,0x02,0x02,0xE0,0x26}
};

void Usart_Init(void)
{
    uart_init(UART_1,115200,UART1_TX_B12,UART1_RX_B13);
	uart_rx_interrupt(UART_1,1);
}

void Clear_Buffer(uint8 *buffer)
{
	for(int i=0;i<10;i++)
	{
		buffer[i]=0;
	}
}

/*这个代码作用于实现恢复uart接收*/
void Restart_Recv(void)
{
 // 清除接收缓冲区
 	Clear_Buffer(openart_recv_buffer);
 // 重置接收完成标志
 	rx_finish=false;
 // 重置接收字节数
 	rx_num = 0;
 // 启用UART接收中断
 	uart_rx_interrupt(UART_1,1);
}

void Anal_Rec_Data(void)
{
	switch(openart_recv_buffer[2])
	{
		case 0x01:
			x_error = (int)openart_recv_buffer[3] - 120;
			y_error = (int)openart_recv_buffer[4] - 90;
			//刹车操作
			//Pos_trace_flag = true;
			//while(Pos_Trace());
			while(Car_Stop());
			Set_Pick_Up();
			setup_flag = true;
			//Send_Task(3);
			break;
		case 0x02:
			//卸货操作
			//TODO：首先识别数字，完成相应的货舱位置，里面有两个数据，一个是固定角度，如60°对应数字1，120°对应数字2
			//然后根据货物编号，控制相应的舵机转动到相应的位置，并打开相应的盖子
			//最后，控制电机转动到相应的位置，并打开相应的盖子
			if(openart_recv_buffer[3] == 0x31 || openart_recv_buffer[3] == 0x32 || openart_recv_buffer[3] == 0x33 || openart_recv_buffer[3] == 0x34 || openart_recv_buffer[3] == 0x35 || openart_recv_buffer[3] == 0x36 || openart_recv_buffer[3] == 0x37 || openart_recv_buffer[3] == 0x38 || openart_recv_buffer[3] == 0x39 || openart_recv_buffer[3] == 0x30) 
			{
				//货物1
				//TODO：打开盖子，控制电机转动到相应的位置
				if(openart_recv_buffer[3] == 0x31)
				{
					Put_Down_Cards = Picked_Cards_1;
					Picked_Cards_1 = 0;
					Set270_pwm(channel_list[2],180,0);
					//TODO:打开盖子，控制电机转动到相应的位置
				}
				else if(openart_recv_buffer[3] == 0x32)
				{
					Put_Down_Cards = Picked_Cards_2;
					Picked_Cards_2 = 0;
					Set270_pwm(channel_list[2],180,60);
					//TODO:打开盖子，控制电机转动到相应的位置
				}
				else if(openart_recv_buffer[3] == 0x33)
				{
					Put_Down_Cards = Picked_Cards_3;
					Picked_Cards_3 = 0;
					Set270_pwm(channel_list[2],180,180);
				}

				for(;Put_Down_Cards>=0;Put_Down_Cards--)  Set_Put_Down();
				//TODO:检测货物是否取干净
			}
			if(openart_recv_buffer[3] == 0x31) Send_Task(4);
			break;

		case 0x03:
			break;
			
		case 0x04:
			if(openart_recv_buffer[1] == 0x01)
			{
				channel_list[0] = SERVO2_180_PWM;
				channel_list[1] = SERVO2_270_PWM;
			}
			else if(openart_recv_buffer[1] == 0x02)
			{
				channel_list[0] = SERVO1_180_PWM;
				channel_list[1] = SERVO1_270_PWM;
			}
			if(openart_recv_buffer[4] == 0x31 || openart_recv_buffer[4] == 0x32 || openart_recv_buffer[4] == 0x33 || openart_recv_buffer[4] == 0x34 || openart_recv_buffer[4] == 0x35 || openart_recv_buffer[4] == 0x36 || openart_recv_buffer[4] == 0x37 || openart_recv_buffer[4] == 0x38 || openart_recv_buffer[4] == 0x39 || openart_recv_buffer[4] == 0x30)
			{
				if(openart_recv_buffer[4] == 0x31) 
				{
					Picked_Cards_1++;
					Set270_pwm(channel_list[2],0,0);
					//TODO:货舱旋转到对应的角度
				}
				else if(openart_recv_buffer[4] == 0x32) 
				{
					Picked_Cards_2++;
					Set270_pwm(channel_list[2],0,60);
					//TODO:货舱旋转到对应的角度
				}
				else if(openart_recv_buffer[4] == 0x33) 
				{
					Picked_Cards_3++;
					Set270_pwm(channel_list[2],60,180);
					//TODO:货舱旋转到对应的角度
				}
				Set_Pick_Up();
				Send_Task(0);
			}
			break;
		default:
			break;
	}
}

void Send_Task(uint8 task_num)
{
	uart_write_buffer(UART_1,(uint8 const*)&Send_task[task_num],5);
	uart_write_buffer(UART_1,(uint8 const*)&Send_task[task_num+1],5);
}


