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
 * @file main.c
 * @brief Aplicación principal del Reloj Despertador con FreeRTOS para la placa EDU-CIAA-NXP.
 * @details Este módulo contiene la inicialización del hardware, la creación de los recursos
 *          de concurrencia (Tareas y Grupos de Eventos) y la implementación de la máquina de 
 *          estados principal que gestiona el funcionamiento del reloj despertador de forma
 *          no bloqueante[cite: 8, 9].
 */

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

/* === Headers files inclusions ================================================================ */

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"  
#include "screen.h"  

/* === Public macros definitions =============================================================== */

/*! @brief Tiempo máximo de inactividad permitido en los modos de edición (30 segundos). */
#define TIMEOUT_INACTIVIDAD_MS 30000U

/*! @brief Tasa de refresco y escaneo de la tarea de teclado en milisegundos. */
#define TICK_RATE_TECLADO_MS 50U

/*! @brief Cantidad de ticks de teclado requeridos para considerar una pulsación como larga (3 segundos). */
#define PULSACION_LARGA_TICKS (3000U / TICK_RATE_TECLADO_MS)

/*! @brief Máscara de evento para la tecla ACCEPT. */
#define EVENTO_ACCEPT      (1 << 0)
/*! @brief Máscara de evento para la tecla CANCEL. */
#define EVENTO_CANCEL      (1 << 1)
/*! @brief Máscara de evento para la tecla F3 (Decrementar). */
#define EVENTO_F3          (1 << 2)
/*! @brief Máscara de evento para la tecla F4 (Incrementar). */
#define EVENTO_F4          (1 << 3)
/*! @brief Máscara de evento para la pulsación larga de la tecla F1 (Configurar Hora). */
#define EVENTO_F1_LARGO    (1 << 4)
/*! @brief Máscara de evento para la pulsación larga de la tecla F2 (Configurar Alarma). */
#define EVENTO_F2_LARGO    (1 << 5)

/* === Private data type declarations ========================================================== */

/*! 
 * @brief Estados operativos de la máquina de estados del reloj.
 */
typedef enum {
    MODO_SIN_AJUSTAR,       /*!< Estado inicial por defecto; la hora no ha sido configurada. */
    MODO_NORMAL,            /*!< Funcionamiento normal, muestra la hora actual y gestiona la alarma. */
    MODO_MINUTOS,           /*!< Modo de edición de los minutos de la hora actual. */
    MODO_HORAS,             /*!< Modo de edición de las horas de la hora actual. */
    MODO_MINUTOS_ALARMA,    /*!< Modo de edición de los minutos de la alarma. */
    MODO_HORAS_ALARMA       /*!< Modo de edición de las horas de la alarma. */
} modo_t;

/* === Private variables definitions =========================================================== */

/*! @brief Descriptor de la pantalla multiplexada de 7 segmentos. */
display_t display_global; 

/*! @brief Descriptor del reloj para la gestión de tiempo y alarmas. */
clock_t reloj; 

/*! @brief Almacena el estado actual de la máquina de estados del sistema. */
static modo_t modo; 

/*! @brief Bandera lógica que indica de forma asincrónica si la alarma se encuentra en reproducción. */
volatile bool alarma_sonando = false; 

/*! @brief Límite superior para el incremento circular de minutos en formato BCD (59). */
static const uint8_t LIMITE_MINUTOS[2] = { 5, 9 }; 

/*! @brief Límite superior para el incremento circular de horas en formato BCD (23). */
static const uint8_t LIMITE_HORAS[2] = { 2, 3 };   

/*! @brief Descriptor global del grupo de eventos de FreeRTOS para la sincronización entre el teclado y la lógica[cite: 14]. */
EventGroupHandle_t eventos_teclado;

/* === Private function declarations =========================================================== */

void SetPunto(display_t display, uint8_t digito, bool encender);
void CambiarModo(modo_t nuevo_modo, hora_t hora_borrador);
void SonarAlarma(void);
void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]);
void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]);

void TareaTeclado(void * pvParameters);
void TareaDisplay(void * pvParameters);
void TareaLogica(void * pvParameters);
void TareaReloj(void * pvParameters);

/* === Private function implementation ========================================================= */

