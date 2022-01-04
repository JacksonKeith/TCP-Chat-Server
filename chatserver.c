#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define SIZE 2048

char users[64];

pthread_mutex_t lock;
int currClients = 0;

struct thread_arg {
    int sock;
    int id;
};

void *init_client(void *clientSock) {
    struct thread_arg *args = (struct thread_arg *) clientSock;
    int sock = args->sock;
    int id = args->id;
    char buffer[SIZE];
    int n;
    char name[64];
    FILE* fp;
    pthread_mutex_lock(&lock);

    fp = fopen("serveroutput.txt", "w");
    fprintf(fp, "Server Output Log Start!\n");
    fclose(fp);

    recv(sock, name, SIZE - 1, 0);

    printf("%s has joined the server\n", name);

    fp = fopen("serveroutput.txt", "ab");
    fprintf(fp, "%s has joined the server\n", name);
    fclose(fp);

    sprintf(buffer, "%s has joined\n", name);
    for (int i = 0; i < currClients; i++) {
            if (users[i] != sock && users[i] != -1) {
                if (send(users[i], buffer, strlen(buffer),0) < 0) {
                    printf("Error sending message\n");
                    continue;
                }
            }
    }
    memset(buffer, 0, SIZE);
    pthread_mutex_unlock(&lock);

    // Read from buffer
    while((n = recv(sock, buffer, SIZE - 1, 0)) > 0) {
        buffer[n] = '\0';
        pthread_mutex_lock(&lock);
        printf("%s", buffer);

        fp = fopen("serveroutput.txt", "ab");
        fprintf(fp, "%s", buffer);
        fclose(fp);

        for (int i = 0; i < currClients; i++) {
            if (users[i] != sock && users[i] != -1) {
                if (send(users[i], buffer, strlen(buffer),0) < 0) {
                    printf("Error sending message\n");
                    continue;
                }
            }
        }
        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    printf("%s has left the server\n", name);

    fp = fopen("serveroutput.txt", "ab");
    fprintf(fp, "%s has left the server\n", name);
    fclose(fp);

    users[id - 1] = -1;
    memset(buffer, 0, SIZE);
    sprintf(buffer, "%s has left the server\n", name);
    for (int i = 0; i < currClients; i++) {
            if (users[i] != sock && users[i] != -1) {
                if (send(users[i], buffer, strlen(buffer),0) < 0) {
                    printf("Error sending message\n");
                    continue;
                }
            }
    }
    pthread_mutex_unlock(&lock);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sock, clientSock;
    struct sockaddr_in server_address;
    pthread_t thread_id;

    if(argc < 2){
        printf("Please input correct command line args\n");
        exit(1);
    }
    if ((atoi(argv[1]) < 0) || (atoi(argv[1]) > 65535)) {
        printf("Please input valid port number\n");
        exit(1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1]));

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Error binding\n");
        exit(1);
    }
    if (listen(sock, 64) < 0) {
        printf("Error listening\n");
        exit(1);
    }

    printf("Chatserver start!\n");
    while(1) {
        clientSock = accept(sock, (struct sockaddr *)NULL, NULL);
        if (clientSock < 0) {
            printf("Error accepting client\n");
            exit(1);
        }
        pthread_mutex_lock(&lock);
        struct thread_arg args;
        users[currClients] = clientSock;
        currClients++;
        args.sock = clientSock;
        args.id = currClients;
        if (currClients > 63) {
            printf("Error: Too many clients\n");
            exit(1);
        }
        if (pthread_create(&thread_id, NULL, init_client, &args) < 0) {
            printf("Error creating thread\n");
            exit(1);
        }
        pthread_mutex_unlock(&lock);
    }
    pthread_join(thread_id, NULL);
    pthread_mutex_destroy(&lock);
    close(sock);
    return 0;
}
