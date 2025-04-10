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

	char new_env_var[] = "New_Var=This_is_a_new_environment_variable";
	char** new_envp = malloc(12 * sizeof(char*)); //Создали массив с первыми 10 элементами из envp + новый элемент + NULL
	for(int i = 0; i < 10; i++) { //Заполняем из envp
		new_envp[i] = envp[i];
	}
	new_envp[10] = new_env_var;
	new_envp[11] = NULL;

	pid_t pid = fork(); //Объявляем процесс

	if(pid == -1) {
		perror("Ошибка при объявлении pid");
		return -1;
	} else if (pid == 0) { //Вызываем дочерний процесс
		execle("lab4_1", argv[0], argv[1], argv[2], NULL, new_envp); //Вызываем дочерний процесс
		free(new_envp);
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