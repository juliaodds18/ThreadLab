#include <stdbool.h>
#include <assert.h>
#include "crossing.h"
 
int K;
int num_vehicles;
int num_pedestrians;

int pqueue_cnt;
int vqueue_cnt;
int pcrossed_cnt;
int vcrossed_cnt;
int prev_pq;
int prev_vq;
int p;
int v;
 
pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;
 
sem_t vmutex;
sem_t pmutex;
sem_t mutex;
 
void *pedestrians(void *arg);
void *vehicles(void *arg);

void init()
{
    sem_init(&vmutex, 0, 1);
    sem_init(&pmutex, 0, 1);
 
    pqueue_cnt = 0;
    vqueue_cnt = 0;
    pcrossed_cnt = 0;
    vcrossed_cnt = 0;
    prev_pq = 0;
    prev_vq = 0;    
    p = 0;
    v = 0;
 
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
}
 
void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0 , pedestrians, arg);
}
 
void spawn_vehicle(thread_info *arg)
{
    vqueue_cnt++;
    Pthread_create(&vehicle_thread[arg->thread_nr], 0 , vehicles, arg);
}
 
 
void *vehicles(void *arg)
{
    thread_info *info = arg;    
    int place = vehicle_arrive(info);    
    
    P(&vmutex);    
    v++;
    sem_trywait(&pmutex);
    if(prev_vq == 0)
    {
        prev_vq = vqueue_cnt;
    }
    if(v != prev_vq)
    {
        V(&vmutex);
    }
 
    vehicle_drive(info);
    vehicle_leave(info);
    vcrossed_cnt++;
    if(vcrossed_cnt == prev_vq)
    {
        vqueue_cnt -= vcrossed_cnt;
        v = prev_vq = 0;
        vcrossed_cnt = 0;
        V(&pmutex);        
        V(&vmutex);
       
    }
    return NULL;
}
 
void *pedestrians(void *arg)
{
    thread_info *info = arg;    
    int place = pedestrian_arrive(info);
    pqueue_cnt++;
    P(&pmutex);
    p++;
    sem_trywait(&vmutex);
    if(prev_pq == 0)
    {
        prev_pq = pqueue_cnt;
    }
    if(p != prev_pq)
    {
       V(&pmutex);
    }
 
    pedestrian_walk(info);
    pedestrian_leave(info);
    pcrossed_cnt++;
    if(pcrossed_cnt == prev_pq)
    {
        pqueue_cnt -= pcrossed_cnt;
        p = prev_pq = 0;
        pcrossed_cnt = 0;
        V(&vmutex);
        V(&pmutex);
    }
    return NULL;
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
}
