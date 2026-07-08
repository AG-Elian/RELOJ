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
 ** @brief Aplicación de prueba del Poncho para la placa EDU-CIAA-NXP.
 */

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"  
#include "screen.h"  

/*=== Global variables definition ============================================================= */

display_t display_global; // Puntero a la estructura de la pantalla de 7 segmentos
clock_t reloj; // Puntero a la estructura del reloj, que gestiona el tiempo y los ticks
volatile uint32_t contador_ms = 0; // Contador de milisegundos, incrementado en el SysTick_Handler

/*=== Private data type declarations ========================================================== */

typedef enum {
    MODO_SIN_AJUSTAR,
    MODO_NORMAL,
    MODO_MINUTOS,
    MODO_HORAS,
    MODO_MINUTOS_ALARMA,
    MODO_HORAS_ALARMA
} modo_t;

/* === Private function declarations =========================================================== */

// static void Delay(void);

/* === Private function implementation ========================================================= */

// Esta función interrumpe while(1) cada 1 milisegundo exacto
void SysTick_Handler(void) {
    // Refrescamos el display (imprescindible para que se vean los números)
    DisplayRefresh(display_global);
    // Le avisamos al reloj que pasó 1 milisegundo
    RelojTick(reloj);
    contador_ms++; // Incrementamos el contador de milisegundos
}

/* === Public function implementation ========================================================== */

/*! 
 * @brief Función principal del sistema.
 * * Punto de entrada de la aplicación. Se encarga de inicializar los recursos
 * de hardware mediante el Board Support Package (BSP), configurar el estado inicial
 * de la pantalla y el buzzer, y ejecutar el bucle infinito de control (Super Loop).
 * Dentro del bucle gestiona:
 * - El refresco multiplexado de la pantalla de 7 segmentos.
 * - La lectura por sondeo (polling) de botones con filtrado por software.
 * - La lógica de la cámara lenta (Slow-mo) para observar el multiplexado.
 * * @return int Retorna siempre 0 (el flujo no debería salir del bucle infinito).
 */

int main(void) {
    // INICIALIZACIÓN DE HARDWARE
    board_t placa = BoardCreate();
    display_global = placa->display; // Guardamos el puntero a la pantalla para usarlo en el SysTick_Handler
    reloj = RelojCreate(1000, NULL); // Configura el reloj para que genere un tick cada 1 ms
    SysTick_Config(SystemCoreClock / 1000); // Configura el SysTick para que salte 1000 veces por segundo (cada 1 ms)

    // VARIABLES DE CONTROL DE LA APLICACIÓN
    modo_t estado_reloj = MODO_NORMAL; // Estado inicial del reloj
    hora_t hora_actual; // Arreglo para almacenar la hora actual (4 dígitos: HHMM)
    uint32_t tiempo_inicio_f1 = 0;     // Para medir los 3 segundos de F1

    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; 
    RelojSetHora(reloj, hora_inicial);

    // Bucle principal de la aplicación. (Super Loop)
    while(1){

        // MAQUINA DE ESTADOS DEL RELOJ
        switch(estado_reloj) {

            case MODO_SIN_AJUSTAR:
                // Aquí iría la lógica para el modo sin ajustar
                break;
            case MODO_NORMAL:
                // A. Tarea principal del estado: Mostrar la hora
                RelojGetHora(reloj, hora_actual); 
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // B. Evaluar transiciones: F1 presionada por 3 segundos para pasar a MODO_MINUTOS
                if (DigitalInputRead(placa->f1)) {
                    // La tecla ESTÁ siendo presionada, calculo cuánto tiempo pasó
                    if ((contador_ms - tiempo_inicio_f1) >= 3000) {
                        // ¡Pasaron 3 segundos! Cambiamos de estado
                        estado_reloj = MODO_MINUTOS;
                    }
                } else {
                    // La tecla NO está presionada. 
                    // Anclamos el tiempo de inicio al tiempo actual continuamente.
                    tiempo_inicio_f1 = contador_ms; 
                }    
                break;
            case MODO_MINUTOS:
                // Mostramos el arreglo [1, 1, 1, 1] para saber si el salto funcionó
                hora_actual[0] = 1; hora_actual[1] = 1; hora_actual[2] = 1; hora_actual[3] = 1;
                DisplayWriteBCD(placa->display, hora_actual, 4);
                break;
            case MODO_HORAS:
                // Aquí iría la lógica para ajustar horas
                break;
            case MODO_MINUTOS_ALARMA:
                // Aquí iría la lógica para ajustar minutos de alarma
                break;
            case MODO_HORAS_ALARMA:
                // Aquí iría la lógica para ajustar horas de alarma
                break;
        }
    }

    return 0;
}

/* === End of documentation ==================================================================== */

/*! @} End of module definition for doxygen */