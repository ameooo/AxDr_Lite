/***
 * @Descriptionexternal
 * @Date: 2024-12-29 22:33:19
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-12-29 22:35:13
 * @LastEditors: 弈秋
 */

#ifndef __LED_RGB_H__
#define __LED_RGB_H__

#include "main.h"

/*这里是上文计算所得CCR的宏定义*/
#define CODE_1 (136) // 1码定时器计数次数
#define CODE_0 (68)  // 0码定时器计数次数

#define LED_NUM 1                                // LED数量宏定义，这里我使用一个LED
#define LED_DATA_LEN (24)                        // led 长度，单个需要24bits
#define RESET_LEN (45)                           // 80 ，复位信号
#define WS2812_DATA_LEN (LED_NUM * LED_DATA_LEN) // ws2812灯条需要的数组长度

#define rgb_interval_time(a, b) ((a >= b) ? (a - b) : (a + (0xffffffff - b)))

/*建立一个定义单个LED三原色值大小的结构体*/
typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB_Color_TypeDef;

extern const RGB_Color_TypeDef WS_RED; // 显示红色RGB数据
extern const RGB_Color_TypeDef WS_ORANGE;
extern const RGB_Color_TypeDef WS_YELLOW;
extern const RGB_Color_TypeDef WS_GREEN;
extern const RGB_Color_TypeDef WS_CYAN;
extern const RGB_Color_TypeDef WS_BLUE;
extern const RGB_Color_TypeDef WS_PURPLE;
extern const RGB_Color_TypeDef WS_BLACK;
extern const RGB_Color_TypeDef WS_WHITE;
extern const RGB_Color_TypeDef WS_MAGENTA;

void RGB_SetColor(uint8_t LedId, RGB_Color_TypeDef Color); // 给一个LED装载24个颜色数据码（0码和1码）
void Reset_Load(void);                                     // 该函数用于将数组最后24个数据变为0，代表RESET_code
void RGB_SendArray(void);                                  // 发送最终数组
void RGB_Send_Code(uint8_t code);                          // 发送一个码
void RGB_Init(void);
void RGB_RED(uint16_t Pixel_Len);   // 显示红灯
void RGB_GREEN(uint16_t Pixel_Len); // 显示绿灯
void RGB_BLUE(uint16_t Pixel_Len);  // 显示蓝灯
void RGB_WHITE(uint16_t Pixel_Len); // 显示白灯
void RGB_ShowColor(uint32_t Pixel_Len, RGB_Color_TypeDef rgb);

void rgb_rainbow(uint8_t wait);
void rgb_rainbowCycle(uint8_t wait);

void RGB_LoopColor(uint16_t cycle);

#endif // !__LED_RGB_H__
