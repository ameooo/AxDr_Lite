/*** 
 * @Description: 
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-14 23:19:13
 * @LastEditors: 弈秋
 */

#ifndef _MGFOC_H
#define _MGFOC_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mgesc_state.h"


#define QPR_CURRENT_COMP 0.2f
#define QPR_CURRENT_COMP_12 (0.5f * QPR_CURRENT_COMP)
#define TEST_LIMIT_VOLTAGE 0.5f

void mgesc_current_phase(mc_motor_typedef *tmotor, float angle, float vel);
void mgesc_current_calc(mc_motor_typedef *tmotor, float curr_d, float curr_q);
void mgesc_voltage_output(mc_motor_typedef *tmotor, float volt_d, float volt_q);

#endif
