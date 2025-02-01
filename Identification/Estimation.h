/*
 * @Description:
 * @Date: 2025-01-31 14:54:10
 * @Author: 弈秋仙贝
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-31 22:12:01
 * @LastEditors: 弈秋仙贝
 */

#ifndef _ESTIMATION_H
#define _ESTIMATION_H

#include "main.h"

// 参数辨识运行频率
#define EST_FREQ (20000.0f)          // 固定20kHz [Hz]
#define EST_PERIOD (1.0f / EST_FREQ) // 50us

// 电感辨识的注入信号配置。测量数据相比电桥偏大，可能是死区的原因
#define INJ_FREQ 1000.0f                       // 正弦电流注入频率，频率不同阻抗也不一样
#define SAMPLE_FREQ 20000.0f                   // 采样频率
#define INJ_BY_SAMPLE (INJ_FREQ / SAMPLE_FREQ) // 每个采样点注入的电流周期数
#define SAMPLE_SIZE 200                        // SAMPLE_SIZE * INJ_BY_SAMPLE需要为整数且>=10
#define BANDPASS_FLAG 0                        // 开启带通滤波器

// 磁链辨识的速度
#define EST_SPEED 500.0f // [rpm]

// 环路计算参照Ti手册。自整定的带宽需要给小一些。
#define LOOP_DAMP 5.0f         // 环路阻尼系数
#define PRIORITY_LOOP 0        // 0为速度环优先，使用速度环计算电流环路；1为电流环优先，使用电流环计算速度环路
#define SPEED_BANDWIDTH 100.0f // 速度带宽 [Hz]
#define CURR_BANDWIDTH 400.0f  // 电流带宽 [Hz]

// 校准初始化参数：校准电流 母线电压 最大电流 电机极对数
void Estimation_Init(float measure_curr, float volt_bus, float max_curr, uint16_t pole_pairs_num);

int Resistance_Estimation(float *voltage, float curr_d, float curr_q);
int Inductance_Estimation(float *volt_d, float *volt_q, float curr_d, float curr_q);
int Flux_Linkage_Estimation(float *out_id, float *angle, float int_vq, float int_id);
int Inertia_Estimation(float *out_id, float *angle, float int_vb, float int_id);

// 获取辨识参数 rs:电阻，ld:电感，lq:磁链，flux:磁链，inertia:转动惯量
void Get_EstimationParam(float *rs, float *ld, float *lq, float *flux, float *inertia);
// 自整定环路参数，带宽为[Hz]，速度环kp ki为[rad/s]
void Motor_LoopCalculate(float *current_bandwidth, float *speed_bandwidth, float *spdKp, float *spdKi);

#endif
