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

#include "circular_buffer.h"

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
int8_t L76LM33_Init(UART_HandleTypeDef *huart) {
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
//int8_t L76LM33_Read(GPS_Data *gps_data) {
int8_t L76LM33_Read(L76LM33 *L76_Data) {
	// Read sentence
	if (L76LM33_ReadSentence() != 0) {
		return -1; // Error, don't update
	}

	// Debug received NMEA sentence
	printf("%s\r\n", L76_NMEA_Buffer);

	// Parse sentence
	/*switch (minmea_sentence_id(line, false)) {
		case MINMEA_SENTENCE_RMC: {
			struct minmea_sentence_rmc frame;
			if (minmea_parse_rmc(&frame, line)) {
				printf("$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
						minmea_tocoord(&frame.latitude),
						minmea_tocoord(&frame.longitude),
						minmea_tofloat(&frame.speed));
			}
			else {
				printf("$xxRMC sentence is not parsed\n");
			}
		} break;

		case MINMEA_INVALID: {
			printf("$xxxxx sentence is not valid\n");
		} break;

		default: {
			printf("$xxxxx sentence is not parsed\n");
		} break;
	}*/

	/*if (NMEA_ValidTrame(L76_NMEA_Buffer) != 0 || NMEA_Decode_GPRMC(L76_NMEA_Buffer, gps_data) != 0) {
		return -1; // Cannot parse NMEA sentence
	}*/

	return 0; // OK
}

/**
 * Read NMEA sentence from UART circular buffer into a NMEA buffer.
 *
 * @retval 0 OK
 * @retval -1 ERROR
 * @retval -2 Error, cannot find starting or ending character.
 *
 */
int8_t L76LM33_ReadSentence() {
	if (circular_buffer_empty(L76_circularBuffer)) {
		return -1; // Error, empty UART circular buffer
	}

	// Clear NMEA buffer
	for (int16_t i = 0; i < sizeof(L76_NMEA_Buffer); i++) {
		L76_NMEA_Buffer[i] = 0;
	}


	// Variable to store character from UART buffer
	uint8_t c;


	// Try to find '$' in 100 iterations
	for (uint16_t i = 0; i < 100; i++) {
		// Read character from UART buffer
		if (!circular_buffer_pop(L76_circularBuffer, &c)) {
			return -1; // Error, empty buffer
		}

		if (c == '$') {
			// Set starting character in NMEA buffer
			L76_NMEA_Buffer[0] = '$';

			break; // Found starting characters
		}
	}

	if (c != '$') {
		return -2; // Error, cannot find starting character in 100 iterations
	}


	// Read into NMEA buffer until ending character is found
	for (uint16_t i = 1; i < sizeof(L76_NMEA_Buffer); i++) {
		// Read character from UART buffer
		if (!circular_buffer_pop(L76_circularBuffer, &c)) {
			return -1; // Error, empty buffer
		}

		// Add character to NMEA buffer
		L76_NMEA_Buffer[i] = c;

		if (c == '\n') {
			break; // Found ending character
		}
	}

	if (c != '\n') {
		return -2; // Error, cannot find '\n'
	}

	return 0;
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
