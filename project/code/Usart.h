#ifndef __Usart_H__
#define __Usart_H__   

#include "main.h"


extern uint8 openart_recv_buffer[6];
extern int x_error;
extern int y_error;

void Usart_Init(void);
void Clear_Buffer(uint8 *buffer);
void Anal_Rec_Data(void);
void Send_Task(uint8 task_num);
void Restart_Recv(void);


#endif