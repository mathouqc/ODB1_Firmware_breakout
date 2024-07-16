/*
 * L76LM33_tests.c
 *
 *  Created on: Jul 12, 2024
 *      Author: mathouqc
 */

#include "GAUL_Drivers/Tests/L76LM33_tests.h"

#include "GAUL_Drivers/L76LM33.h"
#include "stdio.h"

extern L76LM33 L76_data;
extern UART_HandleTypeDef huart2; // UART via USB on NUCLEO-F103RB

uint8_t L76TEST_RX_Buffer[120];

int8_t L76LM33_TESTS_LogSentenceUART() {
    // Debug timer High (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    // CODE
    if (L76LM33_ReadSentence(L76TEST_RX_Buffer, sizeof(L76TEST_RX_Buffer)) != 0) {
    	printf("Error L76LM33_ReadSentence\r\n");
    }

    // Debug timer Low (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

    // UART log
    HAL_UART_Transmit(&huart2, L76TEST_RX_Buffer, sizeof(L76TEST_RX_Buffer), 1000);

    return 0; // OK
}

int8_t L76LM33_TESTS_LogSentenceSTLINK() {
    // Debug timer High (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    // CODE
    if (L76LM33_ReadSentence(L76TEST_RX_Buffer, sizeof(L76TEST_RX_Buffer)) != 0) {
    	printf("Error L76LM33_ReadSentence\r\n");
    }

    // Debug timer Low (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

    // STLINK log
	//printf("%s\r\n", L76TEST_RX_Buffer);

    return 0; // OK
}

int8_t L76LM33_TESTS_LogDataUART() {
    // Debug timer High (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    // CODE

    // Debug timer Low (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

    // UART log
    char Data[36];
    sprintf(Data, "DATA\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)Data, 36, 1000);

    return 0; // OK
}

int8_t L76LM33_TESTS_LogDataSTLINK() {
    // Debug timer High (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    // CODE

    // Debug timer Low (to measure execution time with a digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

    // STLINK log
	printf("DATA\r\n");

    return 0; // OK
}
