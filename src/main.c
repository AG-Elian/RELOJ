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

/*! @file main.c
 ** @brief Aplicación de ejemplo para la placa EDU-CIAA-NXP.
 **
 ** @addtogroup samples Samples
 ** @brief Aplicaciones de ejemplo con el framework MUJU.
 ** @{ 
 */

/* === Headers files inclusions =============================================================== */

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

#include "board.h"
#include "chip.h"
#include "digital.h"
#include "placa.h"
#include <stdio.h>


/* === Macros definitions ====================================================================== */

/*! 
 * @name Constantes de Hardware
 * Listado de constantes que representan cómo están conectadas las cosas en la placa.
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

/*!
 * @brief Enumeración con la secuencia de colores del LED RGB.
 * Define los estados de encendido y apagado para iterar en la función de destello.
 */
typedef enum rgb_color_e {
    LED_RED_ON = 0,    //!< Estado para encender el LED Rojo
    LED_RED_OFF,       //!< Estado para apagar el LED Rojo
    LED_GREEN_ON,      //!< Estado para encender el LED Verde
    LED_GREEN_OFF,     //!< Estado para apagar el LED Verde
    LED_BLUE_ON,       //!< Estado para encender el LED Azul
    LED_BLUE_OFF,      //!< Estado para apagar el LED Azul
} rgb_color_t;

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

/*!
 * @brief Alterna el color del LED RGB en secuencia.
 * * @param[in] placa Puntero constante a la estructura con los periféricos de la placa.
 */
static void FlashLed(board_t placa);

/*!
 * @brief Enciende y apaga un LED específico utilizando dos teclas distintas.
 * * @param[in] placa Puntero constante a la estructura con los periféricos de la placa.
 */
static void SwitchLed(board_t placa);

/*!
 * @brief Invierte el estado de un LED al detectar la activación de una tecla.
 * * @param[in] placa Puntero constante a la estructura con los periféricos de la placa.
 */
static void ToggleLed(board_t placa);

/*!
 * @brief Mantiene un LED encendido únicamente mientras la tecla correspondiente esté presionada.
 * * @param[in] placa Puntero constante a la estructura con los periféricos de la placa.
 */
static void TestLed(board_t placa);

/*!
 * @brief Genera un retardo bloqueante por software (aproximadamente 100 ms).
 */
static void Delay(void);

/* === Public variable definitions ============================================================= */

digital_output_t led_verde;     //!< Asociado a LED_3 de la placa (LED verde) para manejo vía biblioteca.
digital_output_t led_rojo;      //!< Asociado al LED_1 de la placa.
digital_output_t led_amarillo;  //!< Asociado al LED_2 de la placa.
digital_output_t led_rgb_red;   //!< Asociado al led rojo del RGB.
digital_output_t led_rgb_green; //!< Asociado al led verde del RGB.
digital_output_t led_rgb_blue;  //!< Asociado al led azul del RGB.

digital_input_t tecla_1;        //!< Asociada a TEC_1 de la placa.
digital_input_t tecla_2;        //!< Asociada a TEC_2 de la placa.
digital_input_t tecla_3;        //!< Asociada a TEC_3 de la placa.
digital_input_t tecla_4;        //!< Asociada a TEC_4 de la placa.

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

static void FlashLed(board_t placa) {
    static int divisor = 0;
    static rgb_color_t state = LED_BLUE_OFF;

    divisor++;
    if (divisor == 5) {
        divisor = 0;
        state = (state + 1) % (LED_BLUE_OFF + 1);

        switch (state) {
        case LED_RED_ON:
            DigitalOutputActivate(placa->led_rgb_red);
            break;
        case LED_GREEN_ON:
            DigitalOutputActivate(placa->led_rgb_green);
            break;
        case LED_BLUE_ON:
            DigitalOutputActivate(placa->led_rgb_blue);
            break;
        default:
            DigitalOutputDeactivate(placa->led_rgb_red);
            DigitalOutputDeactivate(placa->led_rgb_green);
            DigitalOutputDeactivate(placa->led_rgb_blue);
            break;
        }
    }
}

static void SwitchLed(board_t placa) {
    if (DigitalInputRead(placa->tecla_prender)) { // Si la tecla 1 está presionada... El estado activo de la tecla 1 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        DigitalOutputActivate(placa->led_rojo); // Enciendo el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
    if (DigitalInputRead(placa->tecla_apagar)) { // Si la tecla 2 está presionada... El estado activo de la tecla 2 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        DigitalOutputDeactivate(placa->led_rojo); // Apago el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
}

static void ToggleLed(board_t placa) {

    if (DigitalInputHasActivated(placa->tecla_cambiar)) { // Si la tecla 3 está presionada... El estado activo de la tecla 3 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        DigitalOutputToggle(placa->led_amarillo);
    }
}

static void TestLed(board_t placa) {

    if (DigitalInputRead(placa->tecla_probar)) { // Si la tecla 4 está presionada... El estado activo de la tecla 4 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        
        DigitalOutputActivate(placa->led_verde);

    } else {

        DigitalOutputDeactivate(placa->led_verde);
    }
}

static void Delay(void) {
    for (int index = 0; index < 100; index++) {
        for (int delay = 0; delay < 25000; delay++) {
            __asm("NOP");
        }
    }
}

/* === Public function implementation ========================================================== */

/*!
 * @brief Función principal de la aplicación.
 * * Inicializa el hardware llamando a @c BoardCreate y entra en un bucle infinito
 * donde evalúa periódicamente el estado de las teclas y actualiza los LEDs.
 * * @return Código de salida (0 por convención, aunque en este entorno no retorna).
 */
int main(void) {

    board_t placa = BoardCreate(); // Creo un objeto para manejar la placa y lo asocio a la función que inicializa la placa. Esta función se encarga de configurar los leds y las teclas, así como de crear los objetos necesarios para manejarlos a través de la biblioteca digital.h.

    while (true) {
        
        FlashLed(placa);
        SwitchLed(placa);
        ToggleLed(placa);
        TestLed(placa);

        Delay();
    }

    return 0;
}

/* === End of documentation ==================================================================== */

/*! @} End of module definition for doxygen */