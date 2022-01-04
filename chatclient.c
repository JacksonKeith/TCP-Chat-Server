#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>

#define SIZE 2048

char message[SIZE];

void replaceString(char *message) {
    char temp[1024] = {0};
    char *insert = &temp[0];
    const char *messagePointer = message;
    char *replacements[4] = {":)", ":(", ":mytime", ":+1hr"};
    while (1) {
        int i;
        char* flag;
        for (i = 0; i < 4; i++) {
            flag = strstr(messagePointer, replacements[i]);
            if (flag != NULL) {
                break;
            }
        }

        if (flag == NULL) {
            strcpy(insert, messagePointer);
            break;
        }
        memcpy(insert, messagePointer, flag - messagePointer);
        insert += flag - messagePointer;

        if (i == 0) {
            memcpy(insert, "[feeling happy]", strlen("[feeling happy]"));
            insert += strlen("[feeling happy]");
            messagePointer = flag + strlen(":)");
        } else if (i == 1) {
            memcpy(insert, "[feeling sad]", strlen("[feeling sad]"));
            insert += strlen("[feeling sad]");
            messagePointer = flag + strlen(":(");
        } else if (i == 2) {
            time_t mytime;
            struct tm * timeinfo;
            time(&mytime);
            timeinfo = localtime(&mytime);
            char string[1024];
            sprintf(string, "%d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
            memcpy(insert, string, strlen(string));
            insert += strlen(string);
            messagePointer = flag + strlen(":mytime");
        } else {
            time_t mytime;
            struct tm * timeinfo;
            time(&mytime);
            timeinfo = localtime(&mytime);
            char string[1024];
            int hourtime = timeinfo->tm_hour + 1;
            if (hourtime > 23) {
                hourtime = 0;
            }
            sprintf(string, "%d:%d:%d", hourtime, timeinfo->tm_min, timeinfo->tm_sec);
            memcpy(insert, string, strlen(string));
            insert += strlen(string);
            messagePointer = flag + strlen(":+1hr");
        }
    }
    strcpy(message, temp);
}

void *recieve_message(void *socket_pointer) {
    int n;
    int sock = *(int *) socket_pointer;
    while ((n = recv(sock, message, SIZE - 1, 0)) > 0) {
        message[n] = '\0';
        fputs(message, stdout);
    }
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int sock, n;
    struct hostent *server;
    struct sockaddr_in server_address;
    pthread_t thread_id;
    char buffer[SIZE];
    char* password = "cs3251secret";
    char* name = argv[3];
    if(argc < 5){
        printf("Please input correct command line args\n");
        exit(1);
    }
    if (strcmp(argv[4], password) != 0) {
        printf("You have entered an incorrect password! ACCESS DENIED!\n");
        exit(1);
    }
    for (int i = 0; i < strlen(name); i++) {
        if (!isalnum(name[i])) {
            printf("Your name must be alphanumeric! ACCESS DENIED!\n");
            exit(1);
        }
    }

    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 65535) {
        printf("Please input valid port number\n");
        exit(1);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    if ((server = gethostbyname(argv[1])) == NULL) {
        printf("Error finding host\n");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    bcopy((char*)server->h_addr, (char*)&server_address.sin_addr.s_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Error connecting\n");
        exit(1);
    }
    if (pthread_create(&thread_id, NULL, recieve_message, (void*) &sock) < 0) {
        printf("Error creating thread\n");
        exit(1);
    }
    printf("~~~ You have entered the Chat Room ~~~\n");
    write(sock, name, strlen(name));
    while (fgets(message, SIZE - 1, stdin)) {
        if (strlen(message) > 1024) {
            printf("Message too long! Resend shorter message.\n");
            memset(message, 0, SIZE);
            continue;
        }
        if (strstr(message, ":Exit") != NULL) {
            break;
        }
        replaceString(message);
        strcpy(buffer, name);
        strcat(buffer, ": ");
        strcat(buffer, message);
        printf("%s", buffer);
        n = write(sock, buffer, strlen(buffer));
        if (n < 0) {
            printf("Error sending message\n");
            break;
        }
    }
    printf("You have been disconnected\n");
    exit(1);
    pthread_exit(NULL);
    close(sock);
    return 0;
}
