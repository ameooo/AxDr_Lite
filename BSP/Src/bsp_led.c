/*
 * @Description:
 * @Date: 2025-01-22 21:36:59
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-23 22:13:27
 * @LastEditors: 弈秋仙贝
 */

#include "bsp_led.h"

static struct LED_DEV *_led_head = NULL;

/**
 * @brief LED设备初始化
 * 
 * @param dev 
 * @param SetState 
 */
void LED_DEV_Init(struct LED_DEV *dev, void (*LED_SetState)(bool enable))
{
    struct LED_DEV *led_index = _led_head;
    memset(dev, 0, sizeof(struct LED_DEV));

    dev->SetState = LED_SetState;

    while (led_index)
    {
        if (led_index == dev)
        {
            return;
        }
        led_index = led_index->next;
    }
    dev->next = _led_head;
    _led_head = dev;
}

/**
 * @brief LED设备反初始化
 * 
 * @param dev 
 */
void LED_DEV_DeInit(struct LED_DEV *dev)
{
    struct LED_DEV *led_now = _led_head;
    struct LED_DEV *led_last;

    for (; led_now; led_now = led_now->next)
    {
        if (led_now == dev)
        {
            if (dev == _led_head)
            {
                _led_head = dev->next;
                return;
            }

            if (dev->next)
            {
                led_last->next = dev->next;
            }
            else
            {
                led_last->next = NULL;
            }
        }
        led_last = led_now;
    }
}

/**
 * @brief 
 * 
 * @param dev 
 * @param ontime 
 * @param offtime 
 * @param repeats 
 */
void LED_DEV_Ctrl(struct LED_DEV *dev, int ontime, int offtime, int repeats)
{
    dev->ontime = ontime;
    dev->offtime = offtime + ontime;
    dev->repeats = repeats;
    // dev->tick = 0;
    dev->count = 0;
    if (ontime == 0)
    {
        dev->SetState(false);
        dev->enable = false;
    }
}

/**
 * @brief LED设备处理函数
 * 
 */
void LED_DEV_Process(uint8_t ms)
{
    struct LED_DEV *dev;

    for (dev = _led_head; dev != NULL; dev = dev->next)
    {
		dev->tick += ms;
        if (dev->ontime == 0)
        {
            continue;
        }
        else if (dev->tick < dev->ontime)
        {
            if (!dev->enable)
            {
                dev->enable = true;
                dev->SetState(true);
            }
        }
        else if (dev->tick < dev->offtime)
        {
            if (dev->enable)
            {
                dev->enable = false;
                dev->SetState(false);
            }
        }
        else
        {
            dev->tick = 0;
            if (dev->repeats == LED_HOLD_STATE)
            {
            }
            else if (dev->repeats)
            {
                if (++dev->count >= dev->repeats)
                {
                    dev->ontime = 0;
                    dev->SetState(false);
                    dev->enable = false;
                }
            }
        }
    }
}
