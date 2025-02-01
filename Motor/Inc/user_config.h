/***
 * @Description:
 * @Date: 2024-12-27 18:06:37
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-12-28 23:00:59
 * @LastEditors: 弈秋
 */
/***
 * @Description:
 * @Author: 弈秋
 * @Date: 2024-10-26 14:36:21
 * @LastEditTime: 2024-12-28 22:58:57
 * @LastEditors: 弈秋
 */

#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mgesc_state.h"

#define USER_CONFIG_ADDR 0x0807A000
#define USER_CONFIG_FLAG 0xAAAA5555

#define DEADTIME_COMP DEAD_TIME
#define MGESC_PWM_ARR PWM_ARR
#define MGESC_PWM_ARR_HALF (MGESC_PWM_ARR / 2)
#define MGESC_PWM_FREQ 20000                                 // [Hz]
#define MGESC_PWM_PERIOD (1.0f / MGESC_PWM_FREQ)             // [s]
#define CURRENT_CONTROL_FREQ (MGESC_PWM_FREQ)                // 1div
#define CURRENT_CONTROL_PERIOD (1.0f / CURRENT_CONTROL_FREQ) // [s]
#define CURRENT_BANDWIDTH (400)                              // [Hz]

#define CONTROLLER_PERIOD CURRENT_CONTROL_PERIOD

// mt6816
#define ENCODER_CPR 16384
#define ENCODER_CPR_DIV_2 (ENCODER_CPR >> 1)
#define ENCODER_CPR_TO_RAD (6.28318530716f / ENCODER_CPR)
#define ENCODER_SAMPLE_PERIOD CURRENT_CONTROL_PERIOD

#define V_REG (3.3f)
#define CURRENT_SHUNT_RES (0.005f)
#define CURRENT_AMP_GAIN (20.0f)
#define VIN_R1 (2000.0f)
#define VIN_R2 (15000.0f + 15000.0f)
#define FAC_CURRENT ((V_REG / 4095.0f) / (CURRENT_SHUNT_RES * CURRENT_AMP_GAIN))
#define VOLTAGE_TO_ADC_FACTOR (((VIN_R2 + VIN_R1) / VIN_R1) * (V_REG / 4095.0f))

#define MGESC_INJECT_VOLTAGE (3.0f)
#define DEFAULT_CALIBRATION_ERPM (2000.0f)      // [2000rpm]
#define DEFAULT_CALIBRATION_FREQUENCY (1000.0f) // [1kHz]
#define DEFAULT_CALIBRATION_CURRENT (10.0f)     // [A]

//
#define DIR_FORWARD 0                          // 正向
#define DIR_REVERSE 1                          // 反向
#define DEFAULT_CURRENT_SAMPLE_DIR DIR_REVERSE // 电流采样方向取反
#define DEFAULT_ENCODER_DIR DIR_FORWARD        // related to v&w phase
#define DEFAULT_ADC_IA_OFFSET 2061
#define DEFAULT_ADC_IB_OFFSET 2056
#define DEFAULT_ADC_IC_OFFSET 2086
#define DEFAULT_ADC_IBUS_OFFSET 2038

#define MOTOR_TYPE_2312s 1
#define MOTOR_TYPE_A12 0
#define MOTOR_TYPE_HALL1 0
#define MOTOR_TYPE_PMSM60 0
#define MOTOR_TYPE_FAN 0

#if MOTOR_TYPE_2312s // 2312s
#define DEFAULT_MOTOR_PP 7
#define DEFAULT_MOTOR_R 0.11f
#define DEFAULT_MOTOR_Ld 0.0000290f
#define DEFAULT_MOTOR_Lq 0.0000285f
#define DEFAULT_FLUX_LINKAGE 0.00103f
#define DEFAULT_ENCODER_OFFSET 1966
#elif MOTOR_TYPE_A12 // A12
#define DEFAULT_MOTOR_PP 20
#define DEFAULT_MOTOR_R 0.061f
#define DEFAULT_MOTOR_Ld 0.0000140f
#define DEFAULT_MOTOR_Lq 0.0000230f
#define DEFAULT_FLUX_LINKAGE 0.0028f
#define DEFAULT_ENCODER_OFFSET 460
#elif MOTOR_TYPE_HALL1 // PMSM60
#define DEFAULT_MOTOR_PP 4
#define DEFAULT_MOTOR_R 0.74f
#define DEFAULT_MOTOR_Ld 0.001400f
#define DEFAULT_MOTOR_Lq 0.001410f
#define DEFAULT_FLUX_LINKAGE 0.01214f
#define DEFAULT_ENCODER_OFFSET 0
#elif MOTOR_TYPE_PMSM60 // PMSM60
#define DEFAULT_MOTOR_PP 4
#define DEFAULT_MOTOR_R 0.16f
#define DEFAULT_MOTOR_Ld 0.000180f
#define DEFAULT_MOTOR_Lq 0.000195f
#define DEFAULT_FLUX_LINKAGE 0.0145f
#define DEFAULT_ENCODER_OFFSET 0
#elif MOTOR_TYPE_FAN
#define DEFAULT_MOTOR_PP 1
#define DEFAULT_MOTOR_R 0.56f
#define DEFAULT_MOTOR_Ld 0.000101f
#define DEFAULT_MOTOR_Lq 0.000129f
#define DEFAULT_FLUX_LINKAGE 0.00152f
#define DEFAULT_ENCODER_OFFSET 0
#endif
#define DEFAULT_MAX_CURRENR 12.0f    // 限制电流[A]
#define DEFAULT_MAX_VELOCITY 500.0f  // 校准速度[RPM]
#define DEFAULT_INERTIA 2.3e-6f      // 转动惯量 [Nm*m^2]
#define DEFAULT_TORQUE_CONSTANT 1.0f // 转矩常数 [Nm/A]

// default pi & acc
#define DEFAULT_VEL_KP 0.1f
#define DEFAULT_VEL_KI 1.0f
#define DEFAULT_POS_KP 0.5f
#define DEFAULT_TORQUE_RAMP_RATE 5.0f
#define DEFAULT_VELOCITY_RAMP_RATE 10000.0f
#define DEFAULT_POSITION_RAMP_RATE 30.0f
#define DEFAULT_CURRENT_RAMP_RATE 300.0f

#define DEFAULT_PLL_BANDWIDTH 1000

#define OFFSET_LUT_SIZE 128

void motor_config_init(motor_config_typedef *tconfig);

void Set_MotorElecParam(motor_config_typedef *tconfig, float rs, float ld, float lq, float flux);
void Set_CurrentLoopBandwidth(motor_config_typedef *tconfig, float bandwidth);
void Set_MotorMechParam(motor_config_typedef *tconfig, float inertia);
void Set_VelocityLoopParam(motor_config_typedef *tconfig, float kp, float ki);
int Set_LoopBandwidth(motor_config_typedef *tconfig, float bandwidth, float damp, uint8_t type);

void LCD_DisplayMotorParam(void);
void Write_UserConfig(motor_config_typedef *tconfig);

#endif
