#include <stdio.h>
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
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int            shmid;
    struct TERM  *shmaddr;
    char word1[80];
    char output[100];
    int n;
    int re=0;
    unsigned char out[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    int tempch[16];
    double p;
    int res=0;
//    printf("\n===>\n");

    //key_t key= ftok("/home",'p');
    //shmid = shmget(key, sizeof(struct TERM), 0666);
    shmid = shmget((key_t) KEY, sizeof(struct TERM), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("    ENC1: shared memory error1\n");
        return 1;
    }
    shmaddr = (struct TERM *) shmat(shmid, NULL, 0);
    if ((int) shmaddr == -1)
    {
        printf("    ENC1: shared memory error2\n");
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

    if (strcmp(argv[1],"send")==0)
    {

         sem_wait(sem1);  //CRITICAL SECTION 1
         strcpy(word1,shmaddr->input1);
         p=shmaddr->probability1;
         sem_post(sem1);  //END OF CRITICAL SECTION 1

         MD5_Init(&md5);
         MD5_Update(&md5,word1,strlen(word1));
         MD5_Final(output,&md5);
//         printf("\n    ENC1: calculated checksum:");
//         for(n=0; n<MD5_DIGEST_LENGTH; n++)
//             printf("%02x", output[n]);
//         printf("\n");

         sem_wait(sem2);  //CRITICAL SECTION 2
         for (int b=0; b<16; b++)
             shmaddr->checksum2[b]=output[b];
     //    printf("\n    ENC1: passed inside SHM:");
     //    for(n=0; n<MD5_DIGEST_LENGTH; n++)
     //        printf("%02x",shmaddr->checksum2[n]);
     //    printf("\n");
         shmaddr->probability2 = p;
         strcpy(shmaddr->input2,word1);
         sem_post(sem2);  //END OF CRITICAL SECTION 2


         while(1)
         {
             int status;
             int w;
             int pid = fork ();
             if (pid == -1)
                 printf("\nfork error\n");
             if (pid == 0)
             { // I am the child
                 char *args[]={"./chan","send",NULL};
                 execvp(args[0],args);
             }
//             printf ("\n    ENC1: Waiting for CHAN to complete.\n");

             if ((w = waitpid (pid, &status, 0)) == -1)
                 printf ("\n    ENC1: CHAN error\n");
//             if (w == pid)
//                 printf ("\n    ENC1: CHAN returned\n");
             if (shmaddr->resend==0)
                 break;
             else
             {
                 res++;
                 shmaddr->resend=0;
//                 printf("!!!RESEND!!!\n");
             }
             if (res>=3000)
             {
               sem_wait(sem4);  //CRITICAL SECTION 4
               strcpy(shmaddr->input4,"\n!Problem in receiving the right message!\n");
               sem_post(sem4);  //CRITICAL SECTION 4
               shmaddr->status1 = FILLED;

               exit(5);
             }



         }

    }
    else if (strcmp(argv[1],"receive")==0)
    {

//        printf("\naction= %s\n",argv[1]);
        int var=1;
        res=0;
        while(var==1)
        {
             var=0;
             int status;
             int w;
             int pid = fork ();
             if (pid == -1)
                 printf("\nfork error\n");
             if (pid == 0)
             { // I am the child
                 char *args[]={"./chan","receive",NULL};
                 execvp(args[0],args);
             }
//             printf ("\n    ENC1: Waiting for CHAN to complete.\n");
             if ((w = waitpid (pid, &status, 0)) == -1)
                 printf ("\n    ENC1: CHAN error\n");
//             if (w == pid)
//                 printf ("\n    ENC1: CHAN returned\n");


             sem_wait(sem2);  //CRITICAL SECTION 2
//             printf("\n            ENC1: message received => %s\n",shmaddr->input2);
             strcpy(word1,shmaddr->input2);
             for(int i = 0; i < 16; i++)
                 tempch[i] = shmaddr->checksum2[i];
             sem_post(sem2);  //CRITICAL SECTION 3

             MD5_Init(&md5);
             MD5_Update(&md5,word1,strlen(word1));
             MD5_Final(output,&md5);
//             printf("\n            ENC1: calculated checksum:");
//             for(n=0; n<MD5_DIGEST_LENGTH; n++)
//                 printf("%02x", output[n]);
//             printf("\n");
             for (int b=0; b<16; b++)
                 if (tempch[b]!=output[b])
                 {
//                       printf("\n            ENC1: Wrong message sent\n");
                       var=1;
                       res++;
                       continue;
//                       exit(3);
                 }
             if (res>=20000)
             {
                   sem_wait(sem4);  //CRITICAL SECTION 4
                   strcpy(shmaddr->input4,"***");
                   sem_post(sem4);  //CRITICAL SECTION 4
                   shmaddr->status2 = FILLED;

                   exit(6);
             }

             sem_wait(sem1);  //CRITICAL SECTION 1
             strcpy(shmaddr->input1,word1);
             sem_post(sem1);  //CRITICAL SECTION 1

        }

    }


//    printf("\n<===\n");
  	exit(0);
}
