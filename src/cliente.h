//
// Created by rafael on 03/11/21.
//

#ifndef TP_SO_CLIENTE_H
#define TP_SO_CLIENTE_H

#include "biblio.h"

typedef struct Cliente{
    int cliente;
    char nome[100];
    char especialidade[100];
    char sintomas[100];
    int prioridade;
    int n_fila;
    int n_especilistas;

}cliente;


#endif //TP_SO_CLIENTE_H
