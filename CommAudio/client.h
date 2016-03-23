#ifndef CLIENT_H
#define CLIENT_H

#include "shared.h"

void startFileTransfer();
bool setupTcpSocket();
bool setUdpSocket();
bool setupClientMulticastSocket();
void clientCleanup();
void startClientMulticastSession();
void CALLBACK ClientMcastWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientMcastThread(LPVOID lpParameter);
#endif // CLIENT_H
