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

/*=== Private data type declarations ========================================================== */

typedef enum {
    MODO_SIN_AJUSTAR,
    MODO_NORMAL,
    MODO_MINUTOS,
    MODO_HORAS,
    MODO_MINUTOS_ALARMA,
    MODO_HORAS_ALARMA
} modo_t;

/* === Private variables definitions ========================================================= */

display_t display_global; // Puntero a la estructura de la pantalla de 7 segmentos

clock_t reloj; // Puntero a la estructura del reloj, que gestiona el tiempo y los ticks
volatile uint32_t contador_ms = 0; // Contador de milisegundos, incrementado en el SysTick_Handler
static modo_t modo;

volatile bool alarma_sonando = false; // Bandera para saber si el buzzer debe sonar

static const uint8_t LIMITE_MINUTOS[2] = { 5, 9 }; // Límite de minutos en formato BCD (59)
static const uint8_t LIMITE_HORAS[2] = { 2, 3 };   // Límite de horas en formato BCD (23)

/* === Private function declarations =========================================================== */


/* === Private function implementation ========================================================= */

// Esta función interrumpe while(1) cada 1 milisegundo exacto
void SysTick_Handler(void) {
    DisplayRefresh(display_global);
    RelojTick(reloj);
    contador_ms++;
}

// Función auxiliar para controlar el estado absoluto de los puntos (estaban a destiempo)
void SetPunto(display_t display, uint8_t digito, bool encender) {
    // Esta memoria estática "recuerda" si los puntos están físicamente prendidos o apagados
    static bool estado_puntos[4] = {false, false, false, false};
    
    // Solo usamos el Toggle si el estado que deseamos es diferente al físico actual
    if (estado_puntos[digito] != encender) {
        DisplayToggleDots(display, digito, digito);
        estado_puntos[digito] = encender; // Actualizamos nuestro registro
    }
}

void CambiarModo(modo_t valor) {
    modo=valor;

    switch (modo)
    {
    case MODO_SIN_AJUSTAR:
        DisplayFlashDigits(display_global, 0, 3, 250);
        break;
    
    case MODO_NORMAL:
        DisplayFlashDigits(display_global, 0, 0, 0);
        break;

    case MODO_MINUTOS:
        DisplayFlashDigits(display_global, 2, 3, 250);
        break;

    case MODO_HORAS:
        DisplayFlashDigits(display_global, 0, 1, 250);
        break;

    case MODO_MINUTOS_ALARMA:
        DisplayFlashDigits(display_global, 2, 3, 250);
        break;

    case MODO_HORAS_ALARMA:
        DisplayFlashDigits(display_global, 0, 1, 250);
        break;

    default:
        break;
    }
}

void SonarAlarma(void) {
    alarma_sonando = true; // Establecemos la bandera para indicar que la alarma debe sonar
}

void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {
    numero[1]++; // Incrementamos el dígito de las unidades
    
    // 1. Manejamos el desbordamiento normal (0 al 9)
    if (numero[1] > 9) {
        numero[1] = 0; // Reiniciamos las unidades a 0
        numero[0]++;   // Incrementamos el dígito de las decenas
    }
    
    // 2. Verificamos el límite global (ej. 23 o 59) SIEMPRE
    if (numero[0] > limite[0] || (numero[0] == limite[0] && numero[1] > limite[1])) {
        numero[0] = 0; // Reiniciamos las decenas a 0
        numero[1] = 0; // Reiniciamos las unidades a 0
    }
}

