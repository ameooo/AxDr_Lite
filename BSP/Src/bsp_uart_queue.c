/*
 * @Description: 
 * @Date: 2024-12-27 21:33:22
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-21 22:24:38
 * @LastEditors: 弈秋仙贝
 */

#include "bsp_uart_queue.h"

// 初始化队列
void InitQueue(Queue_Message_Type *queue)
{
    memset(queue->size, 0, sizeof(queue->size)); // 元素数量初始化为0
    queue->index = 0;                            // 当前元素个数初始化为0
    queue->front = 0;                            // 头部和尾部位置索引初始化为0
    queue->rear = 0;
}

// 判断队列是否为空
int QueueEmpty(Queue_Message_Type *queue)
{
    if (queue->front == queue->rear)
    {
        return 1; // 头部和尾部位置索引相等，队列为空
    }
    return 0; // 队列不为空
}

// 判断队列是否已满
int QueueFull(Queue_Message_Type *queue)
{
    if ((queue->rear + 1) % MAX_QUEUE_SIZE == queue->front)
    {
        return 1; // 头部和尾部位置索引相差1，队列已满
    }
    return 0; // 队列未满
}

// 入队
int EnQueue(Queue_Message_Type *queue, uint8_t *data, uint16_t len)
{
    if (QueueFull(queue))
    {
        return 1; // 队列已满，入队失败
    }
    memcpy(queue->msg[queue->rear], data, len);       // 将数据长度存入队列尾部
    queue->size[queue->rear] = len;                   // 将数据存入队列尾部
    queue->index++;                                   // 当前元素个数加1
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE; // 队列尾部位置索引加1，循环使用
    return 0;                                         // 入队成功
}

// 出队
int DeQueue(Queue_Message_Type *queue, uint8_t *data, uint16_t *len)
{
    if (QueueEmpty(queue))
    {
        return 1; // 队列为空，出队失败
    }
    memcpy(data, queue->msg[queue->front], queue->size[queue->front]); // 将队列头部数据取出
    *len = queue->size[queue->front];                                  // 将队列头部数据长度取出
    queue->index--;                                                    // 当前元素个数减1
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;                // 队列头部位置索引加1，循环使用
    return 0;                                                          // 出队成功
}
// 出队
int DeQueueNORead(Queue_Message_Type *queue, uint16_t *len)
{
    if (QueueEmpty(queue))
    {
        return 1; // 队列为空，出队失败
    }
    *len = queue->size[queue->front];                   // 将队列头部数据长度取出
    queue->index--;                                     // 当前元素个数减1
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE; // 队列头部位置索引加1，循环使用
    return 0;                                           // 出队成功
}

// 获取队列长度
uint16_t QueueLength(Queue_Message_Type *queue)
{
    // return (queue->rear - queue->front + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE; // 队列长度等于尾部位置索引减去头部位置索引，循环使用
    return queue->index;
}
