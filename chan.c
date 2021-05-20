#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "struct.h"
#include <sys/wait.h>



int main(int argc, char *argv[])
{
    srand(time(NULL) ^ (getpid()<<16));
    char buf[80];
    buf[0]='\0';
    float a,prob,x;
    int shmid;
    struct TERM  *shmaddr;
//    printf("\n======>\n");
    int tempch[16];
    double temprob;



    //key_t key= ftok("/home",'p');
    //shmid = shmget(key, sizeof(struct TERM), 0666);
    shmid = shmget((key_t) KEY, sizeof(struct TERM), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("\n        CHAN: shared memory error1\n");
        return 1;
    }

    shmaddr = (struct TERM *) shmat(shmid, NULL, 0);
    if ((int) shmaddr == -1)
    {
        printf("\n        CHAN: shared memory error2\n");
        return 2;
    }


    if ((sem2 = sem_open("sem2", O_CREAT, 0644, 1)) == SEM_FAILED)
    {
         printf("\nP1:semaphore2 error\n");
         return 3;
    }
    if ((sem3 = sem_open("sem3", O_CREAT, 0644, 1)) == SEM_FAILED)
    {
         printf("\nP1:semaphore3 error\n");
         return 3;
    }

    if (strcmp(argv[1],"send")==0)
    {

//         printf("\naction= %s\n",argv[1]);

         sem_wait(sem2);  //CRITICAL SECTION 2
         strcpy(buf,shmaddr->input2);
         for(int i = 0; i < 16; i++)
             tempch[i] = shmaddr->checksum2[i];
         temprob = shmaddr->probability2;
         sem_post(sem2);  //CRITICAL SECTION 2


         a=strlen(buf)-1;
         prob = (temprob*a)/100;
//         printf("\n        CHAN: given probability is: %2.1f per %.f words\n",prob,a);
         for(int i=0;i<a;i++)
         {
                   x = (float)rand()/(float)(RAND_MAX/a);
                   if (x<=prob)
                       buf[i]='x';

         }

         sem_wait(sem3);  //CRITICAL SECTION 3
         strcpy(shmaddr->input3,buf);
//         printf("\n        CHAN: input is now: %s\n",shmaddr->input3);
         for(int i = 0; i < 16; i++)
             shmaddr->checksum3[i] = tempch[i];
         sem_post(sem3);  //CRITICAL SECTION 3


         int status;
         int w;
         int pid = fork ();
         if (pid == -1)
              printf("\nfork error\n");
         if (pid == 0)
         { // I am the child
              char *args[]={"./enc2","send",NULL};
              execvp(args[0],args);
         }
//         printf("\n        CHAN: Waiting for ENC2 to complete.\n");

         if ((w = waitpid (pid, &status, 0)) == -1)
              printf ("\n         CHAN: ENC2 error\n");
//         if (w == pid)
//              printf("\n       CHAN: ENC2 returned\n");
    }
    else if (strcmp(argv[1],"receive")==0)
    {

//         printf("\naction= %s\n",argv[1]);

         int status;
         int w;
         int pid = fork ();
         if (pid == -1)
              printf("\nfork error\n");
         if (pid == 0)
         { // I am the child
              char *args[]={"./enc2","receive",NULL};
              execvp(args[0],args);
         }
  //       printf("\n        CHAN: Waiting for ENC2 to complete.\n");

         if ((w = waitpid (pid, &status, 0)) == -1)
              printf ("\n         CHAN: ENC2 error\n");
//         if (w == pid)
//              printf("\n       CHAN: ENC2 returned\n");

         sem_wait(sem3);  //CRITICAL SECTION 3
         strcpy(buf,shmaddr->input3);
         for(int i = 0; i < 16; i++)
              tempch[i] = shmaddr->checksum3[i];
         temprob = shmaddr->probability3;
         sem_post(sem3);  //CRITICAL SECTION 3


         a=strlen(buf)-1;
         prob = (temprob*a)/100;
//         printf("\n        CHAN: given probability is: %2.1f per %.f words\n",prob,a);
         for(int i=0;i<a;i++)
         {
              x = (float)rand()/(float)(RAND_MAX/a);
              if (x<=prob)
                   buf[i]='x';
         }

         sem_wait(sem2);  //CRITICAL SECTION 2
         strcpy(shmaddr->input2,buf);
//         printf("\n        CHAN: input is now: %s\n",shmaddr->input2);
         for(int i = 0; i < 16; i++)
             shmaddr->checksum2[i] = tempch[i];
         sem_post(sem2);  //CRITICAL SECTION 2


     }



//    printf("\n<===\n");
    exit(0);
}
