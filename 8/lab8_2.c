#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/priority_socket"
#define CLIENT_SOCKET_PATH "/tmp/client_socket"
#define BUFFER_SIZE 1024

void sig_handler(int sig) {
    printf("Received SIGPIPE\n");
}

int sck;
int count = 0;
int flag_receiver = 0;
int flag_sender = 0;
int flag_connect = 0;
pthread_t id_receiver;
pthread_t id_sender;
pthread_t id_connect;
struct sockaddr_un addr;
struct sockaddr_un addr_l;

static void* proc_snd() {
    printf("The request handling thread has started.\n");
    while (flag_sender != 1) {
        count++;
        if (send(sck, &count, sizeof(int), 0) == -1) {
            perror("send");
            break;
        }
        printf("Request %d is sent\n", count);
        sleep(1);
    }
    printf("The request handling thread has been closed.\n");
    return NULL;
}

static void* proc_rcv() {
    printf("The answer receiving thread has started.\n");
    while (flag_receiver != 1) {
        char recvbuf[BUFFER_SIZE];
        int reccount = recv(sck, recvbuf, BUFFER_SIZE, 0);
        if (reccount == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Ресурс временно недоступен, ждем немного и пробуем снова
                sleep(1);
                continue;
            } else {
                perror("recv");
                sleep(1);
            }
        } else if (reccount == 0) {
            printf("Disconnected...\n");
            flag_receiver = 1;
            flag_sender = 1;
        } else {
            recvbuf[reccount] = '\0'; // Убедитесь, что строка завершается нулевым символом
            printf("Received response: %s\n", recvbuf);
        }
    }
    printf("The answer receiving thread has been closed.\n");
    return NULL;
}

static void* proc_conn() {
    printf("The connection creating thread has started.\n");
    while (flag_connect != 1) {
        int result = connect(sck, (struct sockaddr*)&addr, sizeof(addr));
        if (result == -1) {
            perror("connect");
            sleep(1);
        } else {
            printf("Connected to server at %s\n", addr.sun_path);

            struct sockaddr_un client_addr;
            socklen_t client_len = sizeof(client_addr);
            getsockname(sck, (struct sockaddr*)&client_addr, &client_len);
            printf("Client socket address: %s\n", client_addr.sun_path);

            pthread_create(&id_receiver, NULL, proc_rcv, NULL);
            pthread_create(&id_sender, NULL, proc_snd, NULL);
            break;
        }
    }
    printf("The connection creating thread has been closed!\n");
    return NULL;
}

int main() {
    printf("The client has started working.\n");
    signal(SIGPIPE, sig_handler);

    sck = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sck == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    memset(&addr_l, 0, sizeof(addr_l));
    addr_l.sun_family = AF_UNIX;
    strncpy(addr_l.sun_path, CLIENT_SOCKET_PATH, sizeof(addr_l.sun_path) - 1);

    unlink(CLIENT_SOCKET_PATH);

    fcntl(sck, F_SETFL, O_NONBLOCK);

    if (bind(sck, (struct sockaddr*)&addr_l, sizeof(addr_l)) == -1) {
        perror("bind");
        close(sck);
        return 1;
    }

    pthread_create(&id_connect, NULL, proc_conn, NULL);

    printf("The program is waiting for a key to be pressed.\n");
    getchar();
    printf("The key is pressed.\n");

    flag_receiver = 1;
    flag_sender = 1;
    flag_connect = 1;

    pthread_join(id_sender, NULL);
    pthread_join(id_receiver, NULL);
    pthread_join(id_connect, NULL);

    if (sck != -1) {
        shutdown(sck, 2);
        close(sck);
    }
    unlink(CLIENT_SOCKET_PATH);

    printf("The client is closed.\n");
    return 0;
}