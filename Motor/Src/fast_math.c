/*
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-31 19:22:28
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "fast_math.h"

#define SIN_TABLE_SIZE 512
#define TABLE_TO_RAD (M_2PI / SIN_TABLE_SIZE)
#define RAD_to_TABLE (SIN_TABLE_SIZE / M_2PI)

static const float sinTable_f32[SIN_TABLE_SIZE + 1] = {
    0.00000000f, 0.01227154f, 0.02454123f, 0.03680722f, 0.04906767f, 0.06132074f,
    0.07356456f, 0.08579731f, 0.09801714f, 0.11022221f, 0.12241068f, 0.13458071f,
    0.14673047f, 0.15885814f, 0.17096189f, 0.18303989f, 0.19509032f, 0.20711138f,
    0.21910124f, 0.23105811f, 0.24298018f, 0.25486566f, 0.26671276f, 0.27851969f,
    0.29028468f, 0.30200595f, 0.31368174f, 0.32531029f, 0.33688985f, 0.34841868f,
    0.35989504f, 0.37131719f, 0.38268343f, 0.39399204f, 0.40524131f, 0.41642956f,
    0.42755509f, 0.43861624f, 0.44961133f, 0.46053871f, 0.47139674f, 0.48218377f,
    0.49289819f, 0.50353838f, 0.51410274f, 0.52458968f, 0.53499762f, 0.54532499f,
    0.55557023f, 0.56573181f, 0.57580819f, 0.58579786f, 0.59569930f, 0.60551104f,
    0.61523159f, 0.62485949f, 0.63439328f, 0.64383154f, 0.65317284f, 0.66241578f,
    0.67155895f, 0.68060100f, 0.68954054f, 0.69837625f, 0.70710678f, 0.71573083f,
    0.72424708f, 0.73265427f, 0.74095113f, 0.74913639f, 0.75720885f, 0.76516727f,
    0.77301045f, 0.78073723f, 0.78834643f, 0.79583690f, 0.80320753f, 0.81045720f,
    0.81758481f, 0.82458930f, 0.83146961f, 0.83822471f, 0.84485357f, 0.85135519f,
    0.85772861f, 0.86397286f, 0.87008699f, 0.87607009f, 0.88192126f, 0.88763962f,
    0.89322430f, 0.89867447f, 0.90398929f, 0.90916798f, 0.91420976f, 0.91911385f,
    0.92387953f, 0.92850608f, 0.93299280f, 0.93733901f, 0.94154407f, 0.94560733f,
    0.94952818f, 0.95330604f, 0.95694034f, 0.96043052f, 0.96377607f, 0.96697647f,
    0.97003125f, 0.97293995f, 0.97570213f, 0.97831737f, 0.98078528f, 0.98310549f,
    0.98527764f, 0.98730142f, 0.98917651f, 0.99090264f, 0.99247953f, 0.99390697f,
    0.99518473f, 0.99631261f, 0.99729046f, 0.99811811f, 0.99879546f, 0.99932238f,
    0.99969882f, 0.99992470f, 1.00000000f, 0.99992470f, 0.99969882f, 0.99932238f,
    0.99879546f, 0.99811811f, 0.99729046f, 0.99631261f, 0.99518473f, 0.99390697f,
    0.99247953f, 0.99090264f, 0.98917651f, 0.98730142f, 0.98527764f, 0.98310549f,
    0.98078528f, 0.97831737f, 0.97570213f, 0.97293995f, 0.97003125f, 0.96697647f,
    0.96377607f, 0.96043052f, 0.95694034f, 0.95330604f, 0.94952818f, 0.94560733f,
    0.94154407f, 0.93733901f, 0.93299280f, 0.92850608f, 0.92387953f, 0.91911385f,
    0.91420976f, 0.90916798f, 0.90398929f, 0.89867447f, 0.89322430f, 0.88763962f,
    0.88192126f, 0.87607009f, 0.87008699f, 0.86397286f, 0.85772861f, 0.85135519f,
    0.84485357f, 0.83822471f, 0.83146961f, 0.82458930f, 0.81758481f, 0.81045720f,
    0.80320753f, 0.79583690f, 0.78834643f, 0.78073723f, 0.77301045f, 0.76516727f,
    0.75720885f, 0.74913639f, 0.74095113f, 0.73265427f, 0.72424708f, 0.71573083f,
    0.70710678f, 0.69837625f, 0.68954054f, 0.68060100f, 0.67155895f, 0.66241578f,
    0.65317284f, 0.64383154f, 0.63439328f, 0.62485949f, 0.61523159f, 0.60551104f,
    0.59569930f, 0.58579786f, 0.57580819f, 0.56573181f, 0.55557023f, 0.54532499f,
    0.53499762f, 0.52458968f, 0.51410274f, 0.50353838f, 0.49289819f, 0.48218377f,
    0.47139674f, 0.46053871f, 0.44961133f, 0.43861624f, 0.42755509f, 0.41642956f,
    0.40524131f, 0.39399204f, 0.38268343f, 0.37131719f, 0.35989504f, 0.34841868f,
    0.33688985f, 0.32531029f, 0.31368174f, 0.30200595f, 0.29028468f, 0.27851969f,
    0.26671276f, 0.25486566f, 0.24298018f, 0.23105811f, 0.21910124f, 0.20711138f,
    0.19509032f, 0.18303989f, 0.17096189f, 0.15885814f, 0.14673047f, 0.13458071f,
    0.12241068f, 0.11022221f, 0.09801714f, 0.08579731f, 0.07356456f, 0.06132074f,
    0.04906767f, 0.03680722f, 0.02454123f, 0.01227154f, 0.00000000f, -0.01227154f,
    -0.02454123f, -0.03680722f, -0.04906767f, -0.06132074f, -0.07356456f,
    -0.08579731f, -0.09801714f, -0.11022221f, -0.12241068f, -0.13458071f,
    -0.14673047f, -0.15885814f, -0.17096189f, -0.18303989f, -0.19509032f,
    -0.20711138f, -0.21910124f, -0.23105811f, -0.24298018f, -0.25486566f,
    -0.26671276f, -0.27851969f, -0.29028468f, -0.30200595f, -0.31368174f,
    -0.32531029f, -0.33688985f, -0.34841868f, -0.35989504f, -0.37131719f,
    -0.38268343f, -0.39399204f, -0.40524131f, -0.41642956f, -0.42755509f,
    -0.43861624f, -0.44961133f, -0.46053871f, -0.47139674f, -0.48218377f,
    -0.49289819f, -0.50353838f, -0.51410274f, -0.52458968f, -0.53499762f,
    -0.54532499f, -0.55557023f, -0.56573181f, -0.57580819f, -0.58579786f,
    -0.59569930f, -0.60551104f, -0.61523159f, -0.62485949f, -0.63439328f,
    -0.64383154f, -0.65317284f, -0.66241578f, -0.67155895f, -0.68060100f,
    -0.68954054f, -0.69837625f, -0.70710678f, -0.71573083f, -0.72424708f,
    -0.73265427f, -0.74095113f, -0.74913639f, -0.75720885f, -0.76516727f,
    -0.77301045f, -0.78073723f, -0.78834643f, -0.79583690f, -0.80320753f,
    -0.81045720f, -0.81758481f, -0.82458930f, -0.83146961f, -0.83822471f,
    -0.84485357f, -0.85135519f, -0.85772861f, -0.86397286f, -0.87008699f,
    -0.87607009f, -0.88192126f, -0.88763962f, -0.89322430f, -0.89867447f,
    -0.90398929f, -0.90916798f, -0.91420976f, -0.91911385f, -0.92387953f,
    -0.92850608f, -0.93299280f, -0.93733901f, -0.94154407f, -0.94560733f,
    -0.94952818f, -0.95330604f, -0.95694034f, -0.96043052f, -0.96377607f,
    -0.96697647f, -0.97003125f, -0.97293995f, -0.97570213f, -0.97831737f,
    -0.98078528f, -0.98310549f, -0.98527764f, -0.98730142f, -0.98917651f,
    -0.99090264f, -0.99247953f, -0.99390697f, -0.99518473f, -0.99631261f,
    -0.99729046f, -0.99811811f, -0.99879546f, -0.99932238f, -0.99969882f,
    -0.99992470f, -1.00000000f, -0.99992470f, -0.99969882f, -0.99932238f,
    -0.99879546f, -0.99811811f, -0.99729046f, -0.99631261f, -0.99518473f,
    -0.99390697f, -0.99247953f, -0.99090264f, -0.98917651f, -0.98730142f,
    -0.98527764f, -0.98310549f, -0.98078528f, -0.97831737f, -0.97570213f,
    -0.97293995f, -0.97003125f, -0.96697647f, -0.96377607f, -0.96043052f,
    -0.95694034f, -0.95330604f, -0.94952818f, -0.94560733f, -0.94154407f,
    -0.93733901f, -0.93299280f, -0.92850608f, -0.92387953f, -0.91911385f,
    -0.91420976f, -0.90916798f, -0.90398929f, -0.89867447f, -0.89322430f,
    -0.88763962f, -0.88192126f, -0.87607009f, -0.87008699f, -0.86397286f,
    -0.85772861f, -0.85135519f, -0.84485357f, -0.83822471f, -0.83146961f,
    -0.82458930f, -0.81758481f, -0.81045720f, -0.80320753f, -0.79583690f,
    -0.78834643f, -0.78073723f, -0.77301045f, -0.76516727f, -0.75720885f,
    -0.74913639f, -0.74095113f, -0.73265427f, -0.72424708f, -0.71573083f,
    -0.70710678f, -0.69837625f, -0.68954054f, -0.68060100f, -0.67155895f,
    -0.66241578f, -0.65317284f, -0.64383154f, -0.63439328f, -0.62485949f,
    -0.61523159f, -0.60551104f, -0.59569930f, -0.58579786f, -0.57580819f,
    -0.56573181f, -0.55557023f, -0.54532499f, -0.53499762f, -0.52458968f,
    -0.51410274f, -0.50353838f, -0.49289819f, -0.48218377f, -0.47139674f,
    -0.46053871f, -0.44961133f, -0.43861624f, -0.42755509f, -0.41642956f,
    -0.40524131f, -0.39399204f, -0.38268343f, -0.37131719f, -0.35989504f,
    -0.34841868f, -0.33688985f, -0.32531029f, -0.31368174f, -0.30200595f,
    -0.29028468f, -0.27851969f, -0.26671276f, -0.25486566f, -0.24298018f,
    -0.23105811f, -0.21910124f, -0.20711138f, -0.19509032f, -0.18303989f,
    -0.17096189f, -0.15885814f, -0.14673047f, -0.13458071f, -0.12241068f,
    -0.11022221f, -0.09801714f, -0.08579731f, -0.07356456f, -0.06132074f,
    -0.04906767f, -0.03680722f, -0.02454123f, -0.01227154f, -0.00000000f};

/**********************************************************************************************
Function: fast_sin_cos
Description: 快速正弦函数和快速余弦函数同时计算
Input: 浮点数x
Output: 正弦值sinX，余弦值cosX
Input_Output: 无
Return: 无
Author: Marc Pony(marc_pony@163.com)
***********************************************************************************************/
void fast_sin_cos(float x, float *sinX, float *cosX)
{
    float a, b; /* Two nearest output values */

    int sign = x > 0.0f ? 1 : -1;
    int si = (int)(x * RAD_to_TABLE + sign * 0.5f); // +0.5后取整等价于round函数
    float d = x - si * TABLE_TO_RAD;
    int ci = si + (SIN_TABLE_SIZE >> 2);
    si &= (SIN_TABLE_SIZE - 1);
    ci &= (SIN_TABLE_SIZE - 1);
    a = sinTable_f32[si];
    b = sinTable_f32[ci];

    *sinX = a + (b - 0.5f * a * d) * d;
    *cosX = b - (a + 0.5f * b * d) * d;
}

