/*
 * @Description:
 * @Date: 2024-12-29 22:33:19
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-26 20:51:24
 * @LastEditors: 弈秋仙贝
 */

#include "LedRGB.h"
#include "tim.h"

/*Some Static Colors------------------------------*/
// const RGB_Color_TypeDef WS_RED = {255, 0, 0}; // 显示红色RGB数据
// const RGB_Color_TypeDef WS_ORANGE = {127, 106, 0};
// const RGB_Color_TypeDef WS_YELLOW = {127, 216, 0};
// const RGB_Color_TypeDef WS_GREEN = {0, 255, 0};
// const RGB_Color_TypeDef WS_CYAN = {0, 255, 255};
// const RGB_Color_TypeDef WS_BLUE = {0, 0, 255};
// const RGB_Color_TypeDef WS_PURPLE = {238, 130, 238};
// const RGB_Color_TypeDef WS_BLACK = {0, 0, 0};
// const RGB_Color_TypeDef WS_WHITE = {255, 255, 255};
// const RGB_Color_TypeDef WS_MAGENTA = {255, 0, 220};

const RGB_Color_TypeDef WS_RED = {85, 0, 0}; // 显示红色RGB数据
const RGB_Color_TypeDef WS_ORANGE = {42, 35, 0};
const RGB_Color_TypeDef WS_YELLOW = {42, 72, 0};
const RGB_Color_TypeDef WS_GREEN = {0, 85, 0};
const RGB_Color_TypeDef WS_CYAN = {0, 85, 85};
const RGB_Color_TypeDef WS_BLUE = {0, 0, 85};
const RGB_Color_TypeDef WS_PURPLE = {79, 43, 79};
const RGB_Color_TypeDef WS_BLACK = {0, 0, 0};
const RGB_Color_TypeDef WS_WHITE = {85, 85, 85};
const RGB_Color_TypeDef WS_MAGENTA = {85, 0, 73};

/*二维数组存放最终PWM输出数组，每一行24个
数据代表一个LED，最后一行24个0代表RESET码*/
uint32_t Pixel_Buf[WS2812_DATA_LEN + RESET_LEN] = {0};

/*
功能：设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
参数：LedId为LED序号，Color：定义的颜色结构体
*/
void RGB_SetColor(uint8_t LedId, RGB_Color_TypeDef Color)
{
    uint8_t i;
    if (LedId > LED_NUM)
        return; // avoid overflow 防止写入ID大于LED总数

    for (i = 0; i < 8; i++)
    {
        Pixel_Buf[LedId * LED_DATA_LEN + i] = (((Color.G << i) & (0x80)) ? (CODE_1) : CODE_0);      // 数组某一行0~7转化存放G
        Pixel_Buf[LedId * LED_DATA_LEN + i + 8] = (((Color.R << i) & (0x80)) ? (CODE_1) : CODE_0);  // 数组某一行8~15转化存放R
        Pixel_Buf[LedId * LED_DATA_LEN + i + 16] = (((Color.B << i) & (0x80)) ? (CODE_1) : CODE_0); // 数组某一行16~23转化存放B}
    }
}

/*
功能：最后一行装在24个0，输出24个周期占空比为0的PWM波，作为最后reset延时，这里总时长为24*1.2=30us > 24us(要求大于24us)
*/
void Reset_Load(void)
{
    uint8_t i;
    for (i = 0; i < RESET_LEN; i++)
    {
        Pixel_Buf[WS2812_DATA_LEN + i] = 0;
    }
}

/*
功能：发送数组
参数：(&htim1)定时器1，(TIM_CHANNEL_2)通道2，((uint32_t *)Pixel_Buf)待发送数组，
            (Pixel_NUM+1)*24)发送个数，数组行列相乘
*/
void RGB_SendArray(void)
{
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf, (WS2812_DATA_LEN + RESET_LEN));
}

