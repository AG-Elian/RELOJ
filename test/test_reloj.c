#include "unity.h"
#include "reloj.h"

#define TICKS_PER_SECOND 100
#define ONE_SECOND       TICKS_PER_SECOND

//Función auxiliar para simular el avance de ticks en el reloj
static void SimulateTicks(clock_t reloj, unsigned int ticks) {
    for (unsigned int i = 0; i < ticks; ++i) {
        RelojTick(reloj); // Llamamos a la función que simula un tick del reloj
    }
}

void setUp(void) {
    // Esta función se ejecuta antes de cada test. 
    // Por ahora la dejamos vacía.
}

void tearDown(void) {
    // Esta función se ejecuta después de cada test.
}

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

void test_avanza_un_seg(void){
    clock_t reloj;
    hora_t hora_actual;
    static const hora_t EXPECTED_TIME = {0, 0, 0, 0, 0, 1}; // Esperamos que avance a 00:00:01

    reloj = RelojCreate(TICKS_PER_SECOND, NULL); // Creamos el reloj con la cantidad de ticks por segundo definida
    SimulateTicks(reloj, ONE_SECOND); // Simulamos el avance de ticks para que el reloj avance un segundo
    RelojGetHora(reloj, hora_actual); // Obtenemos la hora actual del reloj

    TEST_ASSERT_EQUAL_UINT8_ARRAY(EXPECTED_TIME, hora_actual, 6); // Verificamos que la hora actual es la esperada
}