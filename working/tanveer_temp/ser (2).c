#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include "try.c"

#define PORT 4454
#define CPORT 4439
#define MAX_CLIENTS 10
#define MAX_SS 5
#define NUM_INIT_SS 2
#define ALPHABET_SIZE 26
#define MAX_PATH_LENGTH 51
#define LRU_CACHE_SIZE 5

int cnt, ss = 2;
struct Trie *trie;

// LRU implementation
struct LRU_Node
{
    struct Paths *data;
    struct LRU_Node *next;
};

struct LRU_Cache
{
    struct LRU_Node *head;
    int count;
};

// Function to initialize a new LRU cache
struct LRU_Cache *createLRUCache()
{
    struct LRU_Cache *newCache = malloc(sizeof(struct LRU_Cache));
    if (newCache != NULL)
    {
        newCache->head = NULL;
        newCache->count = 0;
    }
    return newCache;
}

// Function to search for a Paths struct in the LRU cache
struct Paths *searchLRUCache(struct LRU_Cache *cache, const char *path)
{
    struct LRU_Node *currentNode = cache->head;
    while (currentNode != NULL)
    {
        if (strcmp(currentNode->data->path, path) == 0)
        {
            // Move the accessed path to the front of the LRU cache
            if (currentNode != cache->head)
            {
                struct LRU_Node *temp = currentNode;
                cache->head = currentNode->next;
                temp->next = NULL;
                currentNode->next = cache->head;
                cache->head = temp;
            }
            return currentNode->data;
        }
        currentNode = currentNode->next;
    }

    // If the path is not found in the LRU cache, return NULL
    return NULL;
}

// Function to insert a Paths struct into the LRU cache
void insertLRUCache(struct LRU_Cache *cache, struct Paths *path)
{
    if (cache->count == LRU_CACHE_SIZE)
    {
        // Remove the least recently used path
        struct LRU_Node *temp = cache->head;
        while (temp->next->next != NULL)
        {
            temp = temp->next;
        }
        struct LRU_Node *last = temp->next;
        temp->next = NULL;
        free(last);
        cache->count--;
    }

    // Insert the new path at the front of the LRU cache
    struct LRU_Node *newNode = malloc(sizeof(struct LRU_Node));
    newNode->data = path;
    newNode->next = cache->head;
    cache->head = newNode;
    cache->count++;
}

// int copy(char *path1, char *path2)
// {
//     char currentDir[100];
//     if (getcwd(currentDir, sizeof(currentDir)) == NULL)
//     {
//         // perror("Error getting current working directory");
//         return -3;
//     }

//     char temp[100];
//     strcpy(temp, path1);
//     // printf("path1: %s\n", path1);
//     char *token = strtok(temp, "/");
//     char fileName[100];
//     while (token != NULL)
//     {
//         strcpy(fileName, token);
//         token = strtok(NULL, "/");
//     }
//     // printf("fileName: --%s--\n", fileName);
//     // fileName[strlen(fileName) - 1] = '\0';
//     // printf("fileName: --%s--\n", fileName);
//     char *lastSlash = strrchr(path1, '/');
//     *lastSlash = '\0';
//     // printf("new path: --%s--\n", path1);
//     if (strcmp(path1, "") != 0)
//     {
//         if (chdir(path1) != 0)
//         {
//             // printf("problem here\n");
//             // perror("Error changing directory");
//             return -2;
//         }
//     }

//     // if (chdir(path1) != 0)
//     // {
//     //     perror("Error changing directory");
//     //     return -2;
//     // }

