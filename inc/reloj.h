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

/*! 
 * @file reloj.h
 * @brief Definición de tipos y funciones (API) para el manejo de un reloj y su alarma.
 */

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/*! @brief Índices del arreglo de la hora (formato BCD sin compactar) */
#define HOUR_TENS   0  //!< Decenas de hora en la primera posición
#define HOUR_ONES   1  //!< Unidades de hora
#define MINUTE_TENS 2  //!< Decenas de minutos
#define MINUTE_ONES 3  //!< Unidades de minutos
#define SECOND_TENS 4  //!< Decenas de segundos
#define SECOND_ONES 5  //!< Unidades de segundos en la última posición

/* === Public data type declarations =============================================================================== */

/*! @brief Puntero opaco a la estructura que gestiona el estado interno del reloj */
typedef struct clock_s *clock_t;

/*! @brief Arreglo de 6 bytes para representar la hora o alarma en formato BCD */
typedef uint8_t hora_t[6];

/* === Public function declarations ================================================================================ */

/*!
 * @brief Crea e inicializa una instancia del reloj.
 * @param[in] ticks_perseconds Cantidad de llamadas a RelojTick necesarias para que transcurra un segundo.
 * @param[in] tick_callback Puntero a la función que se ejecutará cuando la alarma coincida con la hora actual.
 * @return Puntero a la instancia del reloj creado.
 */
clock_t RelojCreate(unsigned int ticks_perseconds, void (*tick_callback)(void));

/*!
 * @brief Consulta la hora actual del reloj.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[out] hora_actual Arreglo donde se almacenará la hora consultada.
 * @return true si la hora del reloj es válida (ya fue ajustada), false si es inválida.
 */
bool RelojGetHora(clock_t reloj, hora_t hora_actual);

/*!
 * @brief Ajusta la hora actual del reloj.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[in] hora_actual Arreglo con la hora deseada a configurar.
 * @return true indicando que la configuración fue exitosa.
 */
bool RelojSetHora(clock_t reloj, hora_t hora_actual);

/*!
 * @brief Incrementa el contador interno de ticks del reloj.
 * Al alcanzar la cantidad de ticks por segundo, avanza la hora. Evalúa el disparo de la alarma.
 * @param[in] reloj Puntero a la instancia del reloj.
 */
void RelojTick(clock_t reloj);

/*!
 * @brief Configura la hora de disparo de la alarma.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[in] hora_alarma Arreglo con la hora a la que debe sonar la alarma.
 */
void RelojSetAlarma(clock_t reloj, hora_t hora_alarma);

/*!
 * @brief Consulta la hora actualmente programada para la alarma.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[out] hora_alarma Arreglo donde se almacenará la hora de la alarma consultada.
 * @return true indicando que se pudo leer la configuración.
 */
bool RelojGetAlarma(clock_t reloj, hora_t hora_alarma);

/*!
 * @brief Permite encender o apagar el disparo de la alarma.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[in] activar Estado deseado (true para habilitar, false para deshabilitar).
 */
void RelojActivarAlarma(clock_t reloj, bool activar);

/*!
 * @brief Consulta si la alarma se encuentra actualmente habilitada.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @return true si la alarma está habilitada, false en caso contrario.
 */
bool RelojAlarmaEstaActiva(clock_t reloj);

/*!
 * @brief Pospone el evento de la alarma sumando minutos a su hora programada.
 * @param[in] reloj Puntero a la instancia del reloj.
 * @param[in] minutos Cantidad de minutos a posponer (Snooze).
 */
void RelojPosponerAlarma(clock_t reloj, uint8_t minutos);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* RELOJ_H_ */