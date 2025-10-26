/*
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-01 20:27:24
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "calibration.h"
#include "mc_task.h"
#include "mgfoc.h"
#include "user_config.h"

#include "Estimation.h"

void Motor_Param_Measuring(mc_motor_typedef *tmotor)
{
    int ret = 0;

    static float vd = 0, vq = 0;
    static float id = 0, iq = 0;
    static float phase_angle = 0;

    switch (tmotor->calib_state)
    {
    case CALIBRATE_START:
        vd = 0.0f;
        vq = 0.0f;
        id = 0.0f;
        iq = 0.0f;
        phase_angle = 0.0f;
        Estimation_Init(tmotor->config.calibrate_current, tmotor->foc.vbus_filter,
                        15.5f, tmotor->config.motor_pole_pairs);
        tmotor->calib_state = CALIBRATE_RESISTANCE;
        break;

    case CALIBRATE_RESISTANCE:
        ret = Resistance_Estimation(&vd, tmotor->foc.id, tmotor->foc.iq);
        mgesc_current_phase(tmotor, 0.0f, 0.0f);
        mgesc_voltage_output(tmotor, vd, 0.0f);
        if (ret == 1)
        {
            tmotor->calib_state = CALIBRATE_INDUCTANCE;
        }
        break;

    case CALIBRATE_INDUCTANCE:
        ret = Inductance_Estimation(&vd, &vq, tmotor->foc.id, tmotor->foc.iq);
        mgesc_current_phase(tmotor, 0.0f, 0.0f);
        mgesc_voltage_output(tmotor, vd, vq);
        if (ret == 1)
        {
            // 接下来需要使用电流环，设置电流环带宽为200Hz
            float motor_param[5] = {0.0f};
            Get_EstimationParam(&motor_param[0], &motor_param[1], &motor_param[2], &motor_param[3], &motor_param[4]);
            Set_MotorElecParam(&tmotor->config, motor_param[0], motor_param[1], motor_param[2], 0.0f);
            Set_CurrentLoopBandwidth(&tmotor->config, 200.0f);
            tmotor->calib_state = CALIBRATE_FLUX_LINKAGE;
        }
        break;

    case CALIBRATE_FLUX_LINKAGE:
        ret = Flux_Linkage_Estimation(&id, &phase_angle, tmotor->foc.vq, tmotor->foc.id_filter);
        mgesc_current_phase(tmotor, phase_angle, 0.0f);
        mgesc_current_calc(tmotor, id, 0.0f);
        if (ret == 1)
        {
            tmotor->calib_state = CALIBRATE_INERTIA;
        }
        break;

    case CALIBRATE_INERTIA:
        ret = Inertia_Estimation(&id, &phase_angle, tmotor->foc.v_beta, tmotor->foc.id);
        mgesc_current_phase(tmotor, phase_angle, 0.0f);
        mgesc_current_calc(tmotor, id, 0.0f);
        if (ret == 1)
        {
            // 参数辨识完成
            float motor_param[5] = {0.0f};
            Get_EstimationParam(&motor_param[0], &motor_param[1], &motor_param[2], &motor_param[3], &motor_param[4]);
            Set_MotorElecParam(&tmotor->config, motor_param[0], motor_param[1], motor_param[2], motor_param[3]);
            Set_MotorMechParam(&tmotor->config, motor_param[4]);

            float loop_param[4] = {0.0f};
            Motor_LoopCalculate(&loop_param[0], &loop_param[1], &loop_param[2], &loop_param[3]);
            Set_CurrentLoopBandwidth(&tmotor->config, loop_param[0]);
            Set_VelocityLoopParam(&tmotor->config, loop_param[2], loop_param[3]);
            tmotor->calib_state = CALIBRATE_DONE;
        }
        break;

    case CALIBRATE_DONE:
        tmotor->calib_state = CALIBRATE_START;
        MC_State_Process_Set(MC_STATE_STOP);
        break;

    case CALIBRATE_ERROR:
        tmotor->calib_state = CALIBRATE_START;
        MC_State_Process_Set(MC_STATE_FAULT_NOW);
        break;

    default:
        break;
    }

    if (ret == -1)
    {
        tmotor->calib_state = CALIBRATE_ERROR;
    }
}
