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

#include <signal.h>

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
    bool esperando_sinal;
    int turnaround_time;
    int core;
}
Processo;

typedef struct {
    int pid_processo;
    int id_core;
    sem_t * id_semaforo;
    bool ocupado;
}
Core;

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

void quantum_acabou(Processo * processos, int qtd_processos, Core * cores, int n_cores);

void terminou_processo(int pid) {
    printf("Processo %d terminou.\n", pid);
    while (1);
}

void verifica_processo_terminou(Processo * processos, int tempo, int qtd_processos, Core * cores, int n_cores);

bool verifica_todos_processos_terminaram(Processo * processos, int qtd_processos);

void verifica_se_tem_cores_disponiveis(Core * cores, int n_cores, Processo * processos);

void delay1segundo() {
    long delay;
    for (delay = 0; delay < 600000000; delay++);
}

void espera_processos_esperarem_semafaro(Processo * processos, Core * cores, int n_cores);

void mandar_sinal(int pid);

void mandar_sinal_para_todos_cores(Core * cores, int n_cores);

void decrementar_tempo_restante(int signum);

void execucao_processo_filho(Processo * processos, int processo, Core * cores);

void print_tabela(Processo * processos, int qtd_processos) {
    printf("Tabela de processos\n");
    printf("ID | Arquivo | Tempo Inicio | Prioridade | Tempo Execucao | Tempo Restante | Tempo Final | PID | Esperando Sinal\n");
    for (int i = 1; i < qtd_processos + 1; i++) {
        printf("%d | %s | %d | %d | %d | %d | %d | %d | %d\n", processos[i].identificador, processos[i].arquivo_executavel, processos[i].tempo_inicio, processos[i].prioridade, processos[i].tempo_execucao, processos[i].tempo_restante, processos[i].tempo_final, processos[i].pid, processos[i].esperando_sinal);
    }
}

int main(int argc, char * argv[]) {
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

    * n_cores = atoi(argv[1]);
    quantum = atoi(argv[2]);

    // Cria o numero de cores passado por argumento
    Core * cores = malloc( * n_cores * sizeof(Core));
    if (cores == NULL) {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < * n_cores; i++) {
        cores[i].ocupado = false;
    }

    // criar um semaforo para cada core na memoria compartilhada
    for (int i = 0; i < * n_cores; i++) {
        sem_t * semaforo = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (semaforo == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        sem_init(semaforo, 1, 0);
        cores[i].id_semaforo = semaforo;
    }

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
            p.esperando_sinal = true;

            processos[p.identificador] = p;
        }
    }

    int tempo = 0;
    int tempo_quantum = 0;

    while (1) {
        printf("-----------------Tempo: %d-----------------------\n", tempo);

        mandar_sinal_para_todos_cores(cores, * n_cores);

        espera_processos_esperarem_semafaro(processos, cores, * n_cores);

        verifica_processo_nasceu(processos, tempo, num_linhas);

        printf("Verificando se os processos terminaram\n");
        verifica_processo_terminou(processos, tempo, num_linhas, cores, * n_cores);

        if (tempo_quantum == quantum) {
            quantum_acabou(processos, num_linhas, cores, * n_cores);
            tempo_quantum = 0;
        }

        verifica_se_tem_cores_disponiveis(cores, * n_cores, processos);

        if (verifica_todos_processos_terminaram(processos, num_linhas)) {
            exibirFila( & ordem_execucao);
            break;
        }

        delay1segundo();

        tempo++;
        tempo_quantum++;
    }

    if (total_processos > 0) {
        printf("Tempos de Turnaround dos processos:\n");
        print_tabela(processos, num_linhas);
        for (int i = 0; i < total_processos + 1; i++) {
            int processo_id = processos_finalizados[i];
            if (processo_id == 0) {
                continue;
            } else {
            printf("Processo %d: Tempo de Turnaround: %d\n", processos[processo_id].identificador, processos[processo_id].turnaround_time);
            }
        }
        printf("Tempo médio de turnaround: %.2f\n", (float) total_turnaround_time / total_processos);
    }

    return 0;
}

