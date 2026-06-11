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
#include "poncho.h"  
#include "screen.h"  

/* === Private function declarations =========================================================== */

static void Delay(void);

/* === Private function implementation ========================================================= */

/*! 
 * @brief Retardo por software (bloqueante).
 * * Implementa un bucle iterativo que ejecuta instrucciones NOP (No Operation) 
 * para generar una demora en el flujo del programa. 
 */

static void Delay(void) {
    for (int delay = 0; delay < 25000; delay++) {
        __asm("NOP");
    }
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

    board_t placa = BoardCreate(); 

    uint8_t digitos[] = {0, 0, 0, 0};
    DisplayWriteBCD(placa->display, digitos, 4);

    // Contadores para separar la velocidad de lectura y de refresco
    uint16_t contador_rebote = 0;
    uint16_t contador_multiplexado = 0;
    
    // Bandera para activar la demostración en cámara lenta
    bool modo_lento = false;

    // Nos aseguramos de que el LED azul (buzzer) arranque apagado (Lógica Negativa)
    DigitalOutputActivate(placa->buzzer);

    while (true) {
        
        // --- 1. REFRESCO DE PANTALLA (MULTIPLEXADO) ---
        contador_multiplexado++;
        // Si el modo lento está activo, tardamos 60 ciclos en refrescar. Si no, refrescamos en cada 1 ciclo.
        uint16_t limite_refresco = modo_lento ? 80 : 1; 
        
        if (contador_multiplexado >= limite_refresco) {
            contador_multiplexado = 0;
            DisplayRefresh(placa->display);
        }

        // --- 2. LECTURA DE BOTONES (ANTIRREBOTE) ---
        contador_rebote++;
        if (contador_rebote >= 40) { 
            contador_rebote = 0;
            
            // F1 a F4: Incrementan los dígitos
            if (DigitalInputHasActivated(placa->f4)) { 
                digitos[0] = (digitos[0] + 1) % 10;
                DisplayWriteBCD(placa->display, digitos, 4);
            }
            if (DigitalInputHasActivated(placa->f3)) { 
                digitos[1] = (digitos[1] + 1) % 10;
                DisplayWriteBCD(placa->display, digitos, 4);
            }
            if (DigitalInputHasActivated(placa->f2)) { 
                digitos[2] = (digitos[2] + 1) % 10;
                DisplayWriteBCD(placa->display, digitos, 4);
            }
            if (DigitalInputHasActivated(placa->f1)) { 
                digitos[3] = (digitos[3] + 1) % 10;
                DisplayWriteBCD(placa->display, digitos, 4);
            }

            // Tecla CANCEL: Activa/Desactiva la demostración del multiplexado
            if (DigitalInputHasActivated(placa->cancel)) {
                modo_lento = !modo_lento; // Alternar entre rápido y lento
                
                // Al activar la cámara lenta, forzamos los números 1, 2, 3, 4
                if (modo_lento) {
                    digitos[0] = 1;
                    digitos[1] = 2;
                    digitos[2] = 3;
                    digitos[3] = 4;
                    DisplayWriteBCD(placa->display, digitos, 4);
                }
            }
        }

        // --- 3. PRUEBA DE BUZZER / LED AZUL ---
        // Mientras mantengamos presionada la tecla ACCEPT, suena el buzzer (LED Azul encendido). Al soltarla, se apaga.
        if (DigitalInputRead(placa->accept)) {
            DigitalOutputDeactivate(placa->buzzer); // Lógica invertida: Prende
        } else {
            DigitalOutputActivate(placa->buzzer);   // Lógica invertida: Apaga
        }

        Delay();
    }

    return 0;
}

/* === End of documentation ==================================================================== */

/*! @} End of module definition for doxygen */