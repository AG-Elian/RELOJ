#include "unity.h"
#include "reloj.h"

void setUp(void) {
    // Esta función se ejecuta antes de cada test
}

void tearDown(void) {
    // Esta función se ejecuta después de cada test
}

void test_reloj_inicia_en_cero_y_estado_invalido(void) {
    uint8_t hora_esperada[] = {0, 0, 0, 0, 0, 0};
    uint8_t hora_actual[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Lleno de basura para asegurar que la función lo sobreescribe
    
    clock_t reloj = reloj_crear(100); // 100 ticks por segundo
    
    TEST_ASSERT_FALSE(reloj_hora_es_valida(reloj));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(hora_esperada, reloj_obtener_hora(reloj, hora_actual), 6);
}