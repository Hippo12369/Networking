#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

// Reciever to recieve server's message
void *reciever(void *args)
{
    int client_socket = *(int *)args;
    char message[256];
    sleep(1);

    while (1)
    {
        recv(client_socket, &message, sizeof(message), 0);
        printf("%s\n", message);
    }
}

int main()
{

    // Client Socket
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Server Address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (connection_status == -1)
    {
        printf("Connection Falied!\n");
        exit(0);
    }

    char message[256];
    recv(client_socket, &message, sizeof(message), 0);

    printf("%s\n", message);

    pthread_t th1;
    pthread_create(&th1, NULL, reciever, (void *)&client_socket);

    // Client's name
    char name[30];
    printf("Enter your name: ");
    scanf("%[^\n]%*c", name);
    int send_error = send(client_socket, &name, strlen(name) + 1, 0);

    // if(send_error == -1){
    //     printf("Connection Lost!");
    //     close(client_socket);
    //     exit(0);
    // }

    char close_server[] = "\\close_server";
    char *cls = "\\close";

    if (strcmp(name, close_server) != 0)
        while (1)
        {
            // printf("You: ");
            scanf("%[^\n]%*c", message);
            send(client_socket, &message, strlen(message) + 1, 0);
            if (strcmp(message, cls) == 0)
            {
                printf("You've been disconnected!\n");
                break;
            }
        }

    close(client_socket);
    return 0;
}