void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {
    if (numero[1] == 0) {
        if (numero[0] == 0) {
            // Si ambos dígitos son 0, reiniciamos al límite
            numero[0] = limite[0];
            numero[1] = limite[1];
        } else {
            numero[0]--; // Decrementamos las decenas
            numero[1] = 9; // Reiniciamos las unidades a 9
        }
    } else {
        numero[1]--; // Decrementamos las unidades
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

    // uint8_t entrada[4] = {0, 0, 0, 0}; 

    // INICIALIZACIÓN DE HARDWARE
    board_t placa = BoardCreate();
    display_global = placa->display; // Guardamos el puntero a la pantalla para usarlo en el SysTick_Handler
    reloj = RelojCreate(1000, SonarAlarma); // Configura el reloj para que genere un tick cada 1 ms
    SysTick_Config(SystemCoreClock / 1000); // Configura el SysTick para que salte 1000 veces por segundo (cada 1 ms)
    DigitalOutputDeactivate(placa->buzzer);

    // VARIABLES DE CONTROL DE LA APLICACIÓN
    hora_t hora_actual; // Arreglo para almacenar la hora actual (4 dígitos: HHMM) "borrador"
    uint32_t tiempo_inicio_f1 = 0;     // Para medir los 3 segundos de F1
    uint32_t tiempo_inicio_f2 = 0;     // Para medir los 3 segundos de F2

    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; 
    RelojSetHora(reloj, hora_inicial);

    //FUERZO EL ESTADO INICIAL DEL RELOJ A MODO_NORMAL
    CambiarModo(MODO_SIN_AJUSTAR);

    // Bucle principal de la aplicación. (Super Loop)
    while(true){

        // MAQUINA DE ESTADOS DEL RELOJ
        switch(modo) {

            case MODO_SIN_AJUSTAR:
                // Tarea: Mostrar la hora (CambiarModo ya hace que parpadee todo)
                RelojGetHora(reloj, hora_actual); 
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // Transición: F1 por 3 segundos para ir a ajustar la hora
                if (DigitalInputRead(placa->f1)) {
                    if ((contador_ms - tiempo_inicio_f1) >= 3000) {
                        CambiarModo(MODO_MINUTOS);
                    }
                } else {
                    tiempo_inicio_f1 = contador_ms; 
                }
                break;

            case MODO_NORMAL:
                // Tarea principal: Mostrar la hora fluida
                RelojGetHora(reloj, hora_actual); 
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // --- MANEJO DE PUNTOS ---
                SetPunto(placa->display, 0, false);
                SetPunto(placa->display, 1, ((contador_ms % 1000) < 500)); // Segundero perfecto
                SetPunto(placa->display, 2, false);
                SetPunto(placa->display, 3, RelojAlarmaEstaActiva(reloj)); // Luz de alarma

                // Transición 1: F1 por 3 segundos (Reajustar la hora)
                if (DigitalInputRead(placa->f1)) {
                    if ((contador_ms - tiempo_inicio_f1) >= 3000) {
                        CambiarModo(MODO_MINUTOS); 
                    }
                } else {
                    tiempo_inicio_f1 = contador_ms; 
                }    
                
                // Transición 2: F2 por 3 segundos (Ajustar alarma)
                if (DigitalInputRead(placa->f2)) {
                    if ((contador_ms - tiempo_inicio_f2) >= 3000) {
                        // Cargamos el borrador con la hora de la ALARMA
                        RelojGetAlarma(reloj, hora_actual); 
                        CambiarModo(MODO_MINUTOS_ALARMA);
                    }
                } else {
                    tiempo_inicio_f2 = contador_ms; 
                } 

                // --- MANEJO DE LA ALARMA SONANDO Y BUZZER ---
                if (alarma_sonando) {
                    DigitalOutputActivate(placa->buzzer); // Hacemos ruido

                    // Aceptar: Pospone por 5 minutos
                    if (DigitalInputHasActivated(placa->accept)) {
                        RelojPosponerAlarma(reloj, 5);
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                    // Cancelar: La apaga por hoy (hasta el día siguiente)
                    if (DigitalInputHasActivated(placa->cancel)) {
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                } else {
                    // Si no está sonando, operan normalmente para Activar/Desactivar
                    if (DigitalInputHasActivated(placa->accept)) {
                        RelojActivarAlarma(reloj, true);
                    }
                    if (DigitalInputHasActivated(placa->cancel)) {
                        RelojActivarAlarma(reloj, false);
                    }
                }
                break;

            case MODO_MINUTOS:
                // En este modo la pantalla parpadea sola gracias a CambiarModo(),
                // solo necesitamos actualizar constantemente el valor en la pantalla
                // con nuestra copia "borrador" local (hora_actual).
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // 1. Evaluar cancelación (abortar ajustes y volver)
                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL);
                }
                
                // 2. Evaluar confirmación (Aceptar minutos, pasar a configurar horas)
                if (DigitalInputHasActivated(placa->accept)) {
                    CambiarModo(MODO_HORAS);
                }

                // 3. Aumentar minutos (F4)
                if (DigitalInputHasActivated(placa->f4)) {
                    // Pasamos el puntero a la parte de los minutos (índice 2)
                    IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }

                // 4. Disminuir minutos (F3)
                if (DigitalInputHasActivated(placa->f3)) {
                    DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                break;
            case MODO_HORAS:
                // En este modo, CambiarModo() ya configuró que parpadeen los primeros 2 dígitos
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // Evaluar cancelación (abortar ajustes y volver a MODO_NORMAL)
                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL);
                }
                
                // Evaluar confirmación (Aceptar horas y volver a MODO_NORMAL)
                if (DigitalInputHasActivated(placa->accept)) {
                    // Aquí se usa la función del reloj para actualizar la hora con los valores ajustados
                    RelojSetHora(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL);
                }

                // Aumentar horas (F4)
                if (DigitalInputHasActivated(placa->f4)) {
                    // Pasamos el puntero al inicio del arreglo (índice 0, decenas de hora)
                    IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }

                // Disminuir horas (F3)
                if (DigitalInputHasActivated(placa->f3)) {
                    DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                break;
            case MODO_MINUTOS_ALARMA:
                // En este modo la pantalla parpadea sola gracias a CambiarModo(),
                // solo necesitamos actualizar constantemente el valor en la pantalla
                // con nuestra copia "borrador" local (hora_actual).
                DisplayWriteBCD(placa->display, hora_actual, 4);

                // Forzamos los 4 puntos encendidos fijos
                SetPunto(placa->display, 0, true);
                SetPunto(placa->display, 1, true);
                SetPunto(placa->display, 2, true);
                SetPunto(placa->display, 3, true);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL); // Descarta cambios
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    CambiarModo(MODO_HORAS_ALARMA); // Pasa a editar horas de la alarma
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                break;

            case MODO_HORAS_ALARMA:

                DisplayWriteBCD(placa->display, hora_actual, 4);

                // Forzamos los 4 puntos encendidos fijos
                SetPunto(placa->display, 0, true);
                SetPunto(placa->display, 1, true);
                SetPunto(placa->display, 2, true);
                SetPunto(placa->display, 3, true);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL); // Descarta cambios
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    // Guardamos el borrador en la Alarma
                    RelojSetAlarma(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL);
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                break;
        }
    }

    return 0;
}

/* === End of documentation ==================================================================== */

/*! @} End of module definition for doxygen */