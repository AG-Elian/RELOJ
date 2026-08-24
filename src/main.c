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
 * @brief   Aplicación principal del Reloj Despertador con FreeRTOS para EDU-CIAA-NXP.
 * @details Implementación de la máquina de estados y control de hardware utilizando
 *          el RTOS para garantizar el determinismo y la concurrencia de tareas.
 */

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

/* === Inclusiones de cabeceras ================================================================ */

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"
#include "screen.h"

/* === Declaraciones de tipos de datos privados ================================================ */

/*!
 * @brief Estados operativos de la máquina de estados del reloj y la alarma.
 */
typedef enum {
    MODO_SIN_AJUSTAR,       /*!< Estado inicial, la hora no ha sido configurada. */
    MODO_NORMAL,            /*!< Funcionamiento normal, muestra la hora actual. */
    MODO_MINUTOS,           /*!< Configuración de los minutos del reloj. */
    MODO_HORAS,             /*!< Configuración de las horas del reloj. */
    MODO_MINUTOS_ALARMA,    /*!< Configuración de los minutos de la alarma. */
    MODO_HORAS_ALARMA       /*!< Configuración de las horas de la alarma. */
} modo_t;

/* === Definiciones de macros y variables globales ============================================= */

/* Máscaras de bits para el Grupo de Eventos del Teclado */
#define EVENTO_F1_PRESIONADA     (1 << 0)
#define EVENTO_F2_PRESIONADA     (1 << 1)
#define EVENTO_F3_PRESIONADA     (1 << 2)
#define EVENTO_F4_PRESIONADA     (1 << 3)
#define EVENTO_ACCEPT_PRESIONADA (1 << 4)
#define EVENTO_CANCEL_PRESIONADA (1 << 5)

/*! @brief Manejador global del grupo de eventos para la comunicación Teclado -> Lógica */
static EventGroupHandle_t eventos_teclado;

/*! @brief Descriptores globales de hardware y lógica */
display_t display_global;
clock_t reloj;
static modo_t modo;

/* Límites aritméticos para formato BCD */
static const uint8_t LIMITE_MINUTOS[2] = { 5, 9 }; 
static const uint8_t LIMITE_HORAS[2]   = { 2, 3 };   

/* === Declaraciones de funciones privadas ===================================================== */

void TareaTeclado(void * pvParameters);
void TareaDisplay(void * pvParameters);
void TareaLogica(void * pvParameters);
void TareaReloj(void * pvParameters);
void CambiarModo(modo_t nuevo_modo, hora_t hora_borrador);
void SetPunto(display_t display, uint8_t digito, bool encender);

/* === Implementación de funciones privadas ==================================================== */

/*! 
 * @brief Controla de forma absoluta el estado físico de un punto decimal específico.
 * Evita el parpadeo de alta frecuencia (efecto PWM) realizando la inversión lógica 
 * (Toggle) únicamente si el estado deseado difiere del estado actual registrado.
 * * @param[in] display Descriptor de la pantalla multiplexada.
 * @param[in] digito  Índice del dígito cuyo punto se desea controlar (0 a 3).
 * @param[in] encender Booleano que define si se enciende (true) o apaga (false) el punto.
 */
void SetPunto(display_t display, uint8_t digito, bool encender) {
    static bool estado_puntos[4] = {false, false, false, false};
    
    if (estado_puntos[digito] != encender) {
        DisplayToggleDots(display, digito, digito);
        estado_puntos[digito] = encender; 
    }
}

/*!
 * @brief Cambia el estado lógico del reloj y actualiza la visualización (parpadeo).
 * @param nuevo_modo    Estado destino de la máquina.
 * @param hora_borrador Buffer donde se resguarda la hora actual al entrar en modo ajuste.
 */
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
            DisplayFlashDigits(display_global, 0, 0, 0); // Apaga destello de dígitos
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 2, false);
            break;

        case MODO_MINUTOS:
            DisplayFlashDigits(display_global, 2, 3, 250); // Parpadean minutos
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 1, false);
            SetPunto(display_global, 2, false);
            SetPunto(display_global, 3, false);
            break;

        case MODO_HORAS:
            DisplayFlashDigits(display_global, 0, 1, 250); // Parpadean horas
            SetPunto(display_global, 0, false);
            SetPunto(display_global, 1, false);
            SetPunto(display_global, 2, false);
            SetPunto(display_global, 3, false);
            break;

        case MODO_MINUTOS_ALARMA:
            if (hora_borrador != NULL) {
                RelojGetAlarma(reloj, hora_borrador); // Carga la alarma actual al borrador
            }
            DisplayFlashDigits(display_global, 2, 3, 250); // Parpadean minutos de alarma
            SetPunto(display_global, 0, true);
            SetPunto(display_global, 1, true);
            SetPunto(display_global, 2, true);
            SetPunto(display_global, 3, true);
            break;

        case MODO_HORAS_ALARMA:
            DisplayFlashDigits(display_global, 0, 1, 250); // Parpadean horas de alarma
            SetPunto(display_global, 0, true);
            SetPunto(display_global, 1, true);
            SetPunto(display_global, 2, true);
            SetPunto(display_global, 3, true);
            break;

        default:
            break;
    }
}

