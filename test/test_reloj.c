#include "unity.h"
#include "reloj.h"

//Al inicializar, el reloj está en 00:00 y con una hora inválida.



void test_reloj_inicial_invalido(void){

    clock_t reloj;
    hora_t hora_actual;
    bool es_valido;
    reloj=RelojCreate(1,NULL);
    es_valido=RelojGetHora(reloj,hora_actual);
    TEST_ASSERT_FALSE(es_valido);
    TEST_ASSERT_EQUAL_UINT8_ARRAY((hora_t){0,0,0,0,0,0}, hora_actual, 6);

}