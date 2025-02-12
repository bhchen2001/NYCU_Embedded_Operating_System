#include "shared_mem.h"

data *shm_data;
int low_bound = 1, upper_bound;
pid_t game_pid;

void handler_timer (int sig) {
    static int mid;

    // check the result and adjust the bounds
    if (strcmp(shm_data->result, SMALLER) == 0) {
        upper_bound = shm_data->guess - 1;
    } 
    else if (strcmp(shm_data->result, BIGGER) == 0) {
        low_bound = shm_data->guess + 1;
    } 
    else if (strcmp(shm_data->result, CORRECT) == 0) {
        printf("[guess] Bingo! The answer is %d\n", shm_data->guess);
        exit(0);
    }

    mid = (low_bound + upper_bound) / 2;
    shm_data->guess = mid;
    
    printf("[guess] Guess %d\n", mid);

    if (kill(game_pid, SIGUSR1) == -1) {
        perror("kill");
    }

    // for next guess in 1 second
    // alarm(1);
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <key> <upper_bound> <pid>\n", argv[0]);
        return 1;
    }

    key_t key = atoi(argv[1]);
    upper_bound = atoi(argv[2]);
    game_pid = (pid_t) atoi(argv[3]);

    // attach shared memory
    int shmid = shmget(key, sizeof(data), 0666);
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

    // set signal handler for SIGALRM
    signal(SIGALRM, handler_timer);

    /*
     * set timer
     *     it_value: trigger SIGALRM 1 second later
     *     it_interval: trigger SIGALRM every 1 second
     */
    struct itimerval timer;
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    // alarm(1);

    while (1) {
        pause();
    }

    return 0;
}