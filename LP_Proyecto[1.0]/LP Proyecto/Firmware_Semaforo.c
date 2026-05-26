#include "Firmware_Semaforo.h"


//Mira si el estado debe ser Cambiado a traves de un Switch imitando el diagrama de estados
enum EstadoSemaforo ChecarEstado(enum EstadoSemaforo estado,struct Configuracion configuracion,int trafico1,int trafico2){
    
    int trafico_sistema = trafico1 || trafico2;//Mira si hay trafico total
    static time_t tiempoX=(time_t)-1;
    if(tiempoX==(time_t)-1){
        tiempoX=time(NULL);
    }
    //Verifica si se debe cambiar el Estado
    switch(estado){
        case E1:
            if(!trafico_sistema && difftime(time(NULL),tiempoX)>configuracion.tiempo_para_estado_nocturno){//verifica condiciones para cambiar a Estado Nocturno
                tiempoX=time(NULL);
                RegistrarCambio(EN);
                return EN;
            }
            if(trafico2 && difftime(time(NULL),tiempoX)>configuracion.tiempo_verde){//Verifica condiciones para cambiar a Estado 2
                tiempoX=time(NULL);
                RegistrarCambio(E2);
                return E2;
            }
            break;
        case E2: 
            if(!trafico_sistema && difftime(time(NULL),tiempoX)>configuracion.tiempo_para_estado_nocturno){//verifica condiciones para cambiar a Estado Nocturno
                tiempoX=time(NULL);
                RegistrarCambio(EN);
                return EN;
            }
            if(trafico2 && difftime(time(NULL),tiempoX)>configuracion.tiempo_amarillo){//Veriifica condiciones para cambiar a Estado 3
                tiempoX=time(NULL);
                RegistrarCambio(E3);
                return E3;
            }
        break;
        case E3: 
            if(!trafico_sistema && difftime(time(NULL),tiempoX)>configuracion.tiempo_para_estado_nocturno){//verifica condiciones para cambiar a Estado Nocturno
                tiempoX=time(NULL);
                RegistrarCambio(EN);
                return EN;
            }
            if(trafico1 && difftime(time(NULL),tiempoX)>configuracion.tiempo_verde){//Verifica condiciones para cambiar a Estado 4
                tiempoX=time(NULL);
                RegistrarCambio(E4);
                return E4;
            }
        break;
        case E4: 
            if(!trafico_sistema && difftime(time(NULL),tiempoX)>configuracion.tiempo_para_estado_nocturno){//verifica condiciones para cambiar a Estado Nocturno
                tiempoX=time(NULL);
                RegistrarCambio(EN);
                return EN;
            }
            if(trafico1 && difftime(time(NULL),tiempoX)>configuracion.tiempo_amarillo){//Verifica condiciones para cambiar a Estado 2
                tiempoX=time(NULL);
                RegistrarCambio(E1);
                return E1;
            }
            break;
        case EN: 
            if(trafico1){
                tiempoX=time(NULL);
                RegistrarCambio(E1);
                return E1;
            }
            if(trafico2){
                tiempoX=time(NULL);
                RegistrarCambio(E3);
                return E3;
            }
        break;
    }
    
    //Retorna el mismo estado si no se debe hacer cambios
    return estado;
}


//Convierte el Estado a texto
char* textoEstado(enum EstadoSemaforo estado){
    switch(estado){
        case E1:
            return "Estado 1";
        break;
        case E2:
            return "Estado 2";
        break;
        case E3:
            return "Estado 3";
        break;
        case E4:
            return "Estado 4";
        break;
        case EN:
            return "Estado Nocturno";
        break;
    } 
}


//Registra el Cambio en el Log
void RegistrarCambio(enum EstadoSemaforo estado){
    char buffer[64]={0};
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);

    FILE *out = fopen("Log.txt", "a");
    if(!out) return;
    static int p = 0;
    if(p==0){
        fputs("------------------------------\n       Nueva Simulacion\n------------------------------\n",out);
        p=1;
    }

    strcat(buffer, "   ");
    strcat(buffer, textoEstado(estado));
    fputs(buffer, out);
    fputs("\n", out);
    fclose(out);

    printf("%s   ", buffer);
}
