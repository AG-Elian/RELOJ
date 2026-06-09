/************************************************************************************************
Copyright (c) 2022-2023, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnología, Universidad Nacional de Tucumán
https://www.microprocesadores.unt.edu.ar/

Copyright (c) 2026, Elian Leandro Aramallo Guantay <aramallog.elian@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

#include "screen.h"
#include <string.h>

/* === Macros definitions ====================================================================== */

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAX_DIGITS 8 // Límite máximo de dígitos soportados por la abstracción

/* === Private data type declarations ========================================================== */

//!< Estructura de estado de la pantalla
struct display_s {
    uint8_t digits;                 //!< Cantidad total de dígitos configurados
    uint8_t active_digit;           //!< Índice del dígito actualmente encendido
    uint8_t memory[MAX_DIGITS];     //!< Buffer de memoria con los segmentos a encender por dígito
    struct display_driver_s driver; //!< Punteros a las funciones de bajo nivel (Poncho)

    // Variables para el parpadeo (Flash)
    uint8_t flash_from;
    uint8_t flash_to;
    uint16_t flash_frequency;
    uint16_t flash_counter;
    uint8_t flash_state; // 0 = Apagado, 1 = Encendido
};

/* === Private variable declarations =========================================================== */

//!< Mapa de 7 segmentos para los números del 0 al 9 usando las macros de screen.h
static const uint8_t IMAGES[] = {
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F,             // 0
    SEGMENT_B | SEGMENT_C,                                                             // 1
    SEGMENT_A | SEGMENT_B | SEGMENT_D | SEGMENT_E | SEGMENT_G,                         // 2
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_G,                         // 3
    SEGMENT_B | SEGMENT_C | SEGMENT_F | SEGMENT_G,                                     // 4
    SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G,                         // 5
    SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G,             // 6
    SEGMENT_A | SEGMENT_B | SEGMENT_C,                                                 // 7
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G, // 8
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G              // 9
};

/* === Public function implementation ========================================================== */

display_t DisplayCreate(uint8_t digits, display_driver_t driver) {
    static struct display_s display = {0};

    display.digits = digits;
    display.active_digit = digits - 1;
    display.flash_frequency = 0;
    display.flash_state = 1; // Comienza mostrándose
    
    // Inyección de dependencias (Patrón Estrategia)
    memcpy(&display.driver, driver, sizeof(struct display_driver_s));
    memset(display.memory, 0, sizeof(display.memory));
    
    // Apagamos hardware al inicializar
    display.driver.UpdateSegments(0);
    display.driver.UpdateDigits(0);

    return &display;
}

void DisplayWriteBCD(display_t display, uint8_t * number, uint8_t size) {
    // Preservamos los puntos decimales actuales
    for (int i = 0; i < size; i++) {
        if (i >= display->digits) break;
        uint8_t dots = display->memory[i] & SEGMENT_P; 
        display->memory[i] = IMAGES[number[i]] | dots;
    }
}

void DisplayRefresh(display_t display) {
    // 1. Apagamos segmentos para evitar "Ghosting" (efecto fantasma)
    display->driver.UpdateSegments(0);

    // 2. Avanzamos al siguiente dígito
    display->active_digit = (display->active_digit + 1) % display->digits;

    // 3. Lógica de Parpadeo (Flashing)
    uint8_t current_segments = display->memory[display->active_digit];
    if (display->flash_frequency > 0) {
        if (display->active_digit >= display->flash_from && display->active_digit <= display->flash_to) {
            if (display->flash_state == 0) {
                current_segments = 0; // Apagamos los segmentos si toca el ciclo de apagado
            }
        }
        
        // El contador avanza sólo una vez por cada barrido completo de pantalla
        if (display->active_digit == 0) {
            display->flash_counter--;
            if (display->flash_counter == 0) {
                display->flash_counter = display->flash_frequency;
                display->flash_state = !display->flash_state;
            }
        }
    }

    // 4. Encendemos el dígito actual
    display->driver.UpdateDigits(display->active_digit);

    // 5. Escribimos los segmentos
    display->driver.UpdateSegments(current_segments);
}

void DisplayFlashDigits(display_t display, uint8_t from, uint8_t to, uint16_t frecuency) {
    display->flash_from = from;
    display->flash_to = to;
    display->flash_frequency = frecuency;
    display->flash_counter = frecuency;
    display->flash_state = 1;
}

void DisplayToggleDots(display_t display, uint8_t from, uint8_t to) {
    for (int i = from; i <= to; i++) {
        if (i < display->digits) {
            // Hacemos un XOR (^) con la máscara del punto para invertir su estado
            display->memory[i] ^= SEGMENT_P;
        }
    }
}

/* === End of documentation ==================================================================== */