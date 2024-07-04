/*
 * BMP280.h
 *
 *  Created on: May 18, 2024
 *      Author: gagnon
 *
 *  Edited on: Jul 4, 2024
 *  	Autor: mathouqc
 */

#include "GAUL_Drivers/BMP280.h"

extern SPI_HandleTypeDef hspi2;

uint8_t RX_Buffer[26];

uint8_t BMP280_Init(BMP280 *BMP_data) {

	// CS_BMP HIGH
	HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

	// Reset
    BMP280_Write(BMP280_REG_RESET, BMP280_RESET_VALUE);
    HAL_Delay(2); // wait for bmp280 reset

    // Check ID
    BMP280_Read(BMP280_REG_ID, RX_Buffer, 1);
    if (RX_Buffer[0] != BMP280_DEVICE_ID) {
        return 1; // Error
    }

    // Calibration
    BMP280_ReadCalibrationData(BMP_data);

    // Set data acquisition options (temp/press oversampling and power mode)
    BMP280_Write(BMP280_REG_CTRL_MEAS, BMP280_SETTING_CTRL_MEAS_NORMAL);
    // Set configuration (rate, filter and interface options)
    BMP280_Write(BMP280_REG_CONFIG, BMP280_SETTING_CONFIG);


    // Ajuster reference
    //BMP280_MeasureReference(BMP_data, T0, 101325.0);


    return 0; // OK
}

uint8_t BMP280_ReadCalibrationData(BMP280 *BMP_data) {
	BMP280_Read(BMP280_REG_CALIB_00, RX_Buffer, 26);

	BMP_data->calib_data.dig_T1 = (RX_Buffer[1] << 8) | RX_Buffer[0];
	BMP_data->calib_data.dig_T2 = (RX_Buffer[3] << 8) | RX_Buffer[2];
	BMP_data->calib_data.dig_T3 = (RX_Buffer[5] << 8) | RX_Buffer[4];
	BMP_data->calib_data.dig_P1 = (RX_Buffer[7] << 8) | RX_Buffer[6];
	BMP_data->calib_data.dig_P2 = (RX_Buffer[9] << 8) | RX_Buffer[8];
    BMP_data->calib_data.dig_P3 = (RX_Buffer[11] << 8) | RX_Buffer[10];
    BMP_data->calib_data.dig_P4 = (RX_Buffer[13] << 8) | RX_Buffer[12];
    BMP_data->calib_data.dig_P5 = (RX_Buffer[15] << 8) | RX_Buffer[14];
    BMP_data->calib_data.dig_P6 = (RX_Buffer[17] << 8) | RX_Buffer[16];
    BMP_data->calib_data.dig_P7 = (RX_Buffer[19] << 8) | RX_Buffer[18];
    BMP_data->calib_data.dig_P8 = (RX_Buffer[21] << 8) | RX_Buffer[20];
    BMP_data->calib_data.dig_P9 = (RX_Buffer[23] << 8) | RX_Buffer[22];

    return 0; // OK
}

uint8_t BMP280_ReadTemperature(BMP280 *BMP_data) {
	// Wait until device is available
	/*BMP280_Read(BMP280_REG_STATUS, RX_Buffer, 1);
	while((RX_Buffer[0] & 0x04) != 0 || (RX_Buffer[0] & 0x01) != 0);*/

	// Read BMP280 adc values
	BMP280_Read(BMP280_REG_TEMP_MSB, RX_Buffer, 3);

    int32_t adc_T = (RX_Buffer[0] << 12) | (RX_Buffer[1] << 4) | ((RX_Buffer[2] >> 4) & 0x0F);

    int32_t var1 = ((((adc_T >> 3) - ((int32_t)BMP_data->calib_data.dig_T1 << 1))) * ((int32_t)BMP_data->calib_data.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)BMP_data->calib_data.dig_T1)) * ((adc_T >> 4) - ((int32_t)BMP_data->calib_data.dig_T1))) >> 12) * ((int32_t)BMP_data->calib_data.dig_T3)) >> 14;
    BMP_data->t_fine = var1 + var2;

    int32_t T = (BMP_data->t_fine * 5 + 128) >> 8;
    BMP_data->temp_C = (float)(T / 100.0); // + devBMP->temperature_ref;

    return 0; // OK
}

/*uint8_t BMP280_ReadPressure(BMP280 *devBMP) {

	// Verifie si la lecture est possible
	while((BMP280_ReadRegister(BMP280_REG_STATUS) & 0x04) != 0 || (BMP280_ReadRegister(BMP280_REG_STATUS) & 0x01) != 0);

    int32_t adc_P = (BMP280_ReadRegister(BMP280_REG_PRESS_MSB) << 12) |
                    (BMP280_ReadRegister(BMP280_REG_PRESS_LSB) << 4) |
                    ((BMP280_ReadRegister(BMP280_REG_PRESS_XLSB) >> 4) & 0x0F);

    int64_t var1 = ((int64_t)devBMP->t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)devBMP->calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)devBMP->calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)devBMP->calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)devBMP->calib_data.dig_P3) >> 8) + ((var1 * (int64_t)devBMP->calib_data.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)devBMP->calib_data.dig_P1) >> 33;
    if (var1 == 0) {
        return 0; // Error (division par 0)
    }
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)devBMP->calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)devBMP->calib_data.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)devBMP->calib_data.dig_P7) << 4);
    p = (uint32_t)p;

    devBMP->pressure_Pa = (float)p / (float)(1 << 8); // + devBMP->pressure_ref;
    return 1; // OK
}*/

/*
uint8_t BMP280_SwapMode(uint8_t mode) {

	if(BMP280_ReadRegister(BMP280_REG_CTRL_MEAS) != mode) {
		BMP280_WriteRegister(BMP280_REG_CTRL_MEAS, mode); // BMP280_SETTING_CTRL_MEAS_NORMAL (0x57)
		printf("BMP mode set to: %i/n", BMP280_SETTING_CTRL_MEAS_NORMAL);
		return 1; // OK
	} else {
		printf("BMP mode set error...");
		return 0; // Error (no change)
	}
}

uint8_t BMP280_MeasureReference(BMP280 *devBMP, float temp_ref, float press_ref) {

    devBMP->temperature_ref = BMP280_ReadTemperature(devBMP) - temp_ref;
    devBMP->pressure_ref = BMP280_ReadPressure(devBMP) - press_ref;
    return 1; // OK
}

float BMP280_PressureToAltitude(float pressure) {

	float altitude = (T0 / alpha) * (1 - pow((pressure / P0), (1 / beta)));
    return altitude;
}*/

uint8_t BMP280_Read(uint8_t reg, uint8_t RX_Buffer[], uint8_t size) {
	// BMP_CS LOW
	HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);

	// Transmit Control byte (Read mode + Register address)
	reg |= 0x80; // Read mode
	HAL_SPI_Transmit(&hspi2, &reg, 1, SPI_TIMEOUT);

	// Receive Data byte
	HAL_SPI_Receive(&hspi2, RX_Buffer, size, SPI_TIMEOUT);

	// BMP_CS HIGH
	HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

	return 0; // OK
}

uint8_t BMP280_Write(uint8_t reg, uint8_t data) {
	// BMP_CS LOW
	HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET);

	// Control byte (Write mode + Register address)
	reg &= ~0x80; // Write mode

	// Transmit Control byte and Data byte
	uint8_t TX_Buffer[2] = { reg, data };
	HAL_SPI_Transmit(&hspi2, TX_Buffer, 2, SPI_TIMEOUT);

	// BMP_CS HIGH
	HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET);

    return 0; // OK
}
