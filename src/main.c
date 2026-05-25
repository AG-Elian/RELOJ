/************************************************************************************************
Copyright (c) 2022-2023, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnología, Universidad Nacional de Tucumán
https://www.microprocesadores.unt.edu.ar/

Copyright (c) 2022-2023, Esteban Volentini <evolentini@herrera.unt.edu.ar>

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

/** \brief EDU-CIAA-NXP board sample application
 **
 ** \addtogroup samples Samples
 ** \brief Samples applications with MUJU Framwork
 ** @{ */

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

// Este es un listado de constantes que representan cómo están conectadas las cosas en la placa
#define LED_R_PORT 2
#define LED_R_PIN  0
#define LED_R_FUNC SCU_MODE_FUNC4
#define LED_R_GPIO 5
#define LED_R_BIT  0

#define LED_G_PORT 2
#define LED_G_PIN  1
#define LED_G_FUNC SCU_MODE_FUNC4
#define LED_G_GPIO 5
#define LED_G_BIT  1

#define LED_B_PORT 2
#define LED_B_PIN  2
#define LED_B_FUNC SCU_MODE_FUNC4
#define LED_B_GPIO 5
#define LED_B_BIT  2

#define LED_1_PORT 2
#define LED_1_PIN  10
#define LED_1_FUNC SCU_MODE_FUNC0
#define LED_1_GPIO 0
#define LED_1_BIT  14

#define LED_2_PORT 2
#define LED_2_PIN  11
#define LED_2_FUNC SCU_MODE_FUNC0
#define LED_2_GPIO 1
#define LED_2_BIT  11

#define LED_3_PORT 2
#define LED_3_PIN  12
#define LED_3_FUNC SCU_MODE_FUNC0
#define LED_3_GPIO 1
#define LED_3_BIT  12

#define TEC_1_PORT 1
#define TEC_1_PIN  0
#define TEC_1_FUNC SCU_MODE_FUNC0
#define TEC_1_GPIO 0
#define TEC_1_BIT  4

#define TEC_2_PORT 1
#define TEC_2_PIN  1
#define TEC_2_FUNC SCU_MODE_FUNC0
#define TEC_2_GPIO 0
#define TEC_2_BIT  8

#define TEC_3_PORT 1
#define TEC_3_PIN  2
#define TEC_3_FUNC SCU_MODE_FUNC0
#define TEC_3_GPIO 0
#define TEC_3_BIT  9

#define TEC_4_PORT 1
#define TEC_4_PIN  6
#define TEC_4_FUNC SCU_MODE_FUNC0
#define TEC_4_GPIO 1
#define TEC_4_BIT  9

/* === Private data type declarations ========================================================== */

/**
 * @brief Enumeration with color sequence of RGB led
 */
typedef enum rgb_color_e {
    LED_RED_ON = 0,
    LED_RED_OFF,
    LED_GREEN_ON,
    LED_GREEN_OFF,
    LED_BLUE_ON,
    LED_BLUE_OFF,
} rgb_color_t;

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

/**
 * @brief Function to configure pins and gpio bits used by board leds
 */
//static void ConfigureLeds(void);

/**
 * @brief Function to configure pins and gpio bits used by board keys
 */
//static void ConfigureKeys(void);

/**
 * @brief Function to flash RGB led in sequence
 */
static void FlashLed(board_t placa);

/**
 * @brief Function to switch on and off a led with two keys
 */
static void SwitchLed(board_t placa);

/**
 * @brief Function to switch on and off a led with a single key
 */
static void ToggleLed(board_t placa);

/**
 * @brief Function to turn on a led while a key is pressed
 */
static void TestLed(board_t placa);

/**
 * @brief Function to generate a delay of approximately 100 ms
 */
static void Delay(void);

/* === Public variable definitions ============================================================= */

digital_output_t led_verde; //Asociado a LED_3 de la placa, es decir, al led verde. Este objeto se va a usar para manejar el led verde a través de la biblioteca digital.h, en lugar de manejarlo directamente con las funciones de bajo nivel del chip.
digital_output_t led_rojo; // Asociado al LED_1 de la placa.
digital_output_t led_amarillo; // Asociado al LED_2 de la placa.
digital_output_t led_rgb_red; // Asociado al led rojo del RGB.
digital_output_t led_rgb_green; // Asociado al led verde del RGB.
digital_output_t led_rgb_blue; // Asociado al led azul del RGB.

digital_input_t tecla_1; // Asociada a TEC_1 de la placa.
digital_input_t tecla_2; // Asociada a TEC_2 de la placa.
digital_input_t tecla_3; // Asociada a TEC_3 de la placa.
digital_input_t tecla_4; // Asociada a TEC_4 de la placa.

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
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, true);
            DigitalOutputActivate(placa->led_rgb_red);
            break;
        case LED_GREEN_ON:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, true);
            DigitalOutputActivate(placa->led_rgb_green);
            break;
        case LED_BLUE_ON:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, true);
            DigitalOutputActivate(placa->led_rgb_blue);
            break;
        default:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, false);
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, false);
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, false);
            DigitalOutputDeactivate(placa->led_rgb_red);
            DigitalOutputDeactivate(placa->led_rgb_green);
            DigitalOutputDeactivate(placa->led_rgb_blue);
            break;
        }
    }
}

static void SwitchLed(board_t placa) {
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_1_GPIO, TEC_1_BIT) == 0) {
    if (DigitalInputRead(placa->tecla_prender)) { // Si la tecla 1 está presionada... El estado activo de la tecla 1 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, true);
        DigitalOutputActivate(placa->led_rojo); // Enciendo el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_2_GPIO, TEC_2_BIT) == 0) {
    if (DigitalInputRead(placa->tecla_apagar)) { // Si la tecla 2 está presionada... El estado activo de la tecla 2 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, false);
        DigitalOutputDeactivate(placa->led_rojo); // Apago el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
}

static void ToggleLed(board_t placa) {
    //static bool last_state = false;
    //bool current_state;

    //current_state = (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_3_GPIO, TEC_3_BIT) == 0);
    //if ((current_state) && (!last_state)) {
    if (DigitalInputHasActivated(placa->tecla_cambiar)) { // Si la tecla 3 está presionada... El estado activo de la tecla 3 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        DigitalOutputToggle(placa->led_amarillo);
        //Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, LED_2_GPIO, LED_2_BIT);
    }
    //last_state = current_state;
}

static void TestLed(board_t placa) {
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_4_GPIO, TEC_4_BIT) == 0) {
    if (DigitalInputRead(placa->tecla_probar)) { // Si la tecla 4 está presionada... El estado activo de la tecla 4 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, true);
        DigitalOutputActivate(placa->led_verde);

    } else {
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, false);
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

int main(void) {

    //BoardSetup();
    //ConfigureLeds();
    //ConfigureKeys();
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

/** @} End of module definition for doxygen */
