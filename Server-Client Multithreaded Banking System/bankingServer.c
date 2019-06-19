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

#include "bankingServer.h"

pthread_mutex_t createX;
pthread_mutex_t depositX;
pthread_mutex_t withdrawX;
pthread_mutex_t queryX;
account account_Array[10000];


 void print_bank_info() { //print info on screen in the tab format for each account that can exist.
	printf("----------------------------------------\n");
	printf("\nCURRENT BANK ACCOUNTS:\n");
	int m = 0;
		for(m = 0; m < 10000; m++) {
			if (strcmp(account_Array[m].name, "\0")== 0)
            {
                m++;
                continue;
            }
			printf("Account Name: %s\t",account_Array[m].name);
			printf("Balance: $%f\t", account_Array[m].balance);
			if(account_Array[m].inservice == TRUE){
                printf("IN SERVICE\t\n");
                continue;
            }
			else{
			printf("\n");
			continue;
			}
		}
	printf("----------------------------------------\n");
}

void *bank_diagnostic(void *s) { //15 second delay for each call to print info
	while(1) {
		print_bank_info();
		sleep(15);
	}
}


boolean printing= FALSE;

int search_Account(char *name)//finds account when it needs to be found for serve or create.
{
	int count= 0;
	while (1)
	{
		if (strcmp(account_Array[count].name, "\0")== 0)
		{
			return count;
		}
		else if (strcmp(account_Array[count].name, name)== 0)
		{
			return count;
		}
		count++;
	}
	return count;
}

void inputParser(char *input, int clientfd)
{
	char operation[9];
	char parameter[256];
	sscanf(input, "%s %s", operation, parameter);
	int i=0;

	if (strcmp(operation, "create")== 0)
	{
		pthread_mutex_lock(&createX);
		while (printing == TRUE)
		{
			sleep(1);
		}

		i= search_Account(parameter);

		if (strcmp(account_Array[i].name, "\0")!= 0)
		{
			write(clientfd, "There is already an account with this name!", 50);
		}
		else
		{
			strcpy(account_Array[i].name, parameter);
			account_Array[i].balance= 0;
			account_Array[i].inservice= FALSE;
			write(clientfd, "Account has been created!", 50);
		}
		pthread_mutex_unlock(&createX);
	}
	else if (strcmp(operation, "quit")== 0)
	{
		write(clientfd, "quitting", 10);
		pthread_exit(0);
	}
	else
	{
		write(clientfd, "This is an invalid operation", 50);
	}
	bzero(parameter, strlen(parameter));
	bzero(operation, strlen(operation));
}

