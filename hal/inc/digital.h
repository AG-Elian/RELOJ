/*********************************************************************************************************************
Copyright 2016-2025, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnologia
Universidad Nacional de Tucuman
http://www.microprocesadores.unt.edu.ar/

Copyright (c) 2026, Elian Leandro Aramallo Guantay <aramallog.elian@gmail.com>

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

//! Representa un evento en el que se ha detectado una activación.
#define ACTIVATE_EVENT 1

//! Representa un evento en el que se ha detectado una desactivación.
#define DEACTIVATE_EVENT -1

/* === Public data type declarations =============================================================================== */

/*!
 * @brief Tipo de dato opaco que representa una salida digital.
 * * Este puntero encapsula la estructura interna que maneja la configuración
 * y estado de un pin de salida digital del microcontrolador.
 */
typedef struct digital_output_s *digital_output_t;

/*!
 * @brief Tipo de dato opaco que representa una entrada digital.
 * * Este puntero encapsula la estructura interna que maneja la configuración
 * y el estado de un pin de entrada digital del microcontrolador.
 */
typedef struct digital_input_s *digital_input_t;


/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/*!
 * @brief Crea y configura una nueva salida digital.
 * * @param[in] puerto Identificador del puerto al que pertenece el pin (ej. GPIOA, GPIOB).
 * @param[in] terminal Número de pin dentro del puerto configurado como salida.
 * @return Puntero al objeto @c digital_output_t creado, o @c NULL si hubo un error.
 */
digital_output_t DigitalOutputCreate(uint32_t puerto, uint8_t terminal);

/*!
 * @brief Crea y configura una nueva entrada digital.
 * * @param[in] gpio Identificador del puerto al que pertenece el pin.
 * @param[in] bit Número de pin dentro del puerto configurado como entrada.
 * @param[in] inverted Booleano que indica si la lógica de entrada está invertida (pull-up / pull-down).
 * @return Puntero al objeto @c digital_input_t creado, o @c NULL si hubo un error.
 */
digital_input_t DigitalInputCreate(uint32_t gpio, uint8_t bit, bool inverted);

/*!
 * @brief Lee el estado lógico actual de una entrada digital.
 * * @param[in] input Puntero al objeto de la entrada digital que se desea leer.
 * @return @c true si la entrada está activa, @c false si está inactiva (considerando la lógica invertida si aplica).
 */
bool DigitalInputRead(digital_input_t input);

/*!
 * @brief Activa una salida digital específica.
 * * @param[in] salida Puntero al objeto de la salida digital a encender/activar.
 */
void DigitalOutputActivate(digital_output_t salida);

/*!
 * @brief Desactiva una salida digital específica.
 * * @param[in] salida Puntero al objeto de la salida digital a apagar/desactivar.
 */
void DigitalOutputDeactivate(digital_output_t salida);

/*!
 * @brief Invierte el estado actual de una salida digital (Toggle).
 * * @param[in] salida Puntero al objeto de la salida digital a alternar.
 */
void DigitalOutputToggle(digital_output_t salida);

/*!
 * @brief Consulta y actualiza los eventos detectados en una entrada digital.
 * * @param[in] self Puntero al objeto de la entrada digital.
 * @return Un entero indicando el último evento detectado (ej. @c ACTIVATE_EVENT, @c DEACTIVATE_EVENT).
 */
int DigitalInputGetEvent(digital_input_t self);

/*!
 * @brief Verifica si ha ocurrido un evento de activación (flanco de subida o bajada según configuración) desde la última consulta.
 * * @param[in] input Puntero al objeto de la entrada digital.
 * @return @c true si la entrada pasó a estado activo, @c false en caso contrario.
 */
bool DigitalInputHasActivated(digital_input_t input);

/*!
 * @brief Verifica si ha ocurrido un evento de desactivación desde la última consulta.
 * * @param[in] input Puntero al objeto de la entrada digital.
 * @return @c true si la entrada pasó a estado inactivo, @c false en caso contrario.
 */
bool DigitalInputHasDeactivated(digital_input_t input);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* DIGITAL_H_ */