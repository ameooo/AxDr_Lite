/*
 * @Description:
 * @Date: 2025-01-13 19:01:43
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-01 20:22:13
 * @LastEditors: 弈秋仙贝
 */

#include "flux_observer.h"
#include "user_config.h"
#include "fast_math.h"

#define FLUX_OBSERVER_V1 0 // 电压型磁链观测器
#define FLUX_OBSERVER_V2 0 // 电压电流混合模型磁链观测器

float PhiVoltA, PhiVoltB;     // α-β电压φ
float PhiActiveA, PhiActiveB; // α-β有效φ

float flux_max = 0.0f;
float flux_min = 0.0f;

void flux_observer_init(mc_motor_typedef *tmotor)
{
    tmotor->observer.Rs = tmotor->config.motor_rs;
    tmotor->observer.Ld = tmotor->config.motor_ld;
    tmotor->observer.Lq = tmotor->config.motor_lq;
    tmotor->observer.flux = tmotor->config.motor_flux_linkage;
    tmotor->observer.dt = CURRENT_CONTROL_PERIOD;
    flux_max = tmotor->config.motor_flux_linkage * 2.0f;
    flux_min = tmotor->config.motor_flux_linkage * 0.5f;

    tmotor->observer.Ualpha_integral = 0.0f;
    tmotor->observer.Ubeta_integral = 0.0f;

    tmotor->observer.flux_kp = 1000.0f;
    tmotor->observer.flux_ki = 100000.0f;
    float bandwidth = 1000.0f;
    tmotor->observer.pll_kp = bandwidth * 2.0f;
    tmotor->observer.pll_ki = SQ(bandwidth);
}

void flux_observer(mc_motor_typedef *tmotor)
{
    flux_observer_typedef *tobserver = &tmotor->observer;

    UTILS_LP_FAST(tobserver->Ialpha_filter, tmotor->foc.i_alpha, 0.3f);
    UTILS_LP_FAST(tobserver->Ibeta_filter, tmotor->foc.i_beta, 0.3f);
    // tobserver->Ialpha_filter = tmotor->foc.i_alpha;
    // tobserver->Ibeta_filter = tmotor->foc.i_beta;

#if FLUX_OBSERVER_V1 // 开环模型，效果一般，速度稍高会发散
    float Lambda_norm = (PhiVoltA * PhiVoltA + PhiVoltB * PhiVoltB);
    float PhiErr = Lambda_norm - (tobserver->PhiEstimate * tobserver->PhiEstimate);

    tobserver->Err_alpha = PhiErr * 0.01f;
    tobserver->PhiEstimate += tobserver->Err_alpha;
    if (tobserver->PhiEstimate > flux_max)
    {
        tobserver->PhiEstimate = flux_max;
    }
    else if (tobserver->PhiEstimate < flux_min)
    {
        tobserver->PhiEstimate = flux_min;
    }

    PhiVoltA += (tmotor->foc.v_alpha - tobserver->Rs * tobserver->Ialpha_filter) * tobserver->dt - tmotor->observer.Ld * tobserver->Ialpha_filter;
    PhiVoltB += (tmotor->foc.v_beta - tobserver->Rs * tobserver->Ibeta_filter) * tobserver->dt - tobserver->Ld * tobserver->Ibeta_filter;

    utils_saturate_vector_2d((float *)&PhiVoltA, (float *)&PhiVoltB, flux_max);

    PhiActiveA = PhiVoltA;
    PhiActiveB = PhiVoltB;
    tobserver->theta = fast_atan2(PhiActiveB, PhiActiveA);
#elif FLUX_OBSERVER_V2
    float PhiCurrD, PhiCurrQ; // d-q电流φ
    float PhiCurrA, PhiCurrB; // α-β电流φ
    float PhiErrA, PhiErrB;   // α-β误差φ

    PhiVoltA += (tmotor->foc.v_alpha - tobserver->Rs * tobserver->Ialpha_filter + tobserver->Err_alpha) * tobserver->dt;
    PhiVoltB += (tmotor->foc.v_beta - tobserver->Rs * tobserver->Ibeta_filter + tobserver->Err_beta) * tobserver->dt;

    PhiActiveA = PhiVoltA - tobserver->Lq * tobserver->Ialpha_filter;
    PhiActiveB = PhiVoltB - tobserver->Lq * tobserver->Ibeta_filter;
    if (PhiActiveA > flux_max)
    {
        PhiEstimate = flux_max;
    }
    else if (PhiActiveA < -flux_max)
    {
        PhiEstimate = -flux_max;
    }
    if (PhiActiveB > flux_max)
    {
        PhiActiveB = flux_max;
    }
    else if (PhiActiveB < -flux_max)
    {
        PhiActiveB = -flux_max;
    }
    tobserver->theta = fast_atan2(PhiActiveB, PhiActiveA);

    float sine, cosine;
    fast_sin_cos(tobserver->theta, &sine, &cosine);
    float curr_d, curr_q;
    curr_d = tobserver->Ialpha_filter * cosine + tobserver->Ibeta_filter * sine;
    curr_q = -tobserver->Ialpha_filter * sine + tobserver->Ibeta_filter * cosine;

    PhiCurrD = tobserver->Ld * curr_d + tobserver->flux;
    PhiCurrQ = tobserver->Lq * curr_q;

    PhiCurrA = PhiCurrD * cosine - PhiCurrQ * sine;
    PhiCurrB = PhiCurrD * sine + PhiCurrQ * cosine;

    PhiErrA = PhiCurrA - PhiVoltA;
    PhiErrB = PhiCurrB - PhiVoltB;

    tobserver->Ualpha_integral += PhiErrA * tobserver->flux_ki * tobserver->dt;
    tobserver->Ubeta_integral += PhiErrB * tobserver->flux_ki * tobserver->dt;

    tobserver->Err_alpha = tobserver->Ualpha_integral + PhiErrA * tobserver->flux_kp;
    tobserver->Err_beta = tobserver->Ubeta_integral + PhiErrB * tobserver->flux_kp;

#endif

    // pll
    float pll_err = tobserver->theta - tobserver->pll_theta;
    if (pll_err < -M_PI)
    {
        pll_err += M_2PI;
    }
    if (pll_err >= M_PI)
    {
        pll_err -= M_2PI;
    }
    tobserver->pll_theta += (pll_err * tobserver->pll_kp + tobserver->pll_int) * CURRENT_CONTROL_PERIOD;
    while (tobserver->pll_theta < -M_PI)
    {
        tobserver->pll_theta += M_2PI;
    }
    while (tobserver->pll_theta >= M_PI)
    {
        tobserver->pll_theta -= M_2PI;
    }
    tobserver->pll_int += pll_err * tobserver->pll_ki * CURRENT_CONTROL_PERIOD;
    tobserver->estimate_elec_vel = tobserver->pll_int;
    tobserver->estimate_mech_vel = tobserver->estimate_elec_vel / tmotor->config.motor_pole_pairs;
}

