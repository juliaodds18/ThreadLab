#include <stdbool.h>
#include <assert.h>
#include "crossing.h"
 
int K;
int num_vehicles;
int num_pedestrians;
 
int to_cross;
int has_crossed;
 
// Buffer structure.
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
 
// Threads for cleanup
pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;
 
// Buffer functions
void sbuf_init(sbuf_t *sp, int n);
void sbuf_free(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, sem_t *sem);
sem_t *sbuf_remove(sbuf_t *sp);
 
//Thread functions
void *pedestrians(void *arg);
void *vehicles(void *arg);
void *control();
 
//Buffers
sbuf_t traffic_v;
sbuf_t traffic_h;
 
//Intersection-Coordinator
pthread_t controller;
 
//Shared Semaphores.
sem_t iswaiting_mutex;
sem_t iscrossing_mutex;
sem_t order_mutex;
 
void init()
{
    sem_init(&iswaiting_mutex, 0, 0);    
    sem_init(&order_mutex, 0, 0);
 
    sem_init(&iscrossing_mutex, 0, 1);
   
    to_cross = 0;
    has_crossed = 0;
   
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
   
    sbuf_init(&traffic_h, K);
    sbuf_init(&traffic_v, K);
    Pthread_create(&controller, 0, control, 0);
}
 
void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0 , pedestrians, arg);
}
 
void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_thread[arg->thread_nr], 0 , vehicles, arg);
}
 
void *vehicles(void *arg)
{
    thread_info *info = arg;
    sem_t stop;
    sem_init(&stop, 0, 0);
     
    int place = vehicle_arrive(info);
    sbuf_insert(&traffic_v, &stop);
    //Let the controller know that a vehicle wants to cross.            
    V(&iswaiting_mutex);
    //Block until controller gives green light.
    P(&stop);
    //Let the controller know that the pedestrian is crossing.
    V(&order_mutex);
    vehicle_drive(info);
    vehicle_leave(info);
    has_crossed++;
    if(has_crossed == to_cross)
    {
       
        V(&iscrossing_mutex);
        return NULL;
    }  
     
    P(&iswaiting_mutex);
    return NULL;
}
 
void *pedestrians(void *arg)
{
    thread_info *info = arg;
    sem_t stop;
    sem_init(&stop, 0, 0);
   
    int place = pedestrian_arrive(info);
   
    sbuf_insert(&traffic_h, &stop);
 
    //Let the controller know that a pedestrian wants to cross.
    V(&iswaiting_mutex);
    //Block until controller gives green light
    P(&stop);
    //Let the controller know the pedestrian is crossing.
    V(&order_mutex);
 
    pedestrian_walk(info);
    pedestrian_leave(info);
    has_crossed++;
    //Let the controller know that everyone has crossed.
    if(has_crossed == to_cross)
    {
        V(&iscrossing_mutex);
        return NULL;
    }
   
    P(&iswaiting_mutex);
    return NULL;
}
void *control()
{
    int cross_count = 0;
    to_cross = 0;
    sem_t *to_unlock;
    while(1)
    {
       
        //Wait & Block while loop.
        P(&iswaiting_mutex);
        //catch vertical traffic.
        to_cross = traffic_v.count;
        has_crossed = 0;
 
        for(int i = 0; i < to_cross; i++ )
        {
            if(traffic_v.front != traffic_v.rear)
            {
                to_unlock = sbuf_remove(&traffic_v);
                V(to_unlock);                          
                P(&order_mutex);
                sem_trywait(&iscrossing_mutex);
            }  
           
        }  
        if(to_cross != 0)
        {        
            P(&iscrossing_mutex);
            if((cross_count + to_cross) == (num_vehicles + num_pedestrians))
            {
                break;
            }
            sem_trywait(&order_mutex);
            cross_count += to_cross;
        }
        //catch horizontal traffic.
        to_cross = traffic_h.count;
        has_crossed = 0;
        for(int i = 0; i < to_cross; i++ )
        {
            if(traffic_h.front != traffic_h.rear)
            {
                to_unlock = sbuf_remove(&traffic_h);
                V(to_unlock);
                P(&order_mutex);
                sem_trywait(&iscrossing_mutex);
            }        
        }        
        if(to_cross != 0)
        {
            P(&iscrossing_mutex);
            if((cross_count + to_cross) == (num_vehicles + num_pedestrians))
            {
                break;
            }
            sem_trywait(&order_mutex);
            cross_count += to_cross;
           
        }
    }
    return NULL;
}
/*Helper functions
*/
 
//Buffer functions,
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
void sbuf_free(sbuf_t *sp)
{
    free(sp->buf);
}
void sbuf_insert(sbuf_t *sp, sem_t *item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->count++;
    sp->buf[(++sp->rear)%(sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
   
}
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
   
    Pthread_join(controller, NULL);
    sbuf_free(&traffic_v);
    sbuf_free(&traffic_h);
       
} 
