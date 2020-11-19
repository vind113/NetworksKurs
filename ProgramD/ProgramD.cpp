#include<stdio.h>
#include<winsock2.h>
#include<thread>
#include<string>
#include<fstream>
#include<iostream>
#include "ProgramD.h"

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable:4996) 

#define BUFLEN 1024
#define PROGRAM_A_TO_D_PORT 2011	
#define PROGRAM_D_TO_A_PORT 2013
#define PROGRAM_C_TO_D_PORT 2015
#define PROGRAM_D_TO_C_PORT 2017

#define PIPE_NAME "\\\\.\\pipe\\Pipe"

// C - повідомлення виводиться на екран.
// D - повідомлення виводиться на екран.
// E - повідомлення виводиться на екран.
void consoleWrite(std::string text) {
	std::cout << text;
}

sockaddr_in getServerAddress(int port)
{
	struct sockaddr_in addr;
	//setup address structure
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

		consoleWrite(buf);
	}
}

void startClientConnectionlessSocketThread(int port)
{
	sockaddr_in serverAddress = getServerAddress(port);
	SOCKET clientSocket = createClientConnectionlessSocket(serverAddress);

	std::thread listener(startClientConnectionlessSocket, clientSocket, serverAddress);
	listener.detach();
}

SOCKET createServerSocket(int port) {
	SOCKET s;
	struct sockaddr_in server;
	WSADATA wsa;

	printf("Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	printf("Initialised.\n");

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	printf("Bind done.\n");

	return s;
}

void sendTextToSocket(const SOCKET& s, int clientPort, std::string text) {
	struct sockaddr_in clientAddress;
	memset((char*)&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	clientAddress.sin_port = htons(clientPort);
	int slen = sizeof(clientAddress);

	if (sendto(s, text.c_str(), text.length(), 0, (struct sockaddr*)&clientAddress, slen) == SOCKET_ERROR)
	{
		printf("Data send failed with error code : %d", WSAGetLastError());
	}
}

// A - повідомлення вводиться з клавіатури.
std::string readConsole() {
	std::cout << "Enter text(ending with new line):\n";
	std::string input;
	std::getline(std::cin, input);
	return input + "\n";
}

HANDLE createNamedPipe() {
	return CreateNamedPipe(TEXT(PIPE_NAME),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		1,
		1024 * 16,
		1024 * 16,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
}

void startNamedPipeServer(const HANDLE& pipe)
{
	DWORD dwRead;
	char buffer[BUFLEN];
	while (pipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(pipe, NULL) != FALSE)
		{
			while (ReadFile(pipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				consoleWrite(buffer);
			}
		}

		DisconnectNamedPipe(pipe);
	}
}

void startNamedPipeServerThread(const HANDLE& pipe) {
	std::thread listener(startNamedPipeServer, pipe);
	listener.detach();
}

int main()
{
	// start named pipe server
	HANDLE hPipe = createNamedPipe();
	startNamedPipeServerThread(hPipe);

	// start listening incoming messages from program a
	startClientConnectionlessSocketThread(PROGRAM_A_TO_D_PORT);
	// start listening incoming messages from program a
	startClientConnectionlessSocketThread(PROGRAM_C_TO_D_PORT);

	SOCKET s = createServerSocket(PROGRAM_D_TO_A_PORT);

	std::string input = "";
	while (input.compare("end") != 0) {
		input = readConsole();
		sendTextToSocket(s, PROGRAM_D_TO_A_PORT, input);
		sendTextToSocket(s, PROGRAM_D_TO_C_PORT, input);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}
