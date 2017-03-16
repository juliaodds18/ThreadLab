#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;
int to_cross;
int has_crossed;
int cross_count;

typedef struct {
    sem_t **buf;
    int n;
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
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, sem_t *sem);
sem_t *sbuf_remove(sbuf_t *sp);

/* SEMAPHORES */
sem_t waiting_mutex;
sem_t crossing_mutex;
sem_t order_mutex;
sem_t *to_unlock;

/* THREADS */ 
pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;
pthread_t control_thread;

/* INIT SECTION */
void init()
{
    sem_init(&waiting_mutex, 0, 0);
    sem_init(&order_mutex, 0, 0);
    sem_init(&crossing_mutex, 0, 1);

    to_cross = 0;
    has_crossed = 0;
    cross_count = 0;

    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);

    sbuf_init(&pedestrian, K);
    sbuf_init(&vehicle, K);
    Pthread_create(&control_thread, 0, control, 0);
}
/* END OF INIT */

/* VEHICLE THREADS */
void *vehicles(void *arg)                       
{
    thread_info *info = arg;
    sem_t stop;
    sem_init(&stop, 0, 0);

    int place = vehicle_arrive(info);
    
    sbuf_insert(&vehicle, &stop);
        //Let the controller know that a vehicle wants to cross.            
    V(&waiting_mutex);
    
    //Block until controller gives green light.
    P(&stop);
    //Let the controller know that the pedestrian is crossing.
    V(&order_mutex);

    vehicle_drive(info);
    vehicle_leave(info);
    has_crossed++;
    // If everyone has crossed the road, unlock the crossing mutex
    if (has_crossed == to_cross) {
        V(&crossing_mutex);
        return NULL;
    }

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
    sem_t stop;
    sem_init(&stop, 0, 0);

    int place = pedestrian_arrive(info);

    
    sbuf_insert(&pedestrian, &stop);
    //Let the controller know that a pedestrian wants to cross.
    V(&waiting_mutex);
    
    //Block until controller gives green light
    P(&stop);
    //Let the controller know the pedestrian is crossing.
    V(&order_mutex);

    pedestrian_walk(info);
    pedestrian_leave(info);
    has_crossed++;

    //Let the controller know that everyone has crossed.
    if (has_crossed == to_cross) {
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
void *control()
{
    while(1)
    {
        //Wait & Block while loop.
        P(&waiting_mutex);
        //catch vertical traffic.
        to_cross = vehicle.count;
        has_crossed = 0;

        for (int i = 0; i < to_cross; i++) {
            // If there is something inside of the vehicle buffer, remove it.
            if (vehicle.front != vehicle.rear) {
                to_unlock = sbuf_remove(&vehicle);
                V(to_unlock);
                P(&order_mutex);
                sem_trywait(&crossing_mutex);
            }

        }

        if (to_cross != 0){
            P(&crossing_mutex);
            //Check if there are any vehicles or pedestrians left. If not, stop.
            if ((cross_count + to_cross) == (num_vehicles + num_pedestrians)) {
                break;
            }
            sem_trywait(&order_mutex);
            cross_count += to_cross;
        }

        //catch horizontal traffic.
        to_cross = pedestrian.count;
        has_crossed = 0;

       for (int i = 0; i < to_cross; i++) {
            // If there is something inside of the pedestrian buffer, remove it. 
            if(pedestrian.front != pedestrian.rear) {
                to_unlock = sbuf_remove(&pedestrian);
                V(to_unlock);
                P(&order_mutex);
                sem_trywait(&crossing_mutex);
            }
        }

        if (to_cross != 0) {
            P(&crossing_mutex);
            // Check if there are any vehicles or pedestrians left. If not, stop. 
            if ((cross_count + to_cross) == (num_vehicles + num_pedestrians)) {
                break;
            }
            sem_trywait(&order_mutex);
            cross_count += to_cross;
        }
    }

    return NULL;
}

/* Create an empty, bounded, shared FIFO buffer with n slots */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = (sem_t **)Calloc(n, sizeof(sem_t *));
    sp->n = n;
    sp->count = 0;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
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
    sp->buf[(++sp->rear)%(sp->n)] = item;
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
    top = sp->buf[(++sp->front)%(sp->n)];
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
