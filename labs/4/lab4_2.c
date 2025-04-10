#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("Программа начала работу\n");

	printf("Program 2 pid: %d\n", getpid()); //Выводим идентификаторы
	printf("Program 2 ppid: %d\n", getppid());
	printf("Argument from command line:\n");
	
	for(int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}

	pid_t pid = fork(); //Объявляем процесс

	if(pid == -1) {
		perror("Ошибка при объявлении pid");
		return -1;
	} else if (pid == 0) { //Вызываем дочерний процесс
		char *new_argv[argc + 1];
		for (int i = 0; i < argc; i++) { //Новый массив с переданными переменными из командной строки с нулем в конце
			new_argv[i] = argv[i];
		}
		new_argv[argc] = NULL;

		execle("lab4_1", new_argv[0], new_argv[1], new_argv[2], NULL, envp); //Вызываем дочерний процесс
	} else if (pid > 0) { //Родительский процесс
		int status;
		while(waitpid(pid, &status, WNOHANG) == 0) {
			printf("Wait\n");
			usleep(500000);
		}
		printf("Program1 exit status: %d\n", WEXITSTATUS(status));
	}

	printf("Program 2 finished\n");
	return 0;
}