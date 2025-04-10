#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char* argv[], char* envp[]) {

	printf("Program 1 pid: %d\n", getpid()); ///Выводим
	printf("Program 1 ppid: %d\n", getppid());

	for(int i = 0; i < argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
		sleep(1);
	}

	for(int i = 0; i < 11; i++) { //Так как envp очень большая переменная, то выведем первые 10 значений
		printf("envp[%d]: %s\n", i, envp[i]);
		sleep(1);
	}

	printf("Program 1 finished\n");
	return 1;
}