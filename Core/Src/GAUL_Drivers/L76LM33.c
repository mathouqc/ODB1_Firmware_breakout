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

// Pointer to UART handler
UART_HandleTypeDef *L76_huart;

// Multi purpose receiving buffer
uint8_t L76_RX_Buffer[120];

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
 * - Set UART handler
 * - Only output GPRMC sentence
 * - Set navigation mode
 *
 * @param L76_data: pointer to a L76LM33 structure.
 * @param huart: pointer to a HAL UART handler.
 *
 * @retval 0 OK
 */
int8_t L76LM33_Init(L76LM33 *L76_data, UART_HandleTypeDef *huart) {
	// Set UART handler
	L76_huart = huart;

	// Only output GPRMC sentence
    char NMEA_RMC[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35\r\n";
    L76LM33_SendCommand(NMEA_RMC, sizeof(NMEA_RMC));
    // Set navigation mode
    char NMEA_NAVMODE[] = "PMTK886,2*2A\r\n";
    L76LM33_SendCommand(NMEA_NAVMODE, sizeof(NMEA_NAVMODE));

    return 0; // OK
}

/**
 * Read and parse a NMEA GPRMC sentence into data structure.
 *
 * @param L76_data: pointer to a L76LM33 structure.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 *
 */
int8_t L76LM33_Read(L76LM33 *L76_data) {
	// Read sentence
	L76LM33_ReadSentence(L76_RX_Buffer, sizeof(L76_RX_Buffer));

	// Parse sentence
	// TODO

	return 0; // OK
}

/**
 * Read NMEA sentence from UART into RX_Buffer.
 *
 * @param RX_Buffer[]: u8bit array to store NMEA sentence.
 * @param bufferSize: size of receiving buffer.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 *
 */
int8_t L76LM33_ReadSentence(uint8_t RX_Buffer[], uint16_t bufferSize) {
	// Clear buffer
	for (int16_t i = 0; i < bufferSize; i++) {
		RX_Buffer[i] = 0;
	}

	if (L76LM33_FindStartingChar(RX_Buffer, 100) != 0) {
		return -1; // Error, cannot find starting character or error with UART
	}

	if (L76LM33_ReadUntilEndingChar(RX_Buffer, 1, 100) != 0) {
		return -1; // Error, cannot find ending character or error with UART
	}

	return 0;
}

/**
 * Tries to find the starting character ($) by reading a maximum number of
 * characters defined by maxIterations.
 *
 * @param maxIterations: maximum number of characters to read to find
 * starting character.
 *
 * @retval 0 Found starting character.
 * @retval -1 Error while reading UART.
 * @retval -2 Error, cannot find starting character in n iterations.
 *
 */
int8_t L76LM33_FindStartingChar(uint8_t RX_Buffer[], uint16_t maxIterations) {
	// Try to find '$'
	for (uint16_t i = 0; i < maxIterations; i++) {
		// Read character from the GPS module
		if (HAL_UART_Receive(L76_huart, RX_Buffer, 1, L76LM33_UART_TIMEOUT) != HAL_OK) {
			return -1; // Error while reading UART
		}

		if (RX_Buffer[0] == '$') {
			return 0; // Found starting characters
		}
	}

	return -2; // Error, cannot find starting character in n iterations
}

/**
 * Read fron UART into RX_Buffer. It stores each character starting at startIdx
 * in RX_Buffer until '\n' is found.
 *
 * @param RX_Buffer[]: u8bit array to store received characters.
 * @param startIdx: starting index to write to RX_Buffer and starting index of the loop. Has
 * to be inside RX_Buffer size.
 * @param maxLen: maximum number of character to read into RX_Buffer. Has to be smaller than
 * RX_Buffer size.
 *
 * @retval 0 Found ending character.
 * @retval -1 Error while reading UART.
 * @retval -2 Error, cannot find '\n' in n iterations.
 *
 */
int8_t L76LM33_ReadUntilEndingChar(uint8_t RX_Buffer[], uint16_t startIdx, uint16_t maxLen) {
	// Read sentence
	for (uint16_t i = startIdx; i < maxLen; i++) {
		uint8_t c; // uint8 for HAL_UART_Receive
		if (HAL_UART_Receive(L76_huart, &c, 1, L76LM33_UART_TIMEOUT) != HAL_OK) {
			return -1; // Error while reading UART
		}

		// Add character to buffer
		RX_Buffer[i] = c;

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

    HAL_UART_Transmit(L76_huart, (uint8_t *)command, size, L76LM33_UART_TIMEOUT); // TODO: add error handler

    return 0; // OK
}
