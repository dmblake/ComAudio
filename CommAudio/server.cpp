#include "server.h"

SOCKET ServerListenSocket;
SOCKET ServerMulticastSocket;
HANDLE hFileTransferThread;
struct ip_mreq ServerMreq;

u_long ttl = MCAST_TTL;
extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;
McastStruct sMcastStruct;

void startServer()
{
    //Start the file transfer thread
    //Sets up the listen socket and accepts incoming connections
    if (!setupServerListenSocket())
    {
        qDebug() << "failed to setup ListenSocket";
    }
    else
    {
        qDebug() << "Listen Socket OK";
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
        qDebug() << "failed to setup multicast socket" << WSAGetLastError();
    }

    if (CreateThread(NULL, 0, ServerMcastThread, NULL, 0, NULL) == NULL)
    {
        qDebug() << "ServerMcastThread could not be created";
    }
}

bool setupServerListenSocket()
{
    qDebug()<< "setupServerListenSocket called";
    createTcpSocket(&ServerListenSocket);

    // set reuseaddr
    setsockopt(ServerListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL));

    // Change the port in the myAddr struct to the default port
    myAddr.sin_port = htons(PORT);

    // Bind the listen socket
    if (bind(ServerListenSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind the listen socket";
        return false;
    }

    // Setup the ListenSocket to listen for incoming connections
    // with a backlog size 5
    if (listen(ServerListenSocket, 5))
    {
        qDebug() << "listen() failed";
        return false;
    }

    if (CreateThread(NULL, 0, ServerAcceptSocketThread, NULL, 0, NULL) == NULL)
    {
        qDebug() << "AcceptSocket Thread could not be created";
    }

    return true;
}

DWORD WINAPI ServerAcceptSocketThread(LPVOID lpParameter)
{
    qDebug() << "inside accept socket thread";
    SOCKET AcceptSocket;
    while(TRUE)
    {
        createAcceptSocket(&ServerListenSocket, &AcceptSocket);
        if (CreateThread(NULL, 0, FileTransferThread, (void*)AcceptSocket, 0, NULL) == NULL)
        {
            qDebug() << "File transfer (Accept) Socket could not be created";
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
    return 0;
}

DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
{
    DWORD nBytesRead;
    HANDLE hFile;
    int nRet;
    hFile = CreateFile
            (L"C:\\luv.mp3",               // file to open
            GENERIC_READ,
            0,
            (LPSECURITY_ATTRIBUTES)NULL,       // share for reading
            OPEN_EXISTING,         // existing file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            (HANDLE)NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        qDebug() << GetLastError();
    }
    else
    {
        qDebug() << "hfile ok";
    }
    char sendBuff[BUF_LEN];
    int ret;

    

    while (ReadFile(hFile, sendBuff, BUF_LEN, &nBytesRead, NULL))
    {
        // Sending Datagrams

        if (nBytesRead > 0)
        {

            qDebug() << sendBuff;
            //mw->printToListView(sendBuff);
            
            nRet = sendto(sMcastStruct.Sock, sendBuff, nBytesRead, 0, (SOCKADDR *)&(sMcastStruct.mcastAddr), sizeof(sockaddr_in));
            if (nRet < 0)
            {
                qDebug() << "sendto failed" << WSAGetLastError();
            }
            Sleep(10);
        }
        else
        {
            qDebug() << "file close and thread ends";
            CloseHandle(hFile);
            ExitThread(3);
        }
    }
    qDebug() << "finished sending";
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
    fillServerMcastStruct(&sMcastStruct);

    // Enable reuseaddr
    if (setsockopt(sMcastStruct.Sock, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(SO_REUSEADDR) failed: " << WSAGetLastError();
        closesocket(sMcastStruct.Sock);
        return false;
    }

    // bind server mcast socket
    if (bind(sMcastStruct.Sock, (struct sockaddr*)&(sMcastStruct.bindAddr), sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind Multicast Socket: " << WSAGetLastError();
        closesocket(sMcastStruct.Sock);
        return false;
    }

    // join mcast group
    if (setsockopt(sMcastStruct.Sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&(sMcastStruct.mreq), sizeof(ip_mreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_ADD_MEMBERSHIP) failed: " << WSAGetLastError();
        closesocket(sMcastStruct.Sock);
        return false;
    }

    // set ttl
    if (setsockopt(sMcastStruct.Sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(u_long)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_MULTICAST_TTL) failed: " << WSAGetLastError();
        closesocket(sMcastStruct.Sock);
        return false;
    }

    // disable loop back
    if (setsockopt(sMcastStruct.Sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&fFlag, sizeof(BOOL)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_MULTICAST_LOOP) failed: " << WSAGetLastError();
        closesocket(sMcastStruct.Sock);
        return false;
    }

    return true;
}

void serverCleanup()
{
    qDebug() << "server cleanup called";
    closesocket(ServerListenSocket);
    closesocket(sMcastStruct.Sock);
    WSACleanup();
}
