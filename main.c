#include <stdio.h>
#include "fila.h"

typedef struct {
    int identificador;
    char arquivo_executavel[50];
    int tempo_inicio;
    int prioridade;
    int tempo_execucao;
    int tempo_final;
} Processo;

Fila fila_prioridade_0;
Fila fila_prioridade_1;
Fila fila_prioridade_2;
Fila fila_prioridade_3;

int prox_processo();

int main() {

    // Entrada do n_cores e quantum
    // formato: escalona <numero_de_cores> <quantum>

    int n_cores, quantum;
    char comando[10];

    // tabela de processos
    Processo processos[100];

     // Ler o comando completo
    scanf("%s %d %d", comando, &n_cores, &quantum);

    // le o arquivo entrada.txt
    FILE *entrada = fopen("entrada.txt", "r");

    // le as linhas do arquivo
    char linha[100];
    while (fgets(linha, 100, entrada) != NULL) {

        Processo p;
        p.tempo_final = 0;

        sscanf(linha, "%d %s %d %d", &p.identificador, p.arquivo_executavel, &p.tempo_inicio, &p.prioridade);

        int tempo = 0;

        sscanf(p.arquivo_executavel, "%*[^0-9]%d", &p.tempo_execucao);

        // coloca os p em processos
        processos[p.identificador] = p;
    }

    // execução do escalonador
    while (1)
    {
        // verifica se algum processo chegou
        //...

        // verifica se algum processo terminou
        //...

        // verifica se o quantum acabou
        //...

        // verifica se todos os processos terminaram
        //...

        // dela de 1 segundo
        long delay; 
        for (delay=0; delay<600000000; delay++);
        break;
    }
    
    

    return 0;
}

int prox_processo() {
    // retorna o proximo processo a ser executado

    if (!estaVazia(&fila_prioridade_0)) {
        return desenfileirar(&fila_prioridade_0);
    } else if (!estaVazia(&fila_prioridade_1)) {
        return desenfileirar(&fila_prioridade_1);
    } else if (!estaVazia(&fila_prioridade_2)) {
        return desenfileirar(&fila_prioridade_2);
    } else if (!estaVazia(&fila_prioridade_3)) {
        return desenfileirar(&fila_prioridade_3);
    }
    else {
        return -1;
    }
}