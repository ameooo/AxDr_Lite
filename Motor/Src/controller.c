/*
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-05 20:15:01
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "controller.h"
#include "mgfoc.h"
#include "user_config.h"
#include "fast_math.h"

void input_reset(motor_controller_typedef *ctrl);

void motor_controller_init(mc_motor_typedef *tmotor)
{
    motor_controller_typedef *tcontroller = &tmotor->controller;

    tcontroller->mode_prev = MOTOR_CONTROL_MODE_IDLE;
    tcontroller->input_torque = 0;
    tcontroller->input_velocity = 0;
    tcontroller->input_position = 0;

    tcontroller->open_loop_power_d = 0;
    tcontroller->open_loop_power_q = 0;
    tcontroller->open_loop_velocity = 0;
    tcontroller->open_loop_position = 0;

    tcontroller->open_loop_power_d = 0.0f;
    tcontroller->open_loop_velocity = 10.0f;
}

void set_controller_open_mode(mc_motor_typedef *tmotor, motor_control_mode_e mode, float power_d, float power_q, float velocity, float position)
{
    motor_controller_typedef *tcontroller = &tmotor->controller;

    if (mode != tcontroller->mode_prev)
    {
        tmotor->config.control_mode = mode;
        input_reset(&tmotor->controller);
    }

    switch (mode)
    {
    case MOTOR_CONTROL_MODE_VOLTAGE_OPENLOOP:
        tcontroller->open_loop_power_d = power_d;
        tcontroller->open_loop_power_q = power_q;
        tcontroller->open_loop_velocity = velocity;
        tcontroller->open_loop_position = position;
        break;

    case MOTOR_CONTROL_MODE_CURRENT_OPENLOOP:
        tcontroller->open_loop_power_d = power_d;
        tcontroller->open_loop_power_q = power_q;
        tcontroller->open_loop_velocity = velocity;
        tcontroller->open_loop_position = position;
        break;
    default:
        break;
    }
}

void set_controller_close_mode(mc_motor_typedef *tmotor, motor_control_mode_e mode, float setpoint)
{
    motor_controller_typedef *tcontroller = &tmotor->controller;

    if (mode != tcontroller->mode_prev)
    {
        tmotor->config.control_mode = mode;
        input_reset(&tmotor->controller);
        // 切换模式时，重设目标值??
        tcontroller->current_setpoint = tmotor->foc.iq_filter;
        tcontroller->velocity_setpoint = tmotor->foc.mech_vel;
        tcontroller->position_setpoint = tmotor->foc.mech_pos;
    }

    switch (mode)
    {
    case MOTOR_CONTROL_MODE_IDLE:
        tcontroller->current_setpoint = 0.0f;
        tcontroller->vel_integral = 0.0f;
        input_reset(&tmotor->controller);
        break;

    case MOTOR_CONTROL_MODE_CURRENT:
        tcontroller->input_current = setpoint;
        break;

    case MOTOR_CONTROL_MODE_TORQUE:
        tcontroller->input_current = setpoint * tmotor->config.torque_constant;
        break;

    case MOTOR_CONTROL_MODE_VELOCITY:
    case MOTOR_CONTROL_MODE_VELOCITY_RAMP:
        utils_truncate_number(&setpoint, -tmotor->config.velocity_limit, +tmotor->config.velocity_limit);
        tcontroller->input_velocity = setpoint;
        break;

    case MOTOR_CONTROL_MODE_POS:
    case MOTOR_CONTROL_MODE_POS_RAMP:
        tcontroller->input_position = setpoint;
        break;

    default:
        break;
    }
}

void input_reset(motor_controller_typedef *tcontroller)
{
    tcontroller->input_torque = 0;
    tcontroller->input_velocity = 0;
    tcontroller->input_position = 0;
    tcontroller->input_current = 0;
}

float controller_loop(mc_motor_typedef *tmotor, float mech_pos, float mech_vel)
{
    motor_controller_typedef *tcontroller = &tmotor->controller;

    switch (tmotor->config.control_mode)
    {
    case MOTOR_CONTROL_MODE_IDLE:
        tcontroller->input_torque = 0.0f;
        tcontroller->input_velocity = 0.0f;
        tcontroller->input_position = 0.0f;
        tcontroller->torque_setpoint = 0.0f;
        break;

    case MOTOR_CONTROL_MODE_CURRENT:
    case MOTOR_CONTROL_MODE_TORQUE:
    {
        // float max_step_size = ABS(CONTROLLER_PERIOD * tmotor->config.current_ramp_rate);
        // float full_step = tcontroller->input_current - tcontroller->current_setpoint;
        // float step = CLAMP(full_step, -max_step_size, max_step_size);
        // tcontroller->current_setpoint += step;
    }
    break;

    case MOTOR_CONTROL_MODE_VELOCITY:
    case MOTOR_CONTROL_MODE_VELOCITY_RAMP:
    {
        float max_step_size = ABS(CONTROLLER_PERIOD * tmotor->config.velocity_ramp_rate);
        float full_step = tcontroller->input_velocity - tcontroller->velocity_setpoint;
        float step = CLAMP(full_step, -max_step_size, max_step_size);
        tcontroller->velocity_setpoint += step;
    }
    break;

    case MOTOR_CONTROL_MODE_POS:
    case MOTOR_CONTROL_MODE_POS_RAMP:
    {
        float max_step_size = ABS(CONTROLLER_PERIOD * tmotor->config.position_ramp_rate);
        float delta_pos = tcontroller->input_position - mech_pos;
        utils_norm_angle_rad(&delta_pos);
        tcontroller->position_setpoint = CLAMP(delta_pos, -max_step_size, max_step_size);
        tcontroller->velocity_setpoint = tcontroller->position_setpoint * tmotor->config.position_kp;
    }
    break;

    default:
        break;
    }

    float vel_des = tcontroller->velocity_setpoint;
    // Velocity limiting
    vel_des = CLAMP(vel_des, -tmotor->config.velocity_limit, +tmotor->config.velocity_limit);
    float current_ref = 0.0f;
    if (tmotor->config.control_mode >= MOTOR_CONTROL_MODE_VELOCITY)
    {
        // float v_err = 0.0f;
        float prop_vel = 0.0f;
        tcontroller->delta_vel = (vel_des - mech_vel);
        prop_vel = tcontroller->delta_vel * tmotor->config.velocty_kp;
        tcontroller->vel_integral += (tmotor->config.velocty_ki * tcontroller->delta_vel * CURRENT_CONTROL_PERIOD);

        float max_vel_integral = tmotor->config.current_limit - prop_vel;
        float min_vel_integral = -tmotor->config.current_limit - prop_vel;
        if (tcontroller->vel_integral > max_vel_integral)
        {
            tcontroller->vel_integral = max_vel_integral;
        }
        if (tcontroller->vel_integral < min_vel_integral)
        {
            tcontroller->vel_integral = min_vel_integral;
        }

        current_ref = prop_vel + tcontroller->vel_integral;
    }
    else
    {
        tcontroller->delta_vel = 0.0f;
        tcontroller->vel_integral = 0.0f;
        current_ref = tcontroller->input_current;
    }

    // current limiting
    float max_current = tmotor->config.current_limit;
    if (current_ref > +max_current)
    {
        current_ref = +max_current;
    }
    if (current_ref < -max_current)
    {
        current_ref = -max_current;
    }

    float max_step_curr = ABS(CONTROLLER_PERIOD * tmotor->config.current_ramp_rate);
    float full_curr_step = current_ref - tcontroller->current_setpoint;
    float step = CLAMP(full_curr_step, -max_step_curr, max_step_curr);
    tcontroller->current_setpoint += step;

    tcontroller->iq_ref = tcontroller->current_setpoint;
    return tcontroller->iq_ref;
}
