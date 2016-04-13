#include "network.h"

struct sockaddr_in myAddr;
struct sockaddr_in mcastAddr;

BOOL tFlag = TRUE;
BOOL fFlag = FALSE;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   startWinsock
-- DATE:       10/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void startWinsock()
--
-- RETURNS:    void
--
-- NOTES:
-- Initiates use of the Winsock DLL.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillMyAddrStruct
-- DATE:       10/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void fillMyAddrStruct()
--
-- RETURNS:    void
--
-- NOTES:
-- Fills the sockaddr_in structure for file transfer.
----------------------------------------------------------------------------------------------------------------------*/
void fillMyAddrStruct()
{
    memset((char *)&myAddr, 0, sizeof(sockaddr_in));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(PORT);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillMyAddrStruct
-- DATE:       12/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void fillMcastAddrStruct()
--
-- RETURNS:    void
--
-- NOTES:
-- Fills the sockaddr_in structure for multicast.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   createUdpSocket
-- DATE:       12/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool createUdpSocket(SOCKET* sock)
--                  SOCKET* sock: A pointer to the UdpSocket to create
--
-- RETURNS:    bool: true if socket is created succesfully, false otherwise
--
-- NOTES:
-- Creates a UDP Socket
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   createTcpSocket
-- DATE:       12/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool createUdpSocket(SOCKET* sock)
--                  SOCKET* sock: A pointer to the TCP socket to create
--
-- RETURNS:    bool: true if socket is created succesfully, false otherwise
--
-- NOTES:
-- Creates a TCP Socket
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   createAcceptSocket
-- DATE:       12/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool createAcceptSocket(SOCKET* listenSocket, SOCKET* acceptSocket)
--                  SOCKET* listenSocket: A pointer to the listen socket
--                  SOCKET* acceptSocket: A pointer to the accept socket to create
--
-- RETURNS:    bool: true if socket is created succesfully, false otherwise
--
-- NOTES:
-- Creates an Accept Socket
----------------------------------------------------------------------------------------------------------------------*/
bool createAcceptSocket(SOCKET* listenSocket, SOCKET* acceptSocket)
{
    if((*acceptSocket = accept(*listenSocket, NULL, NULL)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create Accept Socket" << WSAGetLastError();
        closesocket(*acceptSocket);
    }
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillServerMcastStruct
-- DATE:       20/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void fillServerMcastStruct(McastStruct* mcastStruct)
--                  McastStruct* mcastStruct: The McastStruct data structure
--
-- RETURNS:    void
--
-- NOTES:
-- Fills the multicast data structure used by the server during multicast
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillClientMcastStruct
-- DATE:       20/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void fillClientMcastStruct(McastStruct* mcastStruct)
--                  McastStruct* mcastStruct: The McastStruct data structure
--
-- RETURNS:    void
--
-- NOTES:
-- Fills the multicast data structure used by the client during multicast
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillAddrStruct
-- DATE:       20/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void fillAddrStruct(SOCKADDR_IN* sockaddr, short family, unsigned long addr, unsigned short port)
--                  SOCKADDR_IN* sockaddr: The sockaddr_in structure
--                  short family: The address family
--                  unsigned long addr: The ip address
--                  unsigned short port: The port number
-- 
-- RETURNS:    void
--
-- NOTES:
-- Fills the sockaddr_in structure with the specified parameters
----------------------------------------------------------------------------------------------------------------------*/
void fillAddrStruct(SOCKADDR_IN* sockaddr, short family, unsigned long addr, unsigned short port)
{
    memset((char *)sockaddr, 0, sizeof(sockaddr_in));
    sockaddr->sin_family = family;
    sockaddr->sin_addr.s_addr = addr;
    sockaddr->sin_port = port;
}


