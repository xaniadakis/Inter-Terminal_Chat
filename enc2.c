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
    int re = 0;
    unsigned char out[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    int tempch[16];
    double p;
  //  int res;
//    printf("\n===========>\n");

    //key_t key= ftok("/home",'p');
    //shmid = shmget(key, sizeof(struct TERM), 0666);
    shmid = shmget((key_t) KEY, sizeof(struct TERM), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("            ENC2: shared memory error1\n");
        return 1;
    }
    shmaddr = (struct TERM *) shmat(shmid, NULL, 0);
    if ((int) shmaddr == -1)
    {
        printf("            ENC2: shared memory error2\n");
        return 2;
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
//        printf("\naction= %s\n",argv[1]);

        sem_wait(sem3);  //CRITICAL SECTION 3
//        printf("\n            ENC2: message received => %s\n",shmaddr->input3);
        strcpy(word1,shmaddr->input3);
        for(int i = 0; i < 16; i++)
            tempch[i] = shmaddr->checksum3[i];
        sem_post(sem3);  //CRITICAL SECTION 3



        MD5_Init(&md5);
        MD5_Update(&md5,word1,strlen(word1));
        MD5_Final(output,&md5);
//        printf("\n            ENC2: calculated checksum:");
//        for(n=0; n<MD5_DIGEST_LENGTH; n++)
//            printf("%02x", output[n]);
//        printf("\n");


        for (int b=0; b<16; b++)
        {
            if (tempch[b]!=output[b])
            {
//                printf("\n            ENC2: Wrong message sent\n");
                  shmaddr->resend=1;

                  exit(3);

            }
        }




        sem_wait(sem4);  //CRITICAL SECTION 4
        strcpy(shmaddr->input4,word1);
        sem_post(sem4);  //CRITICAL SECTION 4

        shmaddr->resend=0;
    }
    else if (strcmp(argv[1],"receive")==0)
    {
//        printf("\naction= %s\n",argv[1]);

        sem_wait(sem4);  //CRITICAL SECTION 4
//        printf("\n    ENC2: message received => %s\n",shmaddr->input4);
        strcpy(word1,shmaddr->input4);
        p=shmaddr->probability4;
        sem_post(sem4);  //END OF CRITICAL SECTION 4

        MD5_Init(&md5);
        MD5_Update(&md5,word1,strlen(word1));
        MD5_Final(output,&md5);
//        printf("\n    ENC2: calculated checksum:");
//        for(n=0; n<MD5_DIGEST_LENGTH; n++)
//            printf("%02x", output[n]);
//        printf("\n");

        sem_wait(sem3);  //CRITICAL SECTION 3
        for (int b=0; b<16; b++)
            shmaddr->checksum3[b]=output[b];
    //    printf("\n    ENC2: passed inside SHM:");
    //    for(n=0; n<MD5_DIGEST_LENGTH; n++)
    //        printf("%02x",shmaddr->checksum3[n]);
    //    printf("\n");
        shmaddr->probability3 = p;
        strcpy(shmaddr->input3,word1);
        sem_post(sem3);  //END OF CRITICAL SECTION 3
    }

//    printf("\n<===\n");
  	exit(0);


}
