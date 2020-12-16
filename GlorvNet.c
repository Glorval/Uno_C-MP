#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
#include "GlorvNet.h"

#pragma once


void startup() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void createSock(GSock* sock, int port, char* address) {
	sock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sock->Addr.sin_family = AF_INET;
	sock->Addr.sin_port = htons(port);
	if (address != 0) {
		sock->Addr.sin_addr.s_addr = inet_addr("192.168.100.9");
	}
	else {
		sock->Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
}

int gconnect(GSock* sock) {
	return(connect(sock->sock, (SOCKADDR*)&sock->Addr, sizeof(sock->Addr)));
}

void glisten(GSock* sock) {
	bind(sock->sock, (SOCKADDR*)&sock->Addr, sizeof(sock->Addr));
	printf("Listen:%d\n", listen(sock->sock, 5));
}

void gaccept(GSock* listeningSock, GSock* passedSock) {
	int addrsize = sizeof(listeningSock->Addr);
	passedSock->sock = accept(listeningSock->sock, (SOCKADDR*)&listeningSock->Addr, &addrsize);
}

void gsend(GSock* socket) {
	send(socket->sock, &socket->data, socket->dataSize, 0);
}

void grecv(GSock* socket) {
	int recieved = recv(socket->sock, &socket->data, socket->dataSize, 0);
	if (recieved == -1) {
		recieved = recv(socket->sock, &socket->data, socket->dataSize, 0);
		printf("Error, trying again. %d Recieved\n", recieved);
		if (recieved == -1) {
			printf("Faliure to recieve data, brace yourself!\n");
		}
	}
}