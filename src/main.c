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
 ** @brief Aplicación principal del Reloj Despertador para la placa EDU-CIAA-NXP.
 **
 ** Este módulo contiene el Super Loop principal y la máquina de estados que gestiona
 ** el funcionamiento integral del reloj despertador, controlando la puesta en hora,
 ** la configuración de la alarma, el parpadeo de dígitos y puntos decimales, el
 ** temporizador de inactividad (timeout de 30 segundos) y el silenciado/posposición 
 ** (Snooze) de la alarma.
 **/

#ifndef EDU_CIAA_NXP
#error "This program can only be compiled for the EDU-CIAA-NXP board"
#endif

/* === Headers files inclusions ==================================================================================== */

#include "digital.h"
#include "placa.h"
#include "reloj.h"
#include "poncho.h"  
#include "screen.h"  

/* === Private macros definitions ============================================================= */

/*! @brief Tiempo en milisegundos de pulsación sostenida para cambiar de modo (3 segundos) */
#define TIEMPO_PULSACION_MS 3000U

/*! @brief Tiempo de inactividad máximo en modos de edición para cancelar cambios (30 segundos) */
#define TIMEOUT_INACTIVIDAD_MS 30000U

/* === Private data type declarations ========================================================== */

/*! 
 * @brief Estados de la máquina de estados del reloj y la alarma.
 */
typedef enum {
    MODO_SIN_AJUSTAR,    /*!< Estado inicial por defecto; la pantalla destella completamente */
    MODO_NORMAL,         /*!< Funcionamiento normal; muestra la hora actual y titila el segundero */
    MODO_MINUTOS,        /*!< Modo de configuración de los minutos de la hora actual */
    MODO_HORAS,          /*!< Modo de configuración de las horas de la hora actual */
    MODO_MINUTOS_ALARMA, /*!< Modo de configuración de los minutos de la alarma */
    MODO_HORAS_ALARMA    /*!< Modo de configuración de las horas de la alarma */
} modo_t;

/* === Private variables definitions ========================================================= */

/*! @brief Descriptor global de la pantalla de 7 segmentos para acceso desde la ISR */
display_t display_global; 

/*! @brief Instancia lógica del reloj que gestiona la hora y los eventos de alarma */
clock_t reloj; 

/*! @brief Base de tiempo global incrementada cada 1 ms en el SysTick_Handler */
volatile uint32_t contador_ms = 0; 

/*! @brief Almacena el modo operativo actual de la máquina de estados */
static modo_t modo; 

/*! @brief Bandera lógica global que indica si la alarma se encuentra en reproducción */
volatile bool alarma_sonando = false; 

/*! @brief Límite superior compuesto para minutos en formato BCD descompactado (59) */
static const uint8_t LIMITE_MINUTOS[2] = { 5, 9 }; 

/*! @brief Límite superior compuesto para horas en formato BCD descompactado (23) */
static const uint8_t LIMITE_HORAS[2] = { 2, 3 };   

/* === Private function declarations =========================================================== */

void SetPunto(display_t display, uint8_t digito, bool encender);
void CambiarModo(modo_t nuevo_modo, hora_t hora_borrador);
void SonarAlarma(void);
void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]);
void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]);

/* === Private function implementation ========================================================= */

/*! 
 * @brief Rutina de atención de la interrupción del SysTick (ISR).
 * Interrumpe de manera determinística el lazo principal cada 1 ms (1 kHz) para
 * refrescar el multiplexado de los displays y avanzar el contador de ticks del reloj.
 */
void SysTick_Handler(void) {
    DisplayRefresh(display_global);
    RelojTick(reloj);
    contador_ms++;
}

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
 * @brief Realiza la transición de estados del sistema unificando todas las acciones colaterales.
 * Actualiza el modo operativo y centraliza la configuración de parpadeo de dígitos,
 * encendido de puntos indicativos y la carga del buffer borrador según el estado destino.
 * * @param[in]     nuevo_modo    El nuevo estado de tipo @ref modo_t al que transiciona el reloj.
 * @param[in,out] hora_borrador Arreglo borrador donde se carga la hora o alarma para edición.
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
 * @brief Función callback que se ejecuta automáticamente cuando se dispara la alarma.
 * Registrada en la instanciación del reloj, asienta de forma asincrónica la bandera
 * global para activar las acciones sonoras en el lazo principal.
 */