/**
 * Fast sine and cosine implementation.
 *
 * See http://lab.polygonal.de/?p=205
 *
 * @param x
 * The angle in radians
 * WARNING: Don't use too large angles.
 *
 * @param sin
 * A pointer to store the sine value.
 *
 * @param cos
 * A pointer to store the cosine value.
 */
float user_sin_f32(float x)
{
    float sinVal, fract, in; /* Temporary variables for input, output */
    uint16_t index;          /* Index variable */
    float a, b;              /* Two nearest output values */
    int32_t n;
    float findex;

    /* input x is in radians */
    /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
    in = x * 0.159154943092f;

    /* Calculation of floor value of input */
    n = (int32_t)in;

    /* Make negative values towards -infinity */
    if (x < 0.0f)
    {
        n--;
    }

    /* Map input value to [0 1] */
    in = in - (float)n;

    /* Calculation of index of the table */
    findex = (float)SIN_TABLE_SIZE * in;
    index = (uint16_t)findex;

    /* when "in" is exactly 1, we need to rotate the index down to 0 */
    if (index >= SIN_TABLE_SIZE)
    {
        index = 0;
        findex -= (float)SIN_TABLE_SIZE;
    }

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = sinTable_f32[index];
    b = sinTable_f32[index + 1];

    /* Linear interpolation process */
    sinVal = (1.0f - fract) * a + fract * b;

    /* Return the output value */
    return (sinVal);
}

