#ifndef FILA_H
#define FILA_H

#include <stdio.h>

#define TAMANHO_MAX 100

// Definição da estrutura da fila
typedef struct {
    int itens[TAMANHO_MAX];
    int frente;
    int traseira;
} Fila;

// Funções para manipular a fila
void inicializarFila(Fila *fila);
int estaCheia(Fila *fila);
int estaVazia(Fila *fila);
void enfileirar(Fila *fila, int valor);
int desenfileirar(Fila *fila);
void exibirFila(Fila *fila);

#endif // FILA_H
