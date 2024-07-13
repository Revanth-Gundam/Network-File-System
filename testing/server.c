#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 4444
#define MAX_CLIENTS 10

void *handleClient(void *arg)
{
    int clientSocket = *((int *)arg);
    char buffer[1024];
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        // Send a confirmation message to the client
        send(clientSocket, "Hi client", strlen("Hi client"), 0);

        sleep(3);
        // Handle other communication or tasks with the client here
    }
    close(clientSocket);
    pthread_exit(NULL);
}

int main()
{
    int sockfd, ret;
    struct sockaddr_in serverAddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error in socket");
        exit(EXIT_FAILURE);
    }

    printf("Server Socket is created.\n");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sockfd, MAX_CLIENTS) == 0)
    {
        printf("Listening...\n\n");
    }
    else
    {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    int cnt = 0;
    pthread_t thread[MAX_CLIENTS];

    while (1)
    {
        int clientSocket;
        struct sockaddr_in cliAddr;
        socklen_t addr_size = sizeof(cliAddr);

        // Accept client connection
        clientSocket = accept(sockfd, (struct sockaddr *)&cliAddr, &addr_size);
        if (clientSocket < 0)
        {
            perror("Error in accepting");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n",
               inet_ntoa(cliAddr.sin_addr),
               ntohs(cliAddr.sin_port));

        printf("Clients connected: %d\n\n", ++cnt);

        // Create a thread to handle the client
        if (pthread_create(&thread[cnt - 1], NULL, handleClient, &clientSocket) != 0)
        {
            perror("Error in creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // Close the server socket
    close(sockfd);

    return 0;
}