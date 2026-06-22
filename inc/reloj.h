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

#ifndef RELOJ_H_
#define RELOJ_H_

/** @file reloj.h
 ** @brief Definición de tipos y funciones para el manejo de un reloj
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* Índices del arreglo de la hora (formato BCD sin compactar) */
#define HOUR_TENS   0  // Decenas de hora en la primera posición
#define HOUR_ONES   1  // Unidades de hora
#define MINUTE_TENS 2  // Decenas de minutos
#define MINUTE_ONES 3  // Unidades de minutos
#define SECOND_TENS 4  // Decenas de segundos
#define SECOND_ONES 5  // Unidades de segundos en la última posición

/* === Public data type declarations =============================================================================== */

typedef struct clock_s *clock_t;
typedef uint8_t hora_t[6];

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

clock_t RelojCreate(unsigned int ticks_perseconds, void (*tick_callback)(void));
bool RelojGetHora(clock_t reloj, hora_t hora_actual);
bool RelojSetHora(clock_t reloj, hora_t hora_actual);
void RelojTick(clock_t reloj);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* RELOJ_H_ */