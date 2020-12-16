#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib") //Winsock Library
#pragma once

#define DSIZE 512


struct gsock {
	SOCKET sock;
	SOCKADDR_IN Addr;
	char data[DSIZE];//Associated data for sending/recieving
	int dataSize;//Length of the data
};
typedef struct gsock GSock;

//run at the very start, starts up winsock
void startup();

//Creates a socket with the given port/address
void createSock(GSock* sock, int port, char* address);

//connects the socket
int gconnect(GSock* sock);

//sets up the socket to listen with backlog of five
void glisten(GSock* sock);

//accepts a connection
void gaccept(GSock* listeningSock, GSock* passedSock);

//sends data
void gsend(GSock* sock);

//recieves data
void grecv(GSock* sock);