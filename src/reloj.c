/*********************************************************************************************************************
Copyright 2016-2025, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnología
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

/*! 
 * @file reloj.c
 * @brief Implementación de las funciones para el manejo del reloj y la alarma.
 */

/* === Headers files inclusions ================================================================ */

#include "reloj.h"
#include <stddef.h>

/* === Macros definitions ====================================================================== */

#define UNITS_PER_TEN 10U
#define SECONDS_PER_MINUTE 60U
#define MINUTES_PER_HOUR 60U
#define HOURS_PER_DAY 24U
#define SECONDS_PER_DAY (HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)

/* === Private data type declarations ========================================================== */

/*!
 * @brief Estructura de datos privada que mantiene el estado interno del reloj y la alarma.
 */
struct clock_s {
    uint16_t ticks_counter;        //!< Contador interno para medir fracciones de segundo
    uint16_t ticks_persecond;      //!< Frecuencia de actualización requerida para alcanzar un segundo real
    uint32_t current_time;         //!< Hora actual expresada en segundos
    bool valid_time;               //!< Estado de la validez de la hora del reloj
    uint32_t alarm_time;           //!< Hora programada para la alarma expresada en segundos
    bool alarm_enabled;            //!< Estado de habilitación de la alarma
    void (*alarm_handler)(void);   //!< Función callback que se ejecuta al dispararse la alarma

    bool is_snoozed;               //!< Indica si la alarma fue pospuesta
    uint32_t snooze_target;        //!< Hora temporal a la que debe volver a sonar
};

/* === Private function declarations =========================================================== */

/*!
 * @brief Convierte un arreglo de hora BCD a su equivalente en segundos.
 * @param[in] hora Arreglo de 6 bytes en formato BCD.
 * @return Tiempo total transcurrido expresado en segundos.
 */
static uint32_t time_to_seconds(const hora_t hora);

/*!
 * @brief Convierte una cantidad de segundos a un arreglo de hora en formato BCD.
 * @param[in] seconds Tiempo total expresado en segundos.
 * @param[out] hora Arreglo de 6 bytes donde se almacenará el resultado convertido.
 */
static void seconds_to_time(uint32_t seconds, hora_t hora);

/* === Private variable definitions ============================================================ */

/* === Public variable definition  ============================================================= */

/* === Private function implementations ======================================================== */

static uint32_t time_to_seconds(const hora_t hora) {
    uint32_t seconds = UNITS_PER_TEN * hora[HOUR_TENS] + hora[HOUR_ONES];
    seconds = MINUTES_PER_HOUR * seconds + UNITS_PER_TEN * hora[MINUTE_TENS] + hora[MINUTE_ONES];
    seconds = SECONDS_PER_MINUTE * seconds + UNITS_PER_TEN * hora[SECOND_TENS] + hora[SECOND_ONES];
    return seconds;
}

static void seconds_to_time(uint32_t seconds, hora_t hora) {
    uint32_t time_in_day = seconds % SECONDS_PER_DAY;

    uint32_t hours = time_in_day / (MINUTES_PER_HOUR * SECONDS_PER_MINUTE);
    uint32_t minutes = (time_in_day % (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)) / SECONDS_PER_MINUTE;
    uint32_t secs = time_in_day % SECONDS_PER_MINUTE;

    hora[HOUR_TENS] = hours / UNITS_PER_TEN;
    hora[HOUR_ONES] = hours % UNITS_PER_TEN;
    hora[MINUTE_TENS] = minutes / UNITS_PER_TEN;
    hora[MINUTE_ONES] = minutes % UNITS_PER_TEN;
    hora[SECOND_TENS] = secs / UNITS_PER_TEN;
    hora[SECOND_ONES] = secs % UNITS_PER_TEN;
}

/* === Public function implementation ========================================================== */

/*!
 * @brief Crea una instancia del reloj.
 * @param[in] ticks_persecond Frecuencia de ticks para el incremento del tiempo.
 * @param[in] alarm_handler Puntero a la función que se llamará cuando se active la alarma.
 * @return Puntero a la instancia del reloj creado.
 */
