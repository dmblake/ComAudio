#ifndef SERVER_H
#define SERVER_H

#include "shared.h"

void startServer();
SOCKET createAcceptSocket();
bool setupListenSocket();
DWORD WINAPI FileTransferThread(LPVOID lpParameter);
bool setupServerMulticastSocket();
void serverCleanup();

#define MCAST_TTL 2

#endif // SERVER_H