//     // copy the file from there
//     FILE *fp1, *fp2;
//     char ch;
//     fp1 = fopen(fileName, "r");
//     if (fp1 == NULL)
//     {
//         // perror("Error opening file");
//         // get back to prev directory
//         if (chdir(currentDir) != 0)
//         {
//             // printf("Problem is actually here\n");
//             // perror("Error changing directory");
//             return -2;
//         }
//         return -1;
//     }
//     // get back to the original directory
//     // printf("currentDir: %s\n", currentDir);
//     if (chdir(currentDir) != 0)
//     {
//         // printf("Problem is actually here\n");
//         // perror("Error changing directory");
//         return -2;
//     }
//     // print the actual current directory
//     if (getcwd(currentDir, sizeof(currentDir)) == NULL)
//     {
//         // perror("Error getting current working directory");
//         return -3;
//     }
//     // printf("currentDir: --%s--\n", currentDir);
//     // change directory to path2
//     // printf("path2: --%s--\n", path2);
//     // there is a newline character at the end of path2, remove it
//     path2[strlen(path2) - 1] = '\0';
//     // printf("path2: --%s--\n", path2);
//     // if(strcpy(path2, "") != 0)
//     // {
//     //     if (chdir(path2) != 0)
//     //     {
//     //         printf("Problem is actually here\n");
//     //         // perror("Error changing directory");
//     //         return -2;
//     //     }
//     // }

//     if (strcmp(path2, "") != 0)
//     {
//         char *tempPath2 = strdup(path2);
//         // printf("path2: --%s--\n", tempPath2);
//         if (chdir(tempPath2) != 0)
//         {
//             // printf("Problem is actually here\n");
//             free(tempPath2); // Free the memory allocated by strdup
//             return -2;
//         }
//         free(tempPath2); // Free the memory allocated by strdup
//     }

//     // create the file there
//     fp2 = fopen(fileName, "w");
//     if (fp2 == NULL)
//     {
//         // perror("Error opening file");
//         // get back to prev directory
//         if (chdir(currentDir) != 0)
//         {
//             // printf("Problem is surprisingly here\n");
//             // perror("Error changing directory");
//             return -2;
//         }
//         return -1;
//     }
//     while ((ch = fgetc(fp1)) != EOF)
//     {
//         fputc(ch, fp2);
//     }
//     // get back to the original directory
//     // printf("currentDir: %s\n", currentDir);
//     if (chdir(currentDir) != 0)
//     {
//         // printf("Problem is surprisingly here\n");
//         // perror("Error changing directory");
//         return -2;
//     }
//     fclose(fp1);
//     fclose(fp2);
//     return 0;
// }

struct StorageServer
{
    char ip[16];
    int port;
    int cport;
    int socket;
    int alive;
    char wd[100];
};

struct StorageServer SS[MAX_SS];

void connectToSS()
{
    for (int i = 0; i < MAX_SS; ++i)
    {
        // Create socket
        SS[i].socket = socket(AF_INET, SOCK_STREAM, 0);
        if (SS[i].socket < 0)
        {
            perror("Error in socket");
            exit(EXIT_FAILURE);
        }

        // Set up server address structure
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SS[i].port);
        serverAddr.sin_addr.s_addr = inet_addr(SS[i].ip);

        // Connect to storage server
        if (connect(SS[i].socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            perror("Error in connecting to storage server");
            exit(EXIT_FAILURE);
        }

        printf("Connected to Storage Server %d at %s:%d\n", i + 1, SS[i].ip, SS[i].port);
    }
}

