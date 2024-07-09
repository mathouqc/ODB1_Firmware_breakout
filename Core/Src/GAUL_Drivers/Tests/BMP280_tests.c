/*
 * BMP280_tests.c
 *
 *  Created on: Jul 8, 2024
 *      Author: mathouqc
 */

#include "GAUL_Drivers/Tests/BMP280_tests.h"

#include "GAUL_Drivers/BMP280.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

extern BMP280 bmp_data;
extern UART_HandleTypeDef huart2;

int8_t BMP280_TESTS_LogUART() {
    // Debug timer High (for digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    if (BMP280_ReadAltitude(&bmp_data) != 0) {
    	printf("ReadAltitude Error\r\n");
    	return -1; // Error
    }

    // Debug timer Low (for digital analyzer)
    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

    // UART log
    char Data[32];
    sprintf(Data, "%6.1f kPa %5.1f C %8.2f m\r\n", bmp_data.press_Pa / 1000, bmp_data.temp_C, bmp_data.alt_m);
    HAL_UART_Transmit(&huart2, (uint8_t *)Data, 32, 1000);

    return 0; // OK
}
