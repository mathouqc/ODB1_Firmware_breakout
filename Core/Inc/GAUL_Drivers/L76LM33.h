/*
 * L76LM33.h
 *
 *  Created on: May 12, 2024
 *      Author: gagnon
 *
 *  Edited on: Jul 4, 2024
 *      Autor: mathouqc
 */

#include "stm32f1xx_hal.h"
#include "circular_buffer.h"

#ifndef INC_GAUL_DRIVERS_L76LM33_H_
#define INC_GAUL_DRIVERS_L76LM33_H_

#define L76LM33_UART_TIMEOUT 1000

typedef struct {
    int32_t time;         			// Heure en bytes
    int32_t latitude;     			// Latitude en bytes
    uint8_t latitude_indicator; 	// Indicateur de latitude (N ou S)
    int32_t longitude;				// Longitude en bytes
    uint8_t longitude_indicator;	// Indicateur de longitude (E ou W)
} L76LM33;

int8_t L76LM33_Init(L76LM33 *L76_data, UART_HandleTypeDef *huart);

void L76LM33_RxCallback(UART_HandleTypeDef *huart);

int8_t L76LM33_Read(L76LM33 *L76_data);
int8_t L76LM33_ReadSentence();
int8_t L76LM33_FindStartingChar(uint16_t maxIterations);
int8_t L76LM33_ReadUntilEndingChar();

int8_t L76LM33_SendCommand(char command[], uint8_t size);

#endif /* INC_GAUL_DRIVERS_L76LM33_H_ */
