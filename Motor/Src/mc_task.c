/*
 * @Description:
 * @Date: 2024-10-26 14:57:40
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-31 22:16:26
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "mc_task.h"
#include "calibration.h"
#include "encoder.h"
#include "controller.h"
#include "flux_observer.h"
#include "mgfoc.h"
#include "user_config.h"
#include "fast_math.h"

#include "adc.h"
#include "tim.h"
#include "usart.h"

static int phase_current_sampling(mc_motor_typedef *tmotor);
static int motor_sensor_position_sampling(mc_motor_typedef *tmotor);

static void mc_state_init(mc_motor_typedef *tmotor);
static void mc_state_running(mc_motor_typedef *tmotor);
static void mc_state_fault_now(mc_motor_typedef *tmotor);

static void mc_timer_start(void);
static void mc_timer_stop(void);
static void mc_pwm_enable(void);
static void mc_pwm_disable(void);
static void TIM_CCxNChannelCmd(TIM_TypeDef *TIMx, uint32_t Channel, uint32_t ChannelNState);

mc_motor_typedef mc_motor;

// 电机错误状态
uint8_t pos_sensor_flag = 0;
uint8_t current_sensor_flag = 0;
uint8_t volt_over_flag = 0; // 这里使用24V，其他电压需要修改
uint8_t current_over_flag = 0;
uint8_t current_runaway_flag = 0;
uint8_t speed_runaway_flag = 0;

/**
 * @brief 电机初始化
 *
 */
void MC_Motor_Init(void)
{
    mc_motor.mc_state = MC_STATE_IDLE;
    mc_motor.state_count = 0;
    motor_config_init(&mc_motor.config);
    motor_controller_init(&mc_motor);
    // flux_observer_init(&mc_motor);
    scvm_observer_init(&mc_motor);
    mc_timer_start();
    MC_State_Process_Set(MC_STATE_INIT);
}

/**
 * @brief 电机任务
 *
 */