void *Dyna_SS(void *arg)
{
    int sockfd = *((int *)arg);
    struct sockaddr_in cliAddr;
    socklen_t addr_size = sizeof(cliAddr);
    char message[] = "Hello Storage Server!\n";
    // Accept client connection
    SS[ss].socket = accept(sockfd, (struct sockaddr *)&cliAddr, &addr_size);
    printf("HERE\n");
    if (SS[ss].socket < 0)
    {
        perror("Error in accepting");
        exit(EXIT_FAILURE);
    }

    printf("Storage Server connection accepted from %s:%d\n",
           inet_ntoa(cliAddr.sin_addr),
           ntohs(cliAddr.sin_port));

    send(SS[ss].socket, message, strlen(message), 0);
    // receive the port number, the cport number and the ip address from the storage server as a single string
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(SS[ss].socket, buffer, sizeof(buffer), 0);
    // tokenize the buffer to get the port number, the cport number and the ip address
    char *token = strtok(buffer, " ");
    SS[ss].port = atoi(token);
    token = strtok(NULL, " ");
    SS[ss].cport = atoi(token);
    token = strtok(NULL, " ");
    strcpy(SS[ss].ip, token);
    token = strtok(NULL, " ");
    strcpy(SS[ss].wd, token);
    printf("The port number is %d and the cport number is %d and the ip address is %s and the wokring directory is %s\n", SS[ss].port, SS[ss].cport, SS[ss].ip, SS[ss].wd);
    // send the message "Enter the list of accessible paths and respond with <STOP> when done" to the storage server
    char message1[] = "Enter the list of accessible paths and respond with <STOP> when done";
    send(SS[ss].socket, message1, strlen(message1), 0);
    // continue to receive the paths from the storage server until the storage server sends the message "STOP"
    while (strcmp(buffer, "STOP") != 0)
    {
        memset(buffer, 0, sizeof(buffer));
        recv(SS[ss].socket, buffer, sizeof(buffer), 0);
        if (strcmp(buffer, "STOP") != 0)
        {
            insert(trie, buffer, ss);
        }
        printf("%s\n", buffer);
    }
    ss++;
    return NULL;
}

