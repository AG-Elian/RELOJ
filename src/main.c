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

#include "digital.h"
#include "placa.h"
#include "poncho.h"  // Agregado para el hardware del Poncho
#include "screen.h"  // Agregado para la abstracción de la Pantalla

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

/*!
 * @brief Enumeración con la secuencia de colores del LED RGB.
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

static void FlashLed(board_t placa);
static void SwitchLed(board_t placa);
static void ToggleLed(board_t placa);
static void TestLed(board_t placa);

/*!
 * @brief Genera un retardo bloqueante por software.
 * Adaptado para ser más corto y no interferir con el multiplexado.
 */
static void Delay(void);

/* === Public variable definitions ============================================================= */

// ¡Eliminamos las variables globales de LEDs y Teclas que estaban aquí para cumplir con el feedback del docente!

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
    if (DigitalInputRead(placa->tecla_prender)) { 
        DigitalOutputActivate(placa->led_rojo); 
    }
    if (DigitalInputRead(placa->tecla_apagar)) { 
        DigitalOutputDeactivate(placa->led_rojo); 
    }
}

static void ToggleLed(board_t placa) {
    if (DigitalInputHasActivated(placa->tecla_cambiar)) { 
        DigitalOutputToggle(placa->led_amarillo);
    }
}

static void TestLed(board_t placa) {
    if (DigitalInputRead(placa->tecla_probar)) { 
        DigitalOutputActivate(placa->led_verde);
    } else {
        DigitalOutputDeactivate(placa->led_verde);
    }
}

static void Delay(void) {
    // Redujimos el ciclo anidado externo. Si usamos demoras muy largas, 
    // la pantalla de 7 segmentos parpadeará debido a la interrupción del multiplexado.
    for (int delay = 0; delay < 25000; delay++) {
        __asm("NOP");
    }
}

/* === Public function implementation ========================================================== */

/*!
 * @brief Función principal de la aplicación.
 *
 * @return Código de salida (0 por convención, aunque en este entorno no retorna).
 */
int main(void) {

    board_t placa = BoardCreate(); 
    display_t display = PonchoCreateDisplay(); // Inicializa pines del poncho y la capa HAL de la pantalla

    // Preparamos un arreglo con los números BCD a mostrar (ej: "2026")
    uint8_t digitos[] = {2, 0, 2, 6};
    
    // Escribimos el valor inicial en la memoria del display
    DisplayWriteBCD(display, digitos, 4);

    while (true) {
        
        // Refresco constante del display multiplexado (debe ejecutarse en cada iteración)
        DisplayRefresh(display);
        
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