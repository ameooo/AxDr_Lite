/*** 
 * @Description: 
 * @Date: 2024-12-29 23:53:49
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-21 21:38:14
 * @LastEditors: 弈秋仙贝
 */

#ifndef __APPTASK_H
#define __APPTASK_H

#include "main.h"

#define READ_KEY1 HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)
#define READ_KEY2 HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)
#define READ_KEY3 HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin)
#define READ_KEY4 HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin)

void AppTask_Init(void);
void MC_AppTask_Running(void);

void Get_PotentiometerValue(uint16_t value);

#endif /* __APPTASK_H */
/********************************END OF FILE**********************************/
