#include "Usart.h"

uint8  openart_recv_buffer[6];
int x_error=0;
int y_error=0;
uint8 send_task_num=0;
bool Sended_flag=false;
uint8 Picked_Cards_1 = 0, Picked_Cards_2 = 0, Picked_Cards_3 = 0;
uint8 Put_Down_Cards = 0;

/*	@brief ������������
	0��Ѱ�ҿ�Ƭ
	1������ʶ��
	2����ĸʶ��
	3����Ƭʶ��
	4������
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

/*�������������ʵ�ָֻ�uart����*/
void Restart_Recv(void)
{
 // ������ջ�����
 	Clear_Buffer(openart_recv_buffer);
 // ���ý�����ɱ�־
 	rx_finish=false;
 // ���ý����ֽ���
 	rx_num = 0;
 // ����UART�����ж�
 	uart_rx_interrupt(UART_1,1);
}

void Anal_Rec_Data(void)
{
	switch(openart_recv_buffer[2])
	{
		case 0x01:
			x_error = (int)openart_recv_buffer[3] - 120;
			y_error = (int)openart_recv_buffer[4] - 90;
			//ɲ������
			//Pos_trace_flag = true;
			//while(Pos_Trace());
			while(Car_Stop());
			Set_Pick_Up();
			setup_flag = true;
			//Send_Task(3);
			break;
		case 0x02:
			//ж������
			//TODO������ʶ�����֣������Ӧ�Ļ���λ�ã��������������ݣ�һ���ǹ̶��Ƕȣ���60���Ӧ����1��120���Ӧ����2
			//Ȼ����ݻ����ţ�������Ӧ�Ķ��ת������Ӧ��λ�ã�������Ӧ�ĸ���
			//��󣬿��Ƶ��ת������Ӧ��λ�ã�������Ӧ�ĸ���
			if(openart_recv_buffer[3] == 0x31 || openart_recv_buffer[3] == 0x32 || openart_recv_buffer[3] == 0x33 || openart_recv_buffer[3] == 0x34 || openart_recv_buffer[3] == 0x35 || openart_recv_buffer[3] == 0x36 || openart_recv_buffer[3] == 0x37 || openart_recv_buffer[3] == 0x38 || openart_recv_buffer[3] == 0x39 || openart_recv_buffer[3] == 0x30) 
			{
				//����1
				//TODO���򿪸��ӣ����Ƶ��ת������Ӧ��λ��
				if(openart_recv_buffer[3] == 0x31)
				{
					Put_Down_Cards = Picked_Cards_1;
					Picked_Cards_1 = 0;
					Set270_pwm(channel_list[2],180,0);
					//TODO:�򿪸��ӣ����Ƶ��ת������Ӧ��λ��
				}
				else if(openart_recv_buffer[3] == 0x32)
				{
					Put_Down_Cards = Picked_Cards_2;
					Picked_Cards_2 = 0;
					Set270_pwm(channel_list[2],180,60);
					//TODO:�򿪸��ӣ����Ƶ��ת������Ӧ��λ��
				}
				else if(openart_recv_buffer[3] == 0x33)
				{
					Put_Down_Cards = Picked_Cards_3;
					Picked_Cards_3 = 0;
					Set270_pwm(channel_list[2],180,180);
				}

				for(;Put_Down_Cards>=0;Put_Down_Cards--)  Set_Put_Down();
				//TODO:�������Ƿ�ȡ�ɾ�
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
					//TODO:������ת����Ӧ�ĽǶ�
				}
				else if(openart_recv_buffer[4] == 0x32) 
				{
					Picked_Cards_2++;
					Set270_pwm(channel_list[2],0,60);
					//TODO:������ת����Ӧ�ĽǶ�
				}
				else if(openart_recv_buffer[4] == 0x33) 
				{
					Picked_Cards_3++;
					Set270_pwm(channel_list[2],60,180);
					//TODO:������ת����Ӧ�ĽǶ�
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


