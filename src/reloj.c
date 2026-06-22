/*********************************************************************************************************************
Copyright 2016-2025, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnología
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

/** @file reloj.c
 ** @brief Implementación de funciones para el manejo de un reloj
 **/

/* === Headers files inclusions ================================================================ */


#include "reloj.h"

/* === Macros definitions ====================================================================== */

#define UNITS_PER_TEN 10U
#define SECONDS_PER_MINUTE 60U
#define MINUTES_PER_HOUR 60U
#define HOURS_PER_DAY 24U
#define SECONDS_PER_DAY (HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)


/* === Private data type declarations ========================================================== */

struct clock_s {
    uint16_t ticks_counter; // Contador de ticks
    uint16_t ticks_persecond; // Ticks por segundo
    uint32_t current_time; // Tiempo actual en segundos
    bool valid_time; // Indica si la hora actual es válida
    void *alarm_handler; // Puntero a la función de callback para la alarma (reservado para futuras implementaciones)
};

/* === Private function declarations =========================================================== */

static uint32_t time_to_seconds(const hora_t hora);

static void seconds_to_time(uint32_t seconds, hora_t hora);

/* === Private variable definitions ============================================================ */

/* === Public variable definition  ============================================================= */

/* === Private function implementations ============================================================ */

static uint32_t time_to_seconds(const hora_t hora) {
    uint32_t seconds = UNITS_PER_TEN * hora[HOUR_TENS] + hora[HOUR_ONES]; // Convertimos horas a segundos

    seconds = MINUTES_PER_HOUR * seconds + UNITS_PER_TEN * hora[MINUTE_TENS] + hora[MINUTE_ONES]; // Convertimos horas a minutos
    seconds = SECONDS_PER_MINUTE * seconds + UNITS_PER_TEN * hora[SECOND_TENS] + hora[SECOND_ONES]; // Convertimos minutos a segundos

    return seconds;
}

static void seconds_to_time(uint32_t seconds, hora_t hora) {
    uint32_t time_in_day = seconds % SECONDS_PER_DAY; // Calculamos el tiempo dentro del día

    uint32_t hours = time_in_day / (MINUTES_PER_HOUR * SECONDS_PER_MINUTE); // Calculamos las horas
    uint32_t minutes = (time_in_day % (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)) / SECONDS_PER_MINUTE; // Calculamos los minutos
    uint32_t secs = time_in_day % SECONDS_PER_MINUTE; // Calculamos los segundos

    hora[HOUR_TENS] = hours / UNITS_PER_TEN; // Asignamos el dígito de las horas decenas
    hora[HOUR_ONES] = hours % UNITS_PER_TEN; // Asignamos el dígito de las horas unidades
    hora[MINUTE_TENS] = minutes / UNITS_PER_TEN; // Asignamos el dígito de los minutos decenas
    hora[MINUTE_ONES] = minutes % UNITS_PER_TEN; // Asignamos el dígito de los minutos unidades
    hora[SECOND_TENS] = secs / UNITS_PER_TEN; // Asignamos el dígito de los segundos decenas
    hora[SECOND_ONES] = secs % UNITS_PER_TEN; // Asignamos el dígito de los segundos unidades
}

/* === Public function implementation ========================================================== */

clock_t RelojCreate(unsigned int ticks_persecond, void (*alarm_handler)(void)){
    static struct clock_s clock_instance; // Instancia estática del reloj

    clock_t self = &clock_instance; // Puntero al reloj
    self->ticks_persecond = ticks_persecond; // Configuramos los ticks por segundo
    self->ticks_counter = 0; // Inicializamos el contador de ticks
    self->current_time = 0; // Inicializamos el tiempo actual en segundos
    self->valid_time = false; // La hora no es válida hasta que se configure
    self->alarm_handler = alarm_handler; // Guardamos el puntero al handler de la alarma (aunque no se use por ahora)

    return self;
}

bool RelojGetHora(clock_t self, hora_t hora_actual) {

    seconds_to_time(self->current_time, hora_actual); // Convertimos el tiempo actual en segundos a formato de hora

    return self->valid_time; // Devolvemos si la hora es válida o no
}

bool RelojSetHora(clock_t self, hora_t hora_actual) {
    self->current_time = time_to_seconds(hora_actual); // Convertimos la hora actual a segundos y la guardamos
    self->valid_time = true; // La hora ahora es válida

    return true; // Devolvemos true para indicar que la operación fue exitosa
}

void RelojTick(clock_t self) {
    self->ticks_counter++; // Incrementamos el contador de ticks

    if (self->ticks_counter < self->ticks_persecond) { 
        return;
    }

    self->ticks_counter = 0; // Reiniciamos el contador de ticks
    self->current_time++; // Incrementamos el tiempo actual en segundos
    if (self->current_time >= SECONDS_PER_DAY) {
        self->current_time = 0; // Reiniciamos el tiempo al llegar a un día completo
    }
}

/* === End of public function implementation ================================================== */

/* === End of documentation ==================================================================== */