void *handleClient(void *arg)
{
    int clientSocket = *((int *)arg);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Send a confirmation message to the client
    send(clientSocket, "Hi client", strlen("Hi client"), 0);

    while (1)
    {
        // Receive data from the client
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0); // rn buffer contains the command
        if (bytesRead <= 0)
        {
            // Either the client disconnected or an error occurred
            // if (bytesRead == 0)
            // {
            // Client disconnected
            printf("Client disconnected 1\n");
            cnt--;
            // }
            // else
            // {
            //     perror("Error receiving data from client");
            // }
            // break;
            pthread_exit(NULL);
            return NULL;
        }

        // Process received data or perform other tasks with the client
        printf("Received message from client: %s\n", buffer); // echo the command you just got from the client in full

        // buffer has the data recieved from the client
        // tokenize the buffer to get the command and the path, sometimes 1 path is given but sometimes 2 paths are given
        // if 1 path is given, then the command is either "read" or "delete" or "create" or "append"
        // if 2 paths are given, then the command is "copy and paste"
        char *token = strtok(buffer, " ");
        if (token == NULL)
        {
            printf("No valid command given.\n");
            return NULL;
        }

        char command[10];
        char path1[100];
        char path2[100];
        strcpy(command, token);
        printf("command: --%s--\n", command);
        // if (strcmp(command, "read") == 0) // doesn't work rn
        // {
        //     printf("user called read\n");
        //     token = strtok(NULL, " ");
        //     strcpy(path1, token);
        //     // query for the path from the trie
        //     // struct Paths *result = searchPath(trie, path1);
        //     // we have the path and the storage server number
        //     // to the client, send back the storage server number

        //     send(clientSocket, "9990", strlen("0") + 1, 0);
        // }
        // else if (strcmp(command, "write") == 0) // doesn't work rn
        // {
        //     token = strtok(NULL, " ");
        //     strcpy(path1, token);
        // }

        if (strcmp(command, "read") == 0 || strcmp(command, "write") == 0 || strcmp(command, "get_info") == 0)
        {
            // printf("\nYUP\n");
            token = strtok(NULL, " ");
            strcpy(path1, token);
            printf("path1: --%s--\n", path1);

            if (path1[strlen(path1) - 1] == '\n')
            {
                path1[strlen(path1) - 1] = '\0';
            }
            // send the path to a search for storage server function which will return the storage server number
            // assume for now it is the first storage server
            // int ss = 0;
            int ss = search(trie, path1);
            printf("ss: %d\n", ss);

            int ss_port = 0;
            // get the cport of the ss
            ss_port = SS[ss].cport;

            printf("ss_port: %d\n", ss_port);

            // send ss_port to the client
            char ss_string[10];
            memset(ss_string, 0, sizeof(ss_string));
            sprintf(ss_string, "%d", ss_port);
            send(clientSocket, ss_string, strlen(ss_string), 0);

            // xint ss = 0;
            // send the ss to client
            // char ss_string[10];
            // memset(ss_string, 0, sizeof(ss_string));
            // sprintf(ss_string, "%d", ss);
            // send(clientSocket, ss_string, strlen(ss_string), 0);

            // send the command and path to the storage server together
            char message[100];
            strcpy(message, command);
            strcat(message, " ");
            strcat(message, path1);
            send(SS[ss].socket, message, strlen(message), 0);
            printf("The port of the storage server is %d\n", SS[ss].port);
        }

        else if (strcmp(command, "create") == 0)
        {
            token = strtok(NULL, " ");
            strcpy(path1, token); // this is the path including the actual file name to be created

            // check if there is a newline character in path1. if there is, remove it
            if (path1[strlen(path1) - 1] == '\n')
            {
                path1[strlen(path1) - 1] = '\0';
            }

            char path_to_find_ss[100]; // remove the file name, so that you can search for the place in which you put the new file in the trie
            strcpy(path_to_find_ss, path1);

            char *lastSlash = strrchr(path_to_find_ss, '/');
            if (*(lastSlash - 1) == '.')
                *(lastSlash + 1) = '\0'; // this was giving error, so handling this edge case
            else
                *(lastSlash) = '\0'; // the general case

            printf("path_to_find_ss: %s\n", path_to_find_ss);

            // look for that path in the trie and get the storageNumber
            int ss = search(trie, path_to_find_ss);
            if (ss < 0) // if the path is not found in the trie, then send an error message to the client
            {
                printf("Path does not exist or is inaccessible.\n");

                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "Path does not exist or is inaccessible.\n");
                send(clientSocket, buffer, strlen(buffer), 0);
                continue;
            }

            char message[1024];
            memset(message, 0, sizeof(message));
            strcpy(message, command);
            strcat(message, " ");
            strcat(message, path1);
            printf("Sent message to storage server %d: %s\n", ss, message);
            send(SS[ss].socket, message, strlen(message), 0); // tell the storage server what the command is and what the path is

            // recieve the message from the storage server
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(SS[ss].socket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0)
            {
                // Either the client disconnected or an error occurred
                // if (bytesRead == 0)
                // {
                // Client disconnected
                printf("Client disconnected 2\n");
                cnt--;
                // }
                // else
                // {
                //     perror("Error receiving data from client");
                // }
                // break;
                pthread_exit(NULL);
                return NULL;
            }

            // print the message
            printf("Received message from storage server %d: %s\n", ss, buffer);

            if (strcmp(buffer, "File/folder created successfully!") == 0)
            {
                insert(trie, path1, ss);
                printf("Path %s inserted into trie\n", path1);
            }
            // send the message from the storage server to the respective client - communicating success or failure
            send(clientSocket, buffer, strlen(buffer), 0);
        }

        else if (strcmp(command, "delete") == 0)
        {
            token = strtok(NULL, " ");
            strcpy(path1, token); // this is the path including the file name to be deleted

            char path_to_find_ss[100];
            strcpy(path_to_find_ss, path1);

            char *lastSlash = strrchr(path_to_find_ss, '/');

            if (*(lastSlash - 1) == '.')
                *(lastSlash + 1) = '\0';
            else
                *(lastSlash) = '\0';

            printf("path_to_find_ss: %s\n", path_to_find_ss);

            // look for that path in the trie and get the storageNumber
            int ss = search(trie, path_to_find_ss);
            if (ss < 0)
            {
                printf("Could not find the right storage server.\n");
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "Could not find the right storage server.\n");
                send(clientSocket, buffer, strlen(buffer), 0);
                continue;
            }

            // send the command and path to the storage server together
            char message[100];
            strcpy(message, command);
            strcat(message, " ");
            strcat(message, path1);
            send(SS[ss].socket, message, strlen(message), 0);

            // recieve the message from the storage server
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytesRead = recv(SS[ss].socket, buffer, sizeof(buffer), 0);

            if (bytesRead <= 0)
            {
                // Either the client disconnected or an error occurred
                // if (bytesRead == 0)
                // {
                // Client disconnected
                printf("Client disconnected 3\n");
                cnt--;
                // }
                // else
                // {
                //     perror("Error receiving data from client");
                // }
                pthread_exit(NULL);
                return NULL;
            }

            // print the message
            printf("Received message from storage server %d: %s\n", ss, buffer);

            if (strcmp(buffer, "File/folder deleted successfully!") == 0)
            {
                // delete the path from the trie
                deletion(&trie, path1);
                printf("Path deleted from trie\n");
            }
            // send the buffer message to the respective client - communicating success or failure to client
            send(clientSocket, buffer, strlen(buffer), 0);
        }

        else if (strcmp(command, "path") == 0)
        {
            // printf("\nYUP\n");
            token = strtok(NULL, " ");
            strcpy(path1, token);
            // send the path to a search for storage server function which will return the storage server number
            // assume for now it is the first storage server
            int ss = search(trie, path1);
            if (ss < 0)
            {
                printf("Could not find the right storage server.\n");
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "Could not find the right storage server.\n");
                send(clientSocket, buffer, strlen(buffer), 0);
                continue;
            }
            // int ss = 0;
            // send the command and path to the storage server together
            char message[100];
            strcpy(message, command);
            strcat(message, " ");
            strcat(message, path1);
            send(SS[ss].socket, message, strlen(message), 0);
            printf("The port of the storage server is %d\n", SS[ss].port);
        }
        else if (strcmp(command, "copy") == 0)
        {
            // printf("Copy command received\n");
            //  strcpy(buffer, "Enter 1 if you want to copy a file and 2 if you want to copy a folder\n");
            //  printf("Sending\n");
            //  send(clientSocket, buffer, strlen(buffer), 0);
            //  printf("Sent");
            //  //receive the choice
            //  memset(buffer, 0, sizeof(buffer));
            //  recv(clientSocket, buffer, sizeof(buffer), 0);
            //  if(strcmp(buffer, "1")==0)
            //  {
            //      memset(buffer, 0, sizeof(buffer));
            //      strcpy(buffer, "Enter: <source path> <destination path>\n");
            //      send(clientSocket, buffer, strlen(buffer), 0);
            //  }
            char choice[10];
            token = strtok(NULL, " ");
            strcpy(choice, token);
            // printf("here %s\n", choice);
            if (strncmp(choice, "file", 4) == 0)
            {
                printf("processing file\n");
                // sending the prompt to enter the paths
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "Enter: <source path> <destination path>\n");
                send(clientSocket, buffer, strlen(buffer), 0);

                // receive paths
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                printf("Received: %s\n", buffer);
                // tokenize the buffer to get the paths
                token = strtok(buffer, " ");
                strcpy(path1, token);

                char path_to_find_ss[100];
                strcpy(path_to_find_ss, path1);

                char *lastSlash = strrchr(path_to_find_ss, '/');

                if (*(lastSlash - 1) == '.')
                    *(lastSlash + 1) = '\0';
                else
                    *(lastSlash) = '\0';
                // printf("\npath of src file: %s\n", path_to_find_ss);

                int ss1 = search(trie, path_to_find_ss);
                printf("ss1: %d\n", ss1);
                if (ss1 < 0)
                {
                    printf("IN SS1 <0");
                    // send path does not exist
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "Source path does not exist or is inaccessible.\n");
                    send(clientSocket, buffer, strlen(buffer), 0);
                }
                // store the difference between path1 and path_to_find_ss
                char path1_diff[100];
                strcpy(path1_diff, path1 + strlen(path_to_find_ss));
                // remove the first character from path1_diff
                strcpy(path1_diff, path1_diff + 1);

                token = strtok(NULL, " ");
                strcpy(path2, token);

                int ss2 = search(trie, path2);
                if (ss2 < 0)
                {
                    // send path does not exist
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "Destination path does not exist or is inaccessible.\n");
                    send(clientSocket, buffer, strlen(buffer), 0);
                }
                char path2backup[100];
                strcpy(path2backup, path2);

                memset(buffer, 0, sizeof(buffer));
                // remove the first two characters from the start of the path_to_ss string
                strcpy(buffer, path1 + 2);
                // concatenate SS[ss1].wd to the buffer
                memset(path1, 0, sizeof(path1));
                strcpy(path1, SS[ss1].wd);
                strcat(path1, "/");
                strcat(path1, buffer);
                printf("\nPath1: %s\n", path1);

                // same for the second path
                memset(buffer, 0, sizeof(buffer));
                // remove the first two characters from the start of the path_to_ss string
                strcpy(buffer, path2 + 2);
                // concatenate SS[ss1].wd to the buffer
                memset(path2, 0, sizeof(path2));
                strcpy(path2, SS[ss1].wd);
                strcat(path2, "/");
                strcat(path2, buffer);
                printf("\nPath2: %s\n", path2);

                pid_t pid = fork();

                if (pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0)
                { // Child process
                    // Use execlp to execute the cp command
                    execlp("cp", "cp", path1, path2, NULL);

                    // If execlp fails
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }
                else
                { // Parent process
                    // Wait for the child process to complete
                    int status;
                    waitpid(pid, &status, 0);

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "File copied successfully.\n");
                        //concatenate path1_diff to path2backup
                        strcat(path2backup, path1_diff);
                        insert(trie,path2backup,ss2);
                        send(clientSocket, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        fprintf(stderr, "Error copying file.\n");
                    }
                }
            }
            if (strncmp(choice, "folder", 6) == 0)
            {
                printf("processing file\n");
                // sending the prompt to enter the paths
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "Enter: <source path> <destination path>\n");
                send(clientSocket, buffer, strlen(buffer), 0);

                // receive paths
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                printf("Received: %s\n", buffer);
                // tokenize the buffer to get the paths
                token = strtok(buffer, " ");
                strcpy(path1, token);

                int ss1 = search(trie, path1);
                printf("ss1: %d\n", ss1);
                if (ss1 < 0)
                {
                    printf("IN SS1 <0");
                    // send path does not exist
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "Source path does not exist or is inaccessible.\n");
                    send(clientSocket, buffer, strlen(buffer), 0);
                }

                token = strtok(NULL, " ");
                strcpy(path2, token);

                int ss2 = search(trie, path2);
                if (ss2 < 0)
                {
                    // send path does not exist
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, "Destination path does not exist or is inaccessible.\n");
                    send(clientSocket, buffer, strlen(buffer), 0);
                }
                char path2copy[100], dir_name[20];

                strcpy(path2copy,path2);
                // get everything after the last slash from path2 and store in dir_name
                char *lastSlash = strrchr(path2copy, '/');
                strcpy(dir_name, lastSlash + 1);

                //concatenate / to path2backup
                strcat(path2copy, "/");
                //concatenate dir_name to path2backup
                strcat(path2copy, dir_name);

                memset(buffer, 0, sizeof(buffer));
                // remove the first two characters from the start of the path_to_ss string
                strcpy(buffer, path1 + 2);
                // concatenate SS[ss1].wd to the buffer
                memset(path1, 0, sizeof(path1));
                strcpy(path1, SS[ss1].wd);
                strcat(path1, "/");
                strcat(path1, buffer);
                printf("\nPath1: %s\n", path1);

                // same for the second path
                memset(buffer, 0, sizeof(buffer));
                // remove the first two characters from the start of the path_to_ss string
                strcpy(buffer, path2 + 2);
                // concatenate SS[ss1].wd to the buffer
                memset(path2, 0, sizeof(path2));
                strcpy(path2, SS[ss1].wd);
                strcat(path2, "/");
                strcat(path2, buffer);
                printf("\nPath2: %s\n", path2);

                pid_t pid = fork();

                if (pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0)
                { // Child process
                    // Use execlp to execute the cp command
                    execlp("cp", "cp", "-r", path1, path2, NULL);

                    // If execlp fails
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }
                else
                { // Parent process
                    // Wait for the child process to complete
                    int status;
                    waitpid(pid, &status, 0);

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        // printf("Folder copied successfully.\n");
                        //  send the same message to the client
                        memset(buffer, 0, sizeof(buffer));
                        strcpy(buffer, "Folder copied successfully.\n");
                        insert(trie,path2copy,ss2);
                        send(clientSocket, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        fprintf(stderr, "Error copying folder.\n");
                    }
                }
            }
            // strcpy(path1, token);
            // token = strtok(NULL, " ");
            // if (token != NULL)
            // {
            //     strcpy(path2, token);
            // }
            // else
            // {
            //     printf("2 Paths need to be given\n");
            // }
            // send the path to a search for storage server function which will return the storage server number
            // let them be ss = 0 and ss = 1 for now
            // int ss1 = 0;
            // int ss2 = 1;
            // if (ss1 == -1)
            // {
            //     printf("Path 1 not found\n");
            // }
            // if (ss2 == -1)
            // {
            //     printf("Path 2 not found\n");
            // }
            // // assume the first path has the entire path and the file name while the second path has only path to where it needs to be copied
            // int ack = copy(path1, path2);
            // if (ack == 0)
            // {
            //     printf("Copy successful\n");
            // }
            // else if (ack == -1)
            // {
            //     printf("Error in file copy\n");
            // }
            // else if (ack == -2)
            // {
            //     printf("Error in directory changing\n");
            // }
            // else
            // {
            //     printf("error");
            // }
        }

        // // print the command and the paths
        // printf("Command: %s\n", command);
        // printf("Path 1: %s\n", path1);
        // printf("Path 2: %s\n", path2);

        // Clear the buffer for the next iteration
        memset(buffer, 0, sizeof(buffer));
    }

    // Close the client socket
    close(clientSocket);

    // Exit the thread
    pthread_exit(NULL);
}

