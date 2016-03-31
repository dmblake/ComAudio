#ifndef NETWORK_H
#define NETWORK_H
#include "shared.h"

struct McastStruct
{
  SOCKET Sock;
  SOCKADDR_IN bindAddr;
  SOCKADDR_IN mcastAddr;
  ip_mreq mreq;
};

extern BOOL tFlag;
extern BOOL fFlag;

void startWinsock();
void fillMyAddrStruct();
void fillMcastAddrStruct();
bool createUdpSocket(SOCKET* sock);
bool createTcpSocket(SOCKET* sock);
bool createAcceptSocket(SOCKET* listenSocket, SOCKET* acceptSocket);
void fillServerMcastStruct(McastStruct* mcastStruct);
void fillAddrStruct(SOCKADDR_IN* sockaddr, short family, unsigned long addr, unsigned short port);
void fillClientMcastStruct(McastStruct* mcastStruct);

#endif // NETWORK_H
