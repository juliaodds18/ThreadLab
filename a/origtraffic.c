#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;


void *vehicles (void* arg);
void *pedestrians(void *arg);

/*TODO: Tveir mismunandi lásar fyrir pedestrian og vehicle 
 * sameiginlegur counter uppí k
 * um leið og counter hittar 
 */
// vehicle_buf *vbuf;
// pedestrian_buf *pbuf;
pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;
sem_t vehicle_sem;
sem_t pedes_sem;
int vcounter;
int pcounter;
int remaining_vehicle;
int remaining_pedestrian;
int k;
sem_t w;
sem_t update;
sem_t remaining;
int wait_ped; 
int wait_veh;
int going; 

/* INIT SECTION */
void init()
{   
    // reader writer semaphore
    sem_init(&vehicle_sem, 0, 5);
    sem_init(&pedes_sem, 0, 5);
    sem_init(&w, 0, 1);
    sem_init(&remaining, 0, 3);
    sem_init(&update, 0, 1);
    remaining_vehicle = num_vehicles;
    remaining_pedestrian = num_pedestrians;
    vcounter = 0;
    pcounter = K;
    wait_ped = 0;
    wait_veh = 0;
    k = 5;
    going = 0;
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);  
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
    
    //Læsa vehicles, aflæsa pedestrians 

    // Get the argument pass via the void* 
    thread_info *info = arg;

    while(going != 0) 
        pthread_cond_wait();

    // Following are the needed calls to the 
    //functions in traffic.c 
    // You must call them in the proper order 
    //(see above).
    // Note that the calls can be moved to helper 
    // functions if needed..
    
    int place = vehicle_arrive(info);
    
    P(&update);
    wait_veh++;
    V(&update);

    sem_init(&vehicle_sem, 0, wait_veh);

    P(&vehicle_sem);
    if (wait_veh > 0) {
	vehicle_drive(info);
        vehicle_leave(info);
    }
    V(&vehicle_sem);

    P(&update);
    wait_veh--;
    V(&update);


    /*if(vcounter >= 5 || remaining_vehicle == 0) {
        P(&update);
        vcounter = 0;
        V(&update);
    }*/
    
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
    P(&w);
    pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
    V(&w);
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

    //Læsa pedestrians, aflæsa vehicles

    // Get the argument pass via the void*  
    thread_info *info = arg;

    // Each thread needs to call these functions 
    // in this order
    // Note that the calls can also be made in 
    //helper functions
    
    int place = pedestrian_arrive(info);
    P(&update);
    wait_ped++;
    going = wait_ped;
    V(&update);

    sem_init(&pedes_sem, 0, wait_ped);

    P(&pedes_sem);
    if (wait_ped > 0) {
        pedestrian_walk(info);
        pedestrian_leave(info);
    }
    V(&pedes_sem);

    P(&update);
    wait_ped--;
    going--;
    V(&update);

    /* if(pcounter >= 2 || remaining_pedestrian == 0) {
        P(&update);
	pcounter = 0;
        V(&update);
    } */  
    return NULL;
}
void spawn_pedestrian(thread_info *arg)
{
   /* YOU MUST IMPLEMENT THIS FUNCTION:
    * HERE YOU SHOULD CREAT A THREAD THAT 
    * MUST USE pedestrians(void *arg) AS 
    * THE FUNCTION TO EXECUTE
    */

    P(&w);
    pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
    V(&w);
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
