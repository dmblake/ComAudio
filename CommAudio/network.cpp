#include "network.h"

struct sockaddr_in myAddr;
struct sockaddr_in mcastAddr;

BOOL tFlag = TRUE;
BOOL fFlag = FALSE;

void startWinsock()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        qDebug() << "WSAStartup() failed";
        WSACleanup();
    }
}

void fillMyAddrStruct()
{
    memset((char *)&myAddr, 0, sizeof(sockaddr_in));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(PORT);
}

void fillMcastAddrStruct()
{
    // Fill in the sockaddr for the multicast group
    hostent* hp;
    memset((char *)&mcastAddr, 0, sizeof(struct sockaddr_in));
    mcastAddr.sin_family = AF_INET;
    mcastAddr.sin_port = htons(MCAST_PORT);
    if ((hp = gethostbyname(MCAST_IP)) == NULL)
    {
        qDebug() << "Unknown mcast address";
    }
    memcpy((char *)&mcastAddr.sin_addr, hp->h_addr, hp->h_length);
}

bool createUdpSocket(SOCKET* sock)
{
    if ((*sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create UDP Socket" << WSAGetLastError();
        closesocket(*sock);
        return false;
    }
    return true;
}

bool createTcpSocket(SOCKET* sock)
{
    if ((*sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create TCP Socket" << WSAGetLastError();
        closesocket(*sock);
        return false;
    }
    return true;
}

bool createAcceptSocket(SOCKET* listenSocket, SOCKET* acceptSocket)
{
    if((*acceptSocket = accept(*listenSocket, NULL, NULL)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create Accept Socket" << WSAGetLastError();
        closesocket(*acceptSocket);
    }
    return true;
}

void fillServerMcastStruct(McastStruct* mcastStruct)
{
    // Create udp mcast socket
    createUdpSocket(&(mcastStruct->Sock));
    
    // fill in the multicast bindAddr
    fillAddrStruct(&(mcastStruct->bindAddr), AF_INET, htonl(INADDR_ANY), 0);

    // fill in the mcastAddr
    fillAddrStruct(&(mcastStruct->mcastAddr), AF_INET, inet_addr(MCAST_IP), htons(MCAST_PORT));

    // fill the mreq struct
    mcastStruct->mreq.imr_interface.s_addr = INADDR_ANY;
    mcastStruct->mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
}

void fillClientMcastStruct(McastStruct* mcastStruct)
{
    // Create udp mcast socket
    createUdpSocket(&(mcastStruct->Sock));
    
    // fill in the multicast bindAddr
    fillAddrStruct(&(mcastStruct->bindAddr), AF_INET, htonl(INADDR_ANY), htons(MCAST_PORT));

    // fill the mreq struct
    mcastStruct->mreq.imr_interface.s_addr = INADDR_ANY;
    mcastStruct->mreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);
}

void fillAddrStruct(SOCKADDR_IN* sockaddr, short family, unsigned long addr, unsigned short port)
{
    memset((char *)sockaddr, 0, sizeof(sockaddr_in));
    sockaddr->sin_family = family;
    sockaddr->sin_addr.s_addr = addr;
    sockaddr->sin_port = port;
}