/*! 
 * @brief Controla de forma optimizada el estado físico de un punto decimal de la pantalla.
 * @details Evalúa si el estado deseado difiere del estado actual para evitar inversiones innecesarias 
 *          que causen parpadeos irregulares en la visualización.
 * 
 * @param[in] display  Descriptor de la pantalla multiplexada.
 * @param[in] digito   Índice del dígito cuyo punto decimal se desea modificar (0 a 3).
 * @param[in] encender Booleano que establece el estado deseado: true (encendido) o false (apagado).
 */
void SetPunto(display_t display, uint8_t digito, bool encender) {
    static bool estado_puntos[4] = {false, false, false, false};
    
    if (estado_puntos[digito] != encender) {
        DisplayToggleDots(display, digito, digito);
        estado_puntos[digito] = encender; 
    }
}

/*! 
 * @brief Ejecuta la transición unificada entre los modos operativos del sistema.
 * @details Actualiza la variable de estado global y configura el comportamiento visual 
 *          (destellos y puntos) correspondiente al nuevo modo de manera centralizada.
 * 
 * @param[in]     nuevo_modo    Estado destino de tipo @ref modo_t.
 * @param[in,out] hora_borrador Puntero al arreglo BCD temporal. Requerido al transicionar a configuración de alarma.
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

/*! 
 * @brief Función callback invocada asincrónicamente por la librería reloj.
 * @details Modifica la variable global para solicitar la activación de los 
 *          recursos sonoros gestionados en el lazo principal.
 */
void SonarAlarma(void) {
    alarma_sonando = true; 
}

/*! 
 * @brief Realiza el incremento circular de un arreglo de dígitos BCD.
 * @details Resguarda el acarreo posicional y el retorno hacia cero cuando se 
 *          sobrepasa el límite máximo establecido.
 * 
 * @param[in,out] numero Arreglo de 2 elementos conteniendo las decenas [0] y unidades [1].
 * @param[in]     limite Arreglo de 2 elementos que define el valor límite superior.
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
 * @brief Realiza el decremento circular de un arreglo de dígitos BCD.
 * @details Controla el subdesbordamiento, aplicando un retorno circular hacia 
 *          el límite superior cuando la magnitud desciende por debajo de cero.
 * 
 * @param[in,out] numero Arreglo de 2 elementos conteniendo las decenas [0] y unidades [1].
 * @param[in]     limite Arreglo de 2 elementos que define el valor de reinicio superior.
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

/* === FreeRTOS Tasks Implementation =========================================================== */

/*! 
 * @brief Tarea encargada del barrido continuo del display multiplexado.
 * @details Emplea retardos absolutos a través de la API vTaskDelayUntil para garantizar 
 *          una cadencia estricta de ejecución, eliminando retardos acumulativos y 
 *          parpadeos[cite: 13].
 * 
 * @param[in] pvParameters Puntero opaco a la instancia del hardware (board_t).
 */
void TareaDisplay(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(true) {
        DisplayRefresh(placa->display);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); 
    }
}

/*! 
 * @brief Tarea responsable del avance temporal del componente lógico de reloj.
 * @details Actualiza la librería de reloj llamando de manera determinística a RelojTick,
 *          emulando la temporización antes provista por el hardware SysTick.
 * 
 * @param[in] pvParameters Puntero no utilizado.
 */
void TareaReloj(void * pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(true) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); 
        RelojTick(reloj);
    }
}

/*! 
 * @brief Tarea encargada del escaneo pasivo de las teclas e inyección de eventos.
 * @details Efectúa sondeos de hardware cediendo la ejecución al RTOS por medio de retardos 
 *          para lograr un filtro antirrebote de software[cite: 2]. Al mismo tiempo, contabiliza las retenciones para eventos de pulsación larga.
 * 
 * @param[in] pvParameters Puntero opaco a la instancia del hardware (board_t).
 */
