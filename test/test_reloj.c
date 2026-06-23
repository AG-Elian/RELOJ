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

/*! 
 * @file test_reloj.c
 * @brief Pruebas unitarias para la biblioteca del reloj y alarma (TDD).
 * * Este archivo contiene la suite de pruebas escrita con Unity/Ceedling 
 * para verificar los requerimientos funcionales del reloj y despertador.
 */

#include "unity.h"
#include "reloj.h"

/* === Macros definitions ====================================================================== */

//! Cantidad de ticks que componen un segundo real según la configuración del hardware.
#define TICKS_PER_SECOND 100

//! Constante semántica para simular el paso de exactamente un segundo.
#define ONE_SECOND       TICKS_PER_SECOND

//! Constante semántica para simular el paso de diez segundos.
#define TEN_SECONDS      (10 * TICKS_PER_SECOND)

/* === Private variable definitions ============================================================ */

/*! 
 * @brief Hora inicial por defecto para las pruebas (12:34:56).
 * Representada como un arreglo BCD sin compactar.
 */
static hora_t HORA_INICIAL = {1, 2, 3, 4, 5, 6};

/*!
 * @brief Variable global exclusiva para el test de la alarma.
 * Bandera (flag) que permite verificar si la función de callback fue ejecutada.
 */
static bool alarma_sonando = false;

/* === Private function implementations ============================================================ */

/*!
 * @brief Función auxiliar para simular el paso del tiempo en el reloj.
 * @param[in] reloj Puntero al objeto reloj al que se le simularán los ticks.
 * @param[in] ticks Cantidad de iteraciones (ticks) a simular.
 */
static void SimulateTicks(clock_t reloj, unsigned int ticks) {
    for (unsigned int i = 0; i < ticks; ++i) {
        RelojTick(reloj); // Llamamos a la función que simula un tick del reloj
    }
}

/*!
 * @brief Función de callback simulada para los eventos de la alarma.
 * Cuando el reloj alcanza la hora de la alarma, ejecuta esta función
 * cambiando el estado de la variable global `alarma_sonando` a verdadero.
 */
void simular_evento_alarma(void) {
    alarma_sonando = true;
}

/* === Unity framework functions =============================================================== */

/*!
 * @brief Función de configuración inicial ejecutada antes de cada test.
 */
void setUp(void) {
    // Esta función se ejecuta antes de cada test. 
    // Por ahora la dejamos vacía.
}

/*!
 * @brief Función de limpieza ejecutada después de cada test.
 */
void tearDown(void) {
    // Esta función se ejecuta después de cada test.
}

/* === Test implementations ==================================================================== */

/*!
 * @brief Prueba: Inicialización del reloj.
 * Verifica que al crear el reloj, la hora se inicializa en 00:00:00 y
 * su estado se reporta como inválido hasta que sea ajustado por primera vez.
 */
void test_reloj_inicia_en_cero_y_estado_invalido(void) {
    // 1. Preparar: Definimos los datos que esperamos y las variables a usar
    hora_t hora_esperada = {0, 0, 0, 0, 0, 0};
    hora_t hora_actual = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Lleno de basura para asegurar que la función lo sobreescribe
    
    // Creamos el reloj (pasamos 100 ticks y NULL para el callback por ahora)
    clock_t reloj = RelojCreate(100, NULL);
    
    // 2. Ejecutar y 3. Comprobar:
    // Verificamos que GetHora devuelve false (hora inválida)
    TEST_ASSERT_FALSE(RelojGetHora(reloj, hora_actual));
    
    // Verificamos que el arreglo de la hora actual quedó en 00:00:00
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_esperada, hora_actual, 6);
}

/*!
 * @brief Prueba: Ajuste de hora.
 * Verifica que al ajustar el reloj con una hora específica, ésta se guarda
 * correctamente y el estado del reloj pasa a ser válido.
 */
void test_ajustar_hora_valida(void) {
    // 1. Preparar
    hora_t hora_esperada = {1, 2, 3, 4, 5, 6}; // Representa las 12:34:56
    hora_t hora_actual;
    clock_t reloj = RelojCreate(100, NULL);
    
    // 2. Ejecutar
    // Ajustamos la hora usando la función de nuestra API
    RelojSetHora(reloj, hora_esperada);
    
    // 3. Comprobar
    // Verificamos que GetHora ahora devuelve true (hora válida)
    TEST_ASSERT_TRUE(RelojGetHora(reloj, hora_actual));
    
    // Verificamos que el arreglo interno guardó exactamente la hora que le pasamos
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_esperada, hora_actual, 6);
}

/*!
 * @brief Prueba: Avance de un segundo.
 * Verifica que el reloj incremente su hora en exactamente un segundo
 * tras recibir la cantidad de ticks correspondientes a la frecuencia configurada.
 */
void test_avanza_un_seg(void){
    clock_t reloj;
    hora_t hora_actual;
    static const hora_t EXPECTED_TIME = {0, 0, 0, 0, 0, 1}; // Esperamos que avance a 00:00:01

    reloj = RelojCreate(TICKS_PER_SECOND, NULL); // Creamos el reloj con la cantidad de ticks por segundo definida
    SimulateTicks(reloj, ONE_SECOND); // Simulamos el avance de ticks para que el reloj avance un segundo
    RelojGetHora(reloj, hora_actual); // Obtenemos la hora actual del reloj

    TEST_ASSERT_EQUAL_UINT8_ARRAY(EXPECTED_TIME, hora_actual, 6); // Verificamos que la hora actual es la esperada
}

