#include <stdio.h>
#include <time.h>
#include <string.h>
#pragma once


enum EstadoSemaforo{
    E1,E2,E3,E4,EN
};

struct Configuracion
{
    float tiempo_verde;
    float tiempo_amarillo;
    float tiempo_para_estado_nocturno;
};

enum EstadoSemaforo ChecarEstado(enum EstadoSemaforo estado,struct Configuracion a,int trafico1,int trafico2);

char* textoEstado(enum EstadoSemaforo estado);

void RegistrarCambio(enum EstadoSemaforo estado);
