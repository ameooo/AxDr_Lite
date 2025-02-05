/*
 * @Description:
 * @Date: 2024-12-29 23:53:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-05 20:56:33
 * @LastEditors: 弈秋仙贝
 */

#include "AppTask.h"
#include "AppStateMachine.h"
#include "mc_task.h"
#include "controller.h"
#include "calibration.h"
#include "user_config.h"
#include "fast_math.h"

#include "bsp_timer.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "LedRGB.h"
#include "lcd.h"
#include "display.h"

static void Soft_Timer1_Callback(void *user_data);
static void Soft_Timer2_Callback(void *user_data);

static uint8_t Key1_GetLevel(void);
static uint8_t Key2_GetLevel(void);
static uint8_t Key3_GetLevel(void);
static uint8_t Key4_GetLevel(void);
static void Key_Event_Callback(uint8_t id, KEY_EVENT event);

static void Led1_SetLevel(bool level);
static void Led2_SetLevel(bool level);
static void Led3_SetLevel(bool level);

static struct BSP_TIMER soft_timer1;
static struct BSP_TIMER soft_timer2;

static struct LED_DEV led1;
static struct LED_DEV led2;
static struct LED_DEV led3;

static struct BSP_KEY key1;
static struct BSP_KEY key2;
static struct BSP_KEY key3;
static struct BSP_KEY key4;

mc_state_e MCState;

uint16_t PrevState = 0xFFFF;

uint8_t PR_state = 0xFF;
uint16_t CurrentPR = 0x0;

/**
 * @brief
 *
 */
void AppTask_Init(void)
{
    PrevState = 0xFFFF;

    __HAL_SPI_ENABLE(&hspi3);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    LCD_Init(); // LCD初始化
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);

    BSP_Timer_Init(&soft_timer1,
                   Soft_Timer1_Callback,
                   1,
                   TIMER_RUN_FOREVER,
                   TIMER_TYPE_SOFTWARE);
    BSP_Timer_Start(&soft_timer1);

    BSP_Timer_Init(&soft_timer2,
                   Soft_Timer2_Callback,
                   1,
                   TIMER_RUN_FOREVER,
                   TIMER_TYPE_SOFTWARE);
    BSP_Timer_Start(&soft_timer2);

    LED_DEV_Init(&led1, Led1_SetLevel);
    LED_DEV_Init(&led2, Led2_SetLevel);
    LED_DEV_Init(&led3, Led3_SetLevel);

    BSP_Key_Init(&key1, 1, Key1_GetLevel, KEY_PRESS_LOW);
    BSP_Key_Register(&key1, KEY_CLICK, Key_Event_Callback);
    BSP_Key_Start(&key1);

    BSP_Key_Init(&key2, 2, Key2_GetLevel, KEY_PRESS_LOW);
    BSP_Key_Register(&key2, KEY_CLICK, Key_Event_Callback);
    BSP_Key_Start(&key2);

    BSP_Key_Init(&key3, 3, Key3_GetLevel, KEY_PRESS_LOW);
    BSP_Key_Register(&key3, KEY_CLICK, Key_Event_Callback);
    BSP_Key_Start(&key3);

    BSP_Key_Init(&key4, 4, Key4_GetLevel, KEY_PRESS_LOW);
    BSP_Key_Register(&key4, KEY_CLICK, Key_Event_Callback);
    BSP_Key_Start(&key4);

    display();
}

static uint8_t Key1_GetLevel(void)
{
    return READ_KEY1;
}
static uint8_t Key2_GetLevel(void)
{
    return READ_KEY2;
}
static uint8_t Key3_GetLevel(void)
{
    return READ_KEY3;
}
static uint8_t Key4_GetLevel(void)
{
    return READ_KEY4;
}