void MC_ADC_IRQ_handler(void)
{
    mc_motor_typedef *tmotor = &mc_motor;

    phase_current_sampling(tmotor);

    switch (tmotor->mc_state)
    {
    case MC_STATE_IDLE:

        break;

    case MC_STATE_INIT:
        mc_state_init(tmotor);
        break;

    case MC_STATE_DETECTING:
        Motor_Param_Measuring(tmotor);
        break;

    case MC_STATE_START:
        MC_State_Process_Set(MC_STATE_RUNNING);
        break;

    case MC_STATE_RUNNING:
        if (motor_sensor_position_sampling(tmotor) < 0)
        {
            pos_sensor_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
        mc_state_running(tmotor);
        break;

    case MC_STATE_STOP:

        break;

    case MC_STATE_FAULT_NOW:
        mc_state_fault_now(tmotor);
        break;

    case MC_STATE_ERROR:

        break;

    default:
        break;
    }

    Get_Motor_Info();
}

/**
 * @brief 电机状态处理
 * @param state
 * @return int
 */
int MC_State_Process_Set(mc_state_e state)
{
    mc_motor_typedef *tmotor = &mc_motor;

    int ret = 0;

    switch (state)
    {
    case MC_STATE_IDLE:
        if (tmotor->mc_state == MC_STATE_STOP)
        {
            mc_timer_start();
        }
        mc_pwm_disable();
        tmotor->mc_state = MC_STATE_IDLE;
        break;

    case MC_STATE_INIT:
        mc_pwm_enable();
        tmotor->mc_state = MC_STATE_INIT;
        break;

    case MC_STATE_DETECTING:
        if (tmotor->mc_state == MC_STATE_IDLE)
        {
            mc_pwm_enable();
            tmotor->mc_state = MC_STATE_DETECTING;
        }
        else if (tmotor->mc_state == MC_STATE_STOP)
        {
            mc_timer_start();
            mc_pwm_enable();
            tmotor->mc_state = MC_STATE_DETECTING;
        }
        else
        {
            ret = -1;
        }
        break;

    case MC_STATE_START:
        if ((tmotor->mc_state == MC_STATE_IDLE) || (tmotor->mc_state == MC_STATE_INIT))
        {
            mc_pwm_enable();
            set_controller_close_mode(tmotor, tmotor->config.control_mode, 0.0f);
            tmotor->mc_state = MC_STATE_START;
        }
        else if (tmotor->mc_state == MC_STATE_STOP)
        {
            mc_timer_start();
            mc_pwm_enable();
            set_controller_close_mode(tmotor, tmotor->config.control_mode, 0.0f);
            tmotor->mc_state = MC_STATE_START;
        }
        else
        {
            ret = -1;
        }
        break;

    case MC_STATE_RUNNING:
        if (tmotor->mc_state == MC_STATE_START)
        {
            tmotor->mc_state = MC_STATE_RUNNING;
        }
        else
        {
            ret = -1;
        }
        break;

    case MC_STATE_STOP:
        if (tmotor->mc_state < MC_STATE_STOP)
        {
            set_controller_close_mode(tmotor, MOTOR_CONTROL_MODE_IDLE, 0.0f);
            mc_pwm_disable();
            mc_timer_stop();
            tmotor->mc_state = MC_STATE_STOP;
        }
        else
        {
            ret = -1;
        }
        break;

    case MC_STATE_FAULT_NOW:
        mc_pwm_disable();
        tmotor->mc_state = MC_STATE_FAULT_NOW;
        break;

    default:
        break;
    }

    return ret;
}

/**
 * @brief 获取电机状态
 * @return mc_state_e
 */
mc_state_e MC_State_Process_Get(void)
{
    return mc_motor.mc_state;
}

const uint8_t vofa_tail[4] = {0x00, 0x00, 0x80, 0x7f};
uint8_t foc_data[30] = {0};
void Get_Motor_Info(void)
{
    static uint16_t send_count = 0;
    send_count++;
    if (send_count >= 4)
    {
        send_count = 0;

        // for debug
        // *(float *)&foc_data[0] = mc_motor.encoder.mech_vel;
        // *(float *)&foc_data[4] = mc_motor.scvm.theta;
        // *(float *)&foc_data[8] = mc_motor.encoder.elec_rad;
        // memcpy(&foc_data[12], vofa_tail, 4);
        // HAL_UART_Transmit_DMA(&huart3, foc_data, 16);

        // CDC_Transmit_FS(foc_data, 16);
    }
}

/**
 * @brief 设置电机控制模式
 * @param mode 控制模式
 * @param input 控制量
 */
void MC_SetControlMode(motor_control_mode_e mode, float input)
{
    switch (mode)
    {
    case MOTOR_CONTROL_MODE_IDLE:
    case MOTOR_CONTROL_MODE_CURRENT:
    case MOTOR_CONTROL_MODE_TORQUE:
    case MOTOR_CONTROL_MODE_VELOCITY:
    case MOTOR_CONTROL_MODE_VELOCITY_RAMP:
    case MOTOR_CONTROL_MODE_POS:
    case MOTOR_CONTROL_MODE_POS_RAMP:
        set_controller_close_mode(&mc_motor, mode, input);
        break;

    // 测试使用
    case MOTOR_CONTROL_MODE_VOLTAGE_OPENLOOP:
    case MOTOR_CONTROL_MODE_CURRENT_OPENLOOP:
        set_controller_open_mode(&mc_motor, mode, input, 0.0f, 0.0f, 0.0f);
        break;
    
    default:
        break;
    }
}

static int phase_current_sampling(mc_motor_typedef *tmotor)
{
    foc_state_typedef *tfoc = &tmotor->foc;

    int ret = 0;

    tfoc->adc_raw[0] = ADC1->JDR2;
    tfoc->adc_raw[1] = ADC1->JDR3;
    tfoc->adc_raw[2] = ADC2->JDR1;
    tfoc->adc_raw[3] = ADC1->JDR1;
    tfoc->adc_raw[4] = ADC2->JDR2;

    // 电流采样为反向
    tfoc->ia = (tmotor->config.adc_offsets[0] - tfoc->adc_raw[0]) * FAC_CURRENT;
    tfoc->ib = (tmotor->config.adc_offsets[1] - tfoc->adc_raw[1]) * FAC_CURRENT;
    tfoc->ic = (tmotor->config.adc_offsets[2] - tfoc->adc_raw[2]) * FAC_CURRENT;
    tfoc->ibus = (tmotor->config.adc_offsets[3] - tfoc->adc_raw[3]) * FAC_CURRENT;
    tfoc->vbus = tfoc->adc_raw[4] * VOLTAGE_TO_ADC_FACTOR;
    UTILS_LP_FAST(tfoc->vbus_filter, tfoc->vbus, 0.1f);

    // Clarke transform
    tfoc->i_alpha = tfoc->ia; // ONE_BY_THREE * (2.0f * tfoc->ia - tfoc->ib - tfoc->ic);
    tfoc->i_beta = (tfoc->ib - tfoc->ic) * ONE_BY_SQRT3;

    // 简单的电压保护
    static uint16_t volt_over_cnt = 0;
    if ((tfoc->vbus > 28.0f) || (tfoc->vbus < 20.0f))
    {
        volt_over_cnt++;
        if (volt_over_cnt >= 500) // 25ms
        {
            volt_over_cnt = 0;
            volt_over_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
    }
    else
    {
        if (volt_over_cnt > 0)
            volt_over_cnt--;
    }

    // 简单的电流保护
    static uint16_t current_over_cnt = 0;
    if (ABS(tfoc->i_abs_filter) > 15.5f)
    {
        current_over_cnt++;
        if (current_over_cnt >= 500)
        {
            current_over_cnt = 0;
            current_over_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
    }
    else
    {
        if (current_over_cnt > 0)
            current_over_cnt--;
    }

    // 简单的电流失控保护
    static uint16_t current_runaway_cnt = 0;
    if ((ABS(tfoc->delta_id) > 5.0f) || (ABS(tfoc->delta_iq) > 5.0f))
    {
        current_runaway_cnt++;
        if (current_runaway_cnt >= 500)
        {
            current_runaway_cnt = 0;
            current_runaway_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
    }
    else
    {
        if (current_runaway_cnt > 0)
            current_runaway_cnt--;
    }

    // 简单的失速保护
    // static uint16_t speed_runaway_cnt = 0;
    // if ((ABS(tmotor->controller.delta_vel) > 5.0f) || (ABS(tfoc->iq_filter) > 15.0f))
    // {
    //     speed_runaway_cnt++;
    //     if (speed_runaway_cnt >= 500)
    //     {
    //         speed_runaway_cnt = 0;
    //         speed_runaway_flag = 1;
    //         MC_State_Process_Set(MC_STATE_FAULT_NOW);
    //     }
    // }
    // else
    // {
    //     speed_runaway_cnt = 0;
    // }

    return ret;
}

/**
 * @brief 电机位置传感器
 * @param tmotor
 * @return int
 */
static int motor_sensor_position_sampling(mc_motor_typedef *tmotor)
{
    int res = 0;
    switch (tmotor->config.sensor_mode)
    {
    case MOTOR_SENSOR_MODE_SENSORLESS:
        // res = sensor_absolute_read(tmotor); // 测试使用
        scvm_observer_updata(tmotor);
        break;

    case MOTOR_SENSOR_MODE_OPENLOOP:

        break;

    case MOTOR_SENSOR_MODE_ABSOLUTE_ENCODER:
        res = sensor_absolute_read(tmotor);
        break;

    default:
        break;
    }
    return res;
}

/**
 * @brief 初始化校验编码器和电流传感器并获取电流偏置
 * @param motor
 */
static void mc_state_init(mc_motor_typedef *tmotor)
{
    foc_state_typedef *tfoc = &tmotor->foc;

    static int adc_sum[4] = {0};
    static uint32_t err_cnt = 0;

    if (motor_sensor_position_sampling(tmotor) < 0)
    {
        pos_sensor_flag = 1;
        MC_State_Process_Set(MC_STATE_FAULT_NOW);
    }

    tmotor->state_count++;
    if (tmotor->state_count <= 1000)
    {
        if (tmotor->state_count == 1000)
        {
            err_cnt = 0;
            adc_sum[0] = 0;
            adc_sum[1] = 0;
            adc_sum[2] = 0;
            adc_sum[3] = 0;
        }
    }
    else if (tmotor->state_count <= 2000)
    {
        adc_sum[0] += tfoc->adc_raw[0];
        adc_sum[1] += tfoc->adc_raw[1];
        adc_sum[2] += tfoc->adc_raw[2];
        adc_sum[3] += tfoc->adc_raw[3];
    }
    else if (tmotor->state_count <= 3000)
    {
        tmotor->config.adc_offsets[0] = adc_sum[0] / 1000;
        tmotor->config.adc_offsets[1] = adc_sum[1] / 1000;
        tmotor->config.adc_offsets[2] = adc_sum[2] / 1000;
        tmotor->config.adc_offsets[3] = adc_sum[3] / 1000;
        if ((tfoc->adc_raw[0] > 2200) || (tfoc->adc_raw[0] < 1900) ||
            (tfoc->adc_raw[1] > 2200) || (tfoc->adc_raw[0] < 1900) ||
            (tfoc->adc_raw[2] > 2200) || (tfoc->adc_raw[0] < 1900) ||
            (tfoc->adc_raw[3] > 2200) || (tfoc->adc_raw[0] < 1900))
        {
            current_sensor_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
        else
        {
            // MC_State_Process_Set(MC_STATE_START); // 校验完成直接进入使能
            MC_State_Process_Set(MC_STATE_STOP); // 校验完成直接进入失能
        }
        adc_sum[0] = 0;
        adc_sum[1] = 0;
        adc_sum[2] = 0;
    }

    // 电流采样或驱动桥存在问题，立即进入故障状态
    if ((tfoc->adc_raw[0] > 3000) || (tfoc->adc_raw[0] < 1000) ||
        (tfoc->adc_raw[1] > 3000) || (tfoc->adc_raw[1] < 1000) ||
        (tfoc->adc_raw[2] > 3000) || (tfoc->adc_raw[2] < 1000) ||
        (tfoc->adc_raw[3] > 3000) || (tfoc->adc_raw[3] < 1000))
    {
        err_cnt++;
        if (err_cnt >= 20)
        {
            err_cnt = 0;
            current_sensor_flag = 1;
            MC_State_Process_Set(MC_STATE_FAULT_NOW);
        }
    }
}

static void mc_state_running(mc_motor_typedef *tmotor)
{
    // float mech_vel = 0.0f;
    // float mech_pos = 0.0f;
    // float elec_vel = 0.0f;
    // float elec_pos = 0.0f;

    switch (tmotor->config.sensor_mode)
    {
    case MOTOR_SENSOR_MODE_SENSORLESS:
        tmotor->foc.mech_vel = tmotor->scvm.estimate_mech_vel;
        tmotor->foc.mech_pos = 0.0f;
        tmotor->foc.elec_vel = tmotor->scvm.estimate_elec_vel;
        tmotor->foc.elec_pos = tmotor->scvm.theta;
        if (tmotor->config.control_mode == MOTOR_CONTROL_MODE_VELOCITY_RAMP) // && (ABS(tmotor->controller.input_velocity) > 0.1f)
        {
            float iq_ref = controller_loop(tmotor, tmotor->foc.mech_pos, tmotor->foc.mech_vel);
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_current_calc(tmotor, 0.0f, iq_ref);
        }
        else
        {
            controller_loop(tmotor, tmotor->foc.mech_pos, tmotor->foc.mech_vel);
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_current_calc(tmotor, 0.0f, 0.0f);
        }
        break;

    case MOTOR_SENSOR_MODE_OPENLOOP:
        if (tmotor->config.control_mode >= MOTOR_CONTROL_MODE_CURRENT_OPENLOOP)
        {
            tmotor->controller.open_loop_position += tmotor->controller.open_loop_velocity * CONTROLLER_PERIOD;
            utils_norm_angle_rad(&tmotor->controller.open_loop_position);
            tmotor->foc.elec_vel = tmotor->controller.open_loop_velocity;
            tmotor->foc.elec_pos = tmotor->controller.open_loop_position;
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_current_calc(tmotor, tmotor->controller.open_loop_power_d, tmotor->controller.open_loop_power_q);
        }
        else if (tmotor->config.control_mode >= MOTOR_CONTROL_MODE_VOLTAGE_OPENLOOP)
        {
            tmotor->controller.open_loop_position += tmotor->controller.open_loop_velocity * CONTROLLER_PERIOD;
            utils_norm_angle_rad(&tmotor->controller.open_loop_position);
            tmotor->foc.elec_vel = tmotor->controller.open_loop_velocity;
            tmotor->foc.elec_pos = tmotor->controller.open_loop_position;
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_voltage_output(tmotor, tmotor->controller.open_loop_power_d, tmotor->controller.open_loop_power_q);
        }
        else
        {
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_voltage_output(tmotor, 0.0f, 0.0f);
        }
        break;

    case MOTOR_SENSOR_MODE_ABSOLUTE_ENCODER:
        if (tmotor->config.control_mode >= MOTOR_CONTROL_MODE_TORQUE)
        {
            tmotor->foc.mech_vel = tmotor->encoder.mech_vel;
            tmotor->foc.mech_pos = tmotor->encoder.mech_rad;
            tmotor->foc.elec_vel = tmotor->encoder.elec_vel;
            tmotor->foc.elec_pos = tmotor->encoder.elec_rad;
            float iq_ref = controller_loop(tmotor, tmotor->foc.mech_pos, tmotor->foc.mech_vel);
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_current_calc(tmotor, 0.0f, iq_ref);
        }
        else
        {
            mgesc_current_phase(tmotor, tmotor->foc.elec_pos, tmotor->foc.elec_vel);
            mgesc_current_calc(tmotor, 0.0f, 0.0f);
        }
        break;

    default:
        break;
    }
}

static void mc_state_fault_now(mc_motor_typedef *tmotor)
{
    tmotor->state_count = 0;
    mc_pwm_disable();
    tmotor->mc_state = MC_STATE_ERROR;
}

/**
 * @brief
 *
 */
static void mc_timer_start(void)
{
    __HAL_TIM_MOE_ENABLE(&htim1);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_4, TIM_CCx_ENABLE);
    HAL_TIM_Base_Start(&htim1);
}

static void mc_timer_stop(void)
{
    __HAL_TIM_MOE_DISABLE(&htim1);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_4, TIM_CCx_DISABLE);
    HAL_TIM_Base_Stop(&htim1);
}

/**
 * @brief 定时器pwm输出使能
 */
static void mc_pwm_enable(void)
{
    TIM1->CCR1 = MGESC_PWM_ARR_HALF;
    TIM1->CCR2 = MGESC_PWM_ARR_HALF;
    TIM1->CCR3 = MGESC_PWM_ARR_HALF;
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCx_ENABLE);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCx_ENABLE);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_3, TIM_CCx_ENABLE);

    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCxN_ENABLE);
    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCxN_ENABLE);
    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_3, TIM_CCxN_ENABLE);
}

/**
 * @brief 定时器pwm输出失能
 */
static void mc_pwm_disable(void)
{
    TIM1->CCR1 = MGESC_PWM_ARR_HALF;
    TIM1->CCR2 = MGESC_PWM_ARR_HALF;
    TIM1->CCR3 = MGESC_PWM_ARR_HALF;
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_3, TIM_CCx_DISABLE);

    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCxN_DISABLE);
    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCxN_DISABLE);
    TIM_CCxNChannelCmd(TIM1, TIM_CHANNEL_3, TIM_CCxN_DISABLE);
}

/**
 * @brief 定时器低侧PWM输出使能
 * @param TIMx
 * @param Channel
 * @param ChannelNState
 */
static void TIM_CCxNChannelCmd(TIM_TypeDef *TIMx, uint32_t Channel, uint32_t ChannelNState)
{
    uint32_t tmp;

    tmp = TIM_CCER_CC1NE << (Channel & 0xFU); /* 0xFU = 15 bits max shift */

    /* Reset the CCxNE Bit */
    TIMx->CCER &= ~tmp;

    /* Set or reset the CCxNE Bit */
    TIMx->CCER |= (uint32_t)(ChannelNState << (Channel & 0xFU)); /* 0xFU = 15 bits max shift */
}
