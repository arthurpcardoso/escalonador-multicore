#include "fila.h"

#include <stdbool.h>

#include <stdio.h>

#include <sys/types.h>

#include <sys/mman.h>

#include <fcntl.h>

#include <unistd.h>

#include <sys/shm.h>

#include <sys/ipc.h>

#include <semaphore.h>

#include <stdlib.h>

/*
    Alunos:
    - Arthur Pereira Cardoso 211038191
    - Arthur Rattes 200053523
    - José Barbosa 202014305
    - Victor Hugo Da Silva 190129794
*/

typedef struct {
    int identificador;
    char arquivo_executavel[50];
    int tempo_inicio;
    int prioridade;
    int tempo_execucao;
    int tempo_restante;
    int tempo_final;
    bool liberado_executar;
    int pid;
    bool esperando_semafaro;
    int turnaround_time;
}
Processo;

Fila fila_prioridade_0;
Fila fila_prioridade_1;
Fila fila_prioridade_2;
Fila fila_prioridade_3;

Fila ordem_execucao;

int prox_processo();
int turnaround_times[100];
int total_turnaround_time = 0;
int total_processos = 0;
int processos_finalizados[100];

int verifica_processo_nasceu(Processo * processos, int tempo, int qtd_processos);

void quantum_acabou(Processo * processos, int qtd_processos, int * n_cores);

void terminou_processo(int pid, int * n_cores) {
        printf("Processo %d terminou.\n", pid);
        while (1);
}

void verifica_processo_terminou(Processo * processos, int tempo, int qtd_processos, int * n_cores);

bool verifica_todos_processos_terminaram(Processo * processos, int qtd_processos);

int executa_processo(Processo * processos, int processo, int * n_cores, sem_t * semaforo_clock);

void verifica_se_tem_cores_disponiveis(int * n_cores, Processo * processos, sem_t * semaforo_clock);

void delay1segundo() {
    long delay;
    for (delay = 0; delay < 600000000; delay++);
}

void espera_processos_esperarem_semafaro(Processo * processos);

