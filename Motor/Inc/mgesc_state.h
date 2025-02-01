/***
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-12-28 14:40:03
 * @LastEditors: 弈秋
 */

#ifndef _MGESC_STATE_H
#define _MGESC_STATE_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"

typedef enum
{
	MC_STATE_IDLE = 0,
	MC_STATE_INIT,
	MC_STATE_DETECTING,
	MC_STATE_START,
	MC_STATE_RUNNING,
	MC_STATE_STOP,
	MC_STATE_FAULT_NOW,
	MC_STATE_ERROR,
} mc_state_e;

typedef enum
{
	FOC_MODE_IDLE = 0,
	FOC_MODE_OPENLOOP,
	FOC_MODE_SENSORLESS,
	FOC_MODE_ENCODER,
	FOC_MODE_TEST,
	FOC_MODE_TEST_HFI,
} foc_run_mode_e;

typedef enum
{
	MOTOR_SENSOR_MODE_SENSORLESS,
	MOTOR_SENSOR_MODE_HALL,
	MOTOR_SENSOR_MODE_OPENLOOP,
	MOTOR_SENSOR_MODE_ABSOLUTE_ENCODER,
	MOTOR_SENSOR_MODE_INCREMENTAL_ENCODER,
	MOTOR_SENSOR_MODE_MT6816,
} motor_sensor_mode_e;

typedef enum
{
	MOTOR_CONTROL_MODE_IDLE = 0,
	MOTOR_CONTROL_MODE_VOLTAGE_OPENLOOP,
	MOTOR_CONTROL_MODE_CURRENT_OPENLOOP,
	MOTOR_CONTROL_MODE_CURRENT,
	MOTOR_CONTROL_MODE_TORQUE,
	MOTOR_CONTROL_MODE_VELOCITY,
	MOTOR_CONTROL_MODE_VELOCITY_RAMP,
	MOTOR_CONTROL_MODE_POS,
	MOTOR_CONTROL_MODE_POS_RAMP,
} motor_control_mode_e;

typedef enum
{
	CALIBRATE_IDLE = 0,
	CALIBRATE_ADC_OFFSET,
	CALIBRATE_MOTOR_PARAM,
	CALIBRATE_ENCODER,
	CALIBRATE_HALL_SENSOR,
	CALIBRATE_COGGING_EFFECT,
} motor_calibration_mode_e;

typedef enum
{
	CALIBRATE_START = 0,
	CALIBRATE_RESISTANCE,
	CALIBRATE_INDUCTANCE,
	CALIBRATE_FLUX_LINKAGE,
	CALIBRATE_INERTIA,
	CALIBRATE_DONE,
	CALIBRATE_ERROR,
} motor_calibration_state_e;

typedef struct
{
	// motor_control_mode_e mc_mode;
	motor_control_mode_e mode_prev;

	float open_loop_power_d;
	float open_loop_power_q;
	float open_loop_position;
	float open_loop_velocity;

	float input_torque;
	float input_velocity;
	float input_position;
	float input_current;

	float velocity_setpoint;
	float position_setpoint;
	float torque_setpoint;
	float current_setpoint;

	float delta_vel;
	float vel_integral;
	float iq_ref;
} motor_controller_typedef;

typedef struct
{
	int raw;
	int enc;
	int cnt;
	int pre_cnt[5];
	int turns;
	float delta_cnt;
	float mech_rad;
	float elec_rad;
	float elec_vel; // rad/s
	float mech_vel; // rad/s
	float mech_rpm; // r/min
	float mech_pos;
} motor_encoder_typedef;

// typedef struct
// {
	// motor_calibration_mode_e calib_mode;
	// motor_calibration_state_e calib_state;

	// uint16_t meas_step;
	// uint32_t calib_count;

	// calibrate Rs
	// float calib_current;
	// float calib_voltage_d;
	// float calib_vd_half;

	// // calibrate Ld Lq
	// float current_magnitude;
	// float current_phase;
	// float meas_l_voltage_offset;
	// float meas_bot_current_ld;
	// float meas_top_current_ld;
	// uint16_t meas_ld_count;

	// float meas_bot_current_lq;
	// float meas_top_current_lq;
	// uint16_t meas_lq_count;

	// // calibrate flux
	// float meas_flux_current;
	// float meas_flux_velocity;
