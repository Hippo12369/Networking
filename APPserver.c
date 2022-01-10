#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

// User details
struct socket_node
{
    int flag; // Flag to represent user's connectivity
    int user_id;
    int socket;
    char name[30];
    struct socket_node *next;
} *head = NULL;

// Function to append details of clients
struct socket_node *socket_append(int socket, char *name)
{
    static int ID = 1000;

    struct socket_node *node = (struct socket_node *)malloc(sizeof(struct socket_node));
    node->socket = socket; //client's socket
    node->next = NULL;
    node->user_id = ++ID; // client's ID
    node->flag = 1;       // connectivity

    //client's name
    int i;
    for (i = 0; name[i] != '\0'; i++)
    {
        node->name[i] = name[i];
    }
    node->name[i] = '\0';

    //appending node
    if (head == NULL)
    {
        head = node;
        return node;
    }

    struct socket_node *curr = head;
    while (curr->next)
    {
        curr = curr->next;
    }
    curr->next = node;
    return node;
}

// Function to convert int to string
char *itos(int num)
{
    char *a = (char *)malloc(10 * sizeof(char));
    sprintf(a, "%d", num);
    return a;
}

// reciever thread for each client
void *reciever(void *args)
{

    int *socket_array = (int *)args;
    int client_socket = *socket_array;       //client socket
    int server_socket = *(socket_array + 1); // server socket

    // getting user details
    struct socket_node *user_info = head;
    while (user_info)
    {
        if (user_info->socket == client_socket)
            break;
        user_info = user_info->next;
    }

    // User connected...
    printf("SERVER: %s#%d has connected...", user_info->name, user_info->user_id);

    char message[256];
    char *cls = "\\close";
    while (1)
    {

        printf("\n");

        // recieve message from the client
        recv(client_socket, &message, sizeof(message), 0);
        if (strcmp(message, cls) == 0)
        {
            printf("SERVER: %s#%d has disconnected...\n", user_info->name, user_info->user_id);
            user_info->flag = 0;

            return NULL;
        }

        // Display client's message on the server side
        printf("%s#%d: %s", user_info->name, user_info->user_id, message);

        // Generating message to send to other users ****************
        char message2[300];
        int i = 0, j = 0;
        for (i; user_info->name[i] != '\0'; i++)
        {
            message2[i] = user_info->name[i];
        }
        message2[i++] = '#';
        char *id = itos(user_info->user_id);
        for (; id[j] != '\0'; j++)
        {
            message2[i++] = id[j];
        }
        message2[i++] = ':';
        message2[i++] = ' ';
        for (j = 0; message[j] != '\0'; j++)
        {
            message2[i++] = message[j];
        }
        message2[i] = '\0';
        // **********************************************************

        // fprintf(fp, "%s\n", message2);

        // Sending generated message to every other client.
        struct socket_node *curr = head;
        while (curr != NULL)
        {
            if (curr->socket != client_socket && curr->flag == 1)
            {
                send(curr->socket, &message2, strlen(message2) + 1, 0);
            }
            curr = curr->next;
        }
    }
}

// thread to listen connection requests
void *listener(void *args)
{

    // FILE *fp;
    // fp = fopen("Audit_Log.txt","w");
    // if(fp == NULL){
    //     printf("Audit_Log.txt falied to open");
    // }

    int server_socket = *(int *)args;

    char message[256];
    char connection[] = "Connection Established..."; //connection ACK
    char close_server[] = "\\close_server";          //close server command

    while (1)
    {
        listen(server_socket, 2);

        int client_socket;
        client_socket = accept(server_socket, NULL, NULL); //accept connection

        //Send successful connection ACK
        send(client_socket, &connection, sizeof(connection), 0);

        // Recieve name of the client
        recv(client_socket, &message, sizeof(message), 0);

        if (strcmp(message, close_server) == 0)
        {
            printf("Shutting SERVER...\n");
            return NULL;
        }

        // Making node to store user info
        struct socket_node *node = socket_append(client_socket, message);

        int socket_array[2];
        socket_array[0] = client_socket;
        socket_array[1] = server_socket;

        //thread to recieve message from every user
        pthread_t th1;
        pthread_create(&th1, NULL, reciever, (void *)&socket_array);
    }
}

int main()
{
    // Server Socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        printf("Socket creation failed!\n");
    }

    //Server Address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    pthread_t th1;
    pthread_create(&th1, NULL, listener, (void *)&server_socket);
    pthread_join(th1, NULL);

    // fclose(fp);
    close(server_socket);

    return 0;
}