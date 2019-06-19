#include <sys/mman.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "bankingServer.h"

char command[2000];
boolean endCommand = FALSE;

void *receiveThread(void *sockfd)
{
    int sockfd2= *(int*)sockfd;
    char server_reply[2000];

    while (!endCommand)
    {
        if (recv(sockfd2 , server_reply , 2000 , 0) <= 0)
        {
            printf("Your session has been ended.You have been disconnected from the server\n");
            endCommand= TRUE;
            exit(0);
        }

        puts("Server reply :");
        puts(server_reply);
    }
    pthread_exit(0);
}

void *sendThread(void *connfd)
{
    int sockfd= *((int*)connfd);
    printf("Enter command : \n");
	char operation[9];
	char parameter[256];
    while (!endCommand)
    {
        bzero(command, strlen(command));
        /* fgets(command, sizeof(command), stdin);
        fseek(stdin, 0, SEEK_END); */
		fgets(command, sizeof(command), stdin);
		sscanf(command, "%s %s", operation, parameter);
         if (strlen(command) > 265)
        {
            printf("invalid operation!\n");
            bzero(command, strlen(command));
            printf("Enter command : \n");
            sleep(2);
            continue;
        }
		if (strcmp(operation, "create")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
		else if (strcmp(operation, "serve")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
		else if (strcmp(operation, "query")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
		else if (strcmp(operation, "withdraw")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
		else if (strcmp(operation, "deposit")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
		else if (strcmp(operation, "end")== 0)
   		{
			send(sockfd , command , strlen(command) , 0);
		}
        else if (strcmp(command, "quit\n")== 0)
        {
            send(sockfd , command , strlen(command) , 0);
            endCommand= TRUE;
			printf("Disconnecting From Server With Quit Command\n");
            break;
        }
		else{
			printf("Invalid Command. Please Try Again\n");
			printf("Enter command : \n");
			continue;
		}
        /* if (send(sockfd , command , strlen(command) , 0) == -1)
        {
            perror("Send failed");
            endCommand= TRUE;
            break;
        } */

        sleep(2);
        printf("Enter command : \n");
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
//throttle two seconds and add strcmp on client side except for withdrawal and deposit. also read instead of fgets.
	int sockfd;
	struct sockaddr_in server;
    struct hostent *host;
	int port = atoi(argv[2]);


    if (argc < 3 || (host= gethostbyname(argv[1]))== NULL)
    {
        perror("error retrieving hostname");
        return 0;
    }

	   // signal(SIGINT, intHandle);
	   
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
        perror("Could not create socket");
    else
    	printf("socket has been created!\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)(host-> h_addr));

    while (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
    {
        printf("Connection error, Connecting again in 3 seconds\n");
        sleep(3);
    }

	//Threads for receiving and sending commands to the Server.
    pthread_t sendThread_TID;
    pthread_t receiveThread_TID;

    if (pthread_create(&sendThread_TID, NULL, sendThread, (void *) &sockfd) < 0)
    {
        perror("could not create thread");
    }

    if (pthread_create(&receiveThread_TID, NULL, receiveThread, (void *) &sockfd) < 0)
    {
        perror("could not create thread");
    }


	pthread_join(receiveThread_TID, NULL);
    pthread_join(sendThread_TID, NULL);

    // close the socket
    close(sockfd);
}
