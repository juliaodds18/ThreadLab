#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

void *vehicles (void* arg);
void *pedestrians(void *arg);

pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;

sem_t mutex;

/* INIT SECTION */
void init()
{
    // reader writer semaphore
    sem_init(&mutex, 0, 1);

    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
}
/**** END OF INIT *******************************/

/* VEHICLE THREADS */
void *vehicles(void *arg)
{
    thread_info *info = arg;
    int place = vehicle_arrive(info);
    
    P(&mutex);
    vehicle_drive(info);
    vehicle_leave(info);
    V(&mutex);

    return NULL;
}

void spawn_vehicle(thread_info *arg)
{
    pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
}
/**** END OF VEHICLE THREAD CODE ****************/

/* PEDESTRIAN THREADS ***************************/
void *pedestrians(void *arg)
{
    thread_info *info = arg;
    int place = pedestrian_arrive(info);

    P(&mutex);
    pedestrian_walk(info);
    pedestrian_leave(info);
    V(&mutex);

    return NULL;
}

void spawn_pedestrian(thread_info *arg)
{
    pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
}
/* END OF PEDESTRIAN THREAD CODE */

/* CLEAN UP SECTION */
void clean()
{
    for (int i = 0; i < num_pedestrians; i++) {
        Pthread_join(pedestrian_thread[i], NULL);
    }
    for (int i = 0; i < num_vehicles; i++) {
        Pthread_join(vehicle_thread[i], NULL);
    }
}
