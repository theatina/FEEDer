#ifndef SEMUN_H
#define SEMUN_H                 /* Prevent accidental double inclusion */

#include <sys/types.h>          /* For portability */
#include <sys/sem.h>

//sunartisi anagnwrisis feeder - reader
int whois(const char *whoami);

//check if N, M are digits
int isNumber(char number[]);

//shared Memory
struct shmData
{
    int shm_stoixeio;      		    //stoixeio apo feeder
    long long int shm_t;          	//timestamp

};

//semaphores
int set_sem(int sem_id, int sem_numb, int value);   //set timi tou simaforou me id sem_id - init
int delete_semset(int sem_id);           //diagrafi simaforou
int sem_down(int sem_id, int sem_numb);			  //down - P - wait function gia simaforo 
int sem_up(int sem_id, int sem_numb);				  //up - V - signal function gia simaforo
int sem_up_val(int semid, int sem_numb, int value); //up - V - signal (value times ) function gia simaforo
int sem_zero(int sem_id, int sem_numb);


#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
                ! defined(__sgi) && ! defined(__APPLE__)
                /* Some implementations already declare this union */

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

#endif

#endif