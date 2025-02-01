/*
 * @Description:
 * @Date: 2025-01-21 22:26:23
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-01 20:32:06
 * @LastEditors: 弈秋仙贝
 */
#include "lcd.h"
#include "display.h"

void display(void)
{
	LCD_ShowString(10, 5, (uint8_t *)"[v_beta_1.0]", GREEN, BLACK, 32, 0);

	LCD_DrawLine(10, 40 - 2, 10, 134, WHITE);
	LCD_DrawLine(230, 40 - 2, 230, 135, WHITE);

	LCD_ShowString(20, 40 - 2, (uint8_t *)" Rs. :", WHITE, BLACK, 16, 0);
	LCD_ShowString(20, 60 - 2, (uint8_t *)" Ld. :", WHITE, BLACK, 16, 0);
	LCD_ShowString(20, 80 - 2, (uint8_t *)" Lq. :", WHITE, BLACK, 16, 0);
	LCD_ShowString(20, 100 - 2, (uint8_t *)"Flux.:", WHITE, BLACK, 16, 0);
	LCD_ShowString(20, 120 - 2, (uint8_t *)" J.  :", WHITE, BLACK, 16, 0);

	LCD_ShowString(152, 40 - 2, (uint8_t *)"Ohm", WHITE, BLACK, 16, 0);
	LCD_ShowString(152, 60 - 2, (uint8_t *)"H", WHITE, BLACK, 16, 0);
	LCD_ShowString(152, 80 - 2, (uint8_t *)"H", WHITE, BLACK, 16, 0);
	LCD_ShowString(152, 100 - 2, (uint8_t *)"Wb", WHITE, BLACK, 16, 0);
	LCD_ShowString(152, 120 - 2, (uint8_t *)"mN*m2", WHITE, BLACK, 16, 0);
}

void display_update(float *values)
{
	char buffer[20];
	sprintf(buffer, "%.6f", values[0]);
	LCD_ShowString(76, 40 - 2, (uint8_t *)buffer, BRRED, BLACK, 16, 0);

	sprintf(buffer, "%.6f", values[1]);
	LCD_ShowString(76, 60 - 2, (uint8_t *)buffer, GREEN, BLACK, 16, 0);

	sprintf(buffer, "%.6f", values[2]);
	LCD_ShowString(76, 80 - 2, (uint8_t *)buffer, GREEN, BLACK, 16, 0);

	sprintf(buffer, "%.6f", values[3]);
	LCD_ShowString(76, 100 - 2, (uint8_t *)buffer, GREEN, BLACK, 16, 0);

	sprintf(buffer, "%.6f", values[4]);
	LCD_ShowString(76, 120 - 2, (uint8_t *)buffer, GREEN, BLACK, 16, 0);
}
