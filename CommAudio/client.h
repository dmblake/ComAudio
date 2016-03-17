#ifndef CLIENT_H
#define CLIENT_H

#include "shared.h"

void startFileTransfer();
bool setupTcpSocket();
bool setUdpSocket();
bool setupClientMulticastSocket();
void clientCleanup();
#endif // CLIENT_H
