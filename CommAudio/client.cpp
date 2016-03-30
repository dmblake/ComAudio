//hank revision

#include "client.h"
#include "mainwindow.h"
SOCKET TcpSocket;
SOCKET UdpSocket;
SOCKET ClientMulticastSocket;
struct ip_mreq ClientMreq;
struct sockaddr_in serverAddr;
struct sockaddr_in peerAddr;
struct sockaddr_in clientMcastAddr;
HANDLE hMulticastThread;

extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;

MainWindow* mw;

void startFileTransfer()
{
    if (!setupTcpSocket())
    {
        qDebug() << "Failed to setup TCP Socket";
    }
}

void startClientMulticastSession()
{
    qDebug() << "startClientMulticastSession called";
    WSAEVENT ThreadEvent;

    if (!setupClientMulticastSocket())
    {
        qDebug() << "failed to setup client multicast socket";
    }
    else
    {
        qDebug() << "Mcast socket ok";
    }

    // Create a worker thread to service completed I/O requests
    if ((hMulticastThread = CreateThread(NULL, 0, ClientMcastThread, (LPVOID)ThreadEvent, 0, NULL)) == NULL)
    {
        qDebug() << "CreateThread() failed with error " << GetLastError();
        return;
    }
}

/*
* Creates and connects the TCP Socket used for both control and file transfer
*/
bool setupTcpSocket()
{
    struct hostent *hp;

    if ((TcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create TCP Socket " << WSAGetLastError();
        return false;
    }

    memset((char *)&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if ((hp = gethostbyname(SERVER_IP)) == NULL)
    {
        qDebug() << "TCP Unknown server address";
        return false;
    }
    memcpy((char *)&serverAddr.sin_addr, hp->h_addr, hp->h_length);

    int enable=1;
    setsockopt(TcpSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));


    if (connect(TcpSocket, (struct sockaddr*)&serverAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to connect to the server" << WSAGetLastError();
        closesocket(TcpSocket);
        WSACleanup();
        return false;
    }

    qDebug() << "Connected: Server Name: " << hp->h_name;

    // Close the socket right way for testing
    closesocket(TcpSocket);
    return true;
}

/*
 * Creates and binds the UDP Socket for mic.
 * Fills the peerAddr sockaddr_in.
 */
bool setUdpSocket()
{
    struct hostent *hp;

    if ((UdpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create UDP Socket";
    }

    memset((char *)&peerAddr, 0, sizeof(sockaddr_in));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(MIC_PORT);

    if ((hp = gethostbyname(PEER_IP)) == NULL)
    {
        qDebug() << "Unkown peer address";
        closesocket(UdpSocket);
        return false;
    }
    memcpy((char *)&peerAddr.sin_addr, hp->h_addr, hp->h_length);

    // Change the port in the myAddr struct to the MIC port
    myAddr.sin_port = htons(MIC_PORT);

    if (bind(UdpSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "bind() udp failed with error" << WSAGetLastError();
        closesocket(UdpSocket);
        return false;
    }
    else
    {
        qDebug() << "accept socket created";
    }

    // close accept socket right away for testing
    closesocket(UdpSocket);
    return true;
}

/*
 * Similar to the server, except the sock options are not set here
 */
bool setupClientMulticastSocket()
{
    char mcast_ip[512] = MCAST_IP;
    if ((ClientMulticastSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create Client Multicast Socket: " << WSAGetLastError();
        closesocket((ClientMulticastSocket));
        return false;
    }

    int enable = 1;
    setsockopt(ClientMulticastSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));

    //
    hostent* hp;
    memset((char *)&mcastAddr, 0, sizeof(struct sockaddr_in));
    mcastAddr.sin_family = AF_INET;
    mcastAddr.sin_port = htons(MCAST_PORT);
    if ((hp = gethostbyname("192.168.1.80")) == NULL)
    {
        qDebug() << "Unknown mcast address";
    }
    memcpy((char *)&mcastAddr.sin_addr, hp->h_addr, hp->h_length);
    qDebug() << "MCAST addr: " << hp->h_name;

    // Change the port in the myAddr struct to the multicast port
    myAddr.sin_port = htons(MCAST_PORT);
    if (bind(ClientMulticastSocket, (struct sockaddr*)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind Client Multicast Socket: " << WSAGetLastError();
        return false;
    }
    else
    {
        qDebug() << "client mcast socket bind() ok";
    }


    // Setting the local IP address of interface and the multicast address group
    ClientMreq.imr_interface.s_addr = INADDR_ANY;
    ClientMreq.imr_multiaddr.s_addr = inet_addr(mcast_ip);

    // Joining the Multicast group
    if (setsockopt(ClientMulticastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ClientMreq, sizeof(ClientMreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_ADD_MEMBERSHIP) failed: " << WSAGetLastError();
        closesocket(ClientMulticastSocket);
        return false;
    }

    // Setting TTL (hops)
    int ttl = 5;
    if (setsockopt(ClientMulticastSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_MULTICAST_TTL) failed: " << WSAGetLastError();
        closesocket(ClientMulticastSocket);
        return false;
    }


    return true;
}

DWORD WINAPI ClientMcastThread(LPVOID lpParameter)
{
    qDebug() << "client mcast thread started";
    DWORD Index;
    DWORD Flags = 0;
    DWORD RecvBytes = 0;
    WSAEVENT EventArray[1];
    LPSOCKET_INFORMATION SocketInfo;
    int addrSize = sizeof(sockaddr_in);

    if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "GlobalAlloc() failed with error %d\n" << GetLastError();
        ExitThread(3);
    }

    SocketInfo->Socket = ClientMulticastSocket;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->DataBuf.len = BUF_LEN;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    sockaddr_in test;
    /*hostent* hp;
    memset((char *)&test, 0, sizeof(struct sockaddr_in));
    mcastAddr.sin_family = AF_INET;
    mcastAddr.sin_port = htons(9001);
    if ((hp = gethostbyname("234.5.6.7")) == NULL)
    {
        qDebug() << "Unknown mcast address";
    }
    memcpy((char *)&test.sin_addr, hp->h_addr, hp->h_length);
    */

    mw->printToListView("TEstiNG");


    if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR*)&mcastAddr,
            &addrSize, &(SocketInfo->Overlapped), ClientMcastWorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSARecv() failed with error" << WSAGetLastError();
        }
        else
        {
            qDebug() << "wsarecvfrom called";
        }

    }


    if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        qDebug() << "failed to create event";
    }
    else
    {
        qDebug() << "event created ok";
    }

    while (TRUE)
    {
        qDebug() << "wait for multiple event";
        Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
        if (Index == WAIT_IO_COMPLETION)
        {
            // An overlapped request completion routine
            // just completed. Continue servicing more completion routines.
            qDebug() << "index = wait io completion";
            continue;
        }
        else
        {
            // A bad error occurred: stop processing!
            // If we were also processing an event
            // object, this could be an index to the event array.
            qDebug() << "error socket closed";
            closesocket(ClientMulticastSocket);
            ExitThread(3);
        }
    }
}

void CALLBACK ClientMcastWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    qDebug() << "inside completion routine";
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
    int addrSize = sizeof(sockaddr_in);

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        qDebug() << "error !=0 and bytes transferred == 0, closing socket";
        closesocket(ClientMulticastSocket);
        GlobalFree(SI);
        return;
    }

    //process io here
    //SI->Buffer
    //BytesTransferred
    //qDebug() << "Received data" << SI->Buffer;
    processIO(SI->Buffer);

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR*)&mcastAddr,
            &addrSize, &(SI->Overlapped), ClientMcastWorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSARecv()1 failed with error " << WSAGetLastError();
        }
    }
}

void processIO(char* data)
{
    qDebug() << data;
}

void clientCleanup()
{
    qDebug() << "client cleanup called";
    closesocket(TcpSocket);
    closesocket(UdpSocket);
    closesocket(ClientMulticastSocket);
    WSACleanup();
}
