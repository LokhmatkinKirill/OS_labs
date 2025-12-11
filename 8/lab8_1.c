#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/priority_socket"
#define BUFFER_SIZE 1024

struct entry {
    int data;
    STAILQ_ENTRY(entry) entries;
};

int socket_listener;
int sck;
STAILQ_HEAD(stailhead, entry);
struct stailhead head;
int flag_receiver = 0;
int flag_sender = 0;
int flag_wait = 0;
pthread_mutex_t mutex;
pthread_t id_receiver;
pthread_t id_sender;
pthread_t id_wait;
struct sockaddr_un addr;
struct sockaddr_un addr_l;

void sig_handler(int sig) {
    printf("get SIGPIPE\n");
}

static void* proc_rcv() {
    printf("The request receiving thread has started. \n");
    while (flag_receiver != 1) {
        int rcvbuf = 0;
        int reccount = recv(sck, &rcvbuf, sizeof(rcvbuf), 0);
        if (reccount == -1) {
            perror("recv");
            sleep(1);
        }
        else if (reccount == 0) {
            printf("Disconnected...\n");
            sleep(1);
        }
        else {
            while (1) {
                int rv = pthread_mutex_trylock(&mutex);
                if (rv == 0) break;
                else {
                    printf("Error: %s\n", strerror(rv));
                    sleep(1);
                }
            }
            struct entry* n = malloc(sizeof(struct entry));
            n->data = rcvbuf;
            STAILQ_INSERT_TAIL(&head, n, entries);
            pthread_mutex_unlock(&mutex);
            printf("Request %d is received. \n", rcvbuf);
        }
    }
    printf("The request receiving thread has been closed!\n");
    return NULL;
}

static void* proc_snd() {
    printf("The request handling thread has started. \n");
    while (flag_sender != 1) {
        while (1) {
            int rv = pthread_mutex_trylock(&mutex);
            if (rv == 0) break;
            else {
                printf("Error: %s\n", strerror(rv));
                sleep(1);
            }
        }
        if (STAILQ_EMPTY(&head) == 0) {
            struct entry* n = STAILQ_FIRST(&head);
            STAILQ_REMOVE_HEAD(&head, entries);
            pthread_mutex_unlock(&mutex);

            // Получаем максимальное количество групп
            long ngroups_max = sysconf(_SC_NGROUPS_MAX);
            if (ngroups_max == -1) {
                perror("sysconf");
                free(n);
                continue;
            }
            
            gid_t *groups = malloc(ngroups_max * sizeof(gid_t));
            if (groups == NULL) {
                perror("malloc");
                free(n);
                continue;
            }
            
            // Получаем список групп
            int ngroups = getgroups(ngroups_max, groups);
            if (ngroups == -1) {
                perror("getgroups");
                free(groups);
                free(n);
                continue;
            }
            
            // Формируем строку с группами
            char sndbuf[BUFFER_SIZE];
            int offset = snprintf(sndbuf, BUFFER_SIZE, "Request %d: Groups: ", n->data);
            
            for (int i = 0; i < ngroups && offset < BUFFER_SIZE; i++) {
                offset += snprintf(sndbuf + offset, BUFFER_SIZE - offset, "%d ", groups[i]);
            }
            
            free(groups);
            
            int sentcount = send(sck, sndbuf, offset, 0);
            if (sentcount == -1) {
                perror("send");
            }
            else {
                printf("Sent response for request %d\n", n->data);
            }
            free(n);
        }
        else {
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }
    }
    printf("The request handling thread has been closed. \n");
    return NULL;
}

static void* proc_wait() {
    printf("The waiting thread has started. \n");
    socklen_t addrlen = sizeof(addr);
    while (flag_wait != 1) {
        sck = accept(socket_listener, (struct sockaddr*)&addr, &addrlen);
        if (sck == -1) {
            perror("accept");
            sleep(1);
        }
        else {
            printf("Connected client: %s\n", addr.sun_path);
            pthread_create(&id_receiver, NULL, proc_rcv, NULL);
            pthread_create(&id_sender, NULL, proc_snd, NULL);
            break;
        }
    }
    printf("The waiting thread has been closed. \n");
    return NULL;
}

int main() {
    printf("The server has started working. \n");
    signal(SIGPIPE, sig_handler);
    pthread_mutex_init(&mutex, NULL);
    STAILQ_INIT(&head);

    memset(&addr_l, 0, sizeof(addr_l));
    addr_l.sun_family = AF_UNIX;
    strncpy(addr_l.sun_path, SOCKET_PATH, sizeof(addr_l.sun_path) - 1);
unlink(SOCKET_PATH);
socket_listener = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(socket_listener, F_SETFL, O_NONBLOCK);
    if (bind(socket_listener, (struct sockaddr*)&addr_l, sizeof(addr_l)) == -1) {
        perror("bind");
        close(socket_listener);
        return 1;
    }

    listen(socket_listener, 5);
    pthread_create(&id_wait, NULL, proc_wait, NULL);

    printf("The program is waiting a key to be pressed. \n");
    getchar();
    printf("The key is pressed. \n");

    flag_receiver = 1;
    flag_sender = 1;
    flag_wait = 1;

    pthread_join(id_sender, NULL);
    pthread_join(id_receiver, NULL);
    pthread_join(id_wait, NULL);

    if (sck != -1) {
        shutdown(sck, 2);
        close(sck);
    }
    close(socket_listener);
    unlink(SOCKET_PATH);
    pthread_mutex_destroy(&mutex);

    printf("The server is shut. \n");
    return 0;
}