int main(int argc, char *argv[]) {
        Processo * processos = mmap(NULL, sizeof(Processo) * 100, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (processos == MAP_FAILED) {
                perror("mmap");
                return 1;
        }

        int * n_cores = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (n_cores == MAP_FAILED) {
                perror("mmap");
                return 1;
        }

        int quantum;
        if (argc != 3) {
                fprintf(stderr, "Uso: %s <numero_de_cores> <quantum>\n", argv[0]);
                return 1;
        }
        
        *n_cores = atoi(argv[1]);
        quantum = atoi(argv[2]);

        FILE * entrada = fopen("entrada.txt", "r");
        if (!entrada) {
                perror("Erro ao abrir o arquivo");
                return 1;
        }

        int num_linhas = 0;
        char linha[100];
        while (fgets(linha, sizeof(linha), entrada)) {
                num_linhas++;
        }

        rewind(entrada);

        printf("Número de linhas: %d\n", num_linhas);

        for (int i = 0; i < num_linhas; i++) {
                if (fgets(linha, sizeof(linha), entrada)) {
                        Processo p;
                        p.tempo_final = 0;

                        sscanf(linha, "%d %s %d %d", & p.identificador, p.arquivo_executavel, & p.tempo_inicio, & p.prioridade);
                        sscanf(p.arquivo_executavel, "%*[^0-9]%d", & p.tempo_execucao);

                        p.tempo_restante = p.tempo_execucao;
                        p.pid = 0;
                        p.esperando_semafaro = false;

                        processos[p.identificador] = p;
                }
        }

        int tempo = 0;
        int tempo_quantum = 0;

        sem_t * semaforo_clock = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (semaforo_clock == MAP_FAILED) {
                perror("mmap");
                return 1;
        }

        sem_t * semaforo_processos = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (semaforo_processos == MAP_FAILED) {
                perror("mmap");
                return 1;
        }

        sem_init(semaforo_clock, 1, 0);
        sem_init(semaforo_processos, 1, 1);

        while (1) {
                printf("-----------------Tempo: %d-----------------------\n", tempo);

                espera_processos_esperarem_semafaro(processos);

                verifica_processo_nasceu(processos, tempo, num_linhas);
                verifica_processo_terminou(processos, tempo, num_linhas, n_cores);

                if (tempo_quantum == quantum) {
                        quantum_acabou(processos, num_linhas, n_cores);
                        tempo_quantum = 0;
                }

                verifica_se_tem_cores_disponiveis(n_cores, processos, semaforo_clock);

                if (verifica_todos_processos_terminaram(processos, num_linhas)) {
                        exibirFila( & ordem_execucao);
                        break;
                }

                delay1segundo();

                tempo++;
                tempo_quantum++;

                sem_post(semaforo_clock);
        }

        if (total_processos > 0) {
        printf("Tempos de Turnaround dos processos:\n");
        for (int i = 0; i < total_processos + 1; i++) {
            int processo_id = processos_finalizados[i];
            printf("Processo %d: Tempo de Turnaround: %d\n", processos[processo_id].identificador, processos[processo_id].turnaround_time);
        }
        printf("Tempo médio de turnaround: %.2f\n", (float)total_turnaround_time / total_processos);
    }

    return 0;

}

int verifica_processo_nasceu(Processo * processos, int tempo, int qtd_processos) {
        for (int i = 1; i < qtd_processos + 1; i++) {
                if (processos[i].identificador == 0) {
                        continue;
                }
                if (processos[i].tempo_inicio == tempo) {
                        printf("Processo %d nasceu\n", processos[i].identificador);
                        printf("Prioridade: %d\n", processos[i].prioridade);
                        if (processos[i].prioridade == 0) {
                                enfileirar( & fila_prioridade_0, i);
                        } else if (processos[i].prioridade == 1) {
                                enfileirar( & fila_prioridade_1, i);
                        } else if (processos[i].prioridade == 2) {
                                enfileirar( & fila_prioridade_2, i);
                        } else if (processos[i].prioridade == 3) {
                                enfileirar( & fila_prioridade_3, i);
                        }
                }
        }
        return 0;
}

void quantum_acabou(Processo * processos, int qtd_processos, int * n_cores) {
        printf("Quantum acabou\n");

        for (int i = 1; i < qtd_processos + 1; i++) {
                if (processos[i].liberado_executar) {
                        processos[i].liberado_executar = false;
                        ( * n_cores) ++;
                        printf("Cores atualizados no quantum: %d\n", * n_cores);
                        if (processos[i].prioridade == 0) {
                                enfileirar( & fila_prioridade_0, i);
                        } else if (processos[i].prioridade == 1) {
                                enfileirar( & fila_prioridade_1, i);
                        } else if (processos[i].prioridade == 2) {
                                enfileirar( & fila_prioridade_2, i);
                        } else if (processos[i].prioridade == 3) {
                                enfileirar( & fila_prioridade_3, i);
                        }
                }
        }
}

void verifica_processo_terminou(Processo * processos, int tempo, int qtd_processos, int * n_cores) {
    for (int i = 1; i < qtd_processos + 1; i++) {
        if (processos[i].tempo_restante == 0 && processos[i].tempo_final == 0) {
            ( * n_cores) ++;
            printf("Cores atualizados pq o processo %d terminou: %d\n", i, * n_cores);
            processos[i].liberado_executar = false;
            processos[i].tempo_final = tempo;
            processos[i].tempo_final = tempo; 
            processos[i].turnaround_time = processos[i].tempo_final - processos[i].tempo_inicio; 
            turnaround_times[total_processos] = processos[i].turnaround_time; 
            total_turnaround_time += processos[i].turnaround_time; 
            total_processos++;
            processos_finalizados[total_processos] = i;
        }
    }
}

bool verifica_todos_processos_terminaram(Processo * processos, int qtd_processos) {
        for (int i = 1; i < qtd_processos + 1; i++) {
                if (processos[i].tempo_restante > 0) {
                        return false;
                }
        }
        return true;
}

int prox_processo() {
        if (!estaVazia( & fila_prioridade_0)) {
                return desenfileirar( & fila_prioridade_0);
        } else if (!estaVazia( & fila_prioridade_1)) {
                return desenfileirar( & fila_prioridade_1);
        } else if (!estaVazia( & fila_prioridade_2)) {
                return desenfileirar( & fila_prioridade_2);
        } else if (!estaVazia( & fila_prioridade_3)) {
                return desenfileirar( & fila_prioridade_3);
        } else {
                return -1;
        }
}

void verifica_se_tem_cores_disponiveis(int * n_cores, Processo * processos, sem_t * semaforo_clock) {
        if ( * n_cores > 0) {
                while ( * n_cores > 0) {
                        printf("Cores disponiveis: %d\n", * n_cores);
                        int processo = prox_processo();
                        printf("Prox Processo: %d\n", processo);
                        if (processo == -1) {
                                break;
                        }
                        if (processo != 0) {
                                ( * n_cores) --;
                                enfileirar( & ordem_execucao, processo);
                                int pid = executa_processo(processos, processo, n_cores, semaforo_clock);
                                if (pid == 0) {
                                        continue;
                                } else {
                                        continue;
                                }
                        }
                }
        }
}

int executa_processo(Processo * processos, int processo, int * n_cores, sem_t * semaforo_clock) {
        printf("Cores atualizados: %d\n", * n_cores);
        processos[processo].liberado_executar = true;
        if (processos[processo].pid != 0) {
                return processos[processo].pid;
        } else {
                int pid = fork();
                if (pid == 0) {
                        processos[processo].pid = getpid();
                        printf("Entrei no filho\n");
                        printf("Processo %d executando\n", processo);
                        while (processos[processo].tempo_restante > 0) {
                                if (processos[processo].liberado_executar) {
                                        if (processos[processo].tempo_restante == 0) {
                                                terminou_processo(processo, n_cores);
                                                return 0;
                                        }
                                        processos[processo].esperando_semafaro = true;
                                        sem_wait(semaforo_clock);
                                        processos[processo].tempo_restante--;
                                        printf("Tempo restante do processo %d: %d\n", processo, processos[processo].tempo_restante);
                                        processos[processo].esperando_semafaro = false;
                                } else {
                                        printf("Processo %d bloqueado\n", processo);
                                        printf("Cores atualizados: %d\n", * n_cores);
                                        while (!processos[processo].liberado_executar);
                                }
                        }
                        terminou_processo(processo, n_cores);
                        return 0;
                } else {
                        return pid;
                }
        }
}

void espera_processos_esperarem_semafaro(Processo * processos) {
        for (int i = 1; i < 100; i++) {
                if (processos[i].identificador == 0) {
                        continue;
                }
                else if (!processos[i].esperando_semafaro) {
                        continue;
                }
                else {
                        while (!processos[i].esperando_semafaro);
                }
        }
}
