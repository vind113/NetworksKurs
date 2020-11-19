#include<stdio.h>
#include<winsock2.h>
#include<thread>
#include<string>
#include<fstream>
#include<iostream>

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable:4996) 

#define BUFLEN 1024
#define PROGRAM_A_TO_B_PORT 2010
#define PROGRAM_A_TO_D_PORT 2011
#define PROGRAM_D_TO_A_PORT 2013

// B - повідомлення виводиться у файл.
void fileWrite(std::string text, std::string path) {
	std::ofstream outputFileStream;
	outputFileStream.open(path, std::ios_base::app);
	outputFileStream << text;
	outputFileStream.close();
}

// C - повідомлення виводиться на екран.
// D - повідомлення виводиться на екран.
// E - повідомлення виводиться на екран.
void consoleWrite(std::string text) {
	std::cout << text;
}

// A - повідомлення виводиться на екран і у файл.
void consoleAndFileWrite(std::string text, std::string path) {
	consoleWrite(text);
	fileWrite(text, path);
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

		consoleAndFileWrite(buf, "A_output.txt");
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

int main()
{
	startClientConnectionlessSocketThread(PROGRAM_D_TO_A_PORT);

	SOCKET s = createServerSocket(PROGRAM_A_TO_B_PORT);

	std::string input = "";
	while (input.compare("end") != 0) {
		input = readConsole();
		sendTextToSocket(s, PROGRAM_A_TO_B_PORT, input);
		sendTextToSocket(s, PROGRAM_A_TO_D_PORT, input);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}