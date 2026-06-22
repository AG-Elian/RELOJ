#include "unity.h"
#include "reloj.h"

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