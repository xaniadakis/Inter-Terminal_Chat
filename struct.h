#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1
#define KEY 8888


sem_t *sem1;
sem_t *sem2;
sem_t *sem3;
sem_t *sem4;
int pshared = 1;
unsigned int value = 1;
int ret = 0;

struct TERM {

	char input1[80];
	double probability1;

	char input2[80];
	double probability2;
	int checksum2[16];

	char input3[80];
	double probability3;
	int checksum3[16];

	char input4[70];
	double probability4;

	int  status1,status2;
	int resend;
};
