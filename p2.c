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
#include <openssl/md5.h>
#define BUFFSIZE 200
void die(char *s)
{
    perror(s);
    exit(1);
}

#define s1 0    
#define s2 1   
#define s3 2       
#define s4 3
#define s0 4
int sem_create(int num_semaphores)
{  /* procedure to create specified number of semaphores */
   int semid;

   /* create new semaphore set of semaphores */
   if ((semid = semget (9253, num_semaphores, 0600)) < 0)
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


int main (void){
  pid_t pid;          /* variable to record process id of child */
  key_t key;          //Key for shared memory
  key = 9136;

  /* shared memory elements */
  int shmid;
  char *buffer;     /* logical base address for buffer */
  char *shm;
  char ins[50];
  char str[50];
  char sumch[50];
  char asd[50];
  char hash[MD5_DIGEST_LENGTH];
  /* semaphore elements */
  int semid;       /* identifier for a semaphore set */
  /* local variables */
  int i = 0;
  int j = 0;


  if ((shmid = shmget((key_t)key,4 * BUFFSIZE * sizeof(char), 0666)) < 0)
    die("shmget");

  if ((shm = shmat(shmid, NULL, 0)) ==   (char *) -1)
    die("shmat");

  /* set up pointers to appropriate places in shared memory segment */
  buffer = shm; /* logical buffer starts at shared segment */
  /* create and initialize semaphores */
  semid = sem_create(5);

  if (-1 == (pid = fork())){ /* check for error in spawning child process */
    perror ("error in fork");
    exit (1);
  }
  if (0 == pid){ //enc2
    while(1){
      printf("ENC2 status: Waiting...\n");
      P(semid,s3); //Perimenei shma energopoihshs
      if(buffer[3*BUFFSIZE - 1]=='0'){ //de3ia roh
        if(buffer[3*BUFFSIZE - 2] == '1'){
          printf("ENC2: Ending procedure\n");
          buffer[4*BUFFSIZE-2] = '1';
          V(semid,s4);
          exit(0);
        }
        i=2*BUFFSIZE;
        j=0;
        while(buffer[i]!='\0'){
          str[j] = buffer[i]; // Diavazei to string'
          i++;
          j++;
        }
        str[j] = '\0';
        memset(asd,'\0',sizeof(asd));
        i=0;
        while(str[i]!='\0'){
          asd[i] = str[i];
          i++;
        }
        MD5(asd, sizeof(asd),hash); //Eyresh toy hash
        i = 0;
        while(buffer[2*BUFFSIZE + i]!='\0'){
          i++;
        }
        i++;
        j=0;
        while(buffer[2*BUFFSIZE + i]!='\0'){
          sumch[j] = buffer[2*BUFFSIZE+i]; // Proigoymeno hash
          i++;
          j++;
        }
        sumch[j] = '\0';
        printf("\nStarting kai final hashes are:  %s %s",sumch,hash);

        if(strcmp(sumch,hash) != 0 ){ //An diaferoyn tote 3anatrexei thn enc1
          printf("\nHashes are different so redoing enc1\n");
          printf("\n");
          V(semid,s1);
        }
        else{
          printf("\nHashes are identical/ pass message to p2\n");
          i=0;
          while(str[i]!='\0'){
            buffer[3*BUFFSIZE+i] = str[i]; //Grafei to string gia na ektypwthei
            i++;
          }
          printf("\n");
          buffer[3*BUFFSIZE+i] = '\0';
          V(semid,s4); //Arxizei thn p2
        }
      }
      else{ //aristerh roh
        if(buffer[4*BUFFSIZE - 2] == '1'){
          printf("ENC2: Ending procedure\n");
          buffer[3*BUFFSIZE-2] = '1';
          V(semid,s2);
          exit(0);
        } 
        i=3*BUFFSIZE;
        j=0;
        while(buffer[i]!='\0'){
          str[j] = buffer[i]; //Diabase to input toy prwtoy buffer
          i++;
          j++;
        }
        str[j] = '\0';
        memset(asd,'\0',sizeof(asd));
        i=0;
        while(str[i]!='\0'){
          asd[i] = str[i];
          i++; 
        }
        MD5(asd, sizeof(asd),hash); //Algorithmos md5 gia thn eyresh toy hash
        i=0;
        while(buffer[3*BUFFSIZE+i]!='\0'){
          buffer[2*BUFFSIZE+i] = buffer[3*BUFFSIZE + i];
          i++;
        }
        buffer[2*BUFFSIZE+i] = '\0';
        i++;
        j=0;
        while(hash[j]!='\0'){
          buffer[2*BUFFSIZE+i] = hash[j]; //Grafei to hash dipla apo to string
          i++;
          j++;
        }
        buffer[2*BUFFSIZE+i] = '\0';
        printf("Enc2 string and hash is = %s, %s\n",str,hash);
        V(semid,s2); //3ekinaei thn chan diergasia
      }
    }
  }
  else{ //p2
    while(1){
      printf("P2 status: Waiting...\n");
      P(semid,s4);
      if(buffer[4*BUFFSIZE - 1]=='0'){ //Ektypwnei to string
        if(buffer[4*BUFFSIZE - 2] == '1'){
          printf("P2: Ending procedure\n");
          shmdt(shm);
          shmctl(shmid,IPC_RMID,NULL);
          printf("Deleting shared memory\n");
          if (semctl (semid, 0, IPC_RMID) < 0)
          {  
            perror ("error in removing semaphore from the system\n");
            exit (1);
          }
          printf ("Semaphore cleanup complete.\n");
          exit(0);
        } 
        i=1;
        for(j=1;j<=4;j++){
          buffer[j*BUFFSIZE - i] = '1'; //Allazei thn roh kykloforias
        }
        printf("\nThe message you wrote is: = %s\n",buffer + 3*BUFFSIZE);
        V(semid,s4);
      } 
      else{
        printf("\nWrite message for p2(terminate to end all procedures): "); //Diavazei string 
        fgets(ins, 50, stdin);
        if(strcmp(ins,"term\n")==0){
          printf("P2: Ending procedure\n");
          buffer[4*BUFFSIZE-2] = '1';
          V(semid,s3);
          exit(0);
        }
        i=3*BUFFSIZE;
        j=0;
        while(ins[j+1]!='\0'){ //Grafei to input ston 4o buffer(ektos apo ton xaraktira "\n")
          buffer[i] = ins[j];
          i++;
          j++;
        }
        buffer[i] = '\0';
        V(semid,s3); //Arxizei thn enc2 
      }
    }
  }
  exit (0);
}