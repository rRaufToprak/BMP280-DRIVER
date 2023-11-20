/*
 * bmp280.c
 * 
 * Created on: 10.11.2023
 * Author: rRaufToprak
*/
#include "bmp280.h"

BMP280_HandleTypeDef bmp280;

void BMP280_SINGLE_WRITE_REG(uint8_t addr, uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c1, 0x76<<1, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}
uint8_t BMP280_SINGLE_READ_REG(uint8_t addr)
{
	uint8_t data;
	HAL_I2C_Mem_Read(&hi2c1, 0x76<<1, addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
	return data;
}
bool BMP280_TEST_ID(void)
{
	uint8_t addr = 0xD0;
	uint8_t id;
	id = BMP280_SINGLE_READ_REG(addr);
	if(id == 0x58) return true;
	else return false;
}
void BMP280Init(void)
{
	BMP280_SINGLE_WRITE_REG(0xE0, 0xB6);
	BMP280_SINGLE_WRITE_REG(BMP280_CTL_MEAS, 0x00);
	BMP280_SINGLE_WRITE_REG(BMP280_CONFIG, 0x60);
	BMP280_SINGLE_WRITE_REG(BMP280_CTL_MEAS, 0x6F);
	BMP280_READ_CALIBRATION();
}
int32_t BMP280_READ_REG16(uint8_t addr){
	uint8_t data[2];
	int16_t reg16_data;
	HAL_I2C_Mem_Read(&hi2c1, 0x76<<1, addr, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
	reg16_data = data[1] << 8 | data[0];
	
	return reg16_data;
}
void BMP280_READ_CALIBRATION(void)
{
	bmp280.dig_T1 = BMP280_READ_REG16(0x88);
	bmp280.dig_T2 = BMP280_READ_REG16(0x8A);
	bmp280.dig_T3 = BMP280_READ_REG16(0x8C);
	bmp280.dig_P1 = BMP280_READ_REG16(0x8E);
	bmp280.dig_P2 = BMP280_READ_REG16(0x90);
	bmp280.dig_P3 = BMP280_READ_REG16(0x92);
	bmp280.dig_P4 = BMP280_READ_REG16(0x94);
	bmp280.dig_P5 = BMP280_READ_REG16(0x96);
	bmp280.dig_P6 = BMP280_READ_REG16(0x98);
	bmp280.dig_P7 = BMP280_READ_REG16(0x9A);
	bmp280.dig_P8 = BMP280_READ_REG16(0x9C);
	bmp280.dig_P9 = BMP280_READ_REG16(0x9E);
}
void BMP280_READ_ADC()
{
	uint8_t adc_data[6];
	HAL_I2C_Mem_Read(&hi2c1, 0x76<<1, 0xF7, I2C_MEMADD_SIZE_8BIT, adc_data, 6, 1000);
	bmp280.adc_press = adc_data[0] << 12 | adc_data[1] << 4 | adc_data[2] >> 4;
	bmp280.adc_temp = adc_data[3] << 12 | adc_data[4] << 4 | adc_data[5] >> 4;
}
int32_t BMP280_COMPENSATE_TEMPERATURE()//Formula from bmp280 datasheet
{

	int32_t var1, var2, T;
	var1 = ((((bmp280.adc_temp>>3) - ((int32_t)bmp280.dig_T1<<1))) * ((int32_t) bmp280.dig_T2))>>11;
	var2 = (((((bmp280.adc_temp>>4) - ((int32_t)bmp280.dig_T1)) * ((bmp280.adc_temp >> 4) - ((int32_t)bmp280.dig_T1))) >> 12) * ((int32_t)bmp280.dig_T3)) >> 14;
	
	bmp280.t_fine = var1 + var2;
	
	T = (bmp280.t_fine * 5 + 128)>>8;
	bmp280.temperature = T/100;
	return T;
}
uint32_t BMP280_COMPENSATE_PRESSURE()//Formula from bmp280 datasheet
{
	int32_t var1, var2;
	uint32_t p;
	
	var1 = (((int32_t)bmp280.t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2)*(var1>>2)) >> 11) * ((int32_t)bmp280.dig_P6);
	var2 = var2 + ((var1 * (int32_t)bmp280.dig_P5) << 1);
	
	var2 = (var2>>2)+(((int32_t)bmp280.dig_P4)<<16);
	var1 = (((bmp280.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)bmp280.dig_P2) * var1)>>1))>>18;
	var1 = ((((32768+var1))*((int32_t)bmp280.dig_P1))>>15);	
	
	if(var1 == 0){
		return 0;
	}
  p = (((uint32_t)(((int32_t)1048576)-bmp280.adc_press)-(var2>>12)))*3125; 
	if (p < 0x80000000){
	p = (p << 1) / ((uint32_t)var1);
	}else{
	p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)bmp280.dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)bmp280.dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + bmp280.dig_P7) >> 4));
	bmp280.pressure = p/100;
	return p;
}

float get_temperature(void)//returns temperature value in DegC
{
	return bmp280.temperature;
}
float get_pressure(void)//returns pressure value in hPa
{
	return bmp280.pressure;
}
float get_altitude(void)//returns altitude value in meters
{
	bmp280.altitude = 44330 * (1.0 - pow((bmp280.pressure)/1013.25, 0.1903));//formula from bmp180 datasheet
	return bmp280.altitude;
}

