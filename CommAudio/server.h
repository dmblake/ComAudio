#ifndef SERVER_H
#define SERVER_H

#include "shared.h"
#include "client.h"
#include "mainwindow.h"
#include "network.h"
#include "circularbuffer.h"
void startServer();
bool setupServerListenSocket();
DWORD WINAPI FileTransferThread(LPVOID lpParameter);
bool setupServerMulticastSocket();
void serverCleanup();
void startServerMulticastSession();
DWORD WINAPI ServerMcastThread(LPVOID lpParameter);
DWORD WINAPI ServerAcceptSocketThread(LPVOID lpParameter);

#define MCAST_TTL 2

#endif // SERVER_H



