/*
 * @Description
 * @Date: 2025-01-31 14:54:10
 * @Author: 弈秋仙贝
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-03 19:36:56
 * @LastEditors: 弈秋仙贝
 */

#ifndef _ESTIMATION_H
#define _ESTIMATION_H

#include "main.h"

// 参数辨识运行频率
#define EST_FREQ (20000.0f)          // 固定20kHz [Hz]
#define EST_PERIOD (1.0f / EST_FREQ) // 50us

// 辨识使用参数
// #define CALIBRATION_CURRENT 8.0f // [A] 校准电流，这里实际使用设置中的 calibrate_current
// #define MAX_VOLTAGE 24.0f        // [V] 母线电压，这里实际使用采样电压 vbus_filter
#define EST_SPEED 3000.0f // 磁链辨识的速度，修改为电气频率 [Hz]

// 电感辨识的注入信号配置。测量数据相比电桥偏大，可能是死区的原因
#define IND_SCALE_FACTOR 0.9f                  // 对测量的电感进行缩放
#define INJ_FREQ 1000.0f                       // 正弦电流注入频率，频率不同阻抗也不一样
#define SAMPLE_FREQ 20000.0f                   // 采样频率
#define INJ_BY_SAMPLE (INJ_FREQ / SAMPLE_FREQ) // 每个采样点注入的电流周期数
#define SAMPLE_SIZE 200                        // SAMPLE_SIZE * INJ_BY_SAMPLE需要为整数且>=10
#define BANDPASS_FLAG 0                        // 开启带通滤波器

// 环路计算参照Ti手册。自整定的带宽需要给小一些。
#define LOOP_DAMP 5.0f         // 环路阻尼系数
#define PRIORITY_LOOP 0        // 0为速度环优先，使用速度环计算电流环路；1为电流环优先，使用电流环计算速度环路
#define SPEED_BANDWIDTH 100.0f // 速度带宽 [Hz]
#define CURR_BANDWIDTH 400.0f  // 电流带宽 [Hz]

// 校准初始化参数 校准电流 母线电压 最大电流 电机极对数
void Estimation_Init(float measure_curr, float volt_bus, float max_curr, uint16_t pole_pairs_num);

// 电阻辨识 输出D轴电压，输入D轴电流，Q轴电流
int Resistance_Estimation(float *out_vd, float in_id, float in_iq);
int Inductance_Estimation(float *out_vd, float *out_vq, float in_id, float in_iq);
int Flux_Linkage_Estimation(float *out_id, float *angle, float in_vq, float in_id);
int Inertia_Estimation(float *out_id, float *angle, float in_vb, float in_id);

// 获取辨识参数 rs:电阻，ld:D轴电感，lq:Q轴电感，flux:磁链，inertia:转动惯量
void Get_EstimationParam(float *rs, float *ld, float *lq, float *flux, float *inertia);
// 自整定环路参数，带宽为[Hz]，速度环kp ki为[rad/s]
void Motor_LoopCalculate(float *current_bandwidth, float *speed_bandwidth, float *spdKp, float *spdKi);

#endif
