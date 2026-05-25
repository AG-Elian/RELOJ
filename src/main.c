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
static void ConfigureLeds(void);

/**
 * @brief Function to configure pins and gpio bits used by board keys
 */
static void ConfigureKeys(void);

/**
 * @brief Function to flash RGB led in sequence
 */
static void FlashLed(void);

/**
 * @brief Function to switch on and off a led with two keys
 */
static void SwitchLed(void);

/**
 * @brief Function to switch on and off a led with a single key
 */
static void ToggleLed(void);

/**
 * @brief Function to turn on a led while a key is pressed
 */
static void TestLed(void);

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

static void ConfigureLeds(void) {
    Chip_SCU_PinMuxSet(LED_R_PORT, LED_R_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_R_FUNC);
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, false);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, true);
    led_rgb_red = DigitalOutputCreate(LED_R_GPIO, LED_R_BIT); // Creo un objeto para manejar el led rojo del RGB y lo asocio a la salida que maneja al led rojo del RGB.

    Chip_SCU_PinMuxSet(LED_G_PORT, LED_G_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_G_FUNC);
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, false);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, true);
    led_rgb_green = DigitalOutputCreate(LED_G_GPIO, LED_G_BIT); // Creo un objeto para manejar el led verde del RGB y lo asocio a la salida que maneja al led verde del RGB.

    Chip_SCU_PinMuxSet(LED_B_PORT, LED_B_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_B_FUNC);
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, false);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, true);
    led_rgb_blue = DigitalOutputCreate(LED_B_GPIO, LED_B_BIT); // Creo un objeto para manejar el led azul del RGB y lo asocio a la salida que maneja al led azul del RGB.

    /******************/
    Chip_SCU_PinMuxSet(LED_1_PORT, LED_1_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_1_FUNC);
    led_rojo = DigitalOutputCreate(LED_1_GPIO, LED_1_BIT); // Creo un objeto para manejar el led rojo y lo asocio a la salida que maneja al led rojo.
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, false);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, true);

    Chip_SCU_PinMuxSet(LED_2_PORT, LED_2_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_2_FUNC);
    led_amarillo = DigitalOutputCreate(LED_2_GPIO, LED_2_BIT); // Creo un objeto para manejar el led amarillo y lo asocio a la salida que maneja al led amarillo.
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_2_GPIO, LED_2_BIT, false);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_2_GPIO, LED_2_BIT, true);

    Chip_SCU_PinMuxSet(LED_3_PORT, LED_3_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_INACT | LED_3_FUNC);
    led_verde = DigitalOutputCreate(LED_3_GPIO, LED_3_BIT); // Creo un objeto para manejar el led verde y lo asocio a la salida que maneja al led verde.
    //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, false); // El led verde se apaga inicialmente
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, true); // El led verde se configura como salida... Esto ya lo hace la función DigitalOutputCreate, así que no es necesario hacerlo aquí.
}

static void ConfigureKeys(void) {
    Chip_SCU_PinMuxSet(TEC_1_PORT, TEC_1_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_1_FUNC);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, TEC_1_GPIO, TEC_1_BIT, false);
    tecla_1 = DigitalInputCreate(TEC_1_GPIO, TEC_1_BIT, true); // Creo un objeto para manejar la tecla 1 y lo asocio a la entrada que maneja a la tecla 1. El estado activo de la tecla 1 es lógico 0, por eso el tercer parámetro es true.
    
    Chip_SCU_PinMuxSet(TEC_2_PORT, TEC_2_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_2_FUNC);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, TEC_2_GPIO, TEC_2_BIT, false);
    tecla_2 = DigitalInputCreate(TEC_2_GPIO, TEC_2_BIT, true); // Creo un objeto para manejar la tecla 2 y lo asocio a la entrada que maneja a la tecla 2. El estado activo de la tecla 2 es lógico 0, por eso el tercer parámetro es true.

    Chip_SCU_PinMuxSet(TEC_3_PORT, TEC_3_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_3_FUNC);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, TEC_3_GPIO, TEC_3_BIT, false);
    tecla_3 = DigitalInputCreate(TEC_3_GPIO, TEC_3_BIT, true); // Creo un objeto para manejar la tecla 3 y lo asocio a la entrada que maneja a la tecla 3. El estado activo de la tecla 3 es lógico 0, por eso el tercer parámetro es true.

    Chip_SCU_PinMuxSet(TEC_4_PORT, TEC_4_PIN, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | TEC_4_FUNC);
    //Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, TEC_4_GPIO, TEC_4_BIT, false);
    tecla_4 = DigitalInputCreate(TEC_4_GPIO, TEC_4_BIT, true); // Creo un objeto para manejar la tecla 4 y lo asocio a la entrada que maneja a la tecla 4. El estado activo de la tecla 4 es lógico 0, por eso el tercer parámetro es true.
}

