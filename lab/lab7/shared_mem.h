#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/time.h>

// shared memory key
#define SHM_KEY 65

// define result strings
#define SMALLER "smaller"
#define BIGGER "bigger"
#define CORRECT "bingo"

typedef struct {
    int guess;
    char result[8];
} data;

#endif // SHARED_MEM_H