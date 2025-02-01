/*** 
 * @Description: 
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-10-26 17:06:50
 * @LastEditors: 弈秋
 */


#ifndef _MGESC_CALC_H
#define _MGESC_CALC_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

void foc_svm(float alpha, float beta, uint32_t PWMFullDutyCycle,
				uint32_t* tAout, uint32_t* tBout, uint32_t* tCout, uint32_t *svm_sector);

#endif /* _MGESC_CALC_H */
