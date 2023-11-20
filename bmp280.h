/*
 * bmp280.h
 * 
 * Created on: 10.11.2023
 * Author: rRaufToprak
*/
#ifndef BMP280_H
#define BMP280_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"

#define BMP280_ADDR 0x76<<1
#define BMP280_ID 0xD0
#define BMP280_RESET 0xE0
#define BMP280_STATUS 0xF3
#define BMP280_CTL_MEAS 0xF4
#define BMP280_CONFIG 0xF5
#define BMP280_PRESS_MSB 0xF7
#define BMP280_PRESS_LSB 0xF8
#define BMP280_PRESS_XLSB 0xF9
#define BMP280_TEMP_MSB 0xFA
#define BMP280_TEMP_LSB 0xFB
#define BMP280_TEMP_XLSB 0xFC
#define BMP280_REG_CALIB 0x88
typedef struct{
	int32_t t_fine;
	int32_t adc_temp, adc_press;
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	float temperature, pressure, altitude;
}BMP280_HandleTypeDef;

extern I2C_HandleTypeDef hi2c1;

void BMP280Init(void);
void BMP280Reset(void);
uint8_t BMP280_SINGLE_READ_REG(uint8_t addr);
void BMP280_SINGLE_WRITE_REG(uint8_t addr, uint8_t data);
int32_t BMP280_READ_REG16(uint8_t addr);
bool BMP280_TEST_ID(void);
void BMP280_READ_CALIBRATION(void);
void BMP280_READ_ADC(void);
int32_t BMP280_COMPENSATE_TEMPERATURE(void);
uint32_t BMP280_COMPENSATE_PRESSURE(void);
float get_temperature(void);
float get_pressure(void);
float get_altitude(void);
#endif

