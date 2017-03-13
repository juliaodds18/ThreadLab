#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

void *vehicles (void* arg);
void *pedestrians(void *arg);
void vbuf_init();
void pbuf_init();

struct vehicle_buf {
    int  *buf;         /* Buffer array */         
    int   n;           /* Maximum number of slots */
    int   front;       /* buf[(front+1)%n] is first item */
    int   rear;        /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
};

struct pedestrian_buf {
    int  *buf;         /* Buffer array */
    int   n;           /* Maximum number of slots */
    int   front;       /* buf[(front+1)%n] is first item */
    int   rear;        /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
}; 

struct vehicle_buf vbuf;
struct pedestrian_buf pbuf;

//typedef struct {
  //  vehicle_buf vbuf;
  //  pedestrian_buf pbuf;     
    pthread_t *pedestrian_thread;
    pthread_t *vehicle_thread;
//} simulator;

/* INIT SECTION */
void init()
{   
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);  
    vbuf_init();
    pbuf_init(); 
}

void vbuf_init()
{
    &vbuf->buf = Calloc(K, sizeof(int)); 
    &vbuf->n = K;                  /* Buffer holds max of n items */
    &vbuf->front = &vbuf->rear = 0;   /* Empty buffer iff front == rear */
    Sem_init(&vbuf->mutex, 0, 1); /* Binary semaphore for locking */
    Sem_init(&vbuf->slots, 0, K); /* Initially, buf has n empty slots */
    Sem_init(&vbuf->items, 0, 0); /* Initially, buf has zero items */
 
}

void pbuf_init()
{
    pbuf->buf = Calloc(K, sizeof(int));
    pbuf->n = K;                  /* Buffer holds max of n items */
    pbuf->front = pbuf->rear = 0;   /* Empty buffer iff front == rear */
    Sem_init(&pbuf->mutex, 0, 1); /* Binary semaphore for locking */
    Sem_init(&pbuf->slots, 0, K); /* Initially, buf has n empty slots */
    Sem_init(&pbuf->items, 0, 0); /* Initially, buf has zero items */
 
}


/**** END OF INIT *******************************/

/* VEHICLE THREADS ******************************/
/* YOU MUST IMPLEMENT THE FUNCTIONALITY BELOW   */
/* THE SPAWNING AND RUNNING OF A VEHICLE        */
void *vehicles(void *arg)                       
{
    /* YOU MAY CHANGE THIS FUNCTION: 
     * THIS IS THE main-function OF VEHICLES 
     * THREADS AND THEY MUST CALL FUNCTIONS
     * IN THE traffic.c CODE, IN ORDER
     * FIRST:   vehicle_arrive()
     * SECOND:  vehicle_drive()
     * LAST:    vehicle_leave()
     * SEE CODE BELOW..
     */

    // Get the argument pass via the void* 
    thread_info *info = arg;

    // Following are the needed calls to the 
    //functions in traffic.c 
    // You must call them in the proper order 
    //(see above).
    // Note that the calls can be moved to helper 
    // functions if needed..
    int place = vehicle_arrive(info);
    P(&vbuf->mutex);
    vehicle_drive(info);
    vehicle_leave(info);
    V(&vbuf->mutex);

    // end of the threads main function 
    // time to die?
    return NULL;
}
void spawn_vehicle(thread_info *arg)
{
    /* YOU MUST IMPLEMENT THIS FUNCTION:
     * HERE YOU SHOULD CREAT A THREAD THAT 
     * MUST USE vehicles(void *arg) AS THE 
     * FUNCTION TO EXECUTE
     */
    pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);

}
/**** END OF VEHICLE THREAD CODE ****************/

/* PEDESTRIAN THREADS ***************************/
/* YOU MUST IMPLEMENT THE FUNCTIONALITY BELOW   */
/* THE SPAWNING AND RUNNING OF A PEDESTRIANS    */
void *pedestrians(void *arg)
{
    /* YOU MAY CHANGE THIS FUNCTION: 
     * THIS IS THE main-function OF PEDESTRIANS 
     * THREADS AND THEY MUST CALL FUNCTIONS
     * IN THE traffic.c CODE, IN ORDER
     * FIRST:   pedestrian_arrive()
     * SECOND:  pedestrian_walk()
     * LAST:    pedestrian_leave()
     * SEE CODE BELOW..
     */

    // Get the argument pass via the void*  
    thread_info *info = arg;

    // Each thread needs to call these functions 
    // in this order
    // Note that the calls can also be made in 
    //helper functions
    int place = pedestrian_arrive(info);
    P(&pbuf->mutex);
    pedestrian_walk(info);
    pedestrian_leave(info);
    V(&pbuf->mutex);

    return NULL;
}
void spawn_pedestrian(thread_info *arg)
{
   /* YOU MUST IMPLEMENT THIS FUNCTION:
     * HERE YOU SHOULD CREAT A THREAD THAT 
     * MUST USE pedestrians(void *arg) AS 
     * THE FUNCTION TO EXECUTE
     */
    pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);

}
/**** END OF VEHICLE THREAD CODE ****************/

/* CLEAN UP SECTION *****************************/
void clean()
{
    /********************************************/
    /* YOU MUST IMPLEMENT THIS FUNCTION         */
    /* HERE YOU DO ANY NECCESSARY CLEAN UP      */
    /********************************************/
    // has to free ??? 
    for (int i = 0; i < num_pedestrians; i++) {
	Pthread_join(pedestrian_thread[i], NULL);
    }
    for (int i = 0; i < num_vehicles; i++) {
        Pthread_join(vehicle_thread[i], NULL);
    }
}
/**** END OF CLEAN UP ***************************/
