#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;


void *vehicles (void* arg);
void *pedestrians(void *arg);


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
int wait_pedestrian;
int wait_vehicle;
int veh;
sem_t w;
sem_t update;
sem_t remaining;

/* INIT SECTION */
void init()
{   
    // reader writer semaphore
    sem_init(&vehicle_sem, 0, num_vehicles);
    sem_init(&pedes_sem, 0, num_pedestrians);
    sem_init(&w, 0, 1);
    sem_init(&remaining, 0, 1);
    sem_init(&update, 0, 1);
    remaining_vehicle = num_vehicles;
    remaining_pedestrian = num_pedestrians;
    veh = 0;
    //pcounter = 1;
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

    // Get the argument pass via the void* 
    thread_info *info = arg;
    // Following are the needed calls to the 
    //functions in traffic.c 
    // You must call them in the proper order 
    //(see above).
    // Note that the calls can be moved to helper 
    // functions if needed..
    int place = vehicle_arrive(info);
    
    P(&remaining);
    wait_vehicle++;
    V(&remaining);
	//printf("never go in here");
	//fflush(stdout);
    //sem_init(&w, 0, wait_vehicle);
    //P(&w);
    //vehicle_drive(info);
    //vehicle_leave(info);
    //V(&w);
    //wait_vehicle = 0;
    if(wait_vehicle > 0) {
        if(veh == 1) {
            P(&vehicle_sem);
            vehicle_drive(info);
            vehicle_leave(info);
            P(&remaining);
            wait_vehicle--;
            V(&remaining);
            V(&vehicle_sem);
        }
        if(wait_vehicle == 0) {
             P(&w);
             veh = 0;
             V(&w);
        }
    }
    //P(&w);
       //V(&w);
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
    //P(&w);
    //remaining_vehicle--;
    pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
   // V(&w);
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
    
    //P(&w);
    if(veh == 1) {
    P(&remaining);
    wait_pedestrian++;
    V(&remaining);
    }
    printf("wait er: %d\n", wait_pedestrian);
    //sem_init(&pedes_sem, 0, wait_pedestrian);
    //P(&w);
    if(wait_pedestrian > 0) {
        if(veh == 0) {
            P(&pedes_sem);
            pedestrian_walk(info);
    	    pedestrian_leave(info);
	    P(&remaining);
   	    wait_pedestrian--;
    	    V(&remaining);
	    V(&pedes_sem);
        }
	if(wait_pedestrian == 0) {
	     P(&w);
	     veh = 1;
	     V(&w);
	}
    }
    
    //V(&pedes_sem);
    //V(&w);
    //wait_pedestrian = 0;
    
   // V(&w);
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
  //  P(&w);
    pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
    //V(&w);
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
