/*
 * @Description: 
 * @Date: 2024-12-27 21:33:22
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-21 22:22:24
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_uart.h"
#include "usart.h"

#define UART3_FIFO_EN 1

#define MAX_MSG_SIZE 256

/* 定义每个串口结构体变量 */
#if UART1_FIFO_EN == 1
static Uart_Msg_T tUart1;
static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE]; /* 发送缓冲区 */
static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE]; /* 接收缓冲区 */
#endif

#if UART2_FIFO_EN == 1
static Uart_Msg_T tUart2;
static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE]; /* 发送缓冲区 */
static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE]; /* 接收缓冲区 */
#endif

#if UART3_FIFO_EN == 1
Uart_Msg_T tUart3;
// 使用dma需要定义在RAM_D2区
static uint8_t uart3_rx_buffer[MAX_MSG_SIZE];
static uint8_t uart3_tx_buffer[MAX_MSG_SIZE];
#endif

#if UART4_FIFO_EN == 1
static Uart_Msg_T tUart4;
// 使用dma需要定义在RAM_D2区
__attribute__((section(".RAM_D2"))) uint8_t uart4_rx_buffer[MAX_MSG_SIZE];
__attribute__((section(".RAM_D2"))) uint8_t uart4_tx_buffer[MAX_MSG_SIZE];
#endif

/**
 * @brief  串口初始化
 */
void bsp_uart_init(void)
{
	InitQueue(&tUart3.rx_queue);
	InitQueue(&tUart3.tx_queue);

	__HAL_UART_CLEAR_IDLEFLAG(&huart3);
	__HAL_UART_DISABLE_IT(&huart3, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_rx_buffer, MAX_MSG_SIZE);
}

/**
 * @brief  将COM端口号转换为UART指针
 */
Uart_Msg_T *ComToUart(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
#if UART1_FIFO_EN == 1
		return &tUart1;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM2)
	{
#if UART2_FIFO_EN == 1
		return &tUart2;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM3)
	{
#if UART3_FIFO_EN == 1
		return &tUart3;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM4)
	{
#if UART4_FIFO_EN == 1
		return &tUart4;
#else
		return 0;
#endif
	}
	else
	{
		Error_Handler();
		return 0;
	}
}

/**
 * @brief  将COM端口号转换为 UART_HandleTypeDef* huartx
 */
UART_HandleTypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
	if (_ucPort == COM1)
	{
#if UART1_FIFO_EN == 1
		return &huart1;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM2)
	{
#if UART2_FIFO_EN == 1
		return &huart2;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM3)
	{
#if UART3_FIFO_EN == 1
		return &huart3;
#else
		return 0;
#endif
	}
	else if (_ucPort == COM4)
	{
#if UART4_FIFO_EN == 1
		return &huart3;
#else
		return 0;
#endif
	}
	else
	{
		/* 不做任何处理 */
		return 0;
	}
}

/**
 * @brief  UART DMA发送函数
 */
void bsp_uart_send_data(COM_PORT_E _ucPort, uint8_t *data, uint16_t len)
{
	Uart_Msg_T *pUart;

	pUart = ComToUart(_ucPort);
	if (pUart == 0)
	{
		return;
	}
	EnQueue(&pUart->tx_queue, data, len);
}

/**
 * @brief  USART中断回调函数
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART3)
	{
		__HAL_UART_CLEAR_OREFLAG(&huart3);
		EnQueue(&tUart3.rx_queue, uart3_rx_buffer, Size);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_rx_buffer, MAX_MSG_SIZE);
	}
}

/**
 * @brief  USART消息处理任务
 */
void Uart_Msg_Task(void)
{
	// 串口发送任务处理
	if (!QueueEmpty(&tUart3.tx_queue))
	{
		if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_BUSY) == RESET)
		{
			uint16_t len = 0;
			DeQueue(&tUart3.tx_queue, uart3_tx_buffer, &len);
			HAL_UART_Transmit_DMA(&huart3, uart3_tx_buffer, len);
		}
	}
	// 串口接收任务处理
	if (!QueueEmpty(&tUart3.rx_queue))
	{
		uint16_t len = 0;
		// uart3_msg_recv_process(tUart3.rx_queue.msg[tUart3.rx_queue.front], tUart3.rx_queue.size[tUart3.rx_queue.front]);
		DeQueueNORead(&tUart3.rx_queue, &len);
	}
}
