//hank revision

#include "client.h"
#include "mainwindow.h"
#include "FileUtil.h"
SOCKET TcpSocket;
SOCKET UdpSocket;
const char* ipAddr;
SOCKET ClientMulticastSocket;
struct ip_mreq ClientMreq;
struct sockaddr_in serverAddr;
struct sockaddr_in peerAddr;
struct sockaddr_in clientMcastAddr;
HANDLE hMulticastThread;
HANDLE hMicThread;
HANDLE hPlaybackThread;
HANDLE hFileReadThread;
McastStruct cMcastStruct;
ThreadSockStruct micUdpStruct;

extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;

BufferManager* bm;
MainWindow* mw;
Playback* playbackBuffer;
CircularBuffer* networkBuffer;
bool playing = false;


// hank
std::vector<std::string> updateServerFiles()
{
    std::string str = getListFromServer(TcpSocket);

    std::vector<std::string> filesAndSizes = split(str, '\n');

    return filesAndSizes;
}

void startClientMulticastSession(BufferManager* bufman)
{
    qDebug() << "startClientMulticastSession called";
    WSAEVENT ThreadEvent;
    bm = bufman;
    //startClient();

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
void startMicrophone(const char * ipaddress, QBuffer* qbuf, BufferManager * bufman){
    ipAddr = ipaddress;
    qDebug() << ipAddr;
    bm = bufman;
    if (!setUdpSocket())
    {
        qDebug() << "Mic Udp socket created";
        return;
    }

    //micUdpStruct.Sock = UdpSocket;
    //micUdpStruct.peerAddr = peerAddr;
    if (!fillPeerAddrStruct(ipAddr))
    {
        qDebug() << "Cannot fill perrAddr";
    }

    if ((hMicThread = CreateThread(NULL, 0, ClientMicRecvThread, NULL,
        0, NULL)) == NULL)
    {
        qDebug() << "CreateMicRecvThread() failed with error " << GetLastError();
        return;
    }

    CreateThread(NULL,0,sendThread,qbuf,0,NULL);
}

/*
* Creates and connects the TCP Socket used for both control and file transfer
*/
bool setupTcpSocket(QString ipaddr)
{
    struct hostent *hp;
    QByteArray ipArray = ipaddr.toUtf8();
    const char* ipAddress = ipArray;

    if ((TcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create TCP Socket " << WSAGetLastError();
        return false;
    }

    memset((char *)&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if ((hp = gethostbyname(ipAddress)) == NULL)
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
        //WSACleanup();
        return false;
    }

    qDebug() << "Connected: Server Name: " << hp->h_name;

    // Close the socket right way for testing
    //closesocket(TcpSocket);
    return true;
}

bool fillPeerAddrStruct(const char* peerIp)
{
    struct hostent *hp;
    
    memset((char *)&peerAddr, 0, sizeof(sockaddr_in));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(MIC_PORT);

    if ((hp = gethostbyname(peerIp)) == NULL)
    {
        qDebug() << "Unkown peer address";
        closesocket(UdpSocket);
        return false;
    }
    memcpy((char *)&peerAddr.sin_addr, hp->h_addr, hp->h_length);
    return true;
}

/*
 * Creates and binds the UDP Socket for mic.
 * Fills the peerAddr sockaddr_in.
 */
bool setUdpSocket()
{
    if ((UdpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create UDP Socket";
    }

    // memset((char *)&peerAddr, 0, sizeof(sockaddr_in));
    // peerAddr.sin_family = AF_INET;
    // peerAddr.sin_port = htons(MIC_PORT);

    // if ((hp = gethostbyname(PEER_IP)) == NULL)
    // {
    //     qDebug() << "Unkown peer address";
    //     closesocket(UdpSocket);
    //     return false;
    // }
    // memcpy((char *)&peerAddr.sin_addr, hp->h_addr, hp->h_length);

    // Change the port in the myAddr struct to the MIC port
    myAddr.sin_port = htons(MIC_PORT);

    // Enable resuseaddr
    if (setsockopt(UdpSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL)) == SOCKET_ERROR)

    {
        qDebug() << "setsockopt(SO_REUSEADDR) failed: " << WSAGetLastError();
        closesocket(UdpSocket);
        return false;
    }
    if (bind(UdpSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "bind() udp failed with error" << WSAGetLastError();
        closesocket(UdpSocket);
        return false;
    }

    // close accept socket right away for testing
    //closesocket(UdpSocket);
    return true;
}

/*
 * Similar to the server, except the sock options are not set here
 */
bool setupClientMulticastSocket()
{
    fillClientMcastStruct(&cMcastStruct);

    // Enable resuseaddr
    if (setsockopt(cMcastStruct.Sock, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(SO_REUSEADDR) failed: " << WSAGetLastError();
        closesocket(cMcastStruct.Sock);
        return false;
    }
    
    // bind the mcast socket
    if (bind(cMcastStruct.Sock, (struct sockaddr*)&(cMcastStruct.bindAddr), sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind Client Multicast Socket: " << WSAGetLastError();
        closesocket(cMcastStruct.Sock);
        return false;
    }

    // join the mcast group
    if (setsockopt(cMcastStruct.Sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&cMcastStruct.mreq, sizeof(ip_mreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_ADD_MEMBERSHIP) failed: " << WSAGetLastError();
        closesocket(cMcastStruct.Sock);
        return false;
    }

    return true;
    
}

DWORD WINAPI ClientMicRecvThread(LPVOID lpParameter)
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

    SocketInfo->Socket = UdpSocket;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->DataBuf.len = BUF_LEN;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    mw->printToListView("TEstiNG");


    if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
        (SOCKADDR*)&(peerAddr), &addrSize, &(SocketInfo->Overlapped),
        ClientMicRecvWorkerRoutine) == SOCKET_ERROR)
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

void CALLBACK ClientMicRecvWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    //qDebug() << "inside completion routine";
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
    int addrSize = sizeof(sockaddr_in);

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        qDebug() << "error !=0 and bytes transferred == 0, closing socket";
        closesocket(cMcastStruct.Sock);
        GlobalFree(SI);
        return;
    }

    //process io here
    //SI->Buffer
    //BytesTransferred
    //qDebug() << "Received data" << SI->Buffer;
    processIO(SI->Buffer, BytesTransferred);

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR*)&(peerAddr),
            &addrSize, &(SI->Overlapped), ClientMicRecvWorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSARecv()1 failed with error " << WSAGetLastError();
        }
    }
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
    BufferManager* bufman = bm;

    if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        qDebug() << "GlobalAlloc() failed with error %d\n" << GetLastError();
        ExitThread(3);
    }

    SocketInfo->Socket = cMcastStruct.Sock;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->DataBuf.len = BUF_LEN;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR*)&(cMcastStruct.mcastAddr),
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
        //qDebug() << "wait for multiple event";
        Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, 10000, TRUE);
        if (Index == WAIT_IO_COMPLETION)
        {
            // An overlapped request completion routine
            // just completed. Continue servicing more completion routines.
            //qDebug() << "index = wait io completion";
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
    //qDebug() << "inside completion routine";
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
    int addrSize = sizeof(sockaddr_in);

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        qDebug() << "error !=0 and bytes transferred == 0, closing socket";
        closesocket(cMcastStruct.Sock);
        GlobalFree(SI);
        return;
    }

    //process io here
    //SI->Buffer
    //BytesTransferred
    //qDebug() << "Received data" << SI->Buffer;
    processIO(SI->Buffer, BytesTransferred);

    ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, (SOCKADDR*)&(cMcastStruct.mcastAddr),
            &addrSize, &(SI->Overlapped), ClientMcastWorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            qDebug() << "WSARecv()1 failed with error " << WSAGetLastError();
        }
    }
}

void processIO(char* data, DWORD len)
{
    //qDebug() << data;
    if (bm->_pb->getSpaceAvailable() > len) {
        bm->_pb->write(data, len);
    }
}

void clientCleanup()
{
    qDebug() << "client cleanup called";
    //closesocket(TcpSocket);
    //closesocket(UdpSocket);
    //closesocket(cMcastStruct.Sock);
    WSACleanup();
}


void downloadFile(const char* filename){
    getFileFromServer(TcpSocket, filename, 357420);
}

DWORD WINAPI sendThread(LPVOID lpParameter){

    QBuffer* sendBuffer = (QBuffer *) lpParameter;
    QByteArray qba;
    qint64 len = 0;
    char temp[BUF_LEN];

    while(1){
        //send function
        qba = sendBuffer->read(BUF_LEN);
        while (bm->_pb->getSpaceAvailable() < qba.length()) {
            // do nothing
        }
        bm->_pb->write(qba.data(), qba.length());
    }
    return 1;
}

DWORD WINAPI receiveThread(LPVOID lpParameter){

    while(1){
        //receive function
    }
    return 1;
}
