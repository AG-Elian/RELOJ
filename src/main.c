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
 ** @brief Aplicación principal del Reloj Despertador con FreeRTOS para la placa EDU-CIAA-NXP.
 **/

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

/* === Headers files inclusions ==================================================================================== */

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"  
#include "screen.h"  

/* === Private macros definitions ============================================================= */

#define TIMEOUT_INACTIVIDAD_MS 30000U
#define TICK_RATE_TECLADO_MS 50U
#define PULSACION_LARGA_TICKS (3000U / TICK_RATE_TECLADO_MS) // 3 segundos equivalen a 60 ticks de 50ms

/* Máscaras de bits para los eventos de teclado */
#define EVENTO_ACCEPT      (1 << 0)
#define EVENTO_CANCEL      (1 << 1)
#define EVENTO_F3          (1 << 2)
#define EVENTO_F4          (1 << 3)
#define EVENTO_F1_LARGO    (1 << 4)
#define EVENTO_F2_LARGO    (1 << 5)

/* === Private data type declarations ========================================================== */

typedef enum {
    MODO_SIN_AJUSTAR,
    MODO_NORMAL,
    MODO_MINUTOS,
    MODO_HORAS,
    MODO_MINUTOS_ALARMA,
    MODO_HORAS_ALARMA
} modo_t;

/* === Private variables definitions ========================================================= */

display_t display_global; 
clock_t reloj; 
static modo_t modo; 
volatile bool alarma_sonando = false; 

static const uint8_t LIMITE_MINUTOS[2] = { 5, 9 }; 
static const uint8_t LIMITE_HORAS[2] = { 2, 3 };   

/* Descriptor del grupo de eventos para la sincronización entre teclado y lógica */
EventGroupHandle_t eventos_teclado;

/* === Private function declarations =========================================================== */

void SetPunto(display_t display, uint8_t digito, bool encender);
void CambiarModo(modo_t nuevo_modo, hora_t hora_borrador);
void SonarAlarma(void);
void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]);
void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]);

/* Prototipos de Tareas de FreeRTOS */
void TareaTeclado(void * pvParameters);
void TareaDisplay(void * pvParameters);
void TareaLogica(void * pvParameters);
void TareaReloj(void * pvParameters);

/* === Private function implementation ========================================================= */

void SetPunto(display_t display, uint8_t digito, bool encender) {
    static bool estado_puntos[4] = {false, false, false, false};
    
    if (estado_puntos[digito] != encender) {
        DisplayToggleDots(display, digito, digito);
        estado_puntos[digito] = encender; 
    }
}

void CambiarModo(modo_t nuevo_modo, hora_t hora_borrador) {
    modo = nuevo_modo;

    switch (modo) {
        case MODO_SIN_AJUSTAR:
            DisplayFlashDigits(display_global, 0, 3, 250);
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 1, false);
            SetPunto(display_global, 2, false);
            SetPunto(display_global, 3, false);
            break;
        
        case MODO_NORMAL:
            DisplayFlashDigits(display_global, 0, 0, 0); 
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 2, false);
            break;

        case MODO_MINUTOS:
            DisplayFlashDigits(display_global, 2, 3, 250); 
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 1, false);
            SetPunto(display_global, 2, false);
            SetPunto(display_global, 3, false);
            break;

        case MODO_HORAS:
            DisplayFlashDigits(display_global, 0, 1, 250); 
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 1, false);
            SetPunto(display_global, 2, false);
            SetPunto(display_global, 3, false);
            break;

        case MODO_MINUTOS_ALARMA:
            if (hora_borrador != NULL) {
                RelojGetAlarma(reloj, hora_borrador); 
            }
            DisplayFlashDigits(display_global, 2, 3, 250); 
            SetPunto(display_global, 0, true);
            SetPunto(display_global, 1, true);
            SetPunto(display_global, 2, true);
            SetPunto(display_global, 3, true);
            break;

        case MODO_HORAS_ALARMA:
            DisplayFlashDigits(display_global, 0, 1, 250); 
            SetPunto(display_global, 0, true);
            SetPunto(display_global, 1, true);
            SetPunto(display_global, 2, true);
            SetPunto(display_global, 3, true);
            break;

        default:
            break;
    }
}

