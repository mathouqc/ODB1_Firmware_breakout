/*
 * Barometer.h
 *
 *  Created on: Jul 8, 2024
 *      Author: mathouqc
 */

#include "stm32f1xx_hal.h"

#ifndef INC_GAUL_BAROMETER_H_
#define INC_GAUL_BAROMETER_H_

#define BAROMETER_MIN_TEMP_C 0
#define BAROMETER_MAX_TEMP_C 100

#define BAROMETER_MIN_PRESS_PA  10000 // 69000 Pa at 10k ft, 30000 Pa at 30k ft, 10000 Pa at 50k ft
#define BAROMETER_MAX_PRESS_PA 150000

#define BAROMETER_MIN_ALT_M -200
#define BAROMETER_MAX_ALT_M 20000 // 65k ft

#define BAROMETER_MIN_SPD_MPS -1715 // Mach 5
#define BAROMETER_MAX_SPD_MPS 1715 // Mach 5

#define BAROMETER_MIN_ACC_MPSS -300
#define BAROMETER_MAX_ACC_MPSS 300

typedef struct {
	float altitude_m;
	float speed_mps;
	float acceleration_mps2;
} Barometer;

int8_t BAROMETER_Init();

int8_t BAROMETER_ReadAltitude(Barometer *barometer);
int8_t BAROMETER_ReadAltitude_Validation(Barometer *barometer);

#endif /* INC_GAUL_BAROMETER_H_ */