/*!
 * @brief Prueba: Avance de múltiples segundos.
 * Verifica que la lógica temporal sume correctamente intervalos mayores
 * (10 segundos) partiendo desde una hora preconfigurada.
 */
void test_avanza_diez_segundos(void){
    clock_t reloj;
    hora_t hora_actual;
    static const hora_t EXPECTED_TIME = {1, 2, 3, 5, 0, 6}; // Esperamos que avance a 12:35:06

    reloj = RelojCreate(TICKS_PER_SECOND, NULL); // Creamos el reloj con la cantidad de ticks por segundo definida
    (void)RelojSetHora(reloj, HORA_INICIAL); // Ajustamos la hora inicial a 12:34:56
    SimulateTicks(reloj, TEN_SECONDS); // Simulamos el avance de ticks para que el reloj avance diez segundos
    RelojGetHora(reloj, hora_actual); // Obtenemos la hora actual del reloj

    TEST_ASSERT_EQUAL_UINT8_ARRAY(EXPECTED_TIME, hora_actual, 6); // Verificamos que la hora actual es la esperada
}

/*!
 * @brief Prueba: Cambio de día (Límite matemático).
 * Verifica que al alcanzar las 23:59:59 y sumar un segundo adicional,
 * el reloj reinicie su contador correctamente a 00:00:00.
 */
void test_cambio_de_dia(void){
    clock_t reloj = RelojCreate(TICKS_PER_SECOND, NULL);
    hora_t hora_actual;
    
    // Configuramos la hora al límite del día (23:59:59)
    hora_t hora_limite = {2, 3, 5, 9, 5, 9};
    RelojSetHora(reloj, hora_limite);

    // Esperamos que al pasar un segundo, el reloj vuelva a 00:00:00
    hora_t hora_esperada = {0, 0, 0, 0, 0, 0}; 

    // Simulamos el paso de un segundo exacto
    SimulateTicks(reloj, ONE_SECOND);
    RelojGetHora(reloj, hora_actual);

    // Comprobamos
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_esperada, hora_actual, 6);
}

/*!
 * @brief Prueba: Configuración y lectura de alarma.
 * Verifica que sea posible almacenar una hora de alarma, leer ese mismo dato,
 * y que por defecto el sistema la mantenga inactiva.
 */
void test_configurar_y_consultar_alarma(void) {
    clock_t reloj = RelojCreate(TICKS_PER_SECOND, NULL);
    hora_t hora_alarma_configurar = {1, 2, 3, 0, 0, 0}; // Alarma a las 12:30:00
    hora_t hora_alarma_leida;

    // 1. Configuramos la alarma
    RelojSetAlarma(reloj, hora_alarma_configurar);

    // 2. Comprobamos que podemos leerla correctamente
    // Asumimos que GetAlarma devuelve true si hay una alarma configurada y false si no
    TEST_ASSERT_TRUE(RelojGetAlarma(reloj, hora_alarma_leida));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_alarma_configurar, hora_alarma_leida, 6);

    // 3. Comprobamos que por defecto no está activa hasta que se lo indiquemos
    TEST_ASSERT_FALSE(RelojAlarmaEstaActiva(reloj));
}

/*!
 * @brief Prueba: Disparo del evento de alarma.
 * Verifica que cuando la hora del reloj coincida exactamente con la hora
 * de la alarma configurada y activada, se ejecute la función callback provista.
 */
void test_alarma_suena_en_la_hora_configurada(void) {
    // 1. Preparar
    // Le entregamos nuestra función simulada al reloj
    clock_t reloj = RelojCreate(TICKS_PER_SECOND, simular_evento_alarma);
    
    hora_t hora_actual = {1, 2, 3, 4, 5, 0}; // 12:34:50
    hora_t hora_alarma = {1, 2, 3, 4, 5, 2}; // 12:34:52
    
    RelojSetHora(reloj, hora_actual);
    RelojSetAlarma(reloj, hora_alarma);
    RelojActivarAlarma(reloj, true); // Activamos la alarma
    
    alarma_sonando = false; // Nos aseguramos de que empiece en false

    // 2. Ejecutar y Comprobar
    // Simulamos 1 segundo. Ahora son 12:34:51. La alarma NO DEBE sonar
    SimulateTicks(reloj, ONE_SECOND);
    TEST_ASSERT_FALSE(alarma_sonando);
    
    // Simulamos otro segundo. Ahora son 12:34:52. La alarma DEBE sonar
    SimulateTicks(reloj, ONE_SECOND);
    TEST_ASSERT_TRUE(alarma_sonando);
}

/*!
 * @brief Prueba: Posponer alarma (Snooze).
 * Verifica que la función para posponer la alarma sume correctamente
 * una cantidad arbitraria de minutos a la hora actual de la alarma.
 */
void test_posponer_alarma(void) {
    clock_t reloj = RelojCreate(TICKS_PER_SECOND, NULL);
    hora_t hora_alarma_inicial = {1, 2, 3, 0, 0, 0}; // Alarma original a las 12:30:00
    hora_t hora_alarma_pospuesta = {1, 2, 3, 5, 0, 0}; // Esperamos que suene a las 12:35:00
    hora_t hora_leida;

    // Configuramos la alarma inicial
    RelojSetAlarma(reloj, hora_alarma_inicial);

    // Posponemos 5 minutos
    RelojPosponerAlarma(reloj, 5);

    // Leemos la nueva hora de la alarma y verificamos
    RelojGetAlarma(reloj, hora_leida);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_alarma_pospuesta, hora_leida, 6);
}