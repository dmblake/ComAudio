#include "network.h"

struct sockaddr_in myAddr;
struct sockaddr_in mcastAddr;

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


