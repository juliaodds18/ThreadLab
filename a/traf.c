#include <stdbool.h>
#include <assert.h>
#include "crossing.h"
/* K fairness not implemented */
 
int K;
int num_vehicles;
int num_pedestrians;
 
int to_cross;
int has_crossed;
 
//Queue/Buffer structure.
typedef struct {
    sem_t **buf;
    int n;
    int front;
    int rear;
    int count;
    sem_t mutex;
    sem_t slots;
    sem_t items;    
}buf_t;
 
//Threadnrs for cleanup
pthread_t *pedestrian_tids;
pthread_t *vehicle_tids;
 
//Buffer functions
void buf_init(buf_t *sp, int n);
void buf_free(buf_t *sp);
void buf_push(buf_t *sp, sem_t *sem);
sem_t *buf_pop(buf_t *sp);
 
//Thread functions
void *pedestrians(void *arg);
void *vehicles(void *arg);
void *control();
 
//Buffers
buf_t traffic_v;
buf_t traffic_h;
 
//Intersection-Coordinator
pthread_t controller;
 
//Shared Semaphores.
sem_t iswaiting_mtx;
sem_t iscrossing_mtx;
sem_t order_mtx;
 
void init()
{
    sem_init(&iswaiting_mtx, 0, 0);    
    sem_init(&order_mtx, 0, 0);
 
    sem_init(&iscrossing_mtx, 0, 1);
   
    to_cross = 0;
    has_crossed = 0;
   
    pedestrian_tids = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_tids = malloc(sizeof(pthread_t) * num_vehicles);
   
    buf_init(&traffic_h, (num_pedestrians + num_pedestrians));
    buf_init(&traffic_v, (num_vehicles + num_pedestrians));
    Pthread_create(&controller, 0, control, 0);
}
 
void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_tids[arg->thread_nr], 0 , pedestrians, arg);
}
 
void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_tids[arg->thread_nr], 0 , vehicles, arg);
}
 
void *vehicles(void *arg)
{
    thread_info *info = arg;
    sem_t stop;
    sem_init(&stop, 0, 0);
     
    int place = vehicle_arrive(info);
    buf_push(&traffic_v, &stop);
    //Let the controller know that a vehicle wants to cross.            
    V(&iswaiting_mtx);
    //Block until controller gives green light.
    P(&stop);
    //Let the controller know that the pedestrian is crossing.
    V(&order_mtx);
    vehicle_drive(info);
    vehicle_leave(info);
    free(info);
    has_crossed++;
    if(has_crossed == to_cross)
    {
       
        V(&iscrossing_mtx);
        return NULL;
    }  
     
    P(&iswaiting_mtx);
    return NULL;
}
 
void *pedestrians(void *arg)
{
    thread_info *info = arg;
    sem_t stop;
    sem_init(&stop, 0, 0);
   
    int place = pedestrian_arrive(info);
   
    buf_push(&traffic_h, &stop);
 
    //Let the controller know that a pedestrian wants to cross.
    V(&iswaiting_mtx);
    //Block until controller gives green light
    P(&stop);
    //Let the controller know the pedestrian is crossing.
    V(&order_mtx);
 
    pedestrian_walk(info);
    pedestrian_leave(info);
    free(info);
    has_crossed++;
    //Let the controller know that everyone has crossed.
    if(has_crossed == to_cross)
    {
        V(&iscrossing_mtx);
        return NULL;
    }
   
    P(&iswaiting_mtx);
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
        P(&iswaiting_mtx);
        //catch vertical traffic.
        to_cross = traffic_v.count;
        has_crossed = 0;
 
        for(int i = 0; i < to_cross; i++ )
        {
            if(traffic_v.front != traffic_v.rear)
            {
                to_unlock = buf_pop(&traffic_v);
                V(to_unlock);                          
                P(&order_mtx);
                sem_trywait(&iscrossing_mtx);
            }  
           
        }  
        if(to_cross != 0)
        {        
            P(&iscrossing_mtx);
            if((cross_count + to_cross) == (num_vehicles + num_pedestrians))
            {
                break;
            }
            sem_trywait(&order_mtx);
            cross_count += to_cross;
        }
        //catch horizontal traffic.
        to_cross = traffic_h.count;
        has_crossed = 0;
        for(int i = 0; i < to_cross; i++ )
        {
            if(traffic_h.front != traffic_h.rear)
            {
                to_unlock = buf_pop(&traffic_h);
                V(to_unlock);
                P(&order_mtx);
                sem_trywait(&iscrossing_mtx);
            }        
        }        
        if(to_cross != 0)
        {
            P(&iscrossing_mtx);
            if((cross_count + to_cross) == (num_vehicles + num_pedestrians))
            {
                break;
            }
            sem_trywait(&order_mtx);
            cross_count += to_cross;
           
        }
    }
    return NULL;
}
/*Helper functions
*/
 
//Buffer functions,
void buf_init(buf_t *sp, int n)
{
    sp->buf = (sem_t **)Calloc(n, sizeof(sem_t *));
    sp->n = n;
    sp->count = 0;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
    sem_init(&sp->items, 0, 0);
   
}
void buf_free(buf_t *sp)
{
    free(sp->buf);
}
void buf_push(buf_t *sp, sem_t *item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->count++;
    sp->buf[(++sp->rear)%(sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
   
}
sem_t *buf_pop(buf_t *sp)
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
        Pthread_join(vehicle_tids[i], NULL);
    }
    for (int i = 0; i < num_pedestrians; i++)
    {
        Pthread_join(pedestrian_tids[i], NULL);
    }
   
    Pthread_join(controller, NULL);
    buf_free(&traffic_v);
    buf_free(&traffic_h);
       
}
