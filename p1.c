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

#define s1 0     /* semaphore array index to check buffer elts used */
#define s2 1    /* semaphore array index to check buffer elts empty */
#define s3 2        /* semaphore index for mutual exclusion to buffer*/
#define s4 3        /* semaphore index for mutual exclusion to buffer*/
#define s0 4

int sem_create(int num_semaphores)
{  /* procedure to create specified number of semaphores */
   int semid;

   /* create new semaphore set of semaphores */
   if ((semid = semget (9253, num_semaphores, IPC_CREAT | 0600)) < 0)
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
  char hash[MD5_DIGEST_LENGTH];

  /* semaphore elements */
  int semid;       /* identifier for a semaphore set */
  /* local variables */
  int i = 0;
  int j = 0;
  char asd[50];

  if ((shmid = shmget((key_t)key,4 * BUFFSIZE * sizeof(char), IPC_CREAT | 0666)) < 0)
    die("shmget");

  if ((shm = shmat(shmid, NULL, 0)) ==   (char *) -1)
    die("shmat");


  /* shared memory segment will be organized as follows:
       0                 200                  400                  600                   800
      --------------------|--------------------|--------------------|--------------------|
      |               | | |                | | |                | | |                | | |
      --------------------|--------------------|--------------------|--------------------|
      ^               ^  ^                 ^  ^                 ^  ^                 ^  ^
      buffer        term flow             term flow            term flow           term flow
   */
  
  /*Arxikopoioyme enan buffer 800 charakthrwn kai ton xwrizoyme se 4 kommatia / ena gia kathe dyada dierasiwn
  0-199->P1-Enc1 ||200-399->Enc1-Chan ||400-599->Chan-Enc2 ||600-799->Enc2-P2 

  /*Kathe buffer exei 2 flags san teleytaioys charakthres, opoy end einai 1 otan theloyme na teleiwsei to programma
  kai flow einai 0 otan to mhnyma exei aristerh orh kai 1 otan exei de3ia.


  /* set up pointers to appropriate places in shared memory segment */
  buffer = shm; /* logical buffer starts at shared segment */
  /* create and initialize semaphores */
  semid = sem_create(5);
  sem_init(semid, s1, 0);
  sem_init(semid, s2, 0);
  sem_init(semid, s3, 0);
  sem_init(semid, s4, 0);
  sem_init(semid, s0, 1);


  i = 0;
  for(i=1;i<=2;i++){
    for(j=1;j<=4;j++){ //mhdenismos toy teleytaioy stoixeioy(flow) kai proteleytaioy(end) gia kathe buffer
      buffer[j*BUFFSIZE - i] = '0';
    }
  }
  if (-1 == (pid = fork())){ /* check for error in spawning child process */
    perror ("error in fork");
    exit (1);
  }
  if (0 == pid){ //p1
    while(1){
      printf("P1 status: Waiting...\n");
      P(semid,s0);
      if(buffer[BUFFSIZE - 1] == '0'){ //An einai h seira toy p1 na steilei
        printf("\nWrite message for p1 (term to terminate all procedures): ");
        fgets(ins,50,stdin);
        if(strcmp(ins,"term\n")==0){ //An einai stop
          printf("P1: Ending procedure\n");
          buffer[BUFFSIZE-2] = '1'; //End flag = 1
          V(semid,s1);//Arixzei to epomeno procedure
          exit(0); //Exit
        }
        i=0;
        while(ins[i+1]!='\0'){
          buffer[i] = ins[i]; //Grapse to input ston prwto buffer (ektos apo ton xaraktira "\n")
          i++;
        }
        buffer[i] = '\0';
        printf("\nOutput string = %s\n",buffer);
        V(semid,s1); //Otan teleiwsei h diergasia tha tre3ei to enc1 
      }
      else{ //Einai h seira toy p2 na steilei
        if(buffer[BUFFSIZE - 2] == '1'){
          printf("P1: Ending procedure\n");
          shmdt(shm); //Detach shm pointer
          shmctl(shmid,IPC_RMID,NULL);//Delete shared memory
          printf("Deleting shared memory\n");
          if (semctl (semid, 0, IPC_RMID) < 0) //Remove semaphores
          {  
            perror ("error in removing semaphore from the system\n");
            exit (1);
          }
          printf ("Semaphore cleanup complete.\n");
          exit(0);
        }
        i = 1;
        for(j=1;j<=4;j++){
          buffer[j*BUFFSIZE - i] = '0'; //Mhdenismos twn flag gia na 3ekinhsei h p1
        }
        printf("\nThe message you wrote is: = %s\n",buffer);
        V(semid,s0);//3anatrexei thn p1 gia to input
      }
    }      
  }
  else{ //enc1
    while(1){
      printf("ENC1 status: Waiting...\n");
      P(semid, s1); //Perimenei shma energopoihshs
      if(buffer[2*BUFFSIZE - 1]=='0'){ //An einai h seira toy p1 na steilei
        if(buffer[BUFFSIZE - 2] == '1'){ //An einai end einai 1 
          printf("ENC1: Ending procedure\n");
          buffer[2*BUFFSIZE-2] = '1'; //End = 1 ston epomeno buffer
          V(semid,s2); //Synexizei sto epomeno procedure
          exit(0);
        }
        i=0;
        while(buffer[i]!='\0'){ //Diabase to input toy prwtoy buffer
          str[i] = buffer[i];
          i++;
        }
        str[i] = '\0';
        memset(asd,'\0',sizeof(asd));
        i=0;
        while(str[i]!='\0'){
          asd[i] = str[i];
          i++;
        }
        MD5(asd, sizeof(asd),hash); //Algorithmos md5 gia thn eyresh toy hash

        i=0;
        while(buffer[i]!='\0'){
          buffer[BUFFSIZE+i] = buffer[i]; //Grafei to string ston deytero buffer
          i++;
        }
        buffer[BUFFSIZE+i] = '\0';
        i++;
        j=0; 
        while(hash[j]!='\0'){
          buffer[BUFFSIZE+i] = hash[j]; //Grafei to hash dipla apo to string
          i++;
          j++;
        }
        buffer[BUFFSIZE+i] = '\0';
        printf("Enc1 string and hash is = %s, %s\n",str,hash);
        V(semid,s2); //3ekinaei thn chan diergasia
      }
      else{ //Einai h seira toy p2 na steilei
        if(buffer[2*BUFFSIZE - 2] == '1'){
          printf("ENC1: Ending procedure\n");
          buffer[BUFFSIZE-2] = '1';
          V(semid,s0);
          exit(0);
        }
        i=BUFFSIZE;
        j=0;
        while(buffer[i]!='\0'){
          str[j] = buffer[i]; // Diabazei to string apo ton deytero buffer
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
        MD5(asd, sizeof(asd),hash);

        i = 0;
        while(buffer[BUFFSIZE + i]!='\0'){
          i++;
        }
        i++;
        j=0;
        while(buffer[BUFFSIZE + i]!='\0'){
          sumch[j] = buffer[2*BUFFSIZE+i]; //Diabazei to hash 
          i++;
          j++;
        }
        sumch[j] = '\0';
        printf("\nStarting kai final hashes are:  %s %s",sumch,hash);
         if(strcmp(sumch,hash) != 0 ){ //An den einai ta idia hash tote 3anarxizei thn enc2
          printf("\nHashes are different so redoing enc1\n");
          printf("\n");
          V(semid,s3);
        }
        else{
          printf("\nHashes are identical/ pass message to p2\n");
          i=0;
          while(str[i]!='\0'){
            buffer[i] = str[i]; //Grafei to string ston prwto buffer gia na diavastei
            i++;
          }
          printf("\n");
          buffer[i] = '\0';
          V(semid,s0);//3ekinaei p1
        }
      }
    }
  }
  exit (0);
}