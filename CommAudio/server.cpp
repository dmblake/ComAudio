#include "server.h"

SOCKET ListenSocket;
SOCKET ServerMulticastSocket;
HANDLE hFileTransferThread;
struct ip_mreq ServerMreq;
extern struct sockaddr_in myAddr;

void startServer()
{
    if((hFileTransferThread = CreateThread(NULL, 0, FileTransferThread, NULL, 0, NULL)) == NULL)
    {
        qDebug() << "create FileTransferThread failed";
    }
}

bool setupListenSocket()
{
    qDebug()<< "setupListenSocket called";
    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to get a socket";
        return false;
    }

    // Change the port in the myAddr struct to the default port
    myAddr.sin_port = PORT;

    // Bind the listen socket
    if (bind(ListenSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind the listen socket";
        return false;
    }

    // Setup the ListenSocket to listen for incoming connections
    // with a backlog size 5
    if (listen(ListenSocket, 5))
    {
        qDebug() << "listen() failed";
        return false;
    }
    return true;
}

DWORD WINAPI FileTransferThread(LPVOID lpParameter)
{
    SOCKET AcceptSocket;
    if (!setupListenSocket())
    {
        ExitThread(3);
    }
    while(TRUE)
    {
        AcceptSocket = createAcceptSocket();
        // close accept socket right away for testing
        closesocket(AcceptSocket);
    }
}

SOCKET createAcceptSocket()
{
    qDebug() << "in createAcceptSocket()";
    SOCKET TempSocket;
    if((TempSocket = accept(ListenSocket, NULL, NULL)) == -1)
    {
        qDebug("Cant accept client");
    }
    return TempSocket;
}

/*
 * Creates and bind a UDP socket.
 * Fills in the information for the ip_mreq structure which contains the
 * multicast information for IPv4 addresses
 * The UDP socket is added to the multicast group and the number of
 * network hops(TTL) is set.
 */
bool setupServerMulticastSocket()
{
    if ((ServerMulticastSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create Multicast Socket: " << WSAGetLastError();
        closesocket((ServerMulticastSocket));
        return false;
    }

    // Change the port in the myAddr struct to the multicast port
    myAddr.sin_port = MCAST_PORT;

    if (bind(ServerMulticastSocket, (struct sockaddr*)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind Multicast Socket: " << WSAGetLastError();
        return false;
    }

    // Setting the local IP address of interface and the multicast address group
    ServerMreq.imr_interface.s_addr = INADDR_ANY;
    ServerMreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);


    // Joining the Multicast group
    if (setsockopt(ServerMulticastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ServerMreq, sizeof(ServerMreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_ADD_MEMBERSHIP) failed: " << WSAGetLastError();
        closesocket(ServerMulticastSocket);
        return false;
    }

    // Setting TTL (hops)
    if (setsockopt(ServerMulticastSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&MCAST_IP, sizeof(MCAST_TTL)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_MULTICAST_TTL) failed: " << WSAGetLastError();
        closesocket(ServerMulticastSocket);
        return false;
    }

    return true;
}

void serverCleanup()
{
    qDebug() << "server cleanup called";
    closesocket(ListenSocket);
    closesocket(ServerMulticastSocket);
    WSACleanup();
}
