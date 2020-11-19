#include<stdio.h>
#include<winsock2.h>
#include<thread>
#include<string>
#include<fstream>
#include<iostream>

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable:4996) 

#define BUFLEN 1024
#define PROGRAM_C_TO_D_PORT 2015
#define PROGRAM_D_TO_C_PORT 2017

LPCTSTR PROGRAM_C_MAILSLOT = TEXT("\\\\.\\mailslot\\program_c_mailslot");
LPCTSTR PROGRAM_E_MAILSLOT = TEXT("\\\\.\\mailslot\\program_e_mailslot");

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

void readFromMailSlot(const HANDLE& handle)
{
	DWORD messageSize = 0;
	DWORD messageCount = 0;
	DWORD cbRead = 0;

	BOOL fResult;
	char lpszBuffer[BUFLEN] = {};
	DWORD cAllMessages;
	HANDLE hEvent;
	OVERLAPPED ov;

	hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("ExampleSlot"));
	if (NULL == hEvent)
		return;

	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.hEvent = hEvent;

	fResult = GetMailslotInfo(handle, (LPDWORD)NULL, &messageSize, &messageCount, (LPDWORD)NULL);

	if (!fResult)
	{
		printf("GetMailslotInfo failed with %d.\n", GetLastError());
		return;
	}

	if (messageSize == MAILSLOT_NO_MESSAGE)
	{
		return;
	}

	cAllMessages = messageCount;

	while (messageCount != 0)
	{
		lpszBuffer[0] = '\0';

		fResult = ReadFile(handle, lpszBuffer, messageSize, &cbRead, &ov);

		if (!fResult)
		{
			printf("ReadFile failed with error %d.\n", GetLastError());
			GlobalFree((HGLOBAL)lpszBuffer);
			return;
		}

		// Display the message. 
		consoleWrite(lpszBuffer);

		GlobalFree((HGLOBAL)lpszBuffer);

		fResult = GetMailslotInfo(handle, (LPDWORD)NULL, &messageSize, &messageCount, (LPDWORD)NULL);

		if (!fResult)
		{
			printf("GetMailslotInfo failed with %d.\n", GetLastError());
			return;
		}
	}

	CloseHandle(hEvent);
}

HANDLE createMailSlotWriteHandle() {
	return CreateFile(PROGRAM_E_MAILSLOT,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
}

HANDLE createMailSlotReadHandle()
{
	HANDLE handle = CreateMailslot(PROGRAM_C_MAILSLOT, 0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot failed with %d\n", GetLastError());
	}

	return handle;
}

void writeToMailSlot(const HANDLE& handle, std::string text)
{
	BOOL fResult;
	DWORD cbWritten;

	fResult = WriteFile(handle, text.c_str(), text.length(), &cbWritten, (LPOVERLAPPED)NULL);

	if (!fResult)
	{
		printf("Write to slot failed with error %d.\n", GetLastError());
	}
}

void startReadFromMailSpot(const HANDLE& handle, int readDelay) {
	while (TRUE)
	{
		readFromMailSlot(handle);
		Sleep(readDelay);
	}
}

void startReadFromMailSpotThread(int readDelay) {
	HANDLE readMailSlotHandle = createMailSlotReadHandle();
	std::thread reader(startReadFromMailSpot, readMailSlotHandle, readDelay);
	reader.detach();
}

int main()
{
	startClientConnectionlessSocketThread(PROGRAM_D_TO_C_PORT);
	startReadFromMailSpotThread(100);

	SOCKET s = createServerSocket(PROGRAM_C_TO_D_PORT);
	HANDLE writeMailSlotHandle = createMailSlotWriteHandle();

	std::string input = "";
	while (input.compare("end") != 0) {
		input = readConsole();
		sendTextToSocket(s, PROGRAM_C_TO_D_PORT, input);
		writeToMailSlot(writeMailSlotHandle, input);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}
