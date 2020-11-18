#pragma once
sockaddr_in getServerAddress();

SOCKET createSocket(const sockaddr_in& serverAddress);

void startClientSocket(const SOCKET& clientSocket, const sockaddr_in& serverAddress);