/*
功能：显示红色
参数：Pixel_Len为显示LED个数
*/
void RGB_Init(void)
{
    uint16_t i;
    for (i = 0; i < LED_NUM; i++) // 给对应个数LED写入红色
    {
        RGB_SetColor(i, WS_BLACK);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示红色
参数：Pixel_Len为显示LED个数
*/
void RGB_RED(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入红色
    {
        RGB_SetColor(i, WS_RED);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示绿色
参数：Pixel_Len为显示LED个数
*/
void RGB_GREEN(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入绿色
    {
        RGB_SetColor(i, WS_GREEN);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示蓝色
参数：Pixel_Len为显示LED个数
*/
void RGB_BLUE(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入蓝色
    {
        RGB_SetColor(i, WS_BLUE);
    }
    Reset_Load();
    RGB_SendArray();
}

/*
功能：显示白色
参数：Pixel_Len为显示LED个数
*/
void RGB_WHITE(uint16_t Pixel_Len)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++) // 给对应个数LED写入白色
    {
        RGB_SetColor(i, WS_WHITE);
    }
    Reset_Load();
    RGB_SendArray();
}

// 显示指定颜色
void RGB_ShowColor(uint32_t Pixel_Len, RGB_Color_TypeDef rgb)
{
    uint16_t i;
    for (i = 0; i < Pixel_Len; i++)
    {
        RGB_SetColor(i, rgb);
    }
    Reset_Load();
    RGB_SendArray();
}

// 用户自定义API接口可根据实际拓展

/*******************************************************************************/
/*									添加部分									   */

// 颜色循环转换
static RGB_Color_TypeDef Wheel(uint8_t WheelPos)
{
    RGB_Color_TypeDef rgb;
    uint8_t rgb_pos = 0xff - WheelPos;
    if (rgb_pos < 85)
    {
        rgb.R = 0xff - rgb_pos * 3;
        rgb.G = 0;
        rgb.B = rgb_pos * 3;
        return rgb;
    }
    if (rgb_pos < 170)
    {
        rgb_pos -= 85;
        rgb.R = 0;
        rgb.G = rgb_pos * 3;
        rgb.B = 0xff - rgb_pos * 3;
        return rgb;
    }
    rgb_pos -= 170;
    rgb.R = rgb_pos * 3;
    rgb.G = 0xff - rgb_pos * 3;
    rgb.B = 0;
    return rgb;
}

// 彩虹呼吸灯
void rgb_rainbow(uint8_t wait)
{
    RGB_Color_TypeDef rgb_temp;
    uint32_t time_now = HAL_GetTick();
    uint16_t i;
    static uint8_t j;
    static uint32_t timestamp = 0;

    uint32_t interval_time = rgb_interval_time(time_now, timestamp);
    if (interval_time >= wait)
    {
        timestamp = time_now;
        j++;
        for (i = 0; i < LED_NUM; i++)
        {
            rgb_temp = Wheel((i + j) & 0xff);
            RGB_SetColor(i, rgb_temp);
        }
        RGB_SendArray();
    }
}

// 彩虹灯旋转
void rgb_rainbowCycle(uint8_t wait)
{
    RGB_Color_TypeDef rgb_temp;
    uint32_t timestamp = HAL_GetTick();
    uint16_t i;
    static uint8_t j;
    static uint32_t next_time = 0;

    if ((timestamp > next_time)) // && (timestamp - next_time < wait*5))
    {
        j++;
        next_time = timestamp + wait;
        for (i = 0; i < LED_NUM; i++)
        {
            rgb_temp = Wheel(((i * 256 / (LED_NUM)) + j) & 0xff);
            RGB_SetColor(i, rgb_temp);
        }
        RGB_SendArray();
    }
}

/**
 * @brief RGB循环
 * @param cycle 循环周期
 */
void RGB_LoopColor(uint16_t cycle)
{
    static uint8_t j;
    static uint32_t timestamp = 0;

    timestamp++;
    if (timestamp >= (cycle * 0.00390625))
    {
        timestamp = 0;
        j++;
        RGB_SetColor(0, Wheel(j));
        RGB_SendArray();
    }
}
