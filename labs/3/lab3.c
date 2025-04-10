#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>


int pipefd[2];

void sig_handler(int signo) {
	printf("\nget SIGINT; %d\n", signo);
	exit(0);
}

void* proc1(void* flag_read) {
	char buf[256];

	while(*((int*)flag_read) != 1) {
		ssize_t rv = read(pipefd[0], buf, sizeof(buf) - 1);
		if (rv == -1) {
			if (errno != EAGAIN) {
                perror("read");
            }
		}
		else if (rv == 0) {
			fprintf(stderr, "End of file\n");
			*((int*)flag_read) = 1;
		}
		else {
			buf[rv] = '\0';
			printf("%s\n", buf);
		}

	}
}

void* proc2(void* flag_write) {
	char buf[256];

	while(*((int*)flag_write) != 1) {

		long size = sysconf(_SC_NGROUPS_MAX);
		if (size == -1) {perror("size"); exit(EXIT_FAILURE);}

		gid_t list[size];

		int ngroups = getgroups(size, list);
		if (ngroups == -1) {perror("getgroups"); exit(EXIT_FAILURE);}

		buf[0] = '\0';
		for (int i = 0; i < ngroups; i++) {
			char temp[20];
			sprintf(temp, "%d ", list[i]);
			strcat(buf, temp);
		}

		ssize_t rv = write(pipefd[1], buf, strlen(buf));
		if (rv == -1) {perror("Write");}

		sleep(1);
	}
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Ошибка: %s <режим(1 - pipe, 2 - pipe2, 3 - fcntl>\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("Программа начала работу\n");

	signal(SIGINT, sig_handler);

	int* exitcode_1;
	int* exitcode_2;
	int flag = 0;
    int* flag_1 = &flag;
    int* flag_2 = &flag;
    int nummethod = atoi(argv[1]);

    if(nummethod == 1) {
    	int rv = pipe(pipefd);
    	if (rv == -1) {perror("pipe"); exit(EXIT_FAILURE);}
    } else if (nummethod == 2) {
    	int rv = pipe2(pipefd, O_NONBLOCK);
    	if (rv == -1) {perror("pipe2"); exit(EXIT_FAILURE);}
    } else if (nummethod == 3) {
    	int rv = pipe(pipefd);
    	if (rv == -1) {perror("pipe"); exit(EXIT_FAILURE);}
    	int rv1 = fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    	int rv2 = fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
    	if ((rv1 == -1) || (rv2 == -1)) {perror("fcntl"); exit(EXIT_FAILURE);}
    } else {
    	printf("Неверно введенный номер метода выполнения\n");
    	return -1;
    }
    pthread_t thread_id1, thread_id2;
    pthread_create(&thread_id2, NULL, proc2, ((void*) flag_2));
    pthread_create(&thread_id1, NULL, proc1, ((void*) flag_1));

    printf("Программа ждет нажатия клавиши\n");
    getchar();
	printf("Клавиша нажата\n");

    flag = 1;

	pthread_join(thread_id1, (void**)&exitcode_1);
    pthread_join(thread_id2, (void**)&exitcode_2);
    
    close(pipefd[0]);
    close(pipefd[1]);

    printf("Программа завершила работу\n");
	return 0;
}
