#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include  "struct.h"
#include <time.h>
#include <sys/wait.h>

int  main(int  argc, char *argv[])
{
     char buf[80];
     int w;
     int            shmid;
     struct TERM  *shmaddr;

     //key_t key= ftok("/home",'p');
     //shmid = shmget(key, sizeof(struct TERM), IPC_CREAT | 0666);

    if(argc<2)
    {
        printf("Wrong parameters\n./p1 <probability_of_fault[0-100]>\n");
        return 1;
    }
    if(atoi(argv[1])<0 || atoi(argv[1])>100)
    {
        printf("Wrong parameters\nprobability_of_fault ranges from 0-100\n");
        return 1;
    }    
     shmid = shmget((key_t) KEY, sizeof(struct TERM), IPC_CREAT | 0666);
     if (shmid < 0)
     {
          printf("\nP1:shared memory error1\n");
          return 1;
     }

     shmaddr = (struct TERM *) shmat(shmid, NULL, 0);
     if ((int) shmaddr == -1)
     {
          printf("\nP1: shared memory error2\n");
          return 2;
     }

     if ((sem1 = sem_open("sem1", O_CREAT, 0644, 1)) == SEM_FAILED)
     {
          printf("\nP1:semaphore1 error\n");
          return 3;
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
     if ((sem4 = sem_open("sem4", O_CREAT, 0644, 1)) == SEM_FAILED)
     {
          printf("\nP1:semaphore4 error\n");
          return 3;
     }

//     shmaddr->status  = NOT_READY;
     while(1)
     {

           shmaddr->status1  = NOT_READY;
           printf("\nType message to send: ");
           scanf("%[^\n]%*c", buf);


           sem_wait(sem1);  //CRITICAL SECTION
           shmaddr->probability1 = atof(argv[1]);
           strcpy(shmaddr->input1,buf);
//           printf("\nP1: message towards ENC1 is: %s\n",shmaddr->input1);
           printf("\n                    P1: probability of fault is: %.1f percent \n",shmaddr->probability1);
           sem_post(sem1);  //END OF CRITICAL SECTION
           shmaddr->resend=0;
           int status;
           int pid = fork ();
           if (pid == -1)
                printf("\nP1: fork error\n");
           if (pid == 0)
           { // I am the child
                char *args[]={"./enc1","send",NULL};
                execvp(args[0],args);
      	   }
//      	   printf ("\nP1: Waiting for ENC1 to complete.\n");
      	   if ((w = wait(&status)) == -1)
      		      printf ("\nP1: error\n");
      	   if (w == pid)
//    		      printf ("\nP1: ENC1 returned\n");
                shmaddr->status1 = FILLED;
           int i=0;
           while (shmaddr->status1 != TAKEN)
           {
                i++;
                sleep(1);
//                if (i==7)
//                    break;
           }
           if (WEXITSTATUS(status)==5)
                printf("!Problem in sending the right message!\n");
           else
                printf("                    P1: Message '%s' is delivered successfully\n",buf);
           if (strcmp(buf,"bye")==0)
                break;


           printf("\nP2 typing...\n");
           //shmaddr->status2  = TAKEN;
           while (shmaddr->status2 != FILLED)
                sleep(1);
           pid = fork ();
           if (pid == -1)
           printf("\nP1: fork error\n");
           if (pid == 0)
           { // I am the child
                char *args[]={"./enc1","receive",NULL};
                execvp(args[0],args);
           }
//           printf ("\nP1: Waiting for ENC1 to complete.\n");
           if ((w = wait(&status)) == -1)
        	      printf ("\nP1: error\n");
//      	   if (w == pid)
//                printf ("\nP1: ENC1 returned\n");

          sem_wait(sem1);  //CRITICAL SECTION 1
          strcpy(buf,shmaddr->input1);
          sem_post(sem1);  //CRITICAL SECTION 1
          if (WEXITSTATUS(status)==6)
               printf("!Problem in receiving the right message!\n");
          else
               printf("P2 says: %s\n",buf);
          shmaddr->status2 = TAKEN;
          if (strcmp(buf,"bye")==0)
                break;
          shmaddr->resend=0;

     }

     shmdt((void *) shmaddr);
     printf("\nP1: detached shared memory...\n");

     shmctl(shmid, IPC_RMID, NULL);
     printf("\nP1: removed shared memory...\n");

     sem_close(sem1);
     sem_close(sem2);
     sem_close(sem3);
     sem_close(sem4);


     printf("\nP1: removed semaphores...\n");

     printf("\nP1: exits...\n");
     return 0;
}
