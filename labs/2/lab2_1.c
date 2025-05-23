#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int flag;
    char sym;
}targs;


void* proc1(void *arg) {
    printf("Поток 1 начал работу\n");

    targs *args = (targs*) arg;

    while(args->flag == 0) {
        for(int i = 0; i < 10; i++) { //Вход к критический участок
            putchar(args->sym);
            fflush(stdout);
            sleep(1);
        } //Выход из критического участка
        sleep(1);
    }

    printf("\nПоток 1 завершил работу");
    pthread_exit((void*)1);
}

void* proc2(void * arg) {
    printf("Поток 2 начал работу\n");

    targs *args = (targs*) arg;

    while(args->flag == 0) {
        for(int i = 0; i < 10; i++) { //Вход в критический участок
            putchar(args->sym);
            fflush(stdout);
            sleep(1);
        } //Выход из критического участка
        sleep(1);
    }

    printf("\nПоток 2 завершил работу");
    pthread_exit((void*)2);
}

int main() {
    printf("Программа начала работу\n");

    pthread_t thread_id1;
    pthread_t thread_id2;
    int* exitcode1;
    int* exitcode2;
    targs arg1 = {0, '1'};
    targs arg2 = {0, '2'};


    pthread_create(&thread_id1, NULL, proc1, &arg1);
    pthread_create(&thread_id2, NULL, proc2, &arg2);

    printf("Программа ждет нажатия клавиши\n");
    getchar();
    printf("Клавиша нажата\n");

    arg1.flag = 1;
    arg2.flag = 1;

    pthread_join(thread_id1, (void**)&exitcode1);
    pthread_join(thread_id2, (void**)&exitcode2);
    
    printf("\nПрограмма завершила работу\n");
    return 0;
}
