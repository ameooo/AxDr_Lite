/*
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-18 22:11:46
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "mgesc_calc.h"
#include "user_config.h"
#include "fast_math.h"

/**
 * @brief svm Space vector modulation. Magnitude must not be larger than sqrt(3)/2, or 0.866 to avoid overmodulation.
 *        See https://github.com/vedderb/bldc/pull/372#issuecomment-962499623 for a full description.
 * @param alpha voltage
 * @param beta Park transformed and normalized voltage
 * @param PWMFullDutyCycle is the peak value of the PWM counter.
 * @param tAout PWM duty cycle phase A (0 = off all of the time, PWMFullDutyCycle = on all of the time)
 * @param tBout PWM duty cycle phase B
 * @param tCout PWM duty cycle phase C
 */
void foc_svm(float alpha, float beta, uint32_t PWMFullDutyCycle,
             uint32_t *tAout, uint32_t *tBout, uint32_t *tCout, uint32_t *svm_sector)
{
    uint32_t sector;

    if (beta >= 0.0f)
    {
        if (alpha >= 0.0f)
        {
            // quadrant I
            if (ONE_BY_SQRT3 * beta > alpha)
            {
                sector = 2;
            }
            else
            {
                sector = 1;
            }
        }
        else
        {
            // quadrant II
            if (-ONE_BY_SQRT3 * beta > alpha)
            {
                sector = 3;
            }
            else
            {
                sector = 2;
            }
        }
    }
    else
    {
        if (alpha >= 0.0f)
        {
            // quadrant IV5
            if (-ONE_BY_SQRT3 * beta > alpha)
            {
                sector = 5;
            }
            else
            {
                sector = 6;
            }
        }
        else
        {
            // quadrant III
            if (ONE_BY_SQRT3 * beta > alpha)
            {
                sector = 4;
            }
            else
            {
                sector = 5;
            }
        }
    }

    // PWM timings
    uint32_t tA, tB, tC;

    switch (sector)
    {

    // sector 1-2
    case 1:
    {
        // Vector on-times
        uint32_t t1 = (alpha - ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t2 = (TWO_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tA = (MGESC_PWM_ARR + t1 + t2) / 2;
        tB = tA - t1;
        tC = tB - t2;

        break;
    }

    // sector 2-3
    case 2:
    {
        // Vector on-times
        uint32_t t2 = (alpha + ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t3 = (-alpha + ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tB = (MGESC_PWM_ARR + t2 + t3) / 2;
        tA = tB - t3;
        tC = tA - t2;

        break;
    }

    // sector 3-4
    case 3:
    {
        // Vector on-times
        uint32_t t3 = (TWO_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t4 = (-alpha - ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tB = (MGESC_PWM_ARR + t3 + t4) / 2;
        tC = tB - t3;
        tA = tC - t4;

        break;
    }

    // sector 4-5
    case 4:
    {
        // Vector on-times
        uint32_t t4 = (-alpha + ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t5 = (-TWO_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tC = (MGESC_PWM_ARR + t4 + t5) / 2;
        tB = tC - t5;
        tA = tB - t4;

        break;
    }

    // sector 5-6
    case 5:
    {
        // Vector on-times
        uint32_t t5 = (-alpha - ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t6 = (alpha - ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tC = (MGESC_PWM_ARR + t5 + t6) / 2;
        tA = tC - t5;
        tB = tA - t6;

        break;
    }

    // sector 6-1
    case 6:
    {
        // Vector on-times
        uint32_t t6 = (-TWO_BY_SQRT3 * beta) * PWMFullDutyCycle;
        uint32_t t1 = (alpha + ONE_BY_SQRT3 * beta) * PWMFullDutyCycle;

        // PWM timings
        tA = (MGESC_PWM_ARR + t6 + t1) / 2;
        tC = tA - t1;
        tB = tC - t6;

        break;
    }
    }

    *tAout = tA;
    *tBout = tB;
    *tCout = tC;
    *svm_sector = sector;
}