clock_t RelojCreate(unsigned int ticks_persecond, void (*alarm_handler)(void)) {
    static struct clock_s clock_instance;

    clock_t self = &clock_instance;
    self->ticks_persecond = ticks_persecond;
    self->ticks_counter = 0;
    self->current_time = 0;
    self->valid_time = false;
    self->alarm_handler = alarm_handler;
    self->alarm_enabled = false;

    self->is_snoozed = false;
    self->snooze_target = 0;

    return self;
}

/*!
 * @brief Obtiene la hora actual del reloj en formato BCD.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[out] hora_actual Arreglo donde se almacenará la hora leída.
 * @return true si la hora es válida, false en caso contrario.
 */
bool RelojGetHora(clock_t self, hora_t hora_actual) {
    seconds_to_time(self->current_time, hora_actual);
    return self->valid_time;
}

/*!
 * @brief Configura la hora actual del reloj.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[in] hora_actual Arreglo en formato BCD con la hora a configurar.
 * @return true indicando que la operación fue exitosa.
 */
bool RelojSetHora(clock_t self, hora_t hora_actual) {
    self->current_time = time_to_seconds(hora_actual);
    self->valid_time = true;
    return true;
}

/*!
 * @brief Registra el paso del tiempo e invoca la alarma si corresponde.
 * @param[in] self Puntero a la instancia del reloj.
 */
void RelojTick(clock_t self) {
    self->ticks_counter++;

    if (self->ticks_counter < self->ticks_persecond) { 
        return;
    }

    self->ticks_counter = 0;
    self->current_time++;
    
    if (self->current_time >= SECONDS_PER_DAY) {
        self->current_time = 0;
    }

    if (self->alarm_enabled && (self->current_time == self->alarm_time)) {
        if (self->alarm_handler != NULL) { 
            self->alarm_handler();
        }
    }
    if (self->alarm_enabled) {
        // Suena si coincide con la alarma original, o si está pospuesta y coincide con el objetivo
        if ((self->current_time == self->alarm_time) || 
            (self->is_snoozed && self->current_time == self->snooze_target)) {
            
            if (self->alarm_handler != NULL) { 
                self->alarm_handler(); 
            }
            
            // Si sonó por un snooze, lo desactivamos para que no suene todos los días a esta hora
            self->is_snoozed = false; 
        }
    }
}

/*!
 * @brief Configura la hora a la que debe sonar la alarma.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[in] hora_alarma Arreglo en formato BCD con la hora deseada.
 */
void RelojSetAlarma(clock_t self, hora_t hora_alarma) {
    self->alarm_time = time_to_seconds(hora_alarma);
}

/*!
 * @brief Obtiene la hora programada para la alarma.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[out] hora_alarma Arreglo donde se guardará la hora leída.
 * @return true indicando éxito en la lectura.
 */
bool RelojGetAlarma(clock_t self, hora_t hora_alarma) {
    seconds_to_time(self->alarm_time, hora_alarma);
    return true;
}

/*!
 * @brief Habilita o deshabilita la alarma del reloj.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[in] activar true para activar, false para desactivar.
 */
void RelojActivarAlarma(clock_t self, bool activar) {
    self->alarm_enabled = activar;
}

/*!
 * @brief Consulta el estado actual de habilitación de la alarma.
 * @param[in] self Puntero a la instancia del reloj.
 * @return true si la alarma está habilitada, false si está inactiva.
 */
bool RelojAlarmaEstaActiva(clock_t self) {
    return self->alarm_enabled;
}

/*!
 * @brief Pospone la alarma agregando una cantidad de minutos a la programación actual.
 * @param[in] self Puntero a la instancia del reloj.
 * @param[in] minutos Cantidad de minutos a posponer.
 */
void RelojPosponerAlarma(clock_t self, uint8_t minutos) {
    // Calculamos el objetivo a partir de la hora ACTUAL
    self->snooze_target = self->current_time + (minutos * SECONDS_PER_MINUTE);
    
    // Evitamos el desbordamiento de día
    if (self->snooze_target >= SECONDS_PER_DAY) {
        self->snooze_target %= SECONDS_PER_DAY;
    }
    
    self->is_snoozed = true; // Activamos el estado de alarma pospuesta
}

/* === End of public function implementation ================================================== */

/* === End of documentation ==================================================================== */