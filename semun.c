////////////////////////////////
//  
//  Christina - Theano Kylafi
//
///////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>

#include "semun.h"

//sunartisi anagnwrisis feeder - reader
int whois(const char *whoami)
{   
    printf("%s here. Pid: %d. Ppid: %d\n", whoami, getpid(), getppid() );
    return 1;
}

//check if N, M are digits
int isNumber(char number[])
{   
    int i;
    for (i=0 ; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

//semaphore functions
int set_sem(int sem_id, int sem_numb, int value)   //set value - initialize semaphore to "value"
{   
    union semun sem_union;
    sem_union.val = value;
    if (semctl(sem_id, sem_numb, SETVAL, sem_union) < 0 )
    {
        fprintf(stderr, "Failed to create semaphore set\n");
        return -2;
    }
    return 1;
}

int delete_semset(int sem_id)  //diagrafi simaforou
{  
    if (semctl(sem_id, 0, IPC_RMID, 0) < 0)
    {   
        fprintf(stderr, "Failed to delete semaphore set\n");
        return -2;
    }
    return 1;
}

int sem_up_val(int semid, int sem_numb, int value)
{
    struct sembuf sem_b;
    sem_b.sem_num = sem_numb;
    sem_b.sem_op = value;         /* increments the sempahore by a certain value */
    sem_b.sem_flg = 0;
    if (semop(semid, &sem_b, 1) < 0) 
    {  
        fprintf(stderr, "sem_down failed\n");
        return -2;
    }
    return 1;
}

int sem_down(int semid, int sem_numb)
{
    struct sembuf sem_b;
    sem_b.sem_num = sem_numb;
    sem_b.sem_op = -1;         /* down - decrements semaphore's value*/
    sem_b.sem_flg = 0;
    if (semop(semid, &sem_b, 1) < 0) {
        fprintf(stderr, "sem_down failed\n");
        return -2;
    }
    return 1;
}

int sem_up(int sem_id, int sem_numb)
{   
    struct sembuf sem_b;
    sem_b.sem_num = sem_numb;
    sem_b.sem_op = 1;          /* up - increments semaphore's value */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) < 0) 
    {
        fprintf(stderr, "sem_up failed\n");
        return -2;
    }
    return 1;
}

int sem_zero(int sem_id, int sem_numb)
{	
    struct sembuf sem_b;
    sem_b.sem_num = sem_numb;
    sem_b.sem_op = 0;          /* wait for zero value, until then, the sempahore blocks the calling process */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) < 0) 
    {
        fprintf(stderr, "sem_up failed\n");
        return -2;
    }
    return 1;
}


