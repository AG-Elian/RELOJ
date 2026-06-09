/*********************************************************************************************************************
Copyright (c) 2015, Esteban Daniel Volentini <evolentini@herrera.unt.edu.ar>
Copyright (c) 2015, Laboratorio de Microprocesadores, Universidad Nacional de Tucumán, Argentina

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*********************************************************************************************************************/

#ifndef SCREEN_H_
#define SCREEN_H_

/** @file screen.h
 ** @brief Abstracción de pantalla multiplexada de siete segmentos
 **
 ** La capa de aplicación escribe valores BCD y refresca el display; el hardware
 ** se controla mediante los callbacks @ref display_driver_t.
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/** @brief Máscara del segmento A */
#define SEGMENT_A (1 << 0)

/** @brief Máscara del segmento B */
#define SEGMENT_B (1 << 1)

/** @brief Máscara del segmento C */
#define SEGMENT_C (1 << 2)

/** @brief Máscara del segmento D */
#define SEGMENT_D (1 << 3)

/** @brief Máscara del segmento E */
#define SEGMENT_E (1 << 4)

/** @brief Máscara del segmento F */
#define SEGMENT_F (1 << 5)

/** @brief Máscara del segmento G */
#define SEGMENT_G (1 << 6)

/** @brief Máscara del punto decimal */
#define SEGMENT_P (1 << 7)

/* === Public data type declarations =============================================================================== */

/**
 * @brief Puntero opaco al descriptor de la pantalla multiplexada
 */
typedef struct display_s * display_t;

/**
 * @brief Callback de bajo nivel para seleccionar el dígito activo en el hardware
 *
 * Debe encender únicamente el dígito indicado y apagar el resto.
 *
 * @param digit Índice del dígito a activar, en el rango @c 0 .. (digits - 1)
 */
typedef void (*display_update_digits_t)(uint8_t digit);

/**
 * @brief Callback de bajo nivel para actualizar el patrón de segmentos encendidos
 *
 * @param segments Máscara de bits con los segmentos activos (@ref SEGMENT_A .. @ref SEGMENT_P).
 *                 El valor @c 0x00 apaga todos los segmentos (pantalla en blanco).
 */
typedef void (*display_update_segments_t)(uint8_t segments);

/**
 * @brief Controlador de bajo nivel de la pantalla multiplexada
 */
typedef struct display_driver_s {
    display_update_digits_t UpdateDigits;     /**< Selecciona el dígito activo */
    display_update_segments_t UpdateSegments; /**< Actualiza los segmentos encendidos */
} const * const display_driver_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea una pantalla multiplexada de siete segmentos
 *
 * @param  digits  Cantidad de dígitos de la pantalla
 * @param  driver  Puntero a la estructura con los callbacks de hardware
 * @return         Descriptor de la pantalla, o @c NULL si no hay instancia disponible
 */
display_t DisplayCreate(uint8_t digits, display_driver_t driver);

/**
 * @brief Escribe un número BCD en la memoria de la pantalla
 *
 * @param  display  Descriptor creado con @ref DisplayCreate
 * @param  number   Puntero al primer dígito BCD (cada elemento en @c 0..9)
 * @param  size     Cantidad de dígitos en @p number
 */
void DisplayWriteBCD(display_t display, uint8_t * number, uint8_t size);

/**
 * @brief Refresca un paso del barrido multiplexado
 *
 * Debe invocarse periódicamente desde una tarea o interrupción de temporización.
 *
 * @param  display  Descriptor creado con @ref DisplayCreate
 */
void DisplayRefresh(display_t display);

/**
 * @brief Configura el parpadeo de un rango de dígitos
 *
 * @param  display    Descriptor creado con @ref DisplayCreate
 * @param  from       Índice del primer dígito que parpadea
 * @param  to         Índice del último dígito que parpadea
 * @param  frecuency  Divisor respecto al ciclo de refresco; @c 0 desactiva el parpadeo
 */
void DisplayFlashDigits(display_t display, uint8_t from, uint8_t to, uint16_t frecuency);

/**
 * @brief Conmuta el punto decimal de un rango de dígitos
 *
 * @param  display  Descriptor creado con @ref DisplayCreate
 * @param  from     Índice del primer dígito
 * @param  to       Índice del último dígito
 */
void DisplayToggleDots(display_t display, uint8_t from, uint8_t to);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H_ */
