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

/*!
 * @file    main.c
 * @brief   Aplicación principal del Reloj Despertador para la placa EDU-CIAA-NXP.
 * @details Implementación de la máquina de estados y control de hardware utilizando
 *          la capa de abstracción (BSP) y la biblioteca de gestión de tiempo.
 */

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

/* === Inclusiones de cabeceras ================================================================ */

#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"
#include "screen.h"

/* === Declaraciones de tipos de datos privados ================================================ */

/*!
 * @brief   Estados operativos de la máquina de estados del reloj y la alarma.
 */
typedef enum {
    MODO_SIN_AJUSTAR,       /*!< Estado inicial, la hora no ha sido configurada. */
    MODO_NORMAL,            /*!< Funcionamiento normal, muestra la hora actual. */
    MODO_MINUTOS,           /*!< Configuración de los minutos del reloj. */
    MODO_HORAS,             /*!< Configuración de las horas del reloj. */
    MODO_MINUTOS_ALARMA,    /*!< Configuración de los minutos de la alarma. */
    MODO_HORAS_ALARMA       /*!< Configuración de las horas de la alarma. */
} modo_t;

/* === Definiciones de variables privadas ====================================================== */

/*! @brief Descriptor global de la pantalla de 7 segmentos. */
display_t display_global;

/*! @brief Instancia lógica del reloj que gestiona el tiempo y los eventos de alarma. */
clock_t reloj;

/*! @brief Base de tiempo del sistema en milisegundos. */
volatile uint32_t contador_ms = 0;

/*! @brief Estado actual de la máquina de estados del sistema. */
static modo_t modo;

/* === Implementación de funciones privadas ==================================================== */

/*!
 * @brief   Decrementa un par de dígitos en formato BCD (Decimal Codificado en Binario).
 * @details Realiza el decremento posicional y evalúa el límite inferior. Si el valor 
 *          alcanza cero, efectúa el retorno circular (rollover) hacia el límite superior indicado.
 * 
 * @param[in,out] numero Arreglo de 2 elementos que almacena las decenas [0] y unidades [1].
 * @param[in]     limite Arreglo de 2 elementos que define el valor máximo de reinicio.
 */
void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {
    if (numero[1] == 0) {
        if (numero[0] == 0) {
            numero[0] = limite[0];
            numero[1] = limite[1];
        } else {
            numero[0]--;
            numero[1] = 9;
        }
    } else {
        numero[1]--;
    }
}

/* === Implementación de funciones públicas ==================================================== */

/*!
 * @brief   Función principal del sistema (Punto de entrada).
 * @details Inicializa los recursos de hardware mediante el BSP y ejecuta 
 *          el flujo de control del reloj despertador.
 * 
 * @return  int Retorna siempre 0 (el flujo no debería salir del bucle de control).
 */
int main(void) {
    
    /* HACER: Inicialización de hardware y variables (BoardCreate, Display, etc.) */

    /* HACER: Creación de Tareas de FreeRTOS (Display, Teclado, Máquina de Estados) */

    /* HACER: vTaskStartScheduler() */

    return 0;
}

/* === Fin de la documentación ================================================================= */
/*! @} End of module definition for doxygen */