void SonarAlarma(void) {
    alarma_sonando = true; 
}

void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {
    numero[1]++; 
    if (numero[1] > 9) {
        numero[1] = 0; 
        numero[0]++;   
    }
    if (numero[0] > limite[0] || (numero[0] == limite[0] && numero[1] > limite[1])) {
        numero[0] = 0; 
        numero[1] = 0; 
    }
}

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

/* === FreeRTOS Tasks Implementation =========================================================== */

/*! @brief Tarea de refresco de pantalla multiplexada. (Mayor Prioridad) */
void TareaDisplay(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(true) {
        DisplayRefresh(placa->display);
        // Garantizamos el barrido constante evitando titileos (ghosting)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); 
    }
}

/*! @brief Tarea para avanzar el tiempo de la lógica base. (Prioridad Alta) */
void TareaReloj(void * pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(true) {
        //Se requiere llamar RelojTick cada milisegundo internamente
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); 
        RelojTick(reloj);
    }
}

/*! @brief Tarea de escaneo de botones con antirrebote pasivo. (Prioridad Baja) */
void TareaTeclado(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    uint32_t contador_f1 = 0;
    uint32_t contador_f2 = 0;

    while(true) {
        // Procesamiento directo de flancos
        if (DigitalInputHasActivated(placa->accept)) xEventGroupSetBits(eventos_teclado, EVENTO_ACCEPT);
        if (DigitalInputHasActivated(placa->cancel)) xEventGroupSetBits(eventos_teclado, EVENTO_CANCEL);
        if (DigitalInputHasActivated(placa->f3))     xEventGroupSetBits(eventos_teclado, EVENTO_F3);
        if (DigitalInputHasActivated(placa->f4))     xEventGroupSetBits(eventos_teclado, EVENTO_F4);

        // Algoritmo de retención: F1 pulsado durante 3 Segundos
        if (DigitalInputRead(placa->f1)) {
            contador_f1++;
            if (contador_f1 == PULSACION_LARGA_TICKS) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F1_LARGO);
            }
        } else { 
            contador_f1 = 0; 
        }

        // Algoritmo de retención: F2 pulsado durante 3 Segundos
        if (DigitalInputRead(placa->f2)) {
            contador_f2++;
            if (contador_f2 == PULSACION_LARGA_TICKS) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F2_LARGO);
            }
        } else { 
            contador_f2 = 0; 
        }

        // Bloqueo pasivo para ceder CPU y generar Antirrebote (Debounce)
        vTaskDelay(pdMS_TO_TICKS(TICK_RATE_TECLADO_MS)); 
    }
}

