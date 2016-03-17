#include "network.h"

struct sockaddr_in myAddr;

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


