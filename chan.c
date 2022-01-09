#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#define BUFFSIZE 200

#define s1 0    
#define s2 1    
#define s3 2     
#define s4 3
#define s0 4
#define PERCENTAGE 5
int sem_create(int num_semaphores)
{  /* procedure to create specified number of semaphores */
   int semid;

   /* create new semaphore set of semaphores */
   if ((semid = semget (9253, num_semaphores,0600)) < 0)
     {  perror ("error in creating semaphore");/* 0600 = read/alter by user */
        exit (1);
     }
   return semid;
}

void sem_init(int semid, int index, int value)
{  /* procedure to initialize specified semaphore to given value */
   if (semctl (semid, index, SETVAL, value) < 0)
     {  perror ("error in initializing first semaphore");
        exit (1);
     }
}

void P(int semid, int index)
{/* procedure to perform a P or wait operation on a semaphore of given index */
  struct sembuf sops[1];  /* only one semaphore operation to be executed */

   sops[0].sem_num = index;/* define operation on semaphore with given index */
   sops[0].sem_op  = -1;   /* subtract 1 to value for P operation */
   sops[0].sem_flg = 0;    /* type "man semop" in shell window for details */

   if (semop (semid, sops, 1) == -1)
     {  perror ("error in semaphore operation");
        exit (1);
     }
}

void V(int semid, int index)
{/* procedure to perform a V or signal operation on semaphore of given index */
   struct sembuf sops[1];  /* define operation on semaphore with given index */

   sops[0].sem_num = index;/* define operation on semaphore with given index */
   sops[0].sem_op  = 1;    /* add 1 to value for V operation */
   sops[0].sem_flg = 0;    /* type "man semop" in shell window for details */

   if (semop (semid, sops, 1) == -1)
     {  perror ("error in semaphore operation");
        exit (1);
     }
}

void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main()
{
    pid_t pid;          /* variable to record process id of child */
    key_t key;          //Key for shared memory
    key = 9136;

    /* shared memory elements */
    int shmid;
    char *buffer;     /* logical base address for buffer */
    char *shm;
    char ins[50];
    char str[50];
    /* semaphore elements */
    int semid;       /* identifier for a semaphore set */
    /* local variables */
    int i = 0;
    int j = 0;
    time_t t;
    int perc = PERCENTAGE;
    
    srand(time(0)); 
    if ((shmid = shmget(key, 4 * BUFFSIZE * sizeof(char), 0666)) < 0)
        die("shmget");
 
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)
        die("shmat");

    semid = sem_create(5);

    buffer = shm; /* logical buffer starts at shared segment */
    while(1){
        printf("CHANNEL status: Waiting...\n");
        P(semid,s2); //Perimenei shma energopoihshs
        if(buffer[2*BUFFSIZE - 1] == '0'){ //An exei de3ia roh
            if(buffer[2*BUFFSIZE - 2] == '1'){
                printf("CHAN: Ending procedure\n");
                buffer[3*BUFFSIZE-2] = '1';
                V(semid,s3);
                exit(0);
            }
            i=BUFFSIZE;             
            j=0;
            while(buffer[i]!='\0'){
                str[j] = buffer[i]; //Diavase string
                i++;
                j++;
            }
            str[j] = '\0';
            printf("Message before deterioration from p1= %s\n",str);
            i=0;    
            while(str[i]!='\0'){
                if(rand()%100 <= perc-1){ //Alloiwpoihsh toy string mesw mia pithanothtas poy orizetai prin
                    str[i] = '*';       // gia kathe gramma toy string
                }
                buffer[2*BUFFSIZE+i] = str[i]; //Grafei to alloiwmeno mhnyma
                i++;
            }
            buffer[2*BUFFSIZE+i] = '\0';
            printf("Message after deterioration(with a chance of %d%%) from p1= %s\n",perc,buffer + 2*BUFFSIZE);

            i++;
            j=0;
            while(buffer[BUFFSIZE+j]!='\0'){
                j++;
            }
            j++;
            while(buffer[BUFFSIZE+j]!='\0'){ 
                buffer[2*BUFFSIZE+i] = buffer[BUFFSIZE+j]; // Metaferei to hash apo ta buffer
                j++;
                i++;
            }
            buffer[2*BUFFSIZE+i] = '\0';
            V(semid,s3); //3ekinaei thn enc2 diergasia.
        }
        else{
            if(buffer[3*BUFFSIZE - 2] == '1'){
                printf("CHAN: Ending procedure\n");
                buffer[2*BUFFSIZE-2] = '1';
                V(semid,s1);
                exit(0);
            }
            i=2*BUFFSIZE;             
            j=0;
            while(buffer[i]!='\0'){
                str[j] = buffer[i]; //Diavase string
                i++;
                j++;
            }
            
            str[j] = '\0';
            printf("Message before deterioration from p2= %s\n",str);
            i=0;
            while(str[i]!='\0'){
                if(rand()%100 <= perc-1){//Alloiwpoihsh toy string mesw mia pithanothtas poy orizetai prin
                   str[i] = '*';       // gia kathe gramma toy string
                }
                buffer[BUFFSIZE+i] = str[i]; //2 = str'
                i++;
            }
            buffer[BUFFSIZE+i] = '\0';
            printf("string after deterioration(with a chance of %d%%) from p2= %s\n",perc,buffer + BUFFSIZE);
            i++;
            j=0;
            while(buffer[2*BUFFSIZE+j]!='\0'){
                j++;
            }
            j++;
            while(buffer[2*BUFFSIZE+j]!='\0'){ 
                buffer[BUFFSIZE+i] = buffer[2*BUFFSIZE+j]; //Metafora twn hash
                j++;
                i++;
            }
            buffer[BUFFSIZE+i] = '\0';
            V(semid,s1);
        }
    }
    exit(0);
}