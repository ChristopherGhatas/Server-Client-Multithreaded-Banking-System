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

#ifndef SERVER_H
#define SERVER_H

typedef enum _boolean {FALSE,TRUE} boolean;

typedef struct account {
	char name[256];
	double balance;
	boolean inservice;
} account;

void* bank_diagnostic(void* s);

void print_bank_info();

int search_Account(char *name);

void inputParser(char *input, int clientfd);

void *clientThread(void* fd);

void *clientAccepting(void *portno);

void *receiveThread(void *sockfd);

void *sendThread(void *connfd);

#endif