// } motor_calibrate_typedef;

typedef struct
{
	uint32_t config_valid;

	motor_sensor_mode_e sensor_mode;
	motor_control_mode_e control_mode;

	int current_sample_dir; // attention: errors can cause current takeoff
	int motor_spin_dir;		// motor_spin_dir or encoder_dir
	int encoder_dir;		// attention: same, errors can cause velocity takeoff
	int encoder_offset;
	int16_t adc_offsets[4];
	uint32_t encoder_cpr;
	float cpr_to_radian;

	int motor_pole_pairs;
	float motor_rs;
	float motor_ld;		   // ld
	float motor_lq;		   // lq
	float motor_ldlq_diff; // lq - ld
	float motor_flux_linkage;
	float max_duty_cycle;
	float calibrate_frequency; // [Hz]
	float calibrate_current;   // [A]
	float current_limit;	   // [A]
	float velocity_limit;	   // [r/min]
	float inertia;
	float torque_constant; //

	// protect
	float v_bus_limit;
	float i_bus_limit;

	// loop param
	// float current_kp;
	// float current_ki;
	float d_current_kp;
	float d_current_ki;
	float q_current_kp;
	float q_current_ki;
	float velocty_kp;
	float velocty_ki;
	float position_kp;

	// accelerates
	float torque_ramp_rate;
	float velocity_ramp_rate;
	float position_ramp_rate;
	float current_ramp_rate;

	uint32_t crc_check;
} motor_config_typedef;

typedef struct
{
	float Rs;
	float Ld;
	float Lq;
	float flux;
	float dt;

	float flux_kp;
	float flux_ki;
	float pll_kp;
	float pll_ki;
	float pll_int;
	float pll_theta;
	float estimate_elec_vel;
	float estimate_mech_vel;

	float Ialpha_filter;
	float Ibeta_filter;
	float Ualpha_integral;
	float Ubeta_integral;
	float Err_alpha;
	float Err_beta;
	float PhiEstimate;
	float theta;
} flux_observer_typedef;

typedef struct
{
	float Ialpha_filter;
	float Ibeta_filter;
	float lambda;
	float alpha_a;
	float alpha_b;
	float alpha_lpf;
	float emf_d;
	float emf_q;
	float omega;
	float theta;
	float estimate_elec_vel;
	float estimate_mech_vel;
} scvm_observer_typedef;

typedef struct
{
	int16_t adc_raw[5];

	float mech_pos, mech_vel;
	float elec_pos, elec_vel;
	
	float ia, ib, ic, ibus;
	float va, vb, vc, vbus;
	float va_filter, vb_filter, vc_filter, vbus_filter;
	float phase, phase_vel;
	float phase_sin, phase_cos;
	float id_ref, iq_ref;
	float delta_id, delta_iq;
	float i_alpha, i_beta;
	float i_abs;
	float i_abs_filter;
	float id, iq;
	float id_filter, iq_filter;
	float vd_int, vq_int;
	float vd, vq;
	float v_alpha, v_beta;
	uint32_t dtc_a, dtc_b, dtc_c;
	uint32_t svm_sector;
} foc_state_typedef;

typedef struct
{
	// TIM_TypeDef *TIM_;

	uint32_t state_count;

	mc_state_e mc_state;
	foc_run_mode_e foc_mode;
	motor_calibration_state_e calib_state;

	motor_controller_typedef controller;
	motor_encoder_typedef encoder;
	// motor_calibrate_typedef calibrate;
	motor_config_typedef config;
	flux_observer_typedef observer;
	scvm_observer_typedef scvm;
	foc_state_typedef foc;
} mc_motor_typedef;

#endif
