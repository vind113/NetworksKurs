#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512	//Max length of buffer
#define CLIENT "127.0.0.1"	//ip address of udp server
#define PORT 2010	//The port on which to listen for incoming data

int main()
{
	
	SOCKET s;
	struct sockaddr_in server;
	WSADATA wsa;

	//Initialise winsock
	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	struct sockaddr_in clientAddress;
	memset((char*)&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	clientAddress.sin_port = htons(PORT);
	int slen = sizeof(clientAddress);
	char buf[BUFLEN] = "Hello";
	//keep listening for data
	while (1)
	{
		//now reply the client with the same data
		if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*)&clientAddress, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}