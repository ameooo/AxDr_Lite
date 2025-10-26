/*
 * @Description:
 * @Date: 2025-01-24 19:17:55
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-02-02 00:09:14
 * @LastEditors: 弈秋仙贝
 */

#include "bsp_flash_port.h"

/*
*********************************************************************************************************
*	函 数 名: bsp_GetSector
*	功能说明: 根据地址计算扇区首地址
*	形    参: 无
*	返 回 值: 扇区号（0-7)
*********************************************************************************************************
*/
uint32_t bsp_GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if (Address > FLASH_BANK2_START)
    {
        sector = (Address - FLASH_BANK2_START) / FLASH_PAGE_SIZE;
    } else {
        sector = (Address - FLASH_BANK1_START) / FLASH_PAGE_SIZE;
    }
    return sector;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参:  _ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize)
{
    uint32_t i;

    if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_MAX_SIZE)
    {
        return 1;
    }

    /* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
    if (_ulSize == 0)
    {
        return 1;
    }

    for (i = 0; i < _ulSize; i++)
    {
        *_ucpDst++ = *(uint8_t *)_ulFlashAddr++;
    }

    return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CmpCpuFlash
*	功能说明: 比较Flash指定地址的数据.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpBuf : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值:
*			FLASH_IS_EQU		0   Flash内容和待写入的数据相等，不需要擦除和写操作
*			FLASH_REQ_WRITE		1	Flash不需要擦除，直接写
*			FLASH_REQ_ERASE		2	Flash需要先擦除,再写
*			FLASH_PARAM_ERR		3	函数参数错误
*********************************************************************************************************
*/
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize)
{
    uint32_t i;
    uint8_t ucIsEqu; /* 相等标志 */
    uint8_t ucByte;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_MAX_SIZE)
    {
        return FLASH_PARAM_ERR; /*　函数参数错误　*/
    }

    /* 长度为0时返回正确 */
    if (_ulSize == 0)
    {
        return FLASH_IS_EQU; /* Flash内容和待写入的数据相等 */
    }

    ucIsEqu = 1; /* 先假设所有字节和待写入的数据相等，如果遇到任何一个不相等，则设置为 0 */
    for (i = 0; i < _ulSize; i++)
    {
        ucByte = *(uint8_t *)_ulFlashAddr;

        if (ucByte != *_ucpBuf)
        {
            if (ucByte != 0xFF)
            {
                return FLASH_REQ_ERASE; /* 需要擦除后再写 */
            }
            else
            {
                ucIsEqu = 0; /* 不相等，需要写 */
            }
        }

        _ulFlashAddr++;
        _ucpBuf++;
    }

    if (ucIsEqu == 1)
    {
        return FLASH_IS_EQU; /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
    }
    else
    {
        return FLASH_REQ_WRITE; /* Flash不需要擦除，直接写 */
    }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_EraseCpuFlash
*	功能说明: 擦除CPU FLASH一个扇区 （128KB)
*	形    参: _ulFlashAddr : Flash地址
*	返 回 值: 0 成功， 1 失败
*			  HAL_OK       = 0x00,
*			  HAL_ERROR    = 0x01,
*			  HAL_BUSY     = 0x02,
*			  HAL_TIMEOUT  = 0x03
*
*********************************************************************************************************
*/
uint8_t bsp_EraseCpuFlash(uint32_t _ulFlashAddr)
{
    uint32_t FirstPage = 0, NbOfPage = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError = 0;
    uint8_t re;

    /* 解锁 */
    HAL_FLASH_Unlock();

    /* 获取此地址所在的扇区 */
    FirstPage = bsp_GetSector(_ulFlashAddr);
    if (FirstPage > 0x00FF)
    {
        return 1;
    }

    /* 固定1个扇区 */
    NbOfPage = 1;

    /* 擦除扇区配置 */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;

    if (_ulFlashAddr >= FLASH_BANK2_START)
    {
        EraseInitStruct.Banks = FLASH_BANK_2;
    }
    else
    {
        EraseInitStruct.Banks = FLASH_BANK_1;
    }

    EraseInitStruct.Page = FirstPage;
    EraseInitStruct.NbPages = NbOfPage;

    /* 扇区擦除 */
    re = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

    /* 擦除完毕后，上锁 */
    HAL_FLASH_Lock();

    return re;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。 必须按32字节整数倍写。不支持跨扇区。扇区大小128KB. \
*			  写之前需要擦除扇区. 长度不是32字节整数倍时，最后几个字节末尾补0写入.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节, 必须是32字节整数倍）
*	返 回 值: 0-成功，1-数据长度或地址溢出，2-写Flash出错(估计Flash寿命到)
*********************************************************************************************************
*/
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize)
{
    uint32_t i;
    // uint8_t ucRet;

    /* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
    if (_ulFlashAddr + _ulSize > FLASH_BASE_ADDR + FLASH_MAX_SIZE)
    {
        return 1;
    }

    /* 长度为0时不继续操作  */
    if (_ulSize == 0)
    {
        return 0;
    }

    // ucRet = bsp_CmpCpuFlash(_ulFlashAddr, _ucpSrc, _ulSize);

    // if (ucRet == FLASH_IS_EQU)
    // {
    //     return 0;
    // }

    __set_PRIMASK(1); /* 关中断 */

    /* FLASH 解锁 */
    HAL_FLASH_Unlock();

    for (i = 0; i < _ulSize / 8; i++)
    {
        uint64_t FlashWord;

        memcpy((char *)&FlashWord, _ucpSrc, 8);
        _ucpSrc += 8;

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, _ulFlashAddr, (uint64_t)(FlashWord)) == HAL_OK)
        {
            _ulFlashAddr = _ulFlashAddr + 8; /* 递增，操作下一个64bit */
        }
        else
        {
            goto err;
        }
    }

    // for(int i=0;i<FLOATSCOUNT;i=i+2){
    //     uint64_t doubleWord = *((uint64_t*) (__float_reg + i)); //read two floats from array as one uint64
    //     status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLOATS_ADDR + i*sizeof(float), doubleWord);
    //     if(status!=HAL_OK) {printf("SaveToFlash fail writing float #%d", i);return 6;}
    // }

    /* 长度不是8字节整数倍 */
    if (_ulSize % 8)
    {
        uint64_t FlashWord;

        FlashWord = 0;
        memcpy((char *)&FlashWord, _ucpSrc, _ulSize % 8);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, _ulFlashAddr, (uint64_t)(FlashWord)) == HAL_OK)
        {
            ; // _ulFlashAddr = _ulFlashAddr + 8;
        }
        else
        {
            goto err;
        }
    }

    /* Flash 加锁，禁止写Flash控制寄存器 */
    HAL_FLASH_Lock();

    __set_PRIMASK(0); /* 开中断 */

    return 0;

err:
    /* Flash 加锁，禁止写Flash控制寄存器 */
    HAL_FLASH_Lock();

    __set_PRIMASK(0); /* 开中断 */

    return 1;
}
