/**
 *
 ******************************************************************************
 * @file           : bsp_uart.h
 * @brief          :
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 *
 *  Created on:
 *      Author:
 */
#ifndef _BSP_UART_H
#define _BSP_UART_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_uart_queue.h"

/* 定义端口号 */
typedef enum
{
	COM1 = 0,	/* USART1 */
	COM2 = 1,	/* USART2 */
	COM3 = 2,	/* USART3 */
	COM4 = 3,	/* UART4 */
	COM5 = 4,	/* UART5 */
	COM6 = 5,	/* USART6 */
	COM7 = 6,	/* UART7 */	
	COM8 = 7	/* UART8 */	
}COM_PORT_E;

typedef struct
{
    Queue_Message_Type tx_queue;
    Queue_Message_Type rx_queue;
} Uart_Msg_T;

void bsp_uart_init(void);
void bsp_uart_send_data(COM_PORT_E _ucPort, uint8_t *data, uint16_t len);
void Uart_Msg_Task(void);

#endif

