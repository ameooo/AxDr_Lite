/*
 * @Description:
 * @Date: 2024-12-27 18:06:37
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-31 22:13:59
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "mgfoc.h"
#include "mgesc_calc.h"
#include "calibration.h"
#include "encoder.h"
#include "user_config.h"
#include "fast_math.h"

#include "adc.h"
#include "spi.h"

/**
 * @brief motor phase
 */
void mgesc_current_phase(mc_motor_typedef *tmotor, float angle, float vel)
{
    foc_state_typedef *tfoc = &tmotor->foc;

    tfoc->phase = angle;
    tfoc->phase_vel = vel;
    // tfoc->phase_sin = user_sin_f32(tfoc->phase);
    // tfoc->phase_cos = user_cos_f32(tfoc->phase);
    fast_sin_cos(tfoc->phase, &tfoc->phase_sin, &tfoc->phase_cos);
}

/**
 * @brief
 */
void mgesc_current_calc(mc_motor_typedef *tmotor, float curr_d, float curr_q)
{
    foc_state_typedef *tfoc = &tmotor->foc;

    tfoc->id_ref = curr_d;
    tfoc->iq_ref = curr_q;

    // Park transform
    tfoc->id = tfoc->i_alpha * tfoc->phase_cos + tfoc->i_beta * tfoc->phase_sin;
    tfoc->iq = -tfoc->i_alpha * tfoc->phase_sin + tfoc->i_beta * tfoc->phase_cos;
    tfoc->i_abs = NORM2_f(tfoc->id, tfoc->iq);

    // filter
    UTILS_LP_FAST(tfoc->id_filter, tfoc->id, 0.25f);
    UTILS_LP_FAST(tfoc->iq_filter, tfoc->iq, 0.25f);
    UTILS_LP_FAST(tfoc->i_abs_filter, tfoc->i_abs, 0.25f);

    // Calculate current PI
    tfoc->delta_id = (tfoc->id_ref - tfoc->id);
    tfoc->delta_iq = (tfoc->iq_ref - tfoc->iq);
    float prop_d = tfoc->delta_id * tmotor->config.d_current_kp;
    float prop_q = tfoc->delta_iq * tmotor->config.q_current_kp;

    // Integral error
    tfoc->vd_int += tfoc->delta_id * tmotor->config.d_current_ki * MGESC_PWM_PERIOD;
    tfoc->vq_int += tfoc->delta_iq * tmotor->config.q_current_ki * MGESC_PWM_PERIOD;

    // Voltage limit
    float max_v_mag = tmotor->config.max_duty_cycle * ONE_BY_SQRT3 * tfoc->vbus_filter;

    float max_d_integral = max_v_mag - prop_d;
    float min_d_integral = -max_v_mag - prop_d;
    float max_q_integral = max_v_mag - prop_q;
    float min_q_integral = -max_v_mag - prop_q;
    if (tfoc->vd_int > max_d_integral)
    {
        tfoc->vd_int = max_d_integral;
    }
    else if (tfoc->vd_int < min_d_integral)
    {
        tfoc->vd_int = min_d_integral;
    }
    if (tfoc->vq_int > max_q_integral)
    {
        tfoc->vq_int = max_q_integral;
    }
    else if (tfoc->vq_int < min_q_integral)
    {
        tfoc->vq_int = min_q_integral;
    }

    tfoc->vd = prop_d + tfoc->vd_int;
    tfoc->vq = prop_q + tfoc->vq_int;

    // float dec_vd = 0.0;
    // float dec_vq = 0.0;
    // float dec_bemf = 0.0;
    // dec_vd = tfoc->phase_vel * tmotor->config.motor_lq * tfoc->iq_filter;
    // dec_vq = tfoc->phase_vel * tmotor->config.motor_ld * tfoc->id_filter;
    // dec_bemf = tfoc->phase_vel * tmotor->config.motor_flux_linkage;

    // tfoc->vd -= dec_vd;
    // tfoc->vq += dec_vq + dec_bemf;

    utils_saturate_vector_2d((float *)&tfoc->vd, (float *)&tfoc->vq, max_v_mag);

    // 测试使用，用了限制最大输出电压
    // if (tfoc->vd > TEST_LIMIT_VOLTAGE)
    // {
    //     tfoc->vd = TEST_LIMIT_VOLTAGE;
    // }
    // else if (tfoc->vd < (-TEST_LIMIT_VOLTAGE))
    // {
    //     tfoc->vd = (-TEST_LIMIT_VOLTAGE);
    // }
    // if (tfoc->vq > TEST_LIMIT_VOLTAGE)
    // {
    //     tfoc->vq = TEST_LIMIT_VOLTAGE;
    // }
    // else if (tfoc->vq < (-TEST_LIMIT_VOLTAGE))
    // {
    //     tfoc->vq = (-TEST_LIMIT_VOLTAGE);
    // }

    // Inverse Park transform: transforms the (normalized) voltages from the rotor reference frame to the stator frame
    tfoc->v_alpha = tfoc->phase_cos * tfoc->vd - tfoc->phase_sin * tfoc->vq;
    tfoc->v_beta = tfoc->phase_cos * tfoc->vq + tfoc->phase_sin * tfoc->vd;

    // Calculate the duty cycles for all the phases. This also injects a zero modulation signal to
    // be able to fully utilize the bus voltage. See https://microchipdeveloper.com/mct5001:start
    const float pwm_normalize = (1.5f * MGESC_PWM_ARR) / tfoc->vbus_filter;
    // const float voltage_normalize = 1.5f / tfoc->vbus_filter;
    // float mod_alpha_raw = (tfoc->v_alpha * voltage_normalize);
    // float mod_beta_raw = (tfoc->v_beta * voltage_normalize);
    foc_svm(tfoc->v_alpha, tfoc->v_beta, pwm_normalize, &tfoc->dtc_a, &tfoc->dtc_b, &tfoc->dtc_c, &tfoc->svm_sector);

    TIM1->CCR1 = tfoc->dtc_a;
    if (tmotor->config.motor_spin_dir == DIR_FORWARD)
    {
        TIM1->CCR2 = tfoc->dtc_b;
        TIM1->CCR3 = tfoc->dtc_c;
    }
    else
    {
        TIM1->CCR2 = tfoc->dtc_c;
        TIM1->CCR3 = tfoc->dtc_b;
    }
}

