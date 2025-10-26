/*** 
 * @Description: 
 * @Date: 2025-01-13 19:01:43
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-23 23:24:27
 * @LastEditors: 弈秋仙贝
 */

#ifndef __FLUX_OBSERVER_H__
#define __FLUX_OBSERVER_H__

#include "main.h"
#include "mgesc_state.h"

void flux_observer_init(mc_motor_typedef *tmotor);
void flux_observer(mc_motor_typedef *tmotor);

void scvm_observer_init(mc_motor_typedef *tmotor);
void scvm_observer_updata(mc_motor_typedef *tmotor);

#endif /* __FLUX_OBSERVER_H__ */
