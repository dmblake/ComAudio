#ifndef SERVER_H
#define SERVER_H

#include "shared.h"

void startServer();
SOCKET createAcceptSocket();
bool setupListenSocket();
DWORD WINAPI FileTransferThread(LPVOID lpParameter);
bool setupServerMulticastSocket();
void serverCleanup();
void startServerMulticastSession();
DWORD WINAPI ServerMcastThread(LPVOID lpParameter);
DWORD WINAPI AcceptSocketThread(LPVOID lpParameter);

#define MCAST_TTL 2

#endif // SERVER_H



