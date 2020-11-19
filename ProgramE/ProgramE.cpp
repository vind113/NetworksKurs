#include <windows.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <thread>

#define BUFLEN 1024
#define PIPE_NAME "\\\\.\\pipe\\Pipe"

LPCTSTR PROGRAM_C_MAILSLOT = TEXT("\\\\.\\mailslot\\program_c_mailslot");
LPCTSTR PROGRAM_E_MAILSLOT = TEXT("\\\\.\\mailslot\\program_e_mailslot");

// C - повідомлення виводиться на екран.
// D - повідомлення виводиться на екран.
// E - повідомлення виводиться на екран.
void consoleWrite(std::string text) {
    std::cout << text;
}

std::string readConsole() {
    std::cout << "Enter text(ending with new line):\n";
    std::string input;
    std::getline(std::cin, input);
    return input + "\n";
}

HANDLE createNamedPipeConnection() {
    return CreateFile(TEXT(PIPE_NAME),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
}

void sendMessageToNamedPipe(const HANDLE& pipe, std::string text) {
    DWORD dwWritten;
    if (pipe != INVALID_HANDLE_VALUE)
    {
        WriteFile(pipe, text.c_str(), text.length(), &dwWritten, NULL);
    }
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
    return CreateFile(PROGRAM_C_MAILSLOT,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
}

HANDLE createMailSlotReadHandle()
{
    HANDLE handle = CreateMailslot(PROGRAM_E_MAILSLOT, 0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);

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
    startReadFromMailSpotThread(100);

    HANDLE pipe = createNamedPipeConnection();
    HANDLE writeMailSlotHandle = createMailSlotWriteHandle();

    std::string input = "";
    while (input.compare("end") != 0) {
        input = readConsole();
        sendMessageToNamedPipe(pipe, input);
        writeToMailSlot(writeMailSlotHandle, input);
    }

    CloseHandle(pipe);

    return 0;
}