void TareaTeclado(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    uint32_t contador_f1 = 0;
    uint32_t contador_f2 = 0;

    while(true) {
        if (DigitalInputHasActivated(placa->accept)) xEventGroupSetBits(eventos_teclado, EVENTO_ACCEPT);
        if (DigitalInputHasActivated(placa->cancel)) xEventGroupSetBits(eventos_teclado, EVENTO_CANCEL);
        if (DigitalInputHasActivated(placa->f3))     xEventGroupSetBits(eventos_teclado, EVENTO_F3);
        if (DigitalInputHasActivated(placa->f4))     xEventGroupSetBits(eventos_teclado, EVENTO_F4);

        if (DigitalInputRead(placa->f1)) {
            contador_f1++;
            if (contador_f1 == PULSACION_LARGA_TICKS) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F1_LARGO);
            }
        } else { 
            contador_f1 = 0; 
        }

        if (DigitalInputRead(placa->f2)) {
            contador_f2++;
            if (contador_f2 == PULSACION_LARGA_TICKS) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F2_LARGO);
            }
        } else { 
            contador_f2 = 0; 
        }

        vTaskDelay(pdMS_TO_TICKS(TICK_RATE_TECLADO_MS)); 
    }
}

/*! 
 * @brief Tarea núcleo que evalúa la máquina de estados e interacciones del usuario.
 * @details Se sirve de la espera pasiva de FreeRTOS para mantenerse suspendida hasta la 
 *          generación de un evento[cite: 2, 8]. Administra paralelamente el temporizador
 *          de inactividad utilizando los límites de timeout en el grupo de eventos.
 * 
 * @param[in] pvParameters Puntero opaco a la instancia del hardware (board_t).
 */
void TareaLogica(void * pvParameters) {
    board_t placa = (board_t)pvParameters;
    hora_t hora_actual; 
    EventBits_t eventos;
    uint32_t tiempo_inactividad = 0;

    CambiarModo(MODO_SIN_AJUSTAR, NULL);

    while(true) {
        eventos = xEventGroupWaitBits(
            eventos_teclado, 
            EVENTO_ACCEPT | EVENTO_CANCEL | EVENTO_F3 | EVENTO_F4 | EVENTO_F1_LARGO | EVENTO_F2_LARGO,
            pdTRUE, 
            pdFALSE, 
            pdMS_TO_TICKS(50)
        );

        if (eventos != 0) {
            tiempo_inactividad = 0; 
        } else {
            tiempo_inactividad += 50; 
        }

        if (modo == MODO_MINUTOS || modo == MODO_HORAS || modo == MODO_MINUTOS_ALARMA || modo == MODO_HORAS_ALARMA) {
            if (tiempo_inactividad >= TIMEOUT_INACTIVIDAD_MS) {
                CambiarModo(MODO_NORMAL, NULL); 
                tiempo_inactividad = 0;
            }
        }

        if (modo == MODO_SIN_AJUSTAR || modo == MODO_NORMAL) {
            RelojGetHora(reloj, hora_actual);
        }
        DisplayWriteBCD(placa->display, hora_actual, 4);

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
                    CambiarModo(MODO_MINUTOS_ALARMA, hora_actual); 
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
                    RelojSetHora(reloj, hora_actual); 
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
                    RelojSetAlarma(reloj, hora_actual); 
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (eventos & EVENTO_F4) IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                if (eventos & EVENTO_F3) DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                break;
        }
    }
}

/* === Public function implementation ========================================================== */

/*! 
 * @brief Punto de entrada principal del firmware de la aplicación.
 * @details Se encarga de instanciar los componentes hardware, inicializar la estructura
 *          de recursos del sistema operativo e invocar el Scheduler de tareas de FreeRTOS.
 * 
 * @return int El entorno de FreeRTOS intercepta el retorno; la aplicación no debería culminar.
 */
int main(void) {
    board_t placa = BoardCreate();
    display_global = placa->display; 
    
    reloj = RelojCreate(1000, SonarAlarma); 
    DigitalOutputDeactivate(placa->buzzer);

    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; 
    RelojSetHora(reloj, hora_inicial);

    eventos_teclado = xEventGroupCreate(); 

    xTaskCreate(TareaDisplay, "Display", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(TareaReloj,   "Reloj",   configMINIMAL_STACK_SIZE, NULL,         tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(TareaLogica,  "Logica",  configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(TareaTeclado, "Teclado", configMINIMAL_STACK_SIZE, (void*)placa, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    return 0; 
}
/* === End of documentation ==================================================================== */