int verifica_processo_nasceu(Processo * processos, int tempo, int qtd_processos) {
    for (int i = 1; i < qtd_processos + 1; i++) {
        if (processos[i].identificador == 0) {
            printf("Processo %d não existe\n", i);
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

void quantum_acabou(Processo * processos, int qtd_processos, Core * cores, int n_cores) {
    printf("Quantum acabou\n");

    for (int i = 0; i < n_cores; i++) {
        if (cores[i].ocupado) {
            cores[i].ocupado = false;
            // verifica qual é o processo que está executando no core
            for (int j = 1; j < qtd_processos + 1; j++) {
                if (processos[j].pid == cores[i].pid_processo) {
                    // enfileira o processo que estava executando
                    if (processos[j].prioridade == 0) {
                        printf("Enfileirando processo %d na fila 0\n", j);
                        enfileirar( & fila_prioridade_0, j);
                    } else if (processos[j].prioridade == 1) {
                        printf("Enfileirando processo %d na fila 1\n", j);
                        enfileirar( & fila_prioridade_1, j);
                    } else if (processos[j].prioridade == 2) {
                        printf("Enfileirando processo %d na fila 2\n", j);
                        enfileirar( & fila_prioridade_2, j);
                    } else if (processos[j].prioridade == 3) {
                        printf("Enfileirando processo %d na fila 3\n", j);
                        enfileirar( & fila_prioridade_3, j);
                    }
                }
            }
        }
    }
    // print nas filas
    exibirFila( & fila_prioridade_0);
    exibirFila( & fila_prioridade_1);
    exibirFila( & fila_prioridade_2);
    exibirFila( & fila_prioridade_3);
}

void verifica_processo_terminou(Processo * processos, int tempo, int qtd_processos, Core * cores, int n_cores) {
    for (int i = 1; i < qtd_processos + 1; i++) {
        if (processos[i].tempo_restante == 0 && processos[i].tempo_final == 0) {
            for (int j = 0; j < n_cores; j++) {
                if (cores[j].pid_processo == processos[i].pid) {
                    cores[j].ocupado = false;
                    cores[j].pid_processo = 0;
                    printf("Core %d liberado pq o processo %d terminou\n", j, i);
                    break;
                }
            }
            printf("Processo %d terminou na verificação de todos\n", i);
            processos[i].liberado_executar = false;
            processos[i].tempo_final = tempo;
            processos[i].turnaround_time = processos[i].tempo_final - processos[i].tempo_inicio;
            turnaround_times[total_processos] = processos[i].turnaround_time;
            total_turnaround_time += processos[i].turnaround_time;
            processos_finalizados[total_processos] = i;
            total_processos++;
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
    int processo = -1;
    if (!estaVazia( & fila_prioridade_0)) {
        processo = desenfileirar( & fila_prioridade_0);
    } else if (!estaVazia( & fila_prioridade_1)) {
        processo = desenfileirar( & fila_prioridade_1);
    } else if (!estaVazia( & fila_prioridade_2)) {
        processo = desenfileirar( & fila_prioridade_2);
    } else if (!estaVazia( & fila_prioridade_3)) {
        processo = desenfileirar( & fila_prioridade_3);
    }

    if (processo == 0) {
        return prox_processo();
    }

    return processo;
}

void verifica_se_tem_cores_disponiveis(Core * cores, int n_cores, Processo * processos) {
    for (int i = 0; i < n_cores; i++) {
        if (!cores[i].ocupado) {
            printf("Core %d disponível\n", i);
            int processo = prox_processo();
            //if (processo == 0) processo = prox_processo();
            printf("Próximo Processo: %d\n", processo);
            if (processo == -1) {
                break;
            }
            if (processo != 0) {
                cores[i].ocupado = true;
                processos[processo].core = i;
                // verificar se o processo já foi criado
                if (processos[processo].pid == 0) {
                    int pid = fork();
                    if (pid == 0) {
                        execucao_processo_filho(processos, processo, cores);
                    } else {
                        processos[processo].pid = pid;
                        cores[i].pid_processo = pid;
                    }
                } else {
                    cores[i].pid_processo = processos[processo].pid;
                }

            }
        }
    }
}

void espera_processos_esperarem_semafaro(Processo * processos, Core * cores, int n_cores) {
    for (int i = 0; i < n_cores; i++) {
        if (cores[i].ocupado) {
            sem_wait(cores[i].id_semaforo);
        }
    }
}

// função para mandar sinal para os procesos que estão no core executarem
void mandar_sinal(int pid) {
    kill(pid, SIGCONT);
}

void mandar_sinal_para_todos_cores(Core * cores, int n_cores) {
    for (int i = 0; i < n_cores; i++) {
        if (cores[i].ocupado) {
            mandar_sinal(cores[i].pid_processo);
        }
    }
}

void decrementar_tempo_restante(int signum) {
    // This function will be called when the signal is received
}

void execucao_processo_filho(Processo * processos, int processo, Core * cores) {
    printf("Processo %d executando\n", processo);
    signal(SIGCONT, decrementar_tempo_restante);
    while (processos[processo].tempo_restante > 0) {
        pause();
        processos[processo].tempo_restante--;
        printf("Tempo restante do processo %d: %d\n", processo, processos[processo].tempo_restante);
        sem_post(cores[processos[processo].core].id_semaforo);
    }
    terminou_processo(processo);
}