/*** 
 * @Description: 
 * @Date: 2024-12-27 21:33:22
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-21 22:24:45
 * @LastEditors: 弈秋仙贝
 */

#ifndef _BSP_UART_QUEUE_H
#define _BSP_UART_QUEUE_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef uint8_t DataType;
#define MAX_QUEUE_SIZE 5 // 队列的最大数据包数
#define MAX_MSG_SIZE 256 // 每个数据包的最大长度

typedef struct Queue
{
    DataType msg[MAX_QUEUE_SIZE][MAX_MSG_SIZE]; // 队列的数组
    uint16_t size[MAX_QUEUE_SIZE];              // 队列中元素的长度
    uint16_t index;                             // 队列中元素的个数
    uint16_t front, rear;                       // 队列的头部和尾部位置索引
} Queue_Message_Type;

void InitQueue(Queue_Message_Type *queue);
int QueueEmpty(Queue_Message_Type *queue);
int QueueFull(Queue_Message_Type *queue);
int EnQueue(Queue_Message_Type *queue, uint8_t *data, uint16_t len);
int DeQueue(Queue_Message_Type *queue, uint8_t *data, uint16_t *len);
int DeQueueNORead(Queue_Message_Type *queue, uint16_t *len);
uint16_t QueueLength(Queue_Message_Type *queue);

#endif
