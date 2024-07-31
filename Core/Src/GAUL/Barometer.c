/*
 * Barometer.c
 *
 *  Created on: Jul 8, 2024
 *      Author: mathouqc
 */

#include "GAUL/Barometer.h"

#include "GAUL_Drivers/BMP280.h"

extern SPI_HandleTypeDef hspi2;

BMP280 _bmp_data;

/**
 * Initialize sensors for barometer.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 */
int8_t BAROMETER_Init() {
	if (BMP280_Init(&_bmp_data, &hspi2) != 0) {
		return -1;
	}

	return 0;
}

/**
 * Reads temperature and pressure values from the sensors,
 * then calculate the altitude from them.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 */
int8_t BAROMETER_ReadAltitude(Barometer *barometer) {
	if (BMP280_ReadAltitude(&_bmp_data) != 0) {
		return -1;
	}

	barometer->altitude_m = _bmp_data.alt_m;

	return 0;
}

/**
 * Reads temperature and pressure values from the sensors,
 * then calculate the altitude from them and validate the values
 * using macros.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 * @retval -2 VALIDATION ERROR
 */
int8_t BAROMETER_ReadAltitude_Validation(Barometer *barometer) {
	if (BMP280_ReadAltitude(&_bmp_data) != 0) {
		return -1; // Error reading altitude
	}

	if (_bmp_data.temp_C < BAROMETER_MIN_TEMP_C || _bmp_data.temp_C > BAROMETER_MAX_TEMP_C) {
		return -2; // Temperature exceed thresholds
	}

	if (_bmp_data.press_Pa < BAROMETER_MIN_PRESS_PA || _bmp_data.press_Pa > BAROMETER_MAX_PRESS_PA) {
		return -2; // Pressure exceed thresholds
	}

	if (_bmp_data.alt_m < BAROMETER_MIN_ALT_M || _bmp_data.alt_m > BAROMETER_MAX_ALT_M) {
		return -2; // Altitude exceed thresholds
	}

	barometer->altitude_m = _bmp_data.alt_m;

	return 0;
}
