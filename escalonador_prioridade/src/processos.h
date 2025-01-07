#ifndef PROCESSOS_H
#define PROCESSOS_H

typedef struct {
    int id;
    char arquivo[256];
    int tempo_inicio;
    int prioridade;
    int tempo_execucao;
    int tempo_termino;
    int tempo_espera;
} Processo;

int ler_processos(const char *nome_arquivo, Processo processos[]);
void executar_processos();
void adicionar_processos(Processo processos[], int num_processos);

#endif