void *clientThread(void* fd)
{
	int clientfd, thisAccount= 0;
	char buffer[2000];
    char operation[9], parameter[256];
    int temp= 0;

    pthread_mutex_init(&createX, NULL); //4 mutexes for each process
    pthread_mutex_init(&depositX, NULL);
    pthread_mutex_init(&withdrawX, NULL);
    pthread_mutex_init(&queryX, NULL);

    char buffer2[100];

	clientfd= *((int *)fd); //cast it back to an int

   	while (TRUE)
   	{
   		bzero(operation, sizeof(operation));
		bzero(parameter, sizeof(parameter));
		bzero(buffer, sizeof(buffer));
   		read(clientfd, buffer, 2000);
   		sscanf(buffer, "%s %s", operation, parameter);

   		if (strcmp(operation, "serve")== 0)
   		{
   			thisAccount= search_Account(parameter); // index of the current account
   			if (strcmp(account_Array[thisAccount].name, "\0")== 0)
   			{
   				write(clientfd, "this account does not exist!", 50);
   				continue;
   			}
   			if (account_Array[thisAccount].inservice== TRUE)
   			{
   				write(clientfd, "there is already a client in session with this account!", 60);
   				continue;
   			}
   			account_Array[thisAccount].inservice= TRUE;
   			write(clientfd, "Account is now in session!", 50);
   			while (TRUE)
   			{
   				bzero(buffer2, sizeof(buffer2));
   				bzero(buffer, sizeof(buffer));
   				read(clientfd, buffer, 2000);
   				sscanf(buffer, "%s %s", operation, parameter);

   				if (strcmp(operation, "deposit")== 0)
   				{
   					pthread_mutex_lock(&depositX);
   					if (atof(parameter) <= 0)
   					{
   						write(clientfd, "Number must be greater than 0!", 60);
   					}
   					else
   					{
	   					account_Array[thisAccount].balance += atof(parameter);
	   					int temp= snprintf(buffer2, 100, "your balance is : $%f", account_Array[thisAccount].balance);
	   					buffer2[temp]= '\0';
	   					write(clientfd, buffer2, sizeof(buffer2));
   					}
   					pthread_mutex_unlock(&depositX);
   				}
   				else if (strcmp(operation, "withdraw")== 0)
   				{
   					pthread_mutex_unlock(&withdrawX);
   					if (atof(parameter) > account_Array[thisAccount].balance)
   					{
   						write(clientfd, "Insufficient funds to withdraw this amount", 100);
   					}
   					else if (atof(parameter) <= 0)
   					{
   						write(clientfd, "Number must be greater than 0", 60);
   					}
   					else
   					{
   						account_Array[thisAccount] . balance -= atof(parameter);
   						int temp= snprintf(buffer2, 100, "your balance is : $%f", account_Array[thisAccount] .balance);
	   					buffer2[temp]= '\0';
	   					write(clientfd, buffer2, sizeof(buffer2));
   					}
   					pthread_mutex_unlock(&withdrawX);
   				}
   				else if (strcmp(operation, "query")== 0)
   				{
   					pthread_mutex_lock(&queryX);
   					temp= snprintf(buffer2, 100, "your balance is : $%f", account_Array[thisAccount] .balance);
   					buffer2[temp]= '\0';
   					write(clientfd, buffer2, sizeof(buffer2));
   					pthread_mutex_unlock(&queryX);
   				}
   				else if (strcmp(operation, "end")== 0)
   				{
   					account_Array[thisAccount].inservice= FALSE;
   					write(clientfd, "your session has be disconnected.  You can start another session", 100);
   					break;
   				}
   				else if (strcmp(operation, "quit")== 0)
   				{
   					account_Array[thisAccount].inservice= FALSE;
   					write(clientfd, "disconnecting from server", 50);
            pthread_exit(0);
   				}
   				else
   				{
   					write(clientfd, "This is an invalid operation for session Accounts.", 50);
   				}
   			}
   		}
   		else
		{
			inputParser(buffer, clientfd);
		}
    }
    pthread_exit(0);
}

void *clientAccepting(void *portno)
{
	int port = *((int*)portno);
	int sockfd, clientfd;
    struct sockaddr_in server, client;
    int len= sizeof(struct sockaddr_in);

    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
        perror("Could not create socket");
/*     else
    	printf("socket has been created!\n"); */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

 	//bind socket to port
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
    	perror("error with bind");
    	return 0;
    }

    int option= 0;

    if (listen(sockfd, 20) != 0)
    {
     	perror("error with listen");
     	exit(0);
    }
    else
    {
    	puts("The Server is now listening...");
    }

   pthread_t thread; 

    while ((clientfd= accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&len))){//accept while loop where its a blocking call when waiting for each accept.
	   printf("connection successful with the Client\n");
	      if (pthread_create(&thread, NULL, clientThread, &clientfd) < 0){//creates thread for each client that is accepted.
	            perror("could not create thread");
	    	}
    }

    if (clientfd <= 0)
    {
    	perror("error accepting\n");
    }

    pthread_join(thread, NULL);

    close(clientfd);
    close(sockfd);
    pthread_exit(0);
}

int main(int argc, char **argv){
	signal(SIGPIPE, SIG_IGN);

	int r= 0;
    while (r != 10000)
    {
   		account_Array[r].name[0]= '\0';
   		r++;
    }

    pthread_t thread_accounts;

    if (pthread_create(&thread_accounts, NULL, bank_diagnostic, (void *) NULL) < 0){ //creates thread to print bank info every 15 seconds.
        perror("could not create thread");
	}

    pthread_t accepting_Thread;
	int portno = atoi(argv[1]);
	int* port;
	*port = portno;
    if (pthread_create(&accepting_Thread, NULL, clientAccepting, (void*) port ) < 0){//creates thread to handle accepting of new clients and where to direct them.
        perror("could not create thread");
	}

	pthread_join(accepting_Thread, NULL); //joins the acceptor thread so main waits till server is done with all clients.

	return 0;
}