/*!
 * @brief Incrementa posicionalmente un arreglo BCD de 2 dígitos.
 */
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

/*!
 * @brief Decrementa posicionalmente un arreglo BCD de 2 dígitos.
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

/* === Tareas de FreeRTOS ====================================================================== */

/*!
 * @brief Tarea: Escaneo de Teclado (Prioridad 1)
 * @details Sondea periódicamente las entradas digitales aplicando antirrebote pasivo.
 */
void TareaTeclado(void * pvParameters) {
    board_t placa = (board_t) pvParameters;

    while (true) {
        if (DigitalInputHasActivated(placa->f1)) xEventGroupSetBits(eventos_teclado, EVENTO_F1_PRESIONADA);
        if (DigitalInputHasActivated(placa->f2)) xEventGroupSetBits(eventos_teclado, EVENTO_F2_PRESIONADA);
        if (DigitalInputHasActivated(placa->f3)) xEventGroupSetBits(eventos_teclado, EVENTO_F3_PRESIONADA);
        if (DigitalInputHasActivated(placa->f4)) xEventGroupSetBits(eventos_teclado, EVENTO_F4_PRESIONADA);
        if (DigitalInputHasActivated(placa->accept)) xEventGroupSetBits(eventos_teclado, EVENTO_ACCEPT_PRESIONADA);
        if (DigitalInputHasActivated(placa->cancel)) xEventGroupSetBits(eventos_teclado, EVENTO_CANCEL_PRESIONADA);

        vTaskDelay(pdMS_TO_TICKS(40)); // Antirrebote de 40ms
    }
}

/*!
 * @brief Tarea: Multiplexado de Display (Prioridad 3 - Alta)
 * @details Refresca la pantalla estrictamente cada 1 ms para evitar parpadeo.
 */
void TareaDisplay(void * pvParameters) {
    board_t placa = (board_t) pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrecuencia = pdMS_TO_TICKS(1); 

    while (true) {
        DisplayRefresh(placa->display);
        vTaskDelayUntil(&xLastWakeTime, xFrecuencia); 
    }
}

/*!
 * @brief Tarea: Lógica del Reloj (Prioridad 1)
 * @details Despierta únicamente ante eventos del teclado para procesar la máquina de estados.
 */