void SonarAlarma(void) {
    alarma_sonando = true; 
}

/*! 
 * @brief Incrementa un par de dígitos representados en formato BCD descompactado.
 * Resguarda las reglas aritméticas de acarreo posicional y evalúa en cada ciclo el
 * límite compuesto provisto para realizar el reinicio circular (Rollover).
 * * @param[in,out] numero Arreglo de 2 elementos que almacena las decenas [0] y unidades [1].
 * @param[in]     limite Arreglo de 2 elementos que define el valor máximo admisible.
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
 * @brief Decrementa un par de dígitos representados en formato BCD descompactado.
 * Controla el subdesbordamiento de las unidades y decenas realizando el retorno circular
 * hacia el valor límite superior cuando el par posicional alcanza el valor nulo (00).
 * * @param[in,out] numero Arreglo de 2 elementos que almacena las decenas [0] y unidades [1].
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

/* === Public function implementation ========================================================== */

/*! 
 * @brief Función principal del sistema (Punto de entrada).
 * Inicializa el hardware a través de la capa HAL, establece una hora inicial por defecto,
 * fuerza el estado primario no configurado y ejecuta de manera infinita el bucle de control 
 * (Super Loop) que evalúa las entradas lógicas, timeouts por inactividad y las acciones asociadas.
 * * @return int Retorna siempre 0 (el flujo operativo no debería salir del bucle continuo).
 */
