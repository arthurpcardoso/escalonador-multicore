#include <stdio.h>
#define TAMANHO_MAX 100

typedef struct {
    int itens[TAMANHO_MAX];
    int frente;
    int traseira;
} Fila;

// Inicializar a fila
void inicializarFila(Fila *fila) {
    fila->frente = 0;
    fila->traseira = -1;
}

// Verificar se a fila está cheia
int estaCheia(Fila *fila) {
    return fila->traseira == TAMANHO_MAX - 1;
}

// Verificar se a fila está vazia
int estaVazia(Fila *fila) {
    return fila->frente > fila->traseira;
}

// Inserir um elemento na fila
void enfileirar(Fila *fila, int valor) {
    if (estaCheia(fila)) {
        //printf("Erro: A fila está cheia!\n");
        return;
    }
    fila->itens[++fila->traseira] = valor;
}

// Remover um elemento da fila
int desenfileirar(Fila *fila) {
    if (estaVazia(fila)) {
        //printf("Erro: A fila está vazia!\n");
        return -1; // Código de erro
    }
    return fila->itens[fila->frente++];
}

// Exibir os elementos da fila
void exibirFila(Fila *fila) {
    if (estaVazia(fila)) {
        //printf("A fila está vazia.\n");
        return;
    }
    printf("Fila: ");
    for (int i = fila->frente; i <= fila->traseira; i++) {
        printf("%d ", fila->itens[i]);
    }
    printf("\n");
}