void TareaLogica(void * pvParameters) {
    board_t placa = (board_t) pvParameters;
    hora_t hora_actual;

    const EventBits_t eventos_a_escuchar = EVENTO_F1_PRESIONADA | EVENTO_F2_PRESIONADA | 
                                           EVENTO_F3_PRESIONADA | EVENTO_F4_PRESIONADA |
                                           EVENTO_ACCEPT_PRESIONADA | EVENTO_CANCEL_PRESIONADA; 

    while (true) {
        EventBits_t eventos = xEventGroupWaitBits(eventos_teclado, eventos_a_escuchar, pdTRUE, pdFALSE, portMAX_DELAY);

        switch (modo) {
            case MODO_SIN_AJUSTAR:
                DisplayFlashDigits(display_global, 0, 3, 250);
                if (eventos & EVENTO_F1_PRESIONADA) {
                    RelojGetHora(reloj, hora_actual);
                    CambiarModo(MODO_MINUTOS, hora_actual); 
                }
                break; 
            break;

            case MODO_NORMAL:
                if (eventos & EVENTO_F1_PRESIONADA) {
                    RelojGetHora(reloj, hora_actual);
                    CambiarModo(MODO_MINUTOS, hora_actual); 
                }
                if (eventos & EVENTO_F2_PRESIONADA) {
                    RelojGetHora(reloj, hora_actual); 
                    CambiarModo(MODO_MINUTOS_ALARMA, hora_actual); 
                }
                if (eventos & EVENTO_ACCEPT_PRESIONADA) RelojActivarAlarma(reloj, true);
                if (eventos & EVENTO_CANCEL_PRESIONADA) RelojActivarAlarma(reloj, false);
                break;

            case MODO_MINUTOS:
                if (eventos & EVENTO_CANCEL_PRESIONADA) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT_PRESIONADA) CambiarModo(MODO_HORAS, NULL);
                if (eventos & EVENTO_F4_PRESIONADA) IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                if (eventos & EVENTO_F3_PRESIONADA) DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                break;

            case MODO_HORAS:
                if (eventos & EVENTO_CANCEL_PRESIONADA) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT_PRESIONADA) {
                    RelojSetHora(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (eventos & EVENTO_F4_PRESIONADA) IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                if (eventos & EVENTO_F3_PRESIONADA) DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                break;
                
            case MODO_MINUTOS_ALARMA:
                if (eventos & EVENTO_CANCEL_PRESIONADA) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT_PRESIONADA) CambiarModo(MODO_HORAS_ALARMA, NULL);
                if (eventos & EVENTO_F4_PRESIONADA) IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                if (eventos & EVENTO_F3_PRESIONADA) DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                break;

            case MODO_HORAS_ALARMA:
                if (eventos & EVENTO_CANCEL_PRESIONADA) CambiarModo(MODO_NORMAL, NULL);
                if (eventos & EVENTO_ACCEPT_PRESIONADA) {
                    RelojSetAlarma(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (eventos & EVENTO_F4_PRESIONADA) IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                if (eventos & EVENTO_F3_PRESIONADA) DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                break;
            // (Misma lógica implementada de forma análoga para MODO_MINUTOS_ALARMA y MODO_HORAS_ALARMA)
            default:
                break;
        }
        
        // Actualizamos el display si estamos configurando
        if (modo != MODO_NORMAL && modo != MODO_SIN_AJUSTAR) {
            DisplayWriteBCD(placa->display, hora_actual, 4);
        }
    }
}

/*!
 * @brief Tarea: Base de Tiempo (Prioridad 2)
 * @details Incrementa los segundos del reloj con un periodo exacto de 1000 ms.
 */
void TareaReloj(void * pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrecuencia = pdMS_TO_TICKS(500); // 500 ms para permitir el parpadeo
    hora_t hora_actual; 
    bool medio_segundo = false;

    while (true) {
        vTaskDelayUntil(&xLastWakeTime, xFrecuencia);
        
        medio_segundo = !medio_segundo; // Invierte el estado cada 500ms

        // El reloj interno solo avanza cada 1000ms (2 ciclos de 500ms)
        if (medio_segundo) {
            RelojTick(reloj); 
        }

        // Lógica de visualización dinámica de puntos
        if (modo == MODO_NORMAL || modo == MODO_SIN_AJUSTAR) {
            
            // Actualiza los números del display solo cuando cambia el segundo
            if (medio_segundo) {
                RelojGetHora(reloj, hora_actual);
                DisplayWriteBCD(display_global, hora_actual, 4);
            }
            
            // Parpadeo del punto segundero
            SetPunto(display_global, 1, medio_segundo); 
            
            // Indicador fijo de alarma activa
            SetPunto(display_global, 3, RelojAlarmaEstaActiva(reloj)); 
        }
    }
}

/* === Implementación de funciones públicas ==================================================== */

int main(void) {
    /* 1. Hardware y Variables Base */
    board_t placa = BoardCreate();
    display_global = placa->display; 
    reloj = RelojCreate(1, NULL); 

    /* 2. Recursos IPC (Comunicación entre tareas) */
    eventos_teclado = xEventGroupCreate();

    /* 3. Instanciación de Tareas */
    xTaskCreate(TareaTeclado, "Teclado", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(TareaLogica,  "Logica",  configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(TareaReloj,   "Reloj",   configMINIMAL_STACK_SIZE, NULL,         tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(TareaDisplay, "Display", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 3, NULL);

    /* 4. Ejecución del RTOS */
    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; // Arrancamos en 12:00
    RelojSetHora(reloj, hora_inicial);        // Damos validez al reloj
    DisplayWriteBCD(display_global, hora_inicial, 4); // Escribimos en memoria
    
    CambiarModo(MODO_SIN_AJUSTAR, NULL);      // Activamos el destello
    vTaskStartScheduler();

    return 0;
}
/* === Fin del archivo ========================================================================= */