/*! @brief Tarea del cerebro principal y máquina de estados. (Prioridad Normal) */
void TareaLogica(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    hora_t hora_actual; 
    EventBits_t eventos;
    uint32_t tiempo_inactividad = 0;

    CambiarModo(MODO_SIN_AJUSTAR, NULL);

    while(true) {
        // La tarea descansa a menos que presiones un botón o expiren los 50ms de timeout
        eventos = xEventGroupWaitBits(
            eventos_teclado, 
            EVENTO_ACCEPT | EVENTO_CANCEL | EVENTO_F3 | EVENTO_F4 | EVENTO_F1_LARGO | EVENTO_F2_LARGO,
            pdTRUE, 
            pdFALSE, 
            pdMS_TO_TICKS(50)
        );

        // Control de Timeout por Inactividad (30 Segundos)
        if (eventos != 0) {
            tiempo_inactividad = 0; // Si el usuario toco algo, reiniciamos inactividad
        } else {
            tiempo_inactividad += 50; 
        }

        if (modo == MODO_MINUTOS || modo == MODO_HORAS || modo == MODO_MINUTOS_ALARMA || modo == MODO_HORAS_ALARMA) {
            if (tiempo_inactividad >= TIMEOUT_INACTIVIDAD_MS) {
                CambiarModo(MODO_NORMAL, NULL); // Vuelve atrás descartando modificaciones
                tiempo_inactividad = 0;
            }
        }

        // Lectura de tiempo y Refresco Lógico del Display 
        if (modo == MODO_SIN_AJUSTAR || modo == MODO_NORMAL) {
            RelojGetHora(reloj, hora_actual);
        }
        DisplayWriteBCD(placa->display, hora_actual, 4);

        // Gestión de parpadeo de punto central (500ms on / 500ms off)
        if (modo == MODO_SIN_AJUSTAR || modo == MODO_NORMAL) {
            SetPunto(placa->display, 1, ((xTaskGetTickCount() % 1000) < 500));
        }

        switch(modo) {
            case MODO_SIN_AJUSTAR:
                if (eventos & EVENTO_F1_LARGO) {
                    CambiarModo(MODO_MINUTOS, NULL);
                }
                break;

            case MODO_NORMAL:
                SetPunto(placa->display, 3, RelojAlarmaEstaActiva(reloj)); 

                if (eventos & EVENTO_F1_LARGO) {
                    CambiarModo(MODO_MINUTOS, NULL);
                }
                if (eventos & EVENTO_F2_LARGO) {
                    CambiarModo(MODO_MINUTOS_ALARMA, hora_actual); // Pasa la alarma a buffer temporal
                }

                if (alarma_sonando) {
                    DigitalOutputActivate(placa->buzzer); 

                    if (eventos & EVENTO_ACCEPT) {
                        RelojPosponerAlarma(reloj, 5);
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                    if (eventos & EVENTO_CANCEL) {
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                } else {
                    if (eventos & EVENTO_ACCEPT) {
                        RelojActivarAlarma(reloj, true);
                    }
                    if (eventos & EVENTO_CANCEL) {
                        RelojActivarAlarma(reloj, false);
                    }
                }
                break;

            case MODO_MINUTOS:
                if (eventos & EVENTO_CANCEL) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT) CambiarModo(MODO_HORAS, NULL);
                if (eventos & EVENTO_F4) IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                if (eventos & EVENTO_F3) DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                break;

            case MODO_HORAS:
                if (eventos & EVENTO_CANCEL) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT) {
                    RelojSetHora(reloj, hora_actual); // Aplicamos y guardamos la hora 
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (eventos & EVENTO_F4) IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                if (eventos & EVENTO_F3) DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                break;

            case MODO_MINUTOS_ALARMA:
                if (eventos & EVENTO_CANCEL) CambiarModo(MODO_NORMAL, NULL); 
                if (eventos & EVENTO_ACCEPT) CambiarModo(MODO_HORAS_ALARMA, NULL); 
                if (eventos & EVENTO_F4) IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                if (eventos & EVENTO_F3) DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                break;

            case MODO_HORAS_ALARMA:
                if (eventos & EVENTO_CANCEL) CambiarModo(MODO_NORMAL, NULL); 
                if (eventos & EVENTO_ACCEPT) {
                    RelojSetAlarma(reloj, hora_actual); // Aplicamos y guardamos la alarma 
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (eventos & EVENTO_F4) IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                if (eventos & EVENTO_F3) DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                break;
        }
    }
}

/* === Public function implementation ========================================================== */

int main(void) {
    // 1. Inicialización de Hardware y Variables
    board_t placa = BoardCreate();
    display_global = placa->display; 
    
    // Mantenemos 1000 ticks por segundo para que el reloj avance cada 1ms
    reloj = RelojCreate(1000, SonarAlarma); 
    
    DigitalOutputDeactivate(placa->buzzer);

    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; 
    RelojSetHora(reloj, hora_inicial);

    // 2. Creación de recursos IPC de RTOS
    eventos_teclado = xEventGroupCreate(); 

    // 3. Creación de las 4 Tareas de FreeRTOS con sus Prioridades 
    xTaskCreate(TareaDisplay, "Display", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(TareaReloj,   "Reloj",   configMINIMAL_STACK_SIZE, NULL,         tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(TareaLogica,  "Logica",  configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(TareaTeclado, "Teclado", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 1, NULL);

    // 4. Inicio del Planificador
    vTaskStartScheduler();

    return 0; // El programa nunca deberia abandonar el scheduler
}
/* === End of documentation ==================================================================== */