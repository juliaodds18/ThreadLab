#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

/* INIT SECTION *********************************/
void init()
{
    /********************************************/
    /* YOU MUST IMPLEMENT THIS FUNCTION         */
    /* HERE YOU COULD CREATE THE LOCKS ETC.     */
    /********************************************/ 
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
    vehicle_drive(info);
    vehicle_leave(info);

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
    pedestrian_walk(info);
    pedestrian_leave(info);

    return NULL;
}
void spawn_pedestrian(thread_info *arg)
{
   /* YOU MUST IMPLEMENT THIS FUNCTION:
     * HERE YOU SHOULD CREAT A THREAD THAT 
     * MUST USE pedestrians(void *arg) AS 
     * THE FUNCTION TO EXECUTE
     */
}
/**** END OF VEHICLE THREAD CODE ****************/

/* CLEAN UP SECTION *****************************/
void clean()
{
    /********************************************/
    /* YOU MUST IMPLEMENT THIS FUNCTION         */
    /* HERE YOU DO ANY NECCESSARY CLEAN UP      */
    /********************************************/ 
}
/**** END OF CLEAN UP ***************************/
