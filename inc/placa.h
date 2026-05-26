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

#ifndef PLACA_H_
#define PLACA_H_

/** @file placa.h
 ** @brief Declaraciones de la biblioteca para gestión de la placa
 **/

/* === Headers files inclusions ==================================================================================== */

#include "digital.h"

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* === Public data type declarations =============================================================================== */

/*! 
 * @brief Estructura de abstracción de los periféricos de la placa.
 * * Agrupa todas las entradas y salidas digitales de la placa EDU-CIAA para 
 * ser manejadas a través de la biblioteca digital, en lugar de utilizar
 * las funciones de bajo nivel del chip.
 */
typedef struct board_s {
    //! Asociado a LED_3 de la placa, es decir, al led verde.
    digital_output_t led_verde; 
    
    //! Asociado al LED_1 de la placa.
    digital_output_t led_rojo; 
    
    //! Asociado al LED_2 de la placa.
    digital_output_t led_amarillo; 
    
    //! Asociado al LED_R de la placa, es decir, al led rojo del led RGB.
    digital_output_t led_rgb_red; 
    
    //! Asociado al LED_G de la placa, es decir, al led verde del led RGB.
    digital_output_t led_rgb_green; 
    
    //! Asociado al LED_B de la placa, es decir, al led azul del led RGB.
    digital_output_t led_rgb_blue; 

    //! Asociada a TEC_1 de la placa.
    digital_input_t tecla_prender; 
    
    //! Asociada a TEC_2 de la placa.
    digital_input_t tecla_apagar; 
    
    //! Asociada a TEC_3 de la placa.
    digital_input_t tecla_cambiar; 
    
    //! Asociada a TEC_4 de la placa.
    digital_input_t tecla_probar; 
} const * const board_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/*!
 * @brief Inicializa el hardware de la placa.
 * * Configura los pines correspondientes a las teclas y los LEDs utilizando 
 * la capa de abstracción de hardware, y asigna los punteros dentro de la 
 * estructura de la placa.
 * * @return Puntero constante a la estructura @c board_t con los objetos inicializados.
 */
board_t BoardCreate();

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* PLACA_H_ */