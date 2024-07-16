/*
 * L76LM33_tests.h
 *
 *  Created on: Jul 12, 2024
 *      Author: mathouqc
 */

#include "stm32f1xx_hal.h"

#ifndef INC_GAUL_DRIVERS_TESTS_L76LM33_TESTS_H_
#define INC_GAUL_DRIVERS_TESTS_L76LM33_TESTS_H_

#define DEBUG_Pin GPIO_PIN_7
#define DEBUG_GPIO_Port GPIOC

int8_t L76LM33_TESTS_LogSentenceUART();
int8_t L76LM33_TESTS_LogSentenceSTLINK();
int8_t L76LM33_TESTS_LogDataUART();
int8_t L76LM33_TESTS_LogDataSTLINK();

#endif /* INC_GAUL_DRIVERS_TESTS_L76LM33_TESTS_H_ */
