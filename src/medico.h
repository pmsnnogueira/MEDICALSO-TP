//
// Created by rafael on 03/11/21.
//

#ifndef TP_SO_MEDICO_H
#define TP_SO_MEDICO_H

#include "biblio.h"

typedef struct {
    pedido m;
    pthread_mutex_t *trinco;
} medico;

#endif //TP_SO_MEDICO_H