static void Led1_SetLevel(bool level)
{
    if (level)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    }
}
static void Led2_SetLevel(bool level)
{
    if (level)
    {
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    }
}
static void Led3_SetLevel(bool level)
{
    if (level)
    {
        HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief
 *
 */
void MC_AppTask_Running(void)
{
    BSP_Timer_SoftTimerTask();

    AppStateMachine_process();
}

/**
 * @brief
 *
 * @param user_data
 */
static void Soft_Timer1_Callback(void *user_data)
{
    // RGB_LoopColor(6000);

    // 默认使用无感速度模式
    float speed = 0.0f;
    if (MCState == MC_STATE_RUNNING)
    {
        if (CurrentPR <= 300)
        {
            PR_state = 0;
            MC_SetControlMode(MOTOR_CONTROL_MODE_IDLE, 0.0f);
        }
        else if (CurrentPR >= 500)
        {
            if (PR_state > 0)
            {
                return;
            }
			speed = (CurrentPR - 500) * 0.000285f * 2000.0f * RPM2RADPS; // 1/3500 * max_speed
            MC_SetControlMode(MOTOR_CONTROL_MODE_VELOCITY_RAMP, speed);
        }
        else
        {
            // NULL
        }
    }
}

void Get_PotentiometerValue(uint16_t value)
{
    CurrentPR = value;
}

/**
 * @brief
 *
 * @param user_data
 */
static void Soft_Timer2_Callback(void *user_data)
{
    BSP_Key_Handler(1);
    LED_DEV_Process(1);

    MCState = MC_State_Process_Get();

    if (MCState != PrevState)
    {
        PrevState = MCState;

        switch (MCState)
        {
        case MC_STATE_IDLE:
        case MC_STATE_INIT:
            // RGB_WHITE(1);
            LED_DEV_Ctrl(&led1, 500, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led2, 0, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led3, 500, 500, LED_HOLD_STATE);
			
            break;

        case MC_STATE_DETECTING:
            // RGB_BLUE(1);
            LED_DEV_Ctrl(&led1, 0, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led2, 500, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led3, 500, 500, LED_HOLD_STATE);
            break;

        case MC_STATE_START:
        case MC_STATE_RUNNING:
            // RGB_GREEN(1);
            LED_DEV_Ctrl(&led1, 500, 0, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led2, 0, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led3, 500, 500, LED_HOLD_STATE);
            break;

        case MC_STATE_STOP:
            // RGB_Init(1);
            LED_DEV_Ctrl(&led1, 0, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led2, 0, 500, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led3, 500, 500, LED_HOLD_STATE);
            {
                LCD_DisplayMotorParam();
            }
            break;

        case MC_STATE_FAULT_NOW:
        case MC_STATE_ERROR:
            // RGB_RED(1);
            LED_DEV_Ctrl(&led1, 500, 0, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led2, 500, 0, LED_HOLD_STATE);
            LED_DEV_Ctrl(&led3, 500, 0, LED_HOLD_STATE);
            break;

        default:
            break;
        }
    }
}

/**
 * @brief
 *
 * @param id
 * @param event
 */
static void Key_Event_Callback(uint8_t id, KEY_EVENT event)
{
    int res = 0;

    switch (id)
    {
    case 1:
        if (event == KEY_CLICK)
        {
            if (MCState == MC_STATE_STOP)
            {
                res = MC_State_Process_Set(MC_STATE_START);
            }
            else if (MCState == MC_STATE_RUNNING)
            {
                res = MC_State_Process_Set(MC_STATE_STOP);
            }
            else if (MCState == MC_STATE_IDLE)
            {
                res = MC_State_Process_Set(MC_STATE_START);
            }
        }
        break;

    case 2:
        if (event == KEY_CLICK)
        {
            res = MC_State_Process_Set(MC_STATE_DETECTING);
        }
        break;

    case 3:
        if (event == KEY_CLICK)
        {
        }
        break;

    case 4:
        if (event == KEY_CLICK)
        {
        }
        break;

    default:
        break;
    }

    if (res != 0)
    {
    }
}
