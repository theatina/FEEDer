#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

#include "semun.h"

//semaphore numbers
#define readers 0       // mutex gia readers - feeder -- na min diavasoun oi readers prin oloklirwsei tin eggrafi o feeder
                        // kai eksasfalizei na min diavasei 2h fora kapoia diergasia to idio stoixeio
#define feeder 1        // molis teleiwsoun oi N diergasies ti fora, epitrepoun ston feeder na grapsei neo stoixeio
#define writewait 2     // mutex gia write sto arxeio

//main
int main(int argc, char* argv[])
{ 
    FILE *statfile;  //create text file with stats
    int N, M;
    int i;
    int pid;

	//take arguments  - N - M  -----------------------------------------------------------------
    if ( argc != 3 ) /* argc should be 3 for correct execution */
    {   /* argv[0] - programme name */
        printf( "ERROR!! Usage: %s < # of Processes >  < size of Array >\n", argv[0] ); return -2; }
    else if( !isNumber(argv[1]) || !isNumber(argv[2]))   //elegxos gia swsto input sto termatiko - command line
    {   printf( "ERROR!! Usage: %s < # of processes >  < size of array >\n", argv[0] ); return -2; }
    else 
    {   //we set the value of N (number of processes) & M (size of array)
        if ( (N=atoi(argv[1]))< 1 )  //oi diergasies prepei na nai toulaxiston 1
        {   printf("ERROR!! number of processes > 0\n");
            printf( "Usage: %s < # of processes >  < size of array >\n", argv[0] );
            return -2;
        }

        if ( (M=atoi(argv[2]))< 3001 )  //o pinakas prepei na nai toulaxiston 3001 se megethos
        {   printf("ERROR!! size of array > 3000\n");
            printf( "Usage: %s < # of processes >  < size of array >\n", argv[0] );
            return -2;
        }      
    }

    //create int array for both feeder and readers ---------------------------------------------
    char* pinakas;
    pinakas = malloc (M*sizeof(int));

    //creating shared memory  ------------------------------------------------------------------
    char c;
    int shmid;
    key_t key;
    struct shmData *shm, *s;
    struct shmData shmdata;

    if ((key = ftok("./semun.h", 'R')) == -1) //get the key
    {   perror("ftok"); exit(1); }

    if ((shmid = shmget(key, sizeof(shmdata) , IPC_CREAT | 0666)) < 0)   //allocate shared memory segment with size of struct shmdata
    {   shmctl(shmid,IPC_RMID,0); perror("Error - shared memory - shmget."); exit(EXIT_FAILURE); } 
    
    //semaphore set - create & initialize
    int semid3;    //sem id gia to set twn 3 simaforwn

    semid3 = semget(IPC_PRIVATE,3,0666|IPC_CREAT);
    if (semid3 == -1) 
    {   perror("Error in feeder sem - semget."); semctl(feeder,0,IPC_RMID); exit(EXIT_FAILURE); }
    
    if (set_sem(semid3, readers, 0) == -2)    //initial value 0 
    {   perror("Error in readers sem - set_sem"); semctl(readers,0,IPC_RMID); exit(EXIT_FAILURE); }
  
    if (set_sem(semid3,feeder,0) == -2)    //initial value 0 
    {   perror("Error in feeder sem - set_sem"); semctl(feeder,0,IPC_RMID); exit(EXIT_FAILURE); }

    if (set_sem(semid3, writewait, 1) == -2)  //initial value 1 
    {   perror("Error in writewait sem - set_sem"); semctl(writewait,0,IPC_RMID); exit(EXIT_FAILURE); }

    //creating processes -----------------------------------------------------------------------
    int status=0;
    int z,y,k;

    statfile = fopen("statsfile.txt", "w");    //anoigma arxeiou eggrafis

    printf("\nCreating %d processes\nSize of Array = %d\n\n", N, M);
    for(i=0;i<N;i++)
    {
        if ((pid = fork ()) < 0) 
        {   perror("Could not fork"); exit(1); }

        if(pid==0)  //reader -------------------------------------------------------------------
        {  
            //execlp("./child", "child", argv[2], i, semid3, NULL);  //prospathisa na kanw exec alla den ta katafera - kati phgaine strava me ta orismata
            //exit(status);
       
            whois("Reader");

            if ((shm = shmat(shmid, NULL, 0)) < 0)   //attach memory segment to the address space of the calling process
            {   shmctl(shmid,IPC_RMID,0); perror("Error - shared memory - shmat."); exit(EXIT_FAILURE); } 

            s = shm;

            //running average
            struct timespec ts;  //eidiki struct pou periexei tin wra se nsec
            long  ttemp1,ttemp2; /* timestamp in microsecond. */
            float meantime=0;  

            int z;
            for(z=0; z < M; z++)
            {   //readers semaphore down 
                sem_down(semid3, readers);
    
                //critical section twn readers ----- start
                //Read the shmem
                pinakas[z]= s->shm_stoixeio;
                //Get current time
                timespec_get(&ts, TIME_UTC);
                ttemp2 =(long)ts.tv_sec * 1000000000L + ts.tv_nsec;
                //timestamp sto shmem apo feeder
                ttemp1 = s->shm_t;
                //ananewsi running average
                meantime = meantime + (ttemp2-ttemp1)/M;  //tupos gia running average - (wikipedia)
                //critical section ----- end
                
                //wait for zero gia ton readers - block N-1 diergasies wste na mi diavasei panw apo 1 fora kapoia
                sem_zero(semid3, readers);   //H n-osti diergasia apeleutherwnei oles tis proigoumenes - telos anagnwsis 

                //up ton feeder - oles oi diergasies otan apeleutherwnontai apo tin teleutaia
                //kanoun ton feeder up apo mia fora wste i n-osti pou tha ton kanei up 
                //na ton kseblockarei , auto dilwnei oti oles oi diergasies exoun diavasei
                //tou epitrepei na grapsei
                sem_up(semid3, feeder);
            }

            sem_down(semid3, writewait); //mutex metaksi twn diergasiwn gia eggrafi sto arxeio statsfile.txt
            //critical section ----- start       
            fprintf(statfile, "\n\n-----> Pid = %d\n\nRunning Average: %.4f microsec\n\nArray:", getpid(), meantime/1000);

            for(z=0; z < M; z++)
            {   int temppp;
                temppp = z % 30;

                //sxediastiki leptomereia
                if(temppp==0)
                {   fprintf(statfile, "\n"); } 

                fprintf(statfile, "%d  ", pinakas[z]);
            }   
            printf("\n\n-----> Pid = %d\n\nRunning Average: %.4f microsec\n\n", getpid(), meantime/1000 );

            printf("Reader: %d - Pid: %d - Exiting..\n", i, getpid());

            sem_up(semid3, writewait);  //critical section ----- end

            //detach memory segment
            if (shmdt(shm) == -1) 
            {   fprintf(stderr, "shmdt failed\n"); exit(EXIT_FAILURE); }

            exit(status);  //child process exits   
        }                         
    }

    //feeder ---------------------------------------------------------------------
   
        whois("Feeder");

        //memory attach ston feeder
        if ((shm = shmat(shmid, NULL, 0)) < 0)   //attach memory segment to the address space of the calling process
        {   shmctl(shmid,IPC_RMID,0); perror("Error - shared memory - shmat."); exit(EXIT_FAILURE); } 

        s = shm;

        time_t t;
        // Intializes random number generator 
        srand((unsigned) time(&t));

        for(z=0;z<M;z++)
            pinakas[z]= rand(); //tuxaio gemisma pinaka sto feeder
                    
        struct timespec ts;
        long timestamp_nsec; /* timestamp in nanoseconds. */

        for(y=0; y<M; y++)
        {                   
            //cs ----- start
            //eggrafi sti mnimi - stoixeio + timestamp (CS)
            s->shm_stoixeio=pinakas[y];
            //current time in nanoseconds
            timespec_get(&ts, TIME_UTC);
            timestamp_nsec = (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
            //apothikeysi timestamp metaforas tou int apo to feeder, sti shmem
            s->shm_t=timestamp_nsec;
            //cs ------ end

            //signal all the readers oti mporoun na ksekinisoun to diavasma  - telos writing       
            sem_up_val(semid3, readers, N);

            //feeder semaphore down N times wste i teleutaia n- osti diergasia na ton ksipnisi efoson exoun diavasei oles
            for(k=0; k<N; k++)
                sem_down(semid3, feeder);    
        }                   

        //wait for the N children have finished                 
        for (i=0; i < N; i++)
            wait(NULL);
        
        fclose(statfile); //kleisimo arxeiou afou grapsoun kai oi N diergasies

        printf("\n\nAll reader processes have ended\n");
        printf("Feeder process can now end.\n");

    //taktopoihsh domwn, mnimis, simaforwn -----------------------------------------------------
    printf("\n---Taktopoihsh memory---\n");

    //free memory - pinaka
    free(pinakas);

    //detach shmem - delete segment
    if (shmdt(shm) == -1) 
    {   fprintf(stderr, "shmdt failed\n"); exit(EXIT_FAILURE); }

    if(shmctl(shmid,IPC_RMID,0) == -1)
    {   perror("Failed To Delete Shared Memory"); exit(EXIT_FAILURE); }

    //delete semaphore set
    if (semctl(semid3, 0, IPC_RMID, 0) < 0)
    {   perror("Error - semaphores - delete_semset."); exit(EXIT_FAILURE); }

    printf("H mnimi apodesmeuthike.\n\n"); 

    return 0;
}
