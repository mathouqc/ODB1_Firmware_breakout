/*
 * NMEA.h
 *
 *  Created on: Mar 12, 2024
 *      Author: gagnon
 *
 *  Edited on: Jul 4, 2024
 *      Autor: mathouqc
 */

#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <stdint.h>

#ifndef INC_GAUL_DRIVERS_NMEA_H
#define INC_GAUL_DRIVERS_NMEA_H

// Définition des valeurs par défaut en tant que macros
#define DEFAULT_FIX 		56 // "V" (no fix)
#define DEFAULT_LATITUDE 	0x00000000
#define DEFAULT_LONGITUDE 	0x00000000
#define DEFAULT_INDICATOR	0x00

typedef struct {
    int32_t time;         			// Heure en bytes
    uint8_t fix;					// 0: no fix, 1: fix
    int32_t latitude;     			// Latitude en bytes
    uint8_t latitude_indicator; 	// Indicateur de latitude (N ou S)
    int32_t longitude;				// Longitude en bytes
    uint8_t longitude_indicator;	// Indicateur de longitude (E ou W)
} GPS_Data;

uint8_t NMEA_Decode_GPRMC(const char *nmea_sentence, GPS_Data *gps_data);
uint8_t NMEA_ValidTrame(const char *nmea_sentence);

#endif /* INC_GAUL_DRIVERS_NMEA_H */