void mgesc_voltage_output(mc_motor_typedef *tmotor, float volt_d, float volt_q)
{
    foc_state_typedef *tfoc = &tmotor->foc;

    // Park transform
    tfoc->id = tfoc->i_alpha * tfoc->phase_cos + tfoc->i_beta * tfoc->phase_sin;
    tfoc->iq = -tfoc->i_alpha * tfoc->phase_sin + tfoc->i_beta * tfoc->phase_cos;
    tfoc->i_abs = NORM2_f(tfoc->id, tfoc->iq);

    tfoc->vd = volt_d;
    tfoc->vq = volt_q;

    // debug use: limit voltage
    // utils_truncate_number(&m_vd, -TEST_LIMIT_VOLTAGE, TEST_LIMIT_VOLTAGE);
    // utils_truncate_number(&m_vq, -TEST_LIMIT_VOLTAGE, TEST_LIMIT_VOLTAGE);

    // Inverse Park transform: transforms the (normalized) voltages from the rotor reference frame to the stator frame
    tfoc->v_alpha = tfoc->phase_cos * tfoc->vd - tfoc->phase_sin * tfoc->vq;
    tfoc->v_beta = tfoc->phase_cos * tfoc->vq + tfoc->phase_sin * tfoc->vd;

    // Calculate the duty cycles for all the phases. This also injects a zero modulation signal to
    // be able to fully utilize the bus voltage. See https://microchipdeveloper.com/mct5001:start
    // const float pwm_normalize = (1.5f * MGESC_PWM_ARR) / tfoc->vbus_filter;
    const float voltage_normalize = 1.5f / tfoc->vbus_filter;
    float mod_alpha_raw = (tfoc->v_alpha * voltage_normalize);
    float mod_beta_raw = (tfoc->v_beta * voltage_normalize);
    foc_svm(mod_alpha_raw, mod_beta_raw, MGESC_PWM_ARR, &tfoc->dtc_a, &tfoc->dtc_b, &tfoc->dtc_c, &tfoc->svm_sector);

    TIM1->CCR1 = tfoc->dtc_a;
    if (tmotor->config.motor_spin_dir == DIR_FORWARD)
    {
        TIM1->CCR2 = tfoc->dtc_b;
        TIM1->CCR3 = tfoc->dtc_c;
    }
    else
    {
        TIM1->CCR2 = tfoc->dtc_c;
        TIM1->CCR3 = tfoc->dtc_b;
    }
}