float user_cos_f32(float x)
{
    float cosVal, fract, in; /* Temporary variables for input, output */
    uint16_t index;          /* Index variable */
    float a, b;              /* Two nearest output values */
    int32_t n;
    float findex;

    /* input x is in radians */
    /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi, add 0.25 (pi/2) to read sine table */
    in = x * 0.159154943092f + 0.25f;

    /* Calculation of floor value of input */
    n = (int32_t)in;

    /* Make negative values towards -infinity */
    if (in < 0.0f)
    {
        n--;
    }

    /* Map input value to [0 1] */
    in = in - (float)n;

    /* Calculation of index of the table */
    findex = (float)SIN_TABLE_SIZE * in;
    index = (uint16_t)findex;

    /* when "in" is exactly 1, we need to rotate the index down to 0 */
    if (index >= SIN_TABLE_SIZE)
    {
        index = 0;
        findex -= (float)SIN_TABLE_SIZE;
    }

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the cos table */
    a = sinTable_f32[index];
    b = sinTable_f32[index + 1];

    /* Linear interpolation process */
    cosVal = (1.0f - fract) * a + fract * b;

    /* Return the output value */
    return (cosVal);
}

// fast_atan2 based on https://math.stackexchange.com/a/1105038/81278
// Via Odrive project
// https://github.com/odriverobotics/ODrive/blob/master/Firmware/MotorControl/utils.cpp
// This function is MIT licenced, copyright Oskar Weigl/Odrive Robotics
// The origin for Odrive atan2 is public domain. Thanks to Odrive for making
// it easy to borrow.
float fast_atan2(float y, float x)
{
    // a := min (|x|, |y|) / max (|x|, |y|)
    float abs_y = fabsf(y);
    float abs_x = fabsf(x);
    if (abs_x < 1e-6f)
    {
        abs_x = 1e-6f;
    }
    // inject FLT_MIN in denominator to avoid division by zero
    float a = MIN(abs_x, abs_y) / (MAX(abs_x, abs_y));
    // s := a * a
    float s = a * a;
    // r := ((-0.0464964749 * s + 0.15931422) * s - 0.327622764) * s * a + a
    float r =
        ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
    // if |y| > |x| then r := 1.57079637 - r
    if (abs_y > abs_x)
        r = 1.57079637f - r;
    // if x < 0 then r := 3.14159274 - r
    if (x < 0.0f)
        r = 3.14159274f - r;
    // if y < 0 then r := -r
    if (y < 0.0f)
        r = -r;

    return r;
}

