/**
 *
 ******************************************************************************
 * @file           : controller.h
 * @brief          :
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 *
 *  Created on: 22 Feb 2024
 *      Author: amoeee
 */
#ifndef _CONTROLLER_H
#define _CONTROLLER_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mgesc_state.h"

void motor_controller_init(mc_motor_typedef *tmotor);

void set_controller_open_mode(mc_motor_typedef *tmotor, motor_control_mode_e mode, float power_d, float power_q, float velocity, float position);
void set_controller_close_mode(mc_motor_typedef *tmotor, motor_control_mode_e mode, float setpoint);

float controller_loop(mc_motor_typedef *tmotor, float mech_pos, float mech_vel);
// float controller_open_loop(mc_motor_typedef *tmotor, float elec_pos, float mech_vel);

#endif /* _CONTROLLER_H */
/********************************END OF FILE**********************************/
