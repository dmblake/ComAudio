#include "server.h"

SOCKET ListenSocket;
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
    if (!setupListenSocket())
    {
        qDebug() << "failed to setup ListenSocket";
    }
    else
    {
        qDebug() << "Listen Socket OK";
    }    
}


void startServerMulticastSession(BufferManager* bm)
{
    if (!setupServerMulticastSocket())
    {
        qDebug() << "failed to setup multicast socket" << WSAGetLastError();
    }

    if (CreateThread(NULL, 0, ServerMcastThread, bm, 0, NULL) == NULL)
    {
        qDebug() << "ServerMcastThread could not be created";
    }
    qDebug() << "Started ServerMcastThread";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   setupListenSocket
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool setupListenSocket()
--
-- RETURNS:    bool: true if socket is setup succesfully. False otherwise
--
-- NOTES:
-- Creates and binds the Listen Socket and creates a thread to handle connection
-- attemps by clients.
----------------------------------------------------------------------------------------------------------------------*/
bool setupListenSocket()
{
    qDebug()<< "setupListenSocket called";
    createTcpSocket(&ListenSocket);

    // set reuseaddr
    setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL));

    // Change the port in the myAddr struct to the default port
    myAddr.sin_port = htons(PORT);

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

    if (CreateThread(NULL, 0, AcceptSocketThread, NULL, 0, NULL) == NULL)
    {
        qDebug() << "AcceptSocket Thread could not be created";
    }

    return true;
}

// hank revis
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   AcceptSocketThread
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  DWORD WINAPI AcceptSocketThread(LPVOID lpParameter)
--                  LPVOID lpParameter: The thread parameters (unused)
--
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- Creates an accept socket and passes it to a thread that is responsible for
-- file transfer for each client connecting to the server.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI AcceptSocketThread(LPVOID lpParameter)
{
    qDebug() << "inside accept socket thread";
    SOCKET AcceptSocket;
    while(TRUE)
    {
        createAcceptSocket(&ListenSocket, &AcceptSocket);
        if (CreateThread(NULL, 0, FileTransferThread, (void*)AcceptSocket, 0, NULL) == NULL)
        {
            qDebug() << "File transfer (Accept) Socket could not be created";
        }

        //close accept socket after passing a copy to the thread
        //closesocket(AcceptSocket);
    }
}

DWORD WINAPI FileTransferThread(LPVOID lpParameter)
{
    qDebug() << "in file transfer thread";
    SOCKET fileTransferSocket = (SOCKET)lpParameter;
    
    handleControlMessages(fileTransferSocket);

    return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ServerMcastThread
-- DATE:       
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
--                 LPVOID lpParameter: A pointer to the BufferManager object
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
{
    DWORD nBytesRead;
    int nRet;
    BufferManager* bm = (BufferManager*)lpParameter;

    char sendBuff[BUF_LEN];

    while(bm->_isSending) {
        //qDebug() << "_net data : " << bm->_net->getDataAvailable() << "\n_pb data : " << bm->_pb->getDataAvailable();
        //while (bm->_net->getDataAvailable() < BUF_LEN) {};
        nBytesRead = bm->_net->read(sendBuff, BUF_LEN);
        if (nBytesRead > 0) {
            nRet = sendto(sMcastStruct.Sock, sendBuff, nBytesRead, 0, (SOCKADDR *)&(sMcastStruct.mcastAddr), sizeof(sockaddr_in));
            if (nRet < 0)
            {
                qDebug() << "sendto failed" << WSAGetLastError();
            }
        }
    }
    bm->_net->clear(); // clear net after done sending
    qDebug() << "Exiting send thread";
    ExitThread(3);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   setupServerMulticastSocket
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool setupServerMulticastSocket()
--
-- RETURNS:    bool: true if socket is setup succesfully. False otherwise
--
-- NOTES:
-- Creates and bind a UDP multicast socket.
 * Fills in the information for the ip_mreq structure which contains the
 * multicast information for IPv4 addresses
 * The UDP socket is added to the multicast group, the number of
 * network hops(TTL) is set and the loop back is disabled.
----------------------------------------------------------------------------------------------------------------------*/
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
    closesocket(ListenSocket);
    closesocket(sMcastStruct.Sock);
    WSACleanup();
}
