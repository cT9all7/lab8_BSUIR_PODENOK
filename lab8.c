#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<unistd.h>
#include<sys/sem.h>
#include<pthread.h>

union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};

struct sembuf semops;
void up(int semid,int val);
void down(int semid,int val);
int msqid;
int semid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void* producer(void* a)
{
	printf("\nCREATE thread PRODUSER\n");
	char mess[10];
	for(;;)
	{
		sleep(1);
		for(int i=0;i<10;i++) mess[i]=97+rand()%26;
		down(semid,0);
		pthread_mutex_lock(&mutex);
		msgsnd(msqid,&mess,10,0);
		printf("sended : %s\n",mess);
		pthread_mutex_unlock(&mutex);
		up(semid,1);
	}
}

void* consumer(void* a)
{
	printf("\nCREATE thread CONSUMER\n");
	char buffe[10];
	for(;;)
	{
		down(semid,1);
		pthread_mutex_lock(&mutex);
		msgrcv(msqid,&buffe,10,0,0);
		printf("recived: %s\n",buffe);
		pthread_mutex_unlock(&mutex);
		up(semid,0);
	}
}

int main(int argc, char* argv[])
{
	bool flag;
	key_t key;
	key=ftok("/home/kali/progi/lab02/ftok",2);
	
	msqid=msgget(key,(IPC_CREAT|0666));
	semid=semget(key,2,(IPC_CREAT|0666));
	
	union semun opt;
	int val;
	
	opt.val=10;//size queue
	val=semctl(semid,0,SETVAL,opt);
	
	opt.val=0;//items
	val=semctl(semid,1,SETVAL,opt);

	char a;
	int i=0;
	pthread_t pth[12];
	struct msqid_ds* buf;
	for(;;)
	{
		a=getchar();
		if(a=='p')
		{
			if(i!=11)
			{
				pthread_create(&pth[i],NULL,producer,NULL);
				i++;
			}
		}
		else if(a=='P')
		{
			if(i!=11)
			{
				pthread_create(&pth[i],NULL,consumer,NULL);
				i++;
			}
		}
		else if(a=='q')
		{
			msgctl(msqid,IPC_RMID,buf);
			semctl(semid,0,IPC_RMID);
			for(int i1=0;i1<i;i1++) pthread_cancel(pth[i1]);
			return 0;
		}
		
	}
	
	return 0;
}
void up(int semid,int val)
{
	int fal;
	semops.sem_num=val;
	semops.sem_op=1;
	semops.sem_flg=0;
	fal=semop(semid,&semops,1);	
}
void down(int semid,int val)
{
	int fal;
	semops.sem_num=val;
	semops.sem_op=-1;
	semops.sem_flg=0;
	fal=semop(semid,&semops,1);				
}