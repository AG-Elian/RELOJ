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

/** @file placa.c
 ** @brief Implementación de la biblioteca para gestión de la placa
 **/

/* === Headers files inclusions ================================================================ */

#include "placa.h"
#include "board.h"
#include "chip.h"

/* === Macros definitions ====================================================================== */

/*! 
 * @name Constantes de Hardware
 * Mapeo de puertos, pines y funciones para los LEDs y Teclas de la placa.
 * @{ 
 */
#define LED_R_PORT 2             //!< Puerto del SCU para el LED Rojo del RGB
#define LED_R_PIN  0             //!< Pin del SCU para el LED Rojo del RGB
#define LED_R_FUNC SCU_MODE_FUNC4 //!< Función del SCU para el LED Rojo del RGB
#define LED_R_GPIO 5             //!< Puerto GPIO para el LED Rojo del RGB
#define LED_R_BIT  0             //!< Bit GPIO para el LED Rojo del RGB

#define LED_G_PORT 2             //!< Puerto del SCU para el LED Verde del RGB
#define LED_G_PIN  1             //!< Pin del SCU para el LED Verde del RGB
#define LED_G_FUNC SCU_MODE_FUNC4 //!< Función del SCU para el LED Verde del RGB
#define LED_G_GPIO 5             //!< Puerto GPIO para el LED Verde del RGB
#define LED_G_BIT  1             //!< Bit GPIO para el LED Verde del RGB

#define LED_B_PORT 2             //!< Puerto del SCU para el LED Azul del RGB
#define LED_B_PIN  2             //!< Pin del SCU para el LED Azul del RGB
#define LED_B_FUNC SCU_MODE_FUNC4 //!< Función del SCU para el LED Azul del RGB
#define LED_B_GPIO 5             //!< Puerto GPIO para el LED Azul del RGB
#define LED_B_BIT  2             //!< Bit GPIO para el LED Azul del RGB

#define LED_1_PORT 2             //!< Puerto del SCU para el LED 1 (Rojo)
#define LED_1_PIN  10            //!< Pin del SCU para el LED 1
#define LED_1_FUNC SCU_MODE_FUNC0 //!< Función del SCU para el LED 1
#define LED_1_GPIO 0             //!< Puerto GPIO para el LED 1
#define LED_1_BIT  14            //!< Bit GPIO para el LED 1

#define LED_2_PORT 2             //!< Puerto del SCU para el LED 2 (Amarillo)
#define LED_2_PIN  11            //!< Pin del SCU para el LED 2
#define LED_2_FUNC SCU_MODE_FUNC0 //!< Función del SCU para el LED 2
#define LED_2_GPIO 1             //!< Puerto GPIO para el LED 2
#define LED_2_BIT  11            //!< Bit GPIO para el LED 2

#define LED_3_PORT 2             //!< Puerto del SCU para el LED 3 (Verde)
#define LED_3_PIN  12            //!< Pin del SCU para el LED 3
#define LED_3_FUNC SCU_MODE_FUNC0 //!< Función del SCU para el LED 3
#define LED_3_GPIO 1             //!< Puerto GPIO para el LED 3
#define LED_3_BIT  12            //!< Bit GPIO para el LED 3

#define TEC_1_PORT 1             //!< Puerto del SCU para la Tecla 1
#define TEC_1_PIN  0             //!< Pin del SCU para la Tecla 1
#define TEC_1_FUNC SCU_MODE_FUNC0 //!< Función del SCU para la Tecla 1
#define TEC_1_GPIO 0             //!< Puerto GPIO para la Tecla 1
#define TEC_1_BIT  4             //!< Bit GPIO para la Tecla 1

#define TEC_2_PORT 1             //!< Puerto del SCU para la Tecla 2
#define TEC_2_PIN  1             //!< Pin del SCU para la Tecla 2
#define TEC_2_FUNC SCU_MODE_FUNC0 //!< Función del SCU para la Tecla 2
#define TEC_2_GPIO 0             //!< Puerto GPIO para la Tecla 2
#define TEC_2_BIT  8             //!< Bit GPIO para la Tecla 2

#define TEC_3_PORT 1             //!< Puerto del SCU para la Tecla 3
#define TEC_3_PIN  2             //!< Pin del SCU para la Tecla 3
#define TEC_3_FUNC SCU_MODE_FUNC0 //!< Función del SCU para la Tecla 3
#define TEC_3_GPIO 0             //!< Puerto GPIO para la Tecla 3
#define TEC_3_BIT  9             //!< Bit GPIO para la Tecla 3