static void FlashLed(void) {
    static int divisor = 0;
    static rgb_color_t state = LED_BLUE_OFF;

    divisor++;
    if (divisor == 5) {
        divisor = 0;
        state = (state + 1) % (LED_BLUE_OFF + 1);

        switch (state) {
        case LED_RED_ON:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, true);
            DigitalOutputActivate(led_rgb_red);
            break;
        case LED_GREEN_ON:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, true);
            DigitalOutputActivate(led_rgb_green);
            break;
        case LED_BLUE_ON:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, true);
            DigitalOutputActivate(led_rgb_blue);
            break;
        default:
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_R_GPIO, LED_R_BIT, false);
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_G_GPIO, LED_G_BIT, false);
            //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_B_GPIO, LED_B_BIT, false);
            DigitalOutputDeactivate(led_rgb_red);
            DigitalOutputDeactivate(led_rgb_green);
            DigitalOutputDeactivate(led_rgb_blue);
            break;
        }
    }
}

static void SwitchLed(void) {
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_1_GPIO, TEC_1_BIT) == 0) {
    if (DigitalInputRead(tecla_1)) { // Si la tecla 1 está presionada... El estado activo de la tecla 1 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, true);
        DigitalOutputActivate(led_rojo); // Enciendo el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_2_GPIO, TEC_2_BIT) == 0) {
    if (DigitalInputRead(tecla_2)) { // Si la tecla 2 está presionada... El estado activo de la tecla 2 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_1_GPIO, LED_1_BIT, false);
        DigitalOutputDeactivate(led_rojo); // Apago el led rojo usando la función de la biblioteca digital.h, en lugar de usar la función de bajo nivel del chip.
    }
}

static void ToggleLed(void) {
    //static bool last_state = false;
    //bool current_state;

    //current_state = (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_3_GPIO, TEC_3_BIT) == 0);
    //if ((current_state) && (!last_state)) {
    if (DigitalInputHasActivated(tecla_3)) { // Si la tecla 3 está presionada... El estado activo de la tecla 3 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        DigitalOutputToggle(led_amarillo);
        //Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, LED_2_GPIO, LED_2_BIT);
    }
    //last_state = current_state;
}

static void TestLed(void) {
    //if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, TEC_4_GPIO, TEC_4_BIT) == 0) {
    if (DigitalInputRead(tecla_4)) { // Si la tecla 4 está presionada... El estado activo de la tecla 4 es lógico 0, pero la función DigitalInputRead devuelve true cuando la tecla está presionada, así que no es necesario invertir el resultado.
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, true);
        DigitalOutputActivate(led_verde);

    } else {
        //Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_3_GPIO, LED_3_BIT, false);
        DigitalOutputDeactivate(led_verde);
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

    BoardSetup();
    ConfigureLeds();
    ConfigureKeys();

    while (true) {
        FlashLed();
        SwitchLed();
        ToggleLed();
        TestLed();

        Delay();
    }

    return 0;
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