int main(void) {

    // INICIALIZACIÓN DE HARDWARE Y DRIVERS
    board_t placa = BoardCreate();
    display_global = placa->display; 
    reloj = RelojCreate(1000, SonarAlarma); 
    SysTick_Config(SystemCoreClock / 1000); 
    DigitalOutputDeactivate(placa->buzzer);

    // CONTROL INICIAL DE VARIABLES
    hora_t hora_actual; 
    uint32_t tiempo_inicio_f1 = 0;     
    uint32_t tiempo_inicio_f2 = 0;     
    uint32_t tiempo_ultima_actividad = 0;

    hora_t hora_inicial = {1, 2, 0, 0, 0, 0}; 
    RelojSetHora(reloj, hora_inicial);

    // El sistema se inicia forzando el estado sin ajustar exigido por la cátedra
    CambiarModo(MODO_SIN_AJUSTAR, NULL);

    // LAZO INFINITO DE CONTROL (SUPER LOOP)
    while(true){

        // TEMPORIZADOR DE INACTIVIDAD (Timeout de 30 segundos en modos de edición)
        if (modo == MODO_MINUTOS || modo == MODO_HORAS || 
            modo == MODO_MINUTOS_ALARMA || modo == MODO_HORAS_ALARMA) {
            if ((contador_ms - tiempo_ultima_actividad) >= TIMEOUT_INACTIVIDAD_MS) {
                CambiarModo(MODO_NORMAL, NULL); // Descarta cambios por inactividad
            }
        }

        switch(modo) {

            case MODO_SIN_AJUSTAR:
                RelojGetHora(reloj, hora_actual); 
                DisplayWriteBCD(placa->display, hora_actual, 4);

                SetPunto(placa->display, 0, false);
                SetPunto(placa->display, 1, ((contador_ms % 1000) < 500)); 
                SetPunto(placa->display, 2, false);
                SetPunto(placa->display, 3, false);

                // Sondeo no bloqueante de F1 retenida por TIEMPO_PULSACION_MS
                if (DigitalInputRead(placa->f1)) {
                    if ((contador_ms - tiempo_inicio_f1) >= TIEMPO_PULSACION_MS) {
                        tiempo_ultima_actividad = contador_ms;
                        CambiarModo(MODO_MINUTOS, NULL);
                    }
                } else {
                    tiempo_inicio_f1 = contador_ms; 
                }
                break;

            case MODO_NORMAL:
                RelojGetHora(reloj, hora_actual); 
                DisplayWriteBCD(placa->display, hora_actual, 4);

                SetPunto(placa->display, 1, ((contador_ms % 1000) < 500)); 
                SetPunto(placa->display, 3, RelojAlarmaEstaActiva(reloj)); 

                // Transición 1: F1 retenida por TIEMPO_PULSACION_MS (Reconfigurar hora)
                if (DigitalInputRead(placa->f1)) {
                    if ((contador_ms - tiempo_inicio_f1) >= TIEMPO_PULSACION_MS) {
                        tiempo_ultima_actividad = contador_ms;
                        CambiarModo(MODO_MINUTOS, NULL); 
                    }
                } else {
                    tiempo_inicio_f1 = contador_ms; 
                }    
                
                // Transición 2: F2 retenida por TIEMPO_PULSACION_MS (Configurar Alarma)
                if (DigitalInputRead(placa->f2)) {
                    if ((contador_ms - tiempo_inicio_f2) >= TIEMPO_PULSACION_MS) {
                        tiempo_ultima_actividad = contador_ms;
                        CambiarModo(MODO_MINUTOS_ALARMA, hora_actual); // Carga la alarma al borrador internamente
                    }
                } else {
                    tiempo_inicio_f2 = contador_ms; 
                } 

                // Gestión sonora activa y eventos de Snooze/Silenciado
                if (alarma_sonando) {
                    DigitalOutputActivate(placa->buzzer); 

                    if (DigitalInputHasActivated(placa->accept)) {
                        RelojPosponerAlarma(reloj, 5);
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                    if (DigitalInputHasActivated(placa->cancel)) {
                        alarma_sonando = false;
                        DigitalOutputDeactivate(placa->buzzer);
                    }
                } else {
                    if (DigitalInputHasActivated(placa->accept)) {
                        RelojActivarAlarma(reloj, true);
                    }
                    if (DigitalInputHasActivated(placa->cancel)) {
                        RelojActivarAlarma(reloj, false);
                    }
                }
                break;

            case MODO_MINUTOS:
                DisplayWriteBCD(placa->display, hora_actual, 4);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    tiempo_ultima_actividad = contador_ms;
                    CambiarModo(MODO_HORAS, NULL);
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    tiempo_ultima_actividad = contador_ms;
                    IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    tiempo_ultima_actividad = contador_ms;
                    DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                break;

            case MODO_HORAS:
                DisplayWriteBCD(placa->display, hora_actual, 4);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    RelojSetHora(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    tiempo_ultima_actividad = contador_ms;
                    IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    tiempo_ultima_actividad = contador_ms;
                    DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                break;

            case MODO_MINUTOS_ALARMA:
                DisplayWriteBCD(placa->display, hora_actual, 4);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL, NULL); 
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    tiempo_ultima_actividad = contador_ms;
                    CambiarModo(MODO_HORAS_ALARMA, NULL); 
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    tiempo_ultima_actividad = contador_ms;
                    IncrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    tiempo_ultima_actividad = contador_ms;
                    DecrementarBCD(&hora_actual[2], LIMITE_MINUTOS);
                }
                break;

            case MODO_HORAS_ALARMA:
                DisplayWriteBCD(placa->display, hora_actual, 4);

                if (DigitalInputHasActivated(placa->cancel)) {
                    CambiarModo(MODO_NORMAL, NULL); 
                }
                if (DigitalInputHasActivated(placa->accept)) {
                    RelojSetAlarma(reloj, hora_actual);
                    CambiarModo(MODO_NORMAL, NULL);
                }
                if (DigitalInputHasActivated(placa->f4)) {
                    tiempo_ultima_actividad = contador_ms;
                    IncrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                if (DigitalInputHasActivated(placa->f3)) {
                    tiempo_ultima_actividad = contador_ms;
                    DecrementarBCD(&hora_actual[0], LIMITE_HORAS);
                }
                break;
        }
    }

    return 0;
}
/* === End of documentation ==================================================================== */