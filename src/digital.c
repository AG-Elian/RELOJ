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

/** @file digital.c
 ** @brief Implementaciones de la biblioteca para gestión de entradas y salidas digitales
**/

/* === Headers files inclusions ================================================================ */

#include "digital.h"
#include <stdlib.h>
#include "chip.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

struct digital_output_s
{
    uint32_t puerto;  /**< Índice del puerto GPIO asociado a la salida */
    uint8_t terminal; /**< Número de bit dentro del puerto GPIO */
};

struct digital_input_s
{
    uint32_t gpio;  /**< Índice del puerto GPIO asociado a la entrada */
    uint8_t bit;     /**< Número de bit del puerto GPIO */
    bool inverted;   /**< Indica si el estado activo es lógico 1 o lógico 0 */
    bool last_state; /**< Último estado lógico leído, usado por @ref DigitalInputRead */
};

/* === Private function declarations =========================================================== */

/* === Private variable definitions ============================================================ */

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

/* === Public function implementation ========================================================== */

digital_output_t DigitalOutputCreate(uint32_t puerto, uint8_t terminal){
    digital_output_t self;
    self = malloc(sizeof(struct digital_output_s));
    if (self){
        self -> puerto = puerto;
        self -> terminal = terminal;
        DigitalOutputDeactivate(self); // El led se apaga inicialmente
        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, self->puerto, self->terminal, true);
    }
    return self;
}
void DigitalOutputActivate(digital_output_t self){
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, self->puerto, self->terminal, true); 
}

void DigitalOutputDeactivate(digital_output_t self){
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, self->puerto, self->terminal, false);
}

void DigitalOutputToggle(digital_output_t self){
    Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, self->puerto, self->terminal); // Esta función hace exactamente lo mismo que la función Chip_GPIO_SetPinToggle, pero recibe como parámetro un objeto digital_output_t en lugar de recibir el puerto y el terminal por separado.

}

digital_input_t DigitalInputCreate(uint32_t gpio, uint8_t bit, bool inverted){
    digital_input_t self;
    self = malloc(sizeof(struct digital_input_s));
    if (self){
        self -> gpio = gpio;
        self -> bit = bit;
        self -> inverted = inverted;
        self -> last_state = DigitalInputRead(self); // El estado inicial se lee para que el campo last_state tenga un valor coherente.
        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, self->gpio, self->bit, false); // Configuro el pin como entrada.
    }
    return self;
}

bool DigitalInputRead(digital_input_t self){
    bool current_state = Chip_GPIO_GetPinState(LPC_GPIO_PORT, self->gpio, self->bit);
    if (self->inverted){
        current_state = !current_state;
    }
    return current_state;
}
int DigitalInputGetEvent(digital_input_t self){
    int resultado = 0;
    bool current_state = DigitalInputRead(self);
    if ((current_state) && (!self->last_state)){
        resultado = ACTIVATE_EVENT;
    } else if ((!current_state) && (self->last_state)){
        resultado = DEACTIVATE_EVENT;
    }
    self->last_state = current_state;
    return resultado;
}

bool DigitalInputHasActivated(digital_input_t self){
    return DigitalInputGetEvent(self) == ACTIVATE_EVENT;
}

bool DigitalInputHasDeactivated(digital_input_t self){
    return DigitalInputGetEvent(self) == DEACTIVATE_EVENT;
}
/* === End of documentation ==================================================================== */
