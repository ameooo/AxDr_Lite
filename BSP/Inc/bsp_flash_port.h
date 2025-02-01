/***
 * @Description:
 * @Date: 2025-01-24 19:17:55
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-24 20:10:20
 * @LastEditors: 弈秋仙贝
 */

#ifndef __BSP_FLASH_PORT_H
#define __BSP_FLASH_PORT_H

#include "main.h"

#define FLASH_TIMEOUT_VALUE 1000U /* 1 s  */

#define FLASH_BASE_ADDR FLASH_BASE
#define FLASH_BANK1_START FLASH_BASE
#define FLASH_BANK2_START 0x08040000
#define FLASH_PAGE_SIZE 0x800U /* 2 KB */
#define FLASH_MAX_PAGE 128
#define FLASH_MAX_SIZE (FLASH_PAGE_SIZE * FLASH_MAX_PAGE * 2)
#define FLASH_CURR_ADDR(n) (FLASH_BASE_ADDR + FLASH_PAGE_SIZE * n)

#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */

uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize);
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize);
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize);

uint8_t bsp_EraseCpuFlash(uint32_t _ulFlashAddr);

#endif /* __BSP_FLASH_PORT_H */
