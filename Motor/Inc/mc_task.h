/*** 
 * @Description: 
 * @Date: 2024-10-26 14:57:40
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-12-28 20:30:01
 * @LastEditors: 弈秋
 */

#ifndef _MC_TASK_H
#define _MC_TASK_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mgesc_state.h"

#define M1 0
#define M2 1


void MC_Motor_Init(void);
int MC_State_Process_Set(mc_state_e mc_state);
mc_state_e MC_State_Process_Get(void);
void Get_Motor_Info(void);
void MC_SetControlMode(motor_control_mode_e mode, float input);

void MC_ADC_IRQ_handler(void);


#endif /* _MC_TASK_H */