/**
 * Fast atan2
 *
 * See http://www.dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
 *
 * @param y
 * y
 *
 * @param x
 * x
 *
 * @return
 * The angle in radians
 */
float utils_fast_atan2(float y, float x)
{
    float abs_y = fabsf(y) + 1e-20f; // kludge to prevent 0/0 condition
    float angle;

    if (x >= 0.0f)
    {
        float r = (x - abs_y) / (x + abs_y);
        float rsq = r * r;
        angle = ((0.1963f * rsq) - 0.9817f) * r + (M_PI / 4.0f);
    }
    else
    {
        float r = (x + abs_y) / (abs_y - x);
        float rsq = r * r;
        angle = ((0.1963f * rsq) - 0.9817f) * r + (3.0f * M_PI / 4.0f);
    }

    UTILS_NAN_ZERO(angle);

    if (y < 0.0f)
    {
        return (-angle);
    }
    else
    {
        return (angle);
    }
}

/**
 * @brief fir
 */
void fir_filter(float *out, const float *in, const float *coeffs, int n_coeffs, int n_samples)
{
    int i, j;
    float accu;

    for (i = 0; i < n_samples; i++)
    {
        accu = 0.0f;

        for (j = 0; j < n_coeffs; j++)
        {
            accu += coeffs[j] * in[i + j];
        }

        out[i] = accu;
    }
}
