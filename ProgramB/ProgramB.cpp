
#include <stdio.h>
#include <winsock2.h>
#include <thread>
#include <string>
#include <fstream>

#pragma comment(lib,"ws2_32.lib") 

#define BUFLEN 1024
#define PROGRAM_A_TO_B_PORT 2010	

// B - повідомлення виводиться у файл.
void fileWrite(std::string text, std::string path) {
	std::ofstream outputFileStream;
	outputFileStream.open(path, std::ios_base::app);
	outputFileStream << text;
	outputFileStream.close();
}

sockaddr_in getServerAddress(int port)
{
	struct sockaddr_in addr;
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	return addr;
}

SOCKET createClientConnectionlessSocket(const sockaddr_in& serverAddress)
{
	SOCKET clientSocket;
	WSADATA wsa;

	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	bind(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));

	return clientSocket;
}

void startClientConnectionlessSocket(const SOCKET& clientSocket, const sockaddr_in& serverAddress)
{
	int slen = sizeof(serverAddress);
	char buf[BUFLEN];
	//start communication
	while (1)
	{
		memset(buf, '\0', BUFLEN);
		if (recvfrom(clientSocket, buf, BUFLEN, 0, (struct sockaddr*)&serverAddress, &slen) == SOCKET_ERROR)
		{
			printf("Data recieve failed with error code : %d", WSAGetLastError());
		}

		fileWrite(buf, "B_output.txt");
	}
}

void startClientConnectionlessSocketThread(int port)
{
	sockaddr_in serverAddress = getServerAddress(port);
	SOCKET clientSocket = createClientConnectionlessSocket(serverAddress);

	std::thread listener(startClientConnectionlessSocket, clientSocket, serverAddress);
	listener.detach();
}

int main(void)
{
	startClientConnectionlessSocketThread(PROGRAM_A_TO_B_PORT);

	getchar();

	return 0;
}
