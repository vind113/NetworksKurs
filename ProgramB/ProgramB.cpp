/*
	Simple udp client
*/
#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <string>
#include <fstream>
#include "ProgramB.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512	//Max length of buffer
#define PORT 2010	//The port on which to listen for incoming data

// B - повідомлення виводиться у файл.
void fileWrite(std::string text, std::string path) {
	std::ofstream outputFileStream;
	outputFileStream.open(path, std::ios_base::app);
	outputFileStream << text;
	outputFileStream.close();
}

sockaddr_in getServerAddress()
{
	struct sockaddr_in addr;
	//setup address structure
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	return addr;
}

SOCKET createSocket(const sockaddr_in& serverAddress)
{
	SOCKET clientSocket;
	WSADATA wsa;

	//Initialise winsock
	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	bind(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));

	return clientSocket;
}

void startClientSocket(const SOCKET& clientSocket, const sockaddr_in& serverAddress)
{
	int slen = sizeof(serverAddress);
	char buf[BUFLEN];
	//start communication
	while (1)
	{
		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(clientSocket, buf, BUFLEN, 0, (struct sockaddr*)&serverAddress, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		fileWrite(buf, "B_output.txt");
	}
}

int main(void)
{
	sockaddr_in serverAddress = getServerAddress();
	SOCKET clientSocket = createSocket(serverAddress);
	
	std::thread listener(startClientSocket, clientSocket, serverAddress);
	listener.detach();

	getchar();

	return 0;
}