void getIPAddress(char *ipBuffer, size_t bufferSize)
{
    char hostname[256];
    struct hostent *host_info;

    gethostname(hostname, sizeof(hostname));
    host_info = gethostbyname(hostname);

    if (host_info != NULL)
    {
        inet_ntop(AF_INET, host_info->h_addr_list[0], ipBuffer, bufferSize);
    }
    else
    {
        perror("Error getting IP address");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int sockfd, ret, clsockfd;
    struct sockaddr_in serverAddr, cliserAddr;
    char ip[16];
    getIPAddress(ip, sizeof(ip));
    // struct Trie *trie = getNewTrieNode();
    trie = getNewTrieNode();

    // initialize LRU using createLRUCache()
    // LRU_Cache = createLRUCache();
    // @Revanth, here shouldn't it be:
    // struct LRU_Cache *LRU_Cache = createLRUCache();
    // ??

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error in socket");
        exit(EXIT_FAILURE);
    }

    // create new socket for client connections
    clsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clsockfd < 0)
    {
        perror("Error in socket");
        exit(EXIT_FAILURE);
    }
    printf("Server Socket is created.\n");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    memset(&cliserAddr, 0, sizeof(cliserAddr));
    cliserAddr.sin_family = AF_INET;
    cliserAddr.sin_port = htons(CPORT);
    cliserAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }
    // bind the client socket
    ret = bind(clsockfd, (struct sockaddr *)&cliserAddr, sizeof(cliserAddr));
    if (ret < 0)
    {
        perror("Error in CL binding");
        exit(EXIT_FAILURE);
    }

    // for (int i = 0; i < MAX_SS; i++)
    // {
    //     printf("Enter the Port Number of Storage Server For NM Connection %d: ", i + 1);
    //     scanf("%d", &SS[i].port);

    //     printf("Enter the Port Number of Storage Server For Client Connection %d: ", i + 1);
    //     scanf("%d", &SS[i].cport);

    //     printf("Enter the IP Address of Storage Server %d: ", i + 1);
    //     scanf("%s", SS[i].ip);
    //     printf("\n");
    // }

    // connectToSS();

    // for (int i = 0; i < MAX_SS; ++i)
    // {
    //     char message[] = "Hello Storage Server!\n Enter your accessible paths and respond with <STOP> when done\n";
    //     send(SS[i].socket, message, strlen(message), 0);
    //     char buffer[1024];
    //     memset(buffer, 0, sizeof(buffer));
    //     while (strcmp(buffer, "STOP") != 0)
    //     {
    //         memset(buffer, 0, sizeof(buffer));
    //         recv(SS[i].socket, buffer, sizeof(buffer), 0);
    //         if (strcmp(buffer, "STOP") != 0)
    //         {
    //             insert(trie, buffer, i);
    //         }
    //         printf("%s\n", buffer);
    //     }
    // }

    // printf("Here\n");
    //  Listen for connections

    pthread_t one_more_thread;

    if (listen(sockfd, MAX_SS) == 0)
    {
        printf("Listening...\n\n");
    }
    else
    {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    char message[] = "Hello Storage Server!\n";

    for (int i = 0; i < NUM_INIT_SS; i++)
    {
        struct sockaddr_in cliAddr;
        socklen_t addr_size = sizeof(cliAddr);

        // Accept client connection
        SS[i].socket = accept(sockfd, (struct sockaddr *)&cliAddr, &addr_size);
        printf("HERE\n");
        if (SS[i].socket < 0)
        {
            perror("Error in accepting");
            exit(EXIT_FAILURE);
        }

        printf("Storage Server connection accepted from %s:%d\n",
               inet_ntoa(cliAddr.sin_addr),
               ntohs(cliAddr.sin_port));

        send(SS[i].socket, message, strlen(message), 0);
        // receive the port number, the cport number and the ip address from the storage server as a single string
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        recv(SS[i].socket, buffer, sizeof(buffer), 0);
        // tokenize the buffer to get the port number, the cport number and the ip address
        char *token = strtok(buffer, " ");
        SS[i].port = atoi(token);
        token = strtok(NULL, " ");
        SS[i].cport = atoi(token);
        token = strtok(NULL, " ");
        strcpy(SS[i].ip, token);
        token = strtok(NULL, " ");
        strcpy(SS[i].wd, token);
        printf("The port number is %d and the cport number is %d and the ip address is %s and the working directory is %s\n", SS[i].port, SS[i].cport, SS[i].ip, SS[i].wd);
        // send the message "Enter the list of accessible paths and respond with <STOP> when done" to the storage server
        char message1[] = "Enter the list of accessible paths and respond with <STOP> when done";
        send(SS[i].socket, message1, strlen(message1), 0);
        // continue to receive the paths from the storage server until the storage server sends the message "STOP"
        while (strcmp(buffer, "STOP") != 0)
        {
            memset(buffer, 0, sizeof(buffer));
            recv(SS[i].socket, buffer, sizeof(buffer), 0);
            printf("%s\n", buffer);
            if (strcmp(buffer, "STOP") != 0)
            {
                insert(trie, buffer, i);
            }
            // printf("%s\n", buffer);
        }
    }
    if (pthread_create(&one_more_thread, NULL, Dyna_SS, &sockfd) != 0)
    {
        perror("Error in creating thread");
        exit(EXIT_FAILURE);
    }

    // listen for client conncections
    if (listen(clsockfd, MAX_CLIENTS) == 0)
    {
        printf("Listening for Client Connections...\n\n");
    }
    else
    {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    cnt = 0;
    pthread_t thread[MAX_CLIENTS];

    while (1)
    {
        // printf("here\n");
        int clientSocket;
        struct sockaddr_in cliAddr;
        socklen_t addr_size = sizeof(cliAddr);

        // Accept client connection
        clientSocket = accept(clsockfd, (struct sockaddr *)&cliAddr, &addr_size);
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
    close(clsockfd);

    return 0;
}