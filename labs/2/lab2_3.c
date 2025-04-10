#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

pthread_mutex_t mutex;

typedef struct {
    int flag;
    char sym;
} targs;

void* proc1(void *arg) {
    targs *args = (targs*) arg;
    struct timespec tp;

    printf("Поток 1 начал работу\n");

    while (args->flag == 0) {
        clock_gettime(CLOCK_REALTIME, &tp);
        tp.tv_sec += 1;
        int rv;
        while ((rv = pthread_mutex_timedlock(&mutex, &tp)) != 0) {
            if (rv != ETIMEDOUT) {
                printf("Ошибка %s\n", strerror(rv));
            }
            clock_gettime(CLOCK_REALTIME, &tp);
            tp.tv_sec += 1;
        }

        for (int i = 0; i < 10; i++) {
            putchar(args->sym);
            fflush(stdout);
            sleep(1);
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    printf("\nПоток 1 завершил работу\n");
    pthread_exit((void*)1);
}

void* proc2(void *arg) {
    targs *args = (targs*) arg;
    struct timespec tp;

    printf("Поток 2 начал работу\n");

    while (args->flag == 0) {
        clock_gettime(CLOCK_REALTIME, &tp);
        tp.tv_sec += 1;
        int rv;
        while ((rv = pthread_mutex_timedlock(&mutex, &tp)) != 0) {
            if (rv != ETIMEDOUT) {
                printf("Ошибка %s\n", strerror(rv));
            }
            clock_gettime(CLOCK_REALTIME, &tp);
            tp.tv_sec += 1;
        }

        for (int i = 0; i < 10; i++) {
            putchar(args->sym);
            fflush(stdout);
            sleep(1);
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    printf("\nПоток 2 завершил работу\n");
    pthread_exit((void*)2);
}

int main() {
    printf("Программа начала работу\n");

    pthread_mutex_init(&mutex, NULL);

    pthread_t thread_id1;
    pthread_t thread_id2;
    int exitcode1;
    int exitcode2;
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

    pthread_mutex_destroy(&mutex);

    printf("Программа завершила работу\n");
    return 0;
}
