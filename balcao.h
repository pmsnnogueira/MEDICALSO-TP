//
// Created by rafael on 03/11/21.
//

#ifndef TP_SO_BALCAO_H
#define TP_SO_BALCAO_H

#include "biblio.h"

typedef struct{
    int continua;
    int maxclientes, maxmedicos;
    pedido p_cli[5];
    pedido p_med[5];
    int ite_cli , ite_med;//"iteradores"
    pthread_mutex_t *trinco; //mutex
    int tempo; //Tempo para mostrar a lista
}balcao;



#endif //TP_SO_BALCAO_H
