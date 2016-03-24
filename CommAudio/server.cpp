#include "server.h"

SOCKET ListenSocket;
SOCKET ServerMulticastSocket;
HANDLE hFileTransferThread;
struct ip_mreq ServerMreq;

extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;

void startServer()
{
    //Start the file transfer thread
    //Sets up the listen socket and accepts incoming connections
    if (!setupListenSocket())
    {
        qDebug() << "failed to setup ListenSocket";
    }

    if((hFileTransferThread = CreateThread(NULL, 0, FileTransferThread, NULL, 0, NULL)) == NULL)
    {
        qDebug() << "create FileTransferThread failed";
    }

    startServerMulticastSession();
}

void startServerMulticastSession()
{
    if (!setupServerMulticastSocket())
    {
        qDebug() << "failed to setup multicast socket";
    }

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

    if (CreateThread(NULL, 0, ServerMcastThread, NULL, 0, NULL) == NULL)
    {
        qDebug() << "ServerMcastThread could not be created";
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

DWORD WINAPI AcceptSocketThread(LPVOID lpParameter)
{
    SOCKET AcceptSocket;
    while(TRUE)
    {
        AcceptSocket = createAcceptSocket();
        if (CreateThread(NULL, 0, FileTransferThread, (void*)AcceptSocket, 0, NULL) == NULL)
        {
            qDebug() << "File transfer Socket could not be created";
        }
        //close accept socket after passing a copy to the thread
        closesocket(AcceptSocket);
    }
}

DWORD WINAPI FileTransferThread(LPVOID lpParameter)
{
    SOCKET fileTransferSocket = (SOCKET)lpParameter;
    //send stuff here and reveive stuff here

    //close socket right away for testing
    closesocket(fileTransferSocket);
}

DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
{
    DWORD nBytesRead;
    HANDLE hFile;
    hFile = CreateFile
            (L"C:/Users/jose/Desktop/warpeace.txt",               // file to open
            GENERIC_READ,          // open for reading
            FILE_SHARE_READ,       // share for reading
            NULL,                  // default security
            OPEN_EXISTING,         // existing file only
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
            NULL);

    char sendBuff[BUF_LEN];
    while (ReadFile(hFile, sendBuff, BUF_LEN, &nBytesRead, NULL))
    {
        memset((char *)sendBuff, 0, BUF_LEN);
        // Sending Datagrams
        if (nBytesRead > 0)
        {
            sendto(ServerMulticastSocket, sendBuff, nBytesRead, 0, (SOCKADDR *)&mcastAddr, sizeof(sockaddr_in));
        }
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
    bool flag;
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

    // For testing allows the sender to receive as well
    if (setsockopt(ServerMulticastSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&flag, sizeof(flag)) == SOCKET_ERROR)
        {
            qDebug() << "setsockopt(IP_MULTICAST_LOOP) failed: " << WSAGetLastError();
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