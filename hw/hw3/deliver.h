#ifndef DELIVER_H
#define DELIVER_H

#define DELIVERMAN_NUM 2

/*
 * define struct for deliverman
 *     id: deliverman id
 *     remaining_time: remaining time for delivery
 *     remaining_time_mutex: mutex for remaining_time, lock when updating
 *     delivery_mutex: mutex for delivery, lock when delivering
 */
typedef struct {
    int id;
    int remaining_time;
    pthread_mutex_t remaining_time_mutex;
    pthread_mutex_t delivery_mutex;
} Deliverman;

Deliverman deliverman[DELIVERMAN_NUM];

void initialize_deliverman();
int check_deliveryman(int delivery_time);
void update_deliverman_status(int candidate_deliveryman, int delivery_time);

// initialize deliverman's status
void initialize_deliverman() {
    for (int i = 0; i < DELIVERMAN_NUM; i++) {
        deliverman[i].id = i;
        deliverman[i].remaining_time = 0;
        pthread_mutex_init(&deliverman[i].remaining_time_mutex, NULL);
        pthread_mutex_init(&deliverman[i].delivery_mutex, NULL);
    }
}

// check which deliverman has less remaining time
int check_deliveryman(int delivery_time) {
    // assume only two deliverman
    return (deliverman[0].remaining_time <= deliverman[1].remaining_time) ? 0 : 1;
}

// update deliverman's remaining time (when starting delivery or finishing delivery)
void update_deliverman_status(int candidate_deliveryman, int delivery_time) {
    pthread_mutex_lock(&deliverman[candidate_deliveryman].remaining_time_mutex);
    if (delivery_time < 0 && deliverman[candidate_deliveryman].remaining_time < -delivery_time) {
        deliverman[candidate_deliveryman].remaining_time = 0;
    } else {
        deliverman[candidate_deliveryman].remaining_time += delivery_time;
    }
    pthread_mutex_unlock(&deliverman[candidate_deliveryman].remaining_time_mutex);
}

#endif // DELIVER_H