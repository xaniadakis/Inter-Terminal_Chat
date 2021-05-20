/* ----------------------------------------------------------------- */
/* PROGRAM  client.c                                                 */
/*    This is the client program.  It can only be started as the     */
/* server says so.  The client requests the same shared memory the   */
/* server established, attaches it to its own address space, takes   */
/* the data, changes the status to TAKEN, detaches the shared memory,*/
/* and exits.                                                        */
/* ----------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "struct.h"
#include <sys/wait.h>


int main(int  argc, char *argv[])
{
     int            shmid;
     struct TERM  *shmaddr;
     char buf[80];

    if(argc<2)
    {
        printf("Wrong parameters\n./p2 <probability_of_fault[0-100]>\n");
        return 1;
    }
    if(atoi(argv[1])<0 || atoi(argv[1])>100)
    {
        printf("Wrong parameters\nprobability_of_fault ranges from 0-100\n");
        return 1;
    }    
     //key_t key= ftok("/home",'p');
     //shmid = shmget(key, sizeof(struct TERM), 0666);
     shmid = shmget((key_t) KEY, sizeof(struct TERM), IPC_CREAT | 0666);
     if (shmid < 0)
     {
          printf("\nP2: shared memory error1\n");
          return 1;
     }

     shmaddr = (struct TERM *) shmat(shmid, NULL, 0);
     if ((int) shmaddr == -1)
     {
          printf("\nP2: shared memory error2\n");
          return 2;
     }
     if ((sem4 = sem_open("sem4", O_CREAT, 0644, 1)) == SEM_FAILED)
     {
          printf("\nP1:semaphore4 error\n");
          return 3;
     }
     while(1)
     {
          printf("\nP1 typing...\n");
          //shmaddr->status1  = TAKEN;
          while (shmaddr->status1 != FILLED)
              sleep(1);
          sem_wait(sem4);  //CRITICAL SECTION 4
          strcpy(buf,shmaddr->input4);
          sem_post(sem4);  //CRITICAL SECTION 4
          printf("P1 says: %s\n",buf);
          shmaddr->status1 = TAKEN;
          if (strcmp(buf,"bye")==0)
               break;



          shmaddr->status2  = NOT_READY;
          printf("\nType message to send:");
          scanf("%[^\n]%*c", buf);

          sem_wait(sem4);  //CRITICAL SECTION 4
          shmaddr->probability4 = atof(argv[1]);
          strcpy(shmaddr->input4,buf);
//          printf("\nP1: message towards ENC2 is: %s\n",shmaddr->input4);
          printf("\n                    P1: probability of fault is: %.1f percent\n",shmaddr->probability4);
          sem_post(sem4);  //END OF CRITICAL SECTION 4

          shmaddr->status2 = FILLED;
          shmaddr->resend=0;
          int i=0;
          while (shmaddr->status2 != TAKEN)
          {
               i++;
               sleep(1);
//               if (i==7)
//                   break;
          }
          sem_wait(sem4);  //CRITICAL SECTION 4
          strcpy(buf,shmaddr->input4);
          sem_post(sem4); //END OF CRITICAL SECTION 4

          if (strcmp(buf,"***")==0)
                printf("\n!Problem in sending the right message!\n");
          else
                printf("                    P2: Message '%s' delivered successfully\n",buf);
          if (strcmp(buf,"bye")==0)
               break;

     }


     shmdt((void *) shmaddr);
     printf("\nP2: detached its shared memory...\n");
     printf("\nP2: exits...\n");
     return 0;
}
