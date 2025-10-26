/*
 * @Description:
 * @Date: 2024-10-26 14:36:21
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-19 20:29:36
 * @LastEditors: 弈秋仙贝
 */

/* Includes ------------------------------------------------------------------*/
#include "encoder.h"
#include "user_config.h"
#include "fast_math.h"

uint8_t spi_check_parity(uint16_t v);
void spi_bb_delay(void);
int spi_polled_exchange(SPI_TypeDef *spip, uint16_t frame, uint16_t *data);

uint16_t read_err_cnt = 0;
static uint16_t reg_addr_03 = 0x8300;
static uint16_t reg_addr_04 = 0x8400;

/**
 * @brief check parity
 */
uint8_t spi_check_parity(uint16_t v)
{
	v ^= v >> 8;
	v ^= v >> 4;
	v ^= v >> 2;
	v ^= v >> 1;
	return v & 1;
}

int mt6816_read(int *raw)
{
	int ret = 0;
	uint16_t read_data;
	uint16_t reg_data_03;
	uint16_t reg_data_04;

	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	ret = spi_polled_exchange(SPI1, reg_addr_03, &reg_data_03);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
	if (ret < 0)
		goto TIMEOUT;

	spi_bb_delay();

	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	ret = spi_polled_exchange(SPI1, reg_addr_04, &reg_data_04);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

	if (ret < 0)
		goto TIMEOUT;

	read_data = (reg_data_03 << 8) + reg_data_04;

	if (reg_data_04 & 0x02)
		ret = -3;
	if (spi_check_parity(read_data) == 1)
		ret = -2;

	if (ret == 0)
		*raw = (read_data >> 2);

	return ret;
TIMEOUT:
	return ret;
}

/**
 * @brief
 * @param motor
 * @return int
 */
int sensor_absolute_read(mc_motor_typedef *tmotor)
{
	motor_encoder_typedef *tencoder = &tmotor->encoder;

	int ret = 0;

	ret = mt6816_read(&tencoder->raw);

	if (ret < 0)
	{
		read_err_cnt++;
		if (read_err_cnt > 300)
		{
			// err
			return ret;
		}
		tencoder->raw += tencoder->delta_cnt;
	}
	else
	{
		if (read_err_cnt)
			read_err_cnt--;
		if (tmotor->config.encoder_dir == DIR_REVERSE)
		{
			tencoder->raw = (tmotor->config.encoder_cpr - tencoder->raw);
		}
	}

	tencoder->cnt = tencoder->raw - tmotor->config.encoder_offset;
	// utils_norm_enc_value(&tencoder->cnt, tmotor->config.encoder_cpr); // -cpr_div2 ~ +cpr_div2
	int cpr_half = tmotor->config.encoder_cpr >> 1;
	while (tencoder->cnt < -cpr_half)
	{
		tencoder->cnt += tmotor->config.encoder_cpr;
	}
	while (tencoder->cnt > cpr_half)
	{
		tencoder->cnt -= tmotor->config.encoder_cpr;
	}
	tencoder->mech_rad = tencoder->cnt * tmotor->config.cpr_to_radian;
	tencoder->elec_rad = tencoder->mech_rad * tmotor->config.motor_pole_pairs;
	// utils_norm_angle_rad(&tencoder->elec_rad);
	while (tencoder->elec_rad < -M_PI)
    {
        tencoder->elec_rad += M_2PI;
    }
    while (tencoder->elec_rad >= M_PI)
    {
        tencoder->elec_rad -= M_2PI;
    }
	
	// 使用fir和iir滤波计算速度
	tencoder->delta_cnt = (tencoder->cnt - tencoder->pre_cnt[0]);
	for (int i = 0; i < 4; i++)
	{
		tencoder->pre_cnt[i] = tencoder->pre_cnt[i + 1];
	}
	tencoder->pre_cnt[4] = tencoder->cnt;
	while (tencoder->delta_cnt < -cpr_half)
	{
		tencoder->delta_cnt += tmotor->config.encoder_cpr;
	}
	while (tencoder->delta_cnt > cpr_half)
	{
		tencoder->delta_cnt -= tmotor->config.encoder_cpr;
	}
	float vel = tencoder->delta_cnt * 0.2f * CURRENT_CONTROL_FREQ * tmotor->config.cpr_to_radian;
	UTILS_LP_FAST(tencoder->mech_vel, vel, 0.15f);
	tencoder->elec_vel = tencoder->mech_vel * tmotor->config.motor_pole_pairs;
	tencoder->mech_rpm = RADPS2RPM * (tencoder->mech_vel);
	tencoder->mech_pos = tencoder->mech_rad;

	return 0;
}

/**
 * @brief   spi delay
 */
void spi_bb_delay(void)
{
	__NOP();
	__NOP();
	__NOP();
}

int spi_polled_exchange(SPI_TypeDef *pspi, uint16_t frame, uint16_t *data)
{
	/*
	 * Data register must be accessed with the appropriate data size.
	 * Byte size access (uint8_t *) for transactions that are <= 8-bit.
	 * Halfword size access (uint16_t) for transactions that are <= 8-bit.
	 */
	static const uint16_t timeout_cnt_max = 25;
	uint16_t spi_err_cnt = 0;
	uint32_t rece_data = 0;
	if ((pspi->CR2 & SPI_CR2_DS) != RESET)
	{
		pspi->DR = (uint16_t)frame;
		while ((pspi->SR & SPI_SR_RXNE) == 0)
		{
			spi_err_cnt++;
			if (spi_err_cnt >= timeout_cnt_max)
			{
				return -1;
			}
		}
		rece_data = pspi->DR;
		*data = (uint16_t)rece_data;
		return 0;
	}
	else
	{
		pspi->DR = frame;
		while ((pspi->SR & SPI_SR_RXNE) == 0)
		{
			spi_err_cnt++;
			if (spi_err_cnt >= timeout_cnt_max)
			{
				return -1;
			}
		}
		rece_data = pspi->DR;
		*data = (uint16_t)rece_data;
		return 0;
	}
}
