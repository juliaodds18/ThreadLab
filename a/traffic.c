#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;
// How many pedestrian or vehicle wants to cross
int crossing;
// How many pedestrian or vehicle have crossed
int has_crossed;
// How many pedestrian or vehicle have crossed total
int total_crossed;

typedef struct {
    sem_t **buf;
    int front;
    int rear;
    int count;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

/* BUFFERS */
sbuf_t vehicle;
sbuf_t pedestrian;

void *vehicles (void* arg);
void *pedestrians(void *arg);

/* HELPER FUNCIONS */
void *control();
void sbuf_init(sbuf_t *sp);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, sem_t *sem);
sem_t *sbuf_remove(sbuf_t *sp);
void pedestrianCross();
void vehicleCross();
int checkIfDone();

/* SEMAPHORES */ 
// Lets the controll know i vehicle or pedestrian wants to cross
sem_t waiting_mutex;
// Lock when somtehing is crossing
sem_t crossing_mutex;
// Lets the controller know how is crossing 
sem_t control_mutex;
// unlock after removing from buffer 
sem_t *unlock;

/* THREADS */
pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;
pthread_t control_thread;

/* INIT SECTION */
void init()
{
    sem_init(&waiting_mutex, 0, 0);
    sem_init(&control_mutex, 0, 0);
    sem_init(&crossing_mutex, 0, 1);

    crossing = 0;
    has_crossed = 0;
    total_crossed = 0;

    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);

    sbuf_init(&pedestrian);
    sbuf_init(&vehicle);
    Pthread_create(&control_thread, 0, control, 0);
}
/* END OF INIT */

/* VEHICLE THREADS */
void *vehicles(void *arg)
{
    thread_info *info = arg;
    // semaphore to lock until the vehicle can cross
    sem_t wait;
    sem_init(&wait, 0, 0);

    int place = vehicle_arrive(info);

    sbuf_insert(&vehicle, &wait);
    V(&waiting_mutex);

    // locks until all the pedestrians have crossed
    P(&wait);
    V(&control_mutex);

    vehicle_drive(info);
    vehicle_leave(info);
    has_crossed++;

    // When everyone has crossed unlock the crossing mutex
    if (has_crossed == crossing) {
        V(&crossing_mutex);
        return NULL;}

P(&waiting_mutex);
    return NULL;
}
void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
}
/* END OF VEHICLE THREAD CODE */

/* PEDESTRIAN THREADS */
void *pedestrians(void *arg)
{
    thread_info *info = arg;
    // semaphore to lock until the pedestrian can cross
    sem_t wait;
    sem_init(&wait, 0, 0);

    int place = pedestrian_arrive(info);

    sbuf_insert(&pedestrian, &wait);

    V(&waiting_mutex);

    // locks until the vehicles have crossed
    P(&wait);
    V(&control_mutex);

    pedestrian_walk(info);
    pedestrian_leave(info);
    has_crossed++;

    // When everyone has crossed unlock the crossing mutex
    if (has_crossed == crossing) {
        V(&crossing_mutex);
        return NULL;
    }

    P(&waiting_mutex);
    return NULL;
}

void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
}
/* END OF VEHICLE THREAD CODE */

/* HELPER FUNCTIONS */

/* calls the other helper functions */
void *control()
{
    while(1)
    {
        P(&waiting_mutex);
        int ret;

        // Let pedestrians cross
        pedestrianCross();
        ret = checkIfDone();
        if(ret == 0)
            break;

        // Let vehicles cross
        vehicleCross();
        ret = checkIfDone();
        if(ret == 0)
            break;
    }
    return NULL;
}

/* Let the vehice cross */
void vehicleCross() {
    crossing = vehicle.count;
    has_crossed = 0;

    for (int i = 0; i < crossing; i++) {
        // Remove from the vehicle buffer
        if (vehicle.front != vehicle.rear) {
            // unlocks when vehicle have crossed
            unlock = sbuf_remove(&vehicle);
            V(unlock);
            P(&control_mutex);
            sem_trywait(&crossing_mutex);
        }
    }
}

/* Let the pedestrians cross */
void pedestrianCross() {
    // Catch horizontal traffic.
    crossing = pedestrian.count;
    has_crossed = 0;

   for (int i = 0; i < crossing; i++) {
        // Remove from the pedestrian buffer
        if(pedestrian.front != pedestrian.rear) {
            // unlocks when pedestrian have crossed
            unlock = sbuf_remove(&pedestrian);
            V(unlock);
            P(&control_mutex);
            sem_trywait(&crossing_mutex);
        }
    }
}
/* Create an empty, bounded, shared FIFO buffer with n slots */
void sbuf_init(sbuf_t *sp)
{
    sp->buf = (sem_t **)Calloc(K, sizeof(sem_t *));
    sp->count = 0;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, K);
    sem_init(&sp->items, 0, 0);
}

/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp)
{
    free(sp->buf);
}

/* Insert item onto the rear of shared buffer sp */
void sbuf_insert(sbuf_t *sp, sem_t *item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->count++;
    sp->buf[(++sp->rear)%(K)] = item;
    V(&sp->mutex);
    V(&sp->items);
}

/* Remove and return the first item from buffer sp */
sem_t *sbuf_remove(sbuf_t *sp)
{
    sem_t *top;
    P(&sp->items);
    P(&sp->mutex);
    sp->count--;
    top = sp->buf[(++sp->front)%(K)];
    V(&sp->mutex);
    V(&sp->slots);
    return top;
}

/* CLEAN UP SECTION */
void clean()
{
    for (int i = 0; i < num_vehicles; i++)
    {
        Pthread_join(vehicle_thread[i], NULL);
    }
    for (int i = 0; i < num_pedestrians; i++)
    {
        Pthread_join(pedestrian_thread[i], NULL);
    }

    Pthread_join(control_thread, NULL);
    sbuf_deinit(&vehicle);
    sbuf_deinit(&pedestrian);
}
/* END OF CLEAN UP */

/* Check if the traffic is over */
int checkIfDone() {
    if(crossing != 0) {
        P(&crossing_mutex);
        total_crossed += crossing;
        // If all vehicles and pedestrians have corossed return
        if(total_crossed == (num_vehicles + num_pedestrians)) {
            return 0;
        }
        sem_trywait(&control_mutex);

    }
    return 1;
}

