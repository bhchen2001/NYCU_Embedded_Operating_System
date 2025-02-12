#include "shared_mem.h"

data *shm_data;
int answer = 0, shmid;

void handle_sigint (int sig) {
    // detach shared memory
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
    } else {
        printf("Shared memory detached\n");
    }

    // remove shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    } else {
        printf("Shared memory removed\n");
    }

    exit(0);
}

void handle_sigusr1 (int sig, siginfo_t *siginfo, void *context) {
    // get guess process's PID
    int guess_pid = siginfo->si_pid;

    // print guess
    printf("[game] Guess %d, ", shm_data->guess);

    // compare guess with answer
    if (shm_data->guess > answer) {
        strcpy(shm_data->result, SMALLER);
        printf("%s\n", SMALLER);
    } 
    else if (shm_data->guess < answer) {
        strcpy(shm_data->result, BIGGER);
        printf("%s\n", BIGGER);
    } 
    else {
        strcpy(shm_data->result, CORRECT);
        printf("%s\n", CORRECT);
    }
}

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <key> <guess>\n", argv[0]);
        return 1;
    }

    // handle SIGINT
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("initial signal");
        return 1;
    }
    
    key_t key = atoi(argv[1]);
    answer = atoi(argv[2]);

    // create shared memory
    shmid = shmget(key, sizeof(data), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    // attach shared memory
    shm_data = (data *) shmat(shmid, NULL, 0);
    if (shm_data == (data *) -1) {
        perror("shmat");
        return 1;
    }

    // initialize shared memory
    memset(shm_data, 0, sizeof(data));

    // handle SIGUSR1
    struct sigaction sa;
    sa.sa_handler = (void *) handle_sigusr1;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // print PID and wait
    printf("[game] Game PID: %d\n", getpid());
    while (1) {
        pause();
    }

    return 0;
}