#define TEC_4_PORT 1             //!< Puerto del SCU para la Tecla 4
#define TEC_4_PIN  6             //!< Pin del SCU para la Tecla 4
#define TEC_4_FUNC SCU_MODE_FUNC0 //!< Función del SCU para la Tecla 4
#define TEC_4_GPIO 1             //!< Puerto GPIO para la Tecla 4
#define TEC_4_BIT  9             //!< Bit GPIO para la Tecla 4
/*! @} */

/* === Private data type declarations ========================================================== */

/* === Private function declarations =========================================================== */

/*!
 * @brief Configura y crea las instancias para el manejo de los LEDs de la placa.
 * * Asigna los multiplexores del microcontrolador (SCU) a la función correspondiente 
 * e inicializa los objetos @c digital_output_t para cada LED.
 * * @param[in,out] self Puntero a la estructura de la placa que almacenará los punteros de los LEDs.
 */
static void ConfigureLeds(struct board_s * self);

/*!
 * @brief Configura y crea las instancias para el manejo de las teclas de la placa.
 * * Asigna los multiplexores del microcontrolador (SCU) para que actúen como entradas, 
 * habilita las resistencias de pull-up e inicializa los objetos @c digital_input_t.
 * * @param[in,out] self Puntero a la estructura de la placa que almacenará los punteros de las teclas.
 */
static void ConfigureKeys(struct board_s * self);

/* === Private variable definitions ============================================================ */

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Private function implementation ========================================================== */

static void ConfigureLeds(struct board_s * self){
    Chip_SCU_PinMuxSet(LED_R_PORT, LED_R_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_R_FUNC);
    self->led_rgb_red = DigitalOutputCreate(LED_R_GPIO, LED_R_BIT); 

    Chip_SCU_PinMuxSet(LED_G_PORT, LED_G_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_G_FUNC);
    self->led_rgb_green = DigitalOutputCreate(LED_G_GPIO, LED_G_BIT); 

    Chip_SCU_PinMuxSet(LED_B_PORT, LED_B_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_B_FUNC);
    self->led_rgb_blue = DigitalOutputCreate(LED_B_GPIO, LED_B_BIT); 

    /******************/
    Chip_SCU_PinMuxSet(LED_1_PORT, LED_1_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_1_FUNC);
    self->led_rojo = DigitalOutputCreate(LED_1_GPIO, LED_1_BIT); 

    Chip_SCU_PinMuxSet(LED_2_PORT, LED_2_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_2_FUNC);
    self->led_amarillo = DigitalOutputCreate(LED_2_GPIO, LED_2_BIT); 

    Chip_SCU_PinMuxSet(LED_3_PORT, LED_3_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_3_FUNC);
    self->led_verde = DigitalOutputCreate(LED_3_GPIO, LED_3_BIT); 
}

static void ConfigureKeys(struct board_s * self){
    Chip_SCU_PinMuxSet(TEC_1_PORT, TEC_1_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_1_FUNC);
    self->tecla_prender = DigitalInputCreate(TEC_1_GPIO, TEC_1_BIT, true); 
    
    Chip_SCU_PinMuxSet(TEC_2_PORT, TEC_2_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_2_FUNC);
    self->tecla_apagar = DigitalInputCreate(TEC_2_GPIO, TEC_2_BIT, true); 
    
    Chip_SCU_PinMuxSet(TEC_3_PORT, TEC_3_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_3_FUNC);
    self->tecla_cambiar = DigitalInputCreate(TEC_3_GPIO, TEC_3_BIT, true); 
    
    Chip_SCU_PinMuxSet(TEC_4_PORT, TEC_4_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_4_FUNC);
    self->tecla_probar = DigitalInputCreate(TEC_4_GPIO, TEC_4_BIT, true); 
}

/* ===Public function implementation ========================================================== */

/*!
 * @brief Constructor principal que inicializa todos los periféricos de la placa.
 * * Llama a la inicialización de bajo nivel del sistema (BoardSetup) y luego 
 * configura y asigna internamente los LEDs y las teclas.
 * * @return Puntero constante @c board_t a la estructura estática inicializada con los periféricos.
 */
board_t BoardCreate(){
    static struct board_s self;

    BoardSetup(); // Configuro el sistema de la placa, esto es necesario para poder usar las funciones de bajo nivel del chip para configurar los pines y los gpio bits.

    ConfigureLeds(&self);
    
    ConfigureKeys(&self);
    
    return &self;
}

/* === End of documentation ==================================================================== */