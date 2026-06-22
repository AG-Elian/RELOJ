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