void scvm_observer_init(mc_motor_typedef *tmotor)
{
    tmotor->scvm.lambda = 2.0f;
    tmotor->scvm.alpha_a = 1000.0f;
    tmotor->scvm.alpha_b = 1.0f;

    tmotor->scvm.theta = 0.0f;
    tmotor->scvm.omega = 0.0f;
}

/**
 * @brief 开环观测器，效果一般，优点是零速启动不用调参
 *
 * @param tmotor
 */
void scvm_observer_updata(mc_motor_typedef *tmotor)
{
    scvm_observer_typedef *tscvm = &tmotor->scvm;

    float lambda_s = tscvm->lambda * SIGN(tscvm->omega);
    tscvm->alpha_lpf = tscvm->alpha_a + tscvm->alpha_b * 2.0f * tscvm->lambda * ABS(tscvm->omega);
    tscvm->emf_d = tmotor->foc.vd + tscvm->omega * tmotor->config.motor_lq * tmotor->foc.iq_ref;
    tscvm->emf_q = tmotor->foc.vq - tmotor->config.motor_rs * tmotor->foc.iq_ref;
    tscvm->theta += CURRENT_CONTROL_PERIOD * tscvm->omega;
    tscvm->omega += CURRENT_CONTROL_PERIOD * tscvm->alpha_lpf *
                    (((tscvm->emf_q - lambda_s * tscvm->emf_d) /
                      (tmotor->config.motor_flux_linkage)) -
                     tscvm->omega);
    utils_norm_angle_rad(&tscvm->theta);
    UTILS_LP_FAST(tscvm->estimate_elec_vel, tscvm->omega, 0.3f);
    tscvm->estimate_mech_vel = tscvm->estimate_elec_vel / tmotor->config.motor_pole_pairs;
}
