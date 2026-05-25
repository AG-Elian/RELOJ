/*********************************************************************************************************************
Copyright 2016-2025, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnologia
Universidad Nacional de Tucuman
http://www.microprocesadores.unt.edu.ar/

Copyright 2016-2025, Esteban Volentini <evolentini@herrera.unt.edu.ar>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

#ifndef DIGITAL_H_
#define DIGITAL_H_

/** @file digital.h
 ** @brief Declaraciones de la biblioteca para gestión de entradas y salidas digitales
**/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

#define ACTIVATE_EVENT 1

#define DEACTIVATE_EVENT -1

/* === Public data type declarations =============================================================================== */

typedef struct digital_output_s*digital_output_t;

typedef struct digital_input_s*digital_input_t;

int DigitalInputGetEvent(digital_input_t self);

void DigitalOutputActivate(digital_output_t salida);

void DigitalOutputDeactivate(digital_output_t salida);

void DigitalOutputToggle(digital_output_t salida);

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

digital_output_t DigitalOutputCreate(uint32_t puerto, uint8_t terminal);

digital_input_t DigitalInputCreate(uint32_t gpio, uint8_t bit, bool inverted);

bool DigitalInputRead(digital_input_t input);

bool DigitalInputHasActivated(digital_input_t input);

bool DigitalInputHasDeactivated(digital_input_t input);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* DIGITAL_H_ */
