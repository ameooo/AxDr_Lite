/***
 * @Description:
 * @Date: 2025-01-22 21:36:59
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-22 21:43:05
 * @LastEditors: 弈秋仙贝
 */

#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

#define LED_HOLD_STATE 0xFFFF

struct LED_DEV
{
    uint16_t ontime;
    uint16_t offtime;
    uint16_t repeats;
    uint8_t count;
    uint8_t enable;
    uint16_t tick;
    void (*SetState)(bool enable);

    struct LED_DEV *next;
};

void LED_DEV_Init(struct LED_DEV *dev, void (*LED_SetState)(bool enable));
void LED_DEV_DeInit(struct LED_DEV *dev);
void LED_DEV_Ctrl(struct LED_DEV *dev, int ontime, int offtime, int repeat);
void LED_DEV_Process(uint8_t ms);

#endif /*__BSP_LED_H*/
