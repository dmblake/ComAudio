#include "server.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:         server.cpp
--
-- PROGRAM:             CommAudio
--
-- FUNCTIONS:           void startServer()
--                      void startServerMulticastSession(BufferManager* bm)
--                      bool setupListenSocket()
--                      DWORD WINAPI AcceptSocketThread(LPVOID lpParameter)
--                      DWORD WINAPI FileTransferThread(LPVOID lpParameter)
--                      DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
--                      bool setupServerMulticastSocket()
--                      void serverCleanup()
--
-- DATE:                10/03/2016
--
-- REVISIONS:           13/04/2016 - Added cleanup function
--                      10/04/2016 - Added wrapper functions to be called by Qt button pushes
--                      23/03/2016 - Created functions for multicast
--
-- DESIGNER:            Joseph Tam-Huang
--
-- PROGRAMMER:          Joseph Tam-Huang, Dhivya Manohar, Dylan Blake, Hank Lo
--
-- NOTES:               
-- Responsible for creating and setting up all server related sockets. Sends 
-- and processes incoming data from the clients.
----------------------------------------------------------------------------------------------------------------------*/

SOCKET ListenSocket;
SOCKET ServerMulticastSocket;
HANDLE hFileTransferThread;
struct ip_mreq ServerMreq;

u_long ttl = MCAST_TTL;
extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;
McastStruct sMcastStruct;

/*------------------------------------------------------------------------------------------------------------------
-- void startServer()
-- DATE:       10/04/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void startServer()
--
-- RETURNS:    void
--
-- NOTES:
-- Sets up the TCP listen socket.
----------------------------------------------------------------------------------------------------------------------*/
void startServer()
{
    if (!setupListenSocket())
    {
        qDebug() << "failed to setup ListenSocket";
    }
    else
    {
        qDebug() << "Listen Socket OK";
    }    
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   startServerMulticastSession
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void startServerMulticastSession(BufferManager* bm)
--                  BufferManager* bm: The pointer to the buffer manager
--
-- RETURNS:    void
--
-- NOTES:
-- Creates and sets up the multicast socket and starts a thread to handle the 
-- incoming datagrams.
----------------------------------------------------------------------------------------------------------------------*/
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
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   FileTransferThread
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  DWORD WINAPI FileTransferThread(LPVOID lpParameter)
--                  LPVOID lpParameter: The thread parameters (unused)
--
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- Thread responsible for sending and receiving control messages and initiating
-- file transfer.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI FileTransferThread(LPVOID lpParameter)
{
    qDebug() << "in file transfer thread";
    SOCKET fileTransferSocket = (SOCKET)lpParameter;
    
    handleControlMessages(fileTransferSocket);

    return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ServerMcastThread
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
--                 LPVOID lpParameter: A pointer to the BufferManager object
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- Reads data from the circular buffer and send it over the network through the
-- multicast socket.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ServerMcastThread(LPVOID lpParameter)
{
    DWORD nBytesRead;
    int nRet;
    BufferManager* bm = (BufferManager*)lpParameter;

    char sendBuff[BUF_LEN];

    while(bm->_isSending) {
        
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   serverCleanup
-- DATE:       13/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void serverCleanup()
--                  
-- RETURNS:    void
--
-- NOTES:
-- Socket cleanup code.
----------------------------------------------------------------------------------------------------------------------*/
void serverCleanup()
{
    qDebug() << "server cleanup called";
    closesocket(ListenSocket);
    closesocket(sMcastStruct.Sock);
    WSACleanup();
}
