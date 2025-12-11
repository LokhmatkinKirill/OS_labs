#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <grp.h>


int arg1 = 0;
int shm;
sem_t* sem_read;
sem_t* sem_write;
void* local_addr;


void sig_handler(int signo) {
	printf("\nget SIGINT; %d\n", signo);
	arg1 = 1;
	exit(0);
}

void* proc(void *arg) {
    int* args = (int*) arg;

    while(*args != 1) {
    	sem_wait(sem_write);

		long size = sysconf(_SC_NGROUPS_MAX);
		if (size == -1) {perror("size"); exit(EXIT_FAILURE);}

		char groups_str[size*20];
		memcpy(groups_str, local_addr, strlen((char*)local_addr) + 1);		
		printf("Groups: %s\n", groups_str);
		sem_post(sem_read);
    }
}

int main() {
	printf("Программа 2 начала работу\n");

	pthread_t thread;
	int* exitcode;

	long size = sysconf(_SC_NGROUPS_MAX);
	if (size == -1) {
		perror("size"); 
		exit(EXIT_FAILURE);
	}

	const char* shm_name = "/shm";
	shm = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	if (shm == -1) {
		perror("shm_open");
		exit(EXIT_FAILURE);
	}

	printf("Создали разделяемую память\n");

	if (ftruncate(shm, size * 20) == -1) {
		perror("ftruncate");
		close(shm);
		exit(EXIT_FAILURE);
	}

	printf("Изменили размер разделяемой памяти на требуемый\n");

	local_addr = mmap(0, size * 20, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
	if (local_addr == MAP_FAILED) {
		perror("local_addr");
		close(shm);
		exit(EXIT_FAILURE);
	}

	printf("Отобразили разделяемую память на локальный адрес\n");

	const char* sem_name1 = "/sem_read";
	const char* sem_name2 = "/sem_write";

	sem_read = sem_open(sem_name1, O_CREAT, 0666, 0);
	if (sem_read == SEM_FAILED) {
		perror("sem_read");
		munmap(local_addr, size * 20);
		close(shm);
		exit(EXIT_FAILURE);
	}

	printf("Создали семафор чтения\n");

	sem_write = sem_open(sem_name2, O_CREAT, 0666, 0);
	if (sem_write == SEM_FAILED) {
		perror("sem_write");
		sem_close(sem_read);
		munmap(local_addr, size * 20);
		close(shm);
		exit(EXIT_FAILURE);
	}

	printf("Создали семафор записи\n");

	signal(SIGINT, sig_handler);

	if(pthread_create(&thread, NULL, proc, &arg1)) {
		perror("sem_read");
		sem_close(sem_write);
		sem_close(sem_read);
		munmap(local_addr, size * 20);
		close(shm); 
		exit(EXIT_FAILURE);
	}

	printf("Создали поток\n");
	printf("Ждем нажатия клавиши\n");
	getchar();
	arg1 = 1; //На всякий случай ещё раз продублировать установление единицей

	pthread_join(thread, (void**)&exitcode);

	sem_close(sem_read);
	sem_close(sem_write);
	sem_unlink(sem_name1);
	sem_unlink(sem_name2);

	munmap(local_addr, size * 20);
	close(shm);
	shm_unlink(shm_name);
	printf("Программа 2 завершила работу\n");
	return 0;
}