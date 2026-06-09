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

#include "poncho.h"
#include "screen.h"

/* === Private function declarations =========================================================== */

//!< Funciones estáticas que sirven como callbacks para la pantalla
static void PonchoUpdateDigits(uint8_t digit);
static void PonchoUpdateSegments(uint8_t segments);

/* === Private variable definitions ============================================================ */

//!< Instancia del driver que inyectaremos en la pantalla (Patrón Estrategia)
static const struct display_driver_s poncho_driver = {
    .UpdateDigits = PonchoUpdateDigits,
    .UpdateSegments = PonchoUpdateSegments
};

/* === Public function implementation ========================================================== */

/*!
 * @brief Inicializa los pines del Poncho y crea el objeto display.
 * Nota: Debes declarar el prototipo de esta función en poncho.h o bsp.h
 */
display_t PonchoCreateDisplay(void) {
    // Configuración de los pines de los DÍGITOS (Transistores/Multiplexado)
    Chip_SCU_PinMuxSet(DIGIT_1_PORT, DIGIT_1_PIN, DIGIT_1_FUNC);
    Chip_SCU_PinMuxSet(DIGIT_2_PORT, DIGIT_2_PIN, DIGIT_2_FUNC);
    Chip_SCU_PinMuxSet(DIGIT_3_PORT, DIGIT_3_PIN, DIGIT_3_FUNC);
    Chip_SCU_PinMuxSet(DIGIT_4_PORT, DIGIT_4_PIN, DIGIT_4_FUNC);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DIGIT_1_GPIO, DIGIT_1_BIT);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DIGIT_2_GPIO, DIGIT_2_BIT);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DIGIT_3_GPIO, DIGIT_3_BIT);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DIGIT_4_GPIO, DIGIT_4_BIT);

    // Configuración de los pines de los SEGMENTOS (A-G, DP)
    Chip_SCU_PinMuxSet(SEGMENT_A_PORT, SEGMENT_A_PIN, SEGMENT_A_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_B_PORT, SEGMENT_B_PIN, SEGMENT_B_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_C_PORT, SEGMENT_C_PIN, SEGMENT_C_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_D_PORT, SEGMENT_D_PIN, SEGMENT_D_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_E_PORT, SEGMENT_E_PIN, SEGMENT_E_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_F_PORT, SEGMENT_F_PIN, SEGMENT_F_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_G_PORT, SEGMENT_G_PIN, SEGMENT_G_FUNC);
    Chip_SCU_PinMuxSet(SEGMENT_P_PORT, SEGMENT_P_PIN, SEGMENT_P_FUNC);

    Chip_GPIO_SetDir(LPC_GPIO_PORT, SEGMENTS_GPIO, SEGMENTS_MASK, 1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, SEGMENT_P_GPIO, SEGMENT_P_BIT);

    // Apagamos todo al inicio
    Chip_GPIO_SetPortValue(LPC_GPIO_PORT, DIGITS_GPIO, 0);
    Chip_GPIO_SetPortValue(LPC_GPIO_PORT, SEGMENTS_GPIO, 0);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, SEGMENT_P_GPIO, SEGMENT_P_BIT, false);

    // Retornamos el objeto Display configurado con 4 dígitos y nuestro driver
    return DisplayCreate(4, &poncho_driver);
}

/* === Private function implementation ========================================================= */

static void PonchoUpdateDigits(uint8_t digit) {
    // 1. Apagar todos los dígitos (Asumiendo lógica positiva en la EDU-CIAA para los transistores)
    Chip_GPIO_ClearValue(LPC_GPIO_PORT, DIGITS_GPIO, DIGITS_MASK);
    
    // 2. Encender solo el dígito correspondiente
    switch (digit) {
        case 0: Chip_GPIO_SetPinState(LPC_GPIO_PORT, DIGIT_1_GPIO, DIGIT_1_BIT, true); break;
        case 1: Chip_GPIO_SetPinState(LPC_GPIO_PORT, DIGIT_2_GPIO, DIGIT_2_BIT, true); break;
        case 2: Chip_GPIO_SetPinState(LPC_GPIO_PORT, DIGIT_3_GPIO, DIGIT_3_BIT, true); break;
        case 3: Chip_GPIO_SetPinState(LPC_GPIO_PORT, DIGIT_4_GPIO, DIGIT_4_BIT, true); break;
    }
}

static void PonchoUpdateSegments(uint8_t segments) {
    // Escribimos todos los segmentos del A al G de un solo golpe usando la máscara (más rápido)
    Chip_GPIO_SetValue(LPC_GPIO_PORT, SEGMENTS_GPIO, (segments & 0x7F));
    Chip_GPIO_ClearValue(LPC_GPIO_PORT, SEGMENTS_GPIO, (~segments & 0x7F));

    // El punto decimal (P) está en un GPIO distinto (GPIO 5), lo manejamos aparte
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, SEGMENT_P_GPIO, SEGMENT_P_BIT, (segments & SEGMENT_P));
}