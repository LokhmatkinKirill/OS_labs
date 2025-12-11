#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <grp.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int arg1 = 0;
int msgid;


typedef struct{
	long mtype;
	char buff[256];
} TMessage;

void* proc(void *arg) {
    int* args = (int*) arg;
    TMessage message;
	message.mtype = 1;

	while(*args != 1) {

		int result;
		memset(message.buff, 0, sizeof(message.buff));
		message.buff[0] = '\0';
		result = msgrcv(msgid, &message, sizeof(message.buff), message.mtype, IPC_NOWAIT);
		if (result == -1) {
			perror("msgrcv");
			msgctl(msgid, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		printf("Groups: %s\n", message.buff);
		
		sleep(1);
	}
}
	
int main() {
	printf("Программа 2 начала работу\n");
	pthread_t thread;
	int* exitcode;
	key_t key;
	key = ftok("lab7", 'A');

	msgid = msgget(key, 0);
	if (msgid < 0) {
		msgid = msgget(key, IPC_CREAT | 0666);
	}

	if(pthread_create(&thread, NULL, proc, &arg1)) {
		perror("pthread_create");
		msgctl(msgid, IPC_RMID, NULL);
		exit(EXIT_FAILURE);
	}

	printf("Создали поток\nЖдем нажатия клавиши\n");

	getchar();
	arg1 = 1;

	pthread_join(thread, (void**)&exitcode);
	printf("Программа 2 завершила работу\n");
	msgctl(msgid, IPC_RMID, NULL);
	return 0;
}