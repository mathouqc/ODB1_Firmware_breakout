/*
 * L76LM33.c
 *
 * L76LM33 is a GNSS module used to get the position (latitude, longitude) of the rocket.
 *
 *  Created on: May 12, 2024
 *      Author: gagnon
 *
 *  Edited on: Jul 4, 2024
 *      Autor: mathouqc
 */

#include "GAUL_Drivers/L76LM33.h"

#include "stdio.h" // Only for debug

// Pointer to UART handler
UART_HandleTypeDef *L76_huart;

// Received char/byte from UART
uint8_t L76_receivedByte;

// Circular buffer to store UART data from GNSS module
circularBuffer_t *L76_circularBuffer = NULL;

// Buffer to store NMEA sentence
uint8_t L76_NMEA_Buffer[128];


/*
 * Source:
 * LG76 Series GNSS Protocol Specification - Section 2.3. PMTK Messages
 *
 * Only output RMC (Recommended Minimum Specific GNSS Sentence) once every one position fix
 * NMEA_RMC[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35<CR><LF>"
 *
 * Set the navigation mode to "Aviation Mode" (for large acceleration movement, altitude of 10'000m max)
 * NMEA_NAVMODE = "PMTK886,2*2A<CR><LF>"
 */

/**
 * Initialize L76LM33 sensor.
 *
 * @param L76_data: pointer to a L76LM33 structure.
 * @param huart: pointer to a HAL UART handler.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 */
int8_t L76LM33_Init(L76LM33 *L76_data, UART_HandleTypeDef *huart) {
	// Set UART handler
	L76_huart = huart;

	// Initialize circular buffer
	L76_circularBuffer = circular_buffer_init(sizeof(char));

	// Receive UART data with interrupts
	if (HAL_UART_Receive_IT(L76_huart, &L76_receivedByte, 1) != HAL_OK) {
		return -1; // Error with UART
	}

	// Only output GPRMC sentence
    char NMEA_RMC[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35\r\n";
    if (L76LM33_SendCommand(NMEA_RMC, sizeof(NMEA_RMC)) != 0) {
    	return -1; // Error with UART
    }
    // Set navigation mode
    char NMEA_NAVMODE[] = "PMTK886,2*2A\r\n";
    if (L76LM33_SendCommand(NMEA_NAVMODE, sizeof(NMEA_NAVMODE)) != 0) {
    	return -1; // Error with UART
    }

    return 0; // OK
}

/**
 * Callback called on incoming UART data. It is called when HAL_UART_RxCpltCallback is called.
 * Add received byte to UART circular buffer.
 *
 * @param huart: pointer to a HAL UART handler triggering the callback
 */
void L76LM33_RxCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == L76_huart->Instance) {
		// Add data to circular buffer
		circular_buffer_push(L76_circularBuffer, &L76_receivedByte);
		// Receive UART data with interrupts
		HAL_UART_Receive_IT(L76_huart, &L76_receivedByte, 1);
	}
}

/**
 * Read and parse a NMEA GPRMC sentence into data structure. Call this function
 * frequently to have the latest GPS data available.
 *
 * @param L76_data: pointer to a L76LM33 structure to update.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 *
 */
int8_t L76LM33_Read(L76LM33 *L76_data) {
	// Read sentence
	if (L76LM33_ReadSentence() != 0) {
		return -1; // Error, don't update
	}

	// Parse sentence
	// TODO

	return 0; // OK
}

/**
 * Read NMEA sentence from UART circular buffer into a NMEA buffer.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 *
 */
int8_t L76LM33_ReadSentence() {
	if (circular_buffer_empty(L76_circularBuffer)) {
		return -1; // Error, empty circular buffer
	}

	// Clear buffer
	for (int16_t i = 0; i < sizeof(L76_NMEA_Buffer); i++) {
		L76_NMEA_Buffer[i] = 0;
	}

	if (L76LM33_FindStartingChar(100) != 0) {
		return -1; // Error, cannot find starting character
	}

	L76_NMEA_Buffer[0] = '$';

	if (L76LM33_ReadUntilEndingChar() != 0) {
		return -1; // Error, cannot find ending character
	}

	// Debug received NMEA sentence
	printf("%s\r\n", L76_NMEA_Buffer);

	return 0;
}

/**
 * Tries to find the starting character ($) by reading a maximum number of
 * characters from UART buffer.
 *
 * @param maxIterations: maximum number of characters to read to find starting character.
 *
 * @retval 0 Found starting character.
 * @retval -1 Error, empty buffer.
 * @retval -2 Error, cannot find starting character in n iterations.
 *
 */
int8_t L76LM33_FindStartingChar(uint16_t maxIterations) {
	// Try to find '$'
	for (uint16_t i = 0; i < maxIterations; i++) {
		// Variable to store character from UART buffer
		uint8_t c;
		// Read character from UART buffer
		if (!circular_buffer_pop(L76_circularBuffer, &c)) {
			return -1; // Error, empty buffer
		}

		if (c == '$') {
			return 0; // Found starting characters
		}
	}

	return -2; // Error, cannot find starting character in n iterations
}

/**
 * Read each character from UART buffer into NMEA_Buffer starting at startIdx until '\n' is found.
 *
 * @retval 0 Found ending character.
 * @retval -1 Error, empty buffer.
 * @retval -2 Error, cannot find '\n' in n iterations.
 *
 */
int8_t L76LM33_ReadUntilEndingChar() {
	// Read sentence
	for (uint16_t i = 1; i < sizeof(L76_NMEA_Buffer); i++) {
		// Variable to store character from UART buffer
		uint8_t c;
		// Read character from UART buffer
		if (!circular_buffer_pop(L76_circularBuffer, &c)) {
			return -1; // Error, empty buffer
		}

		// Add character to NMEA buffer
		L76_NMEA_Buffer[i] = c;

		if (c == '\n') {
			return 0; // Found ending character
		}
	}

	return -2; // Error, cannot find '\n' in n iterations
}

/**
 * Send array of character to L76LM33 using UART HAL functions.
 *
 * @param command[]: array of character to send.
 * @param size: size of the data to send.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 */
int8_t L76LM33_SendCommand(char command[], uint8_t size) {
    if (command == NULL) {
        return -1; // Error
    }

    if (HAL_UART_Transmit(L76_huart, (uint8_t *)command, size, L76LM33_UART_TIMEOUT) != HAL_OK) {
    	return -1; // Error with UART
    }

    return 0; // OK
}
