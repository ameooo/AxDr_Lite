/***
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-10-26 23:33:22
 * @LastEditors: 弈秋
 */

#ifndef __FAST_MATH_H
#define __FAST_MATH_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <math.h>

void fast_sin_cos(float x, float *sinX, float *cosX);
float user_sin_f32(float x);
float user_cos_f32(float x);
float fast_atan2(float y, float x);
float utils_fast_atan2(float y, float x);

// Constants
#define PI_BY_THREE (1.047197551f)
#define PI_BY_TWO (1.570796326f)
#define M_2PI_BY_THREE (2.0943951024f)
#define M_PI (3.14159265358f)
#define M_2PI (6.28318530716f)
#define ONE_BY_2PI (0.159154943f)
#define ONE_BY_THREE (0.33333333333f)
#define TWO_BY_THREE (2.0f * 0.33333333333f)
#define ONE_BY_SQRT3 (0.57735026919f)
#define TWO_BY_SQRT3 (2.0f * 0.57735026919f)
#define SQRT3_BY_2 (0.86602540378f)
#define COS_30_DEG (0.86602540378f)
#define SIN_30_DEG (0.5f)
#define COS_MINUS_30_DEG (0.86602540378f)
#define SIN_MINUS_30_DEG (-0.5f)
#define ONE_BY_SQRT2 (0.7071067811865475f)

// Return the sign of the argument. -1.0 if negative, 1.0 if zero or positive.
#define SIGN(x) (((x) < 0) ? -1.0f : 1.0f)

// Squared
#define SQ(x) ((x) * (x))

// Two-norm of 2D vector
#define NORM2_f(x, y) (sqrtf(SQ(x) + SQ(y)))

// nan and infinity check for floats
#define UTILS_IS_INF(x) ((x) == (1.0f / 0.0f) || (x) == (-1.0f / 0.0f))
#define UTILS_IS_NAN(x) ((x) != (x))
#define UTILS_NAN_ZERO(x) (x = UTILS_IS_NAN(x) ? 0.0f : x)

// Handy conversions for radians/degrees and RPM/radians-per-second
#define DEG2RAD_f(deg) ((deg) * (float)(M_PI / 180.0f))
#define RAD2DEG_f(rad) ((rad) * (float)(180.0f / M_PI))
#define RPM2RADPS_f(rpm) ((rpm) * (float)((2.0f * M_PI) / 60.0f))
#define RADPS2RPM_f(rad_per_sec) ((rad_per_sec) * (float)(60.0f / (2.0f * M_PI)))
#define DEG2RAD ((float)(M_PI / 180.0f))
#define RAD2DEG ((float)(180.0f / M_PI))
#define RPM2RADPS ((float)((2.0f * M_PI) / 60.0f))
#define RADPS2RPM ((float)(60.0f / (2.0f * M_PI)))

//
#define ABS(a) ((a > 0) ? (a) : (-a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, lower, upper) (MIN(upper, MAX(x, lower)))

/**
 * A simple low pass filter.
 *
 * @param value
 * The filtered value.
 *
 * @param sample
 * Next sample.
 *
 * @param filter_constant
 * Filter constant. Range 0.0 to 1.0, where 1.0 gives the unfiltered value.
 */
#define UTILS_LP_FAST(value, sample, filter_constant) (value -= (filter_constant) * ((value) - (sample)))

/**
 * A fast approximation of a moving average filter with N samples. See
 * https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
 * https://en.wikipedia.org/wiki/Exponential_smoothing
 *
 * It is not entirely the same as it behaves like an IIR filter rather than a FIR filter, but takes
 * much less memory and is much faster to run.
 */
#define UTILS_LP_MOVING_AVG_APPROX(value, sample, N) UTILS_LP_FAST(value, sample, 2.0f / ((N) + 1.0f))

/* Inline functions ------------------------------------------------------------------*/
static inline void utils_step_towards(float *value, float goal, float step)
{
    if (*value < goal)
    {
        if ((*value + step) < goal)
        {
            *value += step;
        }
        else
        {
            *value = goal;
        }
    }
    else if (*value > goal)
    {
        if ((*value - step) > goal)
        {
            *value -= step;
        }
        else
        {
            *value = goal;
        }
    }
}

/**
 * Make sure that -pi <= angle < pi,
 *
 * @param angle
 * The angle to normalize in radians.
 * WARNING: Don't use too large angles.
 */
static inline void utils_norm_angle_rad(float *angle)
{
    while (*angle < -M_PI)
    {
        *angle += M_2PI;
    }
    while (*angle >= M_PI)
    {
        *angle -= M_2PI;
    }
}

static inline void utils_norm_enc_value(int *value, int max)
{
    int half = 0.5f * max;

    while (*value < -half)
    {
        *value += max;
    }
    while (*value >= half)
    {
        *value -= max;
    }
}

static inline void utils_truncate_number(float *number, float min, float max)
{
    if (*number > max)
    {
        *number = max;
    }
    else if (*number < min)
    {
        *number = min;
    }
}

static inline void utils_truncate_number_int(int *number, int min, int max)
{
    if (*number > max)
    {
        *number = max;
    }
    else if (*number < min)
    {
        *number = min;
    }
}

static inline void utils_truncate_number_abs(float *number, float max)
{
    if (*number > max)
    {
        *number = max;
    }
    else if (*number < -max)
    {
        *number = -max;
    }
}

static inline float utils_map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline int utils_map_int(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * Truncate the magnitude of a vector.
 *
 * @param x
 * The first component.
 *
 * @param y
 * The second component.
 *
 * @param max
 * The maximum magnitude.
 *
 * @return
 * True if saturation happened, false otherwise
 */
static inline bool utils_saturate_vector_2d(float *x, float *y, float max)
{
    bool retval = false;
    float mag = NORM2_f(*x, *y);
    max = fabsf(max);

    if (mag < 1e-10f)
    {
        mag = 1e-10f;
    }

    if (mag > max)
    {
        const float f = max / mag;
        *x *= f;
        *y *= f;
        retval = true;
    }

    return retval;
}

#endif
