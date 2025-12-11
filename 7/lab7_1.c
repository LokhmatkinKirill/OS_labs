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
	while(*args != 1){

		long size = sysconf(_SC_NGROUPS_MAX);
		if (size == -1) {
			perror("size");
			msgctl(msgid, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		int result = 0;
		TMessage message;
		message.mtype = 1;
		message.buff[0] = '\0';
		gid_t list[size];

		int ngroups = getgroups(size, list);
		if (ngroups == -1) {
			perror("getgroups");
			msgctl(msgid, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}

		for(int i = 0; i < ngroups; i++) {
			char group_id[20];
			snprintf(group_id, sizeof(group_id), "%d", list[i]);
			strcat(message.buff, group_id);
			if(i < ngroups - 1) {
				strcat(message.buff, " ");
			}
		}

		printf("Groups: %s\n", message.buff);
		result = msgsnd(msgid, &message, strlen(message.buff) + 1, IPC_NOWAIT);
		if (result == -1) {
			perror("msgsnd");
			msgctl(msgid, IPC_RMID, NULL);
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}
}
	
int main() {
	printf("Программа 1 начала работу\n");
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
	printf("Программа 1 завершила работу\n");
	msgctl(msgid, IPC_RMID, NULL);
	return 0;
}