/*
 * @Description:
 * @Date: 2024-12-27 18:06:36
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-09 23:26:28
 * @LastEditors: 弈秋
 */

#include "HwIsr.h"
#include "AppStateMachine.h"
#include "AppTask.h"
#include "mc_task.h"
#include "bsp_timer.h"

#include "adc.h"
#include "tim.h"
#include "spi.h"
#include "usart.h"

uint32_t TIM1_Count[6] = {0};
uint32_t TIM4_Count[6] = {0};

#define ADC_BUFFER_SIZE 2
uint32_t adcBuffer[ADC_BUFFER_SIZE];

void MC_Peripheral_Start(void)
{
    // HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL); // 开启编码器模式
    // HAL_TIM_Base_Start_IT(&htim3);                  // 开启编码器的中断

    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start(&htim4);

    __HAL_SPI_ENABLE(&hspi1);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_Delay(1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_Delay(1);
    HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *)adcBuffer, ADC_BUFFER_SIZE);
}

uint16_t EncoderCount = 0;
uint16_t Diretion = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
    }
    else if (htim->Instance == TIM6)
    {
        Diretion = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);
        EncoderCount = __HAL_TIM_GET_COUNTER(&htim3);
        BSP_Timer_Handler(1);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        TIM4_Count[3] = TIM6->CNT;
        uint16_t tempValue = (adcBuffer[0] >> 16) & 0xFFFF;
        Get_PotentiometerValue(tempValue);
        HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *)adcBuffer, ADC_BUFFER_SIZE);
    }
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        TIM1_Count[0] = TIM1->CNT;
        TIM4_Count[0] = TIM4->CNT;
        MC_ADC_IRQ_handler();
        TIM1_Count[1] = TIM1->CNT;
        TIM4_Count[1] = TIM4->CNT;
        TIM1_Count[2] = TIM1_Count[1] - TIM1_Count[0];
        TIM4_Count[2] = TIM4_Count[1] - TIM4_Count[0];
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // if (GPIO_Pin == ABZ_Z_Pin)
    // {
    //     __HAL_TIM_SetCounter(&htim3, 0); // 清空计数值
    // }
}
