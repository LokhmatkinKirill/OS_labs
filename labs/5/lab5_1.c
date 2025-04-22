#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include<time.h>
#include<errno.h>

int main() {
	printf("Программа 1 начала работу\n");

	sem_t *semaphore;
	FILE *file;
	const char* sem_name = "/sem";

	semaphore = sem_open(sem_name, O_CREAT, 0644, 0);
	if(semaphore == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_post(semaphore);
	printf("Создали семафор\n");

	file = fopen("lab5.txt", "a");
	if(file == NULL) {
		perror("file");
		sem_close(semaphore);
		sem_unlink(sem_name);
		exit(EXIT_FAILURE);
	}
	printf("Открыли файл\n");

	fd_set rfds;
	struct timespec timeout;

	while(1) {
		if(sem_wait(semaphore) == -1) {
			perror("sem_wait");
			break;
		}

		printf("Вошли в цикл, вошли в семафор(1)\n");
		fflush(stdout);
		for(int i = 0; i < 10; i++) {
			printf("%d", 1);
			fflush(stdout);
			fprintf(file, "%d", 1);
			fflush(file);
			sleep(1);
		}
		sem_post(semaphore);

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		timeout.tv_sec = 1;
		timeout.tv_nsec = 0;

		if (pselect(1, &rfds, NULL, NULL, &timeout, NULL) > 0) {
			printf("\nНажали Enter, выходим из программы 1");
			fflush(stdout);
			break;
		}

	}

	fclose(file);
	sem_close(semaphore);
	sem_unlink(sem_name);
	
	printf("\nProgram 1 finished\n");
	return 0;
}