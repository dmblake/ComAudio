#ifndef SHARED_H
#define SHARED_H

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <QDebug>

#define PORT            9999
#define MCAST_PORT      9001
#define MIC_PORT        9002
#define SERVER_IP       "127.0.0.1"
#define PEER_IP         "0.0.0.0"
#define MCAST_IP        "234.5.6.8"
#define BUF_LEN         1024

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[BUF_LEN];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

#endif // SHARED_H
