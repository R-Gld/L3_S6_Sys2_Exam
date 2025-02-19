#define _DEFAULT_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>  // pour sigsuspend et la gestion des signaux

#define MIN_C 0
#define MAX_C 1

#define MAX_SEQUENCE_SIZE 32

long convertWithCheck(const char *arg);
int gen_int_rand(int min, int max);
void ch_gen(int* pt_ch, int size);
void ch_print(const int* pt_ch, int size);
void calc_dec(const int* pt_ch, int size, int* pt_val);

struct shared_data {
    /* Partie séquence */
    int sequence[MAX_SEQUENCE_SIZE];
    size_t sequence_size;
    int l;

    /* Résultats */
    int parent_res;
    int child_res;
};

/* Variable globale pour indiquer la réception du signal */
volatile sig_atomic_t sigusr1_received = 0;

/* Gestionnaire de SIGUSR1 : il se contente de signaler la réception */
void handle_sigusr1(int _not_used) {
    sigusr1_received = 1;
}

int main(const int argc, char ** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const int n = (int) convertWithCheck(argv[1]);
    if (n < 2 || n > 31) {
        fprintf(stderr, "n should be in [2; %d].\n", MAX_SEQUENCE_SIZE-1);
        return EXIT_FAILURE;
    }

    srandom(time(NULL) ^ getpid());
    const int l = gen_int_rand(1, n-1);

    const key_t key = ftok("./", 'h');
    if (key == -1) { perror("ftok"); return EXIT_FAILURE; }

    const int shmID = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0644);
    if (shmID == -1) { perror("shmget"); return EXIT_FAILURE; }

    struct shared_data *data = shmat(shmID, NULL, 0);
    if (data == (void *) -1) { perror("shmat"); return EXIT_FAILURE; }

    if (shmctl(shmID, IPC_RMID, NULL) == -1) { perror("shmctl"); return EXIT_FAILURE; }

    data->child_res = 0;
    data->parent_res = 0;
    data->sequence_size = n;
    data->l = l;

    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    sigset_t block_mask, old_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block_mask, &old_mask) == -1) { perror("sigprocmask"); return EXIT_FAILURE; }

    const pid_t pid = fork();
    if (pid == -1) { perror("fork"); return EXIT_FAILURE; }

    if (pid > 0) {
        /* Processus parent */
        ch_gen(data->sequence, l);
        kill(pid, SIGUSR1);
        while (!sigusr1_received)
            sigsuspend(&old_mask);
        sigusr1_received = 0;

        calc_dec(&data->sequence[l], n-l, &data->parent_res);

        int status;
        if (waitpid(pid, &status, 0) == -1) { perror("waitpid"); return EXIT_FAILURE; }
        if (WIFEXITED(status)) {
            const int final_res = data->child_res * (int) pow(2, n-l) + data->parent_res;
            if (shmdt(data) == -1) { perror("shmdt"); return EXIT_FAILURE; }
            printf("Final result: %d\n", final_res);
        }
        return EXIT_SUCCESS;
    }

    if (pid == 0) {
        /* Processus child */
        ch_gen(&data->sequence[l], n-l);
        kill(getppid(), SIGUSR1);
        while (!sigusr1_received)
            sigsuspend(&old_mask);
        sigusr1_received = 0;

        calc_dec(data->sequence, l, &data->child_res);

        if (shmdt(data) == -1) { perror("shmdt"); return EXIT_FAILURE; }
        return EXIT_SUCCESS;
    }
}

long convertWithCheck(const char *arg) {
    char *endPointer;
    const long result = strtol(arg, &endPointer, 10);
    if ((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN)) ||
        (errno != 0 && result == 0)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }
    if (endPointer == arg) {
        fprintf(stderr, "No Digits were found\n");
        exit(EXIT_FAILURE);
    }
    if (*endPointer != '\0') {
        fprintf(stderr, "Further characters after number: %s\n", endPointer);
        exit(EXIT_FAILURE);
    }
    return result;
}

/**
 * Tire et renvoie un entier pseudo-aléatoire entre min et max bornes incluses.
 */
int gen_int_rand(int min, int max) {
    return (int) (random()/(INT_MAX+1.0)*(max-min+1)+min);
}

/**
 * Génère aléatoirement les chiffres binaires.
 */
void ch_gen(int* pt_ch, int size) {
    srandom(time(NULL)+getpid());  // pour génération aléatoire des séquences
    for (int i = 0; i < size; ++i) {
        pt_ch[i] = gen_int_rand(MIN_C, MAX_C);
    }
}

/**
 * Affiche les chiffres binaires d'un tableau.
 */
void ch_print(const int* pt_ch, int size) {
    for (int i = 0; i < size; i++)
        printf("%d", pt_ch[i]);
}

/**
 * Calcule la valeur décimale d'une suite de chiffres binaires donnés dans un tableau.
 */
void calc_dec(const int* pt_ch, int size, int* pt_val) {
    int power = 1;
    *pt_val = pt_ch[size-1] * power;
    for (int i = size-2; i >= 0; i--) {
        power *= 2;
        *pt_val += power * pt_ch[i];
    }
}
