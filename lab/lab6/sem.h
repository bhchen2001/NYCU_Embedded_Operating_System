#ifndef SEM_H
#define SEM_H

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define SEM_PATH "./sem.h"
#define SEM_KEY 's'

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int sem_init();
int sem_wait();
int sem_signal();
int sem_destroy();

int sem_init() {
    int semid;
    union semun arg;
    key_t key = ftok(SEM_PATH, SEM_KEY);
    if (key == -1) {
        perror("ftok");
        return -1;
    }

    semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return -1;
    }
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl");
        return -1;
    }
    return semid;
}

int sem_wait(int semid) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1) {
        perror("semop");
        return -1;
    }
    return 0;
}

int sem_signal(int semid) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1) {
        perror("semop");
        return -1;
    }
    return 0;
}

int sem_destroy(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        return -1;
    }
    return 0;
}

#endif