/*** 
 * @Description: 
 * @Author: 弈秋
 * @Date: 2024-10-26 14:36:21
 * @LastEditTime: 2024-10-26 16:53:38
 * @LastEditors: 弈秋
 */

#ifndef _ENCODER_H
#define _ENCODER_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mgesc_state.h"

// #define ENCODER_CPR_TO_RAD (M_2PI / ENCODER_CPR)

int sensor_absolute_read(mc_motor_typedef *tmotor);

#endif
