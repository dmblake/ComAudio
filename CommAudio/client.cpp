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
CircularBuffer* networkBuffer;
bool playing = false;
MicrophoneDialog *micD;


// hank
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   updateServerFiles
-- DATE:       11/04/2016
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  std::vector<std::string> updateServerFiles()
--                  
-- RETURNS:    std::vector<std::string>: The list of file names
--
-- NOTES:
-- Retrieves the files available for download from the server.
----------------------------------------------------------------------------------------------------------------------*/
std::vector<std::string> updateServerFiles()
{
    std::string str = getListFromServer(TcpSocket);

    std::vector<std::string> filesAndSizes = split(str, '\n');

    return filesAndSizes;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   startClientMulticastSession
-- DATE:       24/03/2016
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  void startClientMulticastSession(BufferManager* bufman)
--                  BufferManager* bufman: A pointer to the buffer manager
--                  
-- RETURNS:    void
--
-- NOTES:
-- Initialized the client multicast session. 
-- Creates and sets up the multicast socket and starts a worker thread to serivce 
-- the completed I/O requests.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   startClientMulticastSession
-- DATE:       24/03/2016
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  void startClientMulticastSession(BufferManager* bufman)
--                  BufferManager* bufman: A pointer to the buffer manager
--                  
-- RETURNS:    void
--
-- NOTES:
-- Initialized the client multicast session. 
-- Creates and sets up the multicast socket and starts a worker thread to serivce 
-- the completed I/O requests.
----------------------------------------------------------------------------------------------------------------------*/
void startMicrophone(const char * ipaddress, MicrophoneDialog *md, BufferManager * bufman)
{
    micD = md;
    micD->audioOutputDevice = micD->audioOutput->start();
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
        qDebug() << "Cannot fill peerAddr";
    }

    if ((hMicThread = CreateThread(NULL, 0, ClientMicRecvThread, md,
        0, NULL)) == NULL)
    {
        qDebug() << "CreateMicRecvThread() failed with error " << GetLastError();
        return;
    }

    CreateThread(NULL,0,sendThread,md,0,NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   setupTcpSocket
-- DATE:       10/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool setupTcpSocket(QString ipaddr)
--                 QString ipaddr: The ip address of the server
--
-- RETURNS:    bool: true if socket is setup succesfully. False otherwise
--
-- NOTES:
-- Creates and connects the TCP Socket used for both control and file transfer with the server.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   fillPeerAddrStruct
-- DATE:       13/04/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool fillPeerAddrStruct(const char* peerIp)
--                 const char* peerIp: The ip of the client trying to communicate with
--
-- RETURNS:    bool: true if the addr was filled successfully, false otherwise
--
-- NOTES:
-- Fills the peerAddr sockaddr_in.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   setUdpSocket
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool setUdpSocket()
--
-- RETURNS:    bool: true if socket is setup succesfully. False otherwise
--
-- NOTES:
-- Create and bind the client udp socket used for microphone communication.
----------------------------------------------------------------------------------------------------------------------*/
bool setUdpSocket()
{
    if ((UdpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create UDP Socket";
    }

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
    return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   setupClientMulticastSocket
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  bool setupClientMulticastSocket()
--
-- RETURNS:    bool: true if socket is setup succesfully. False otherwise
--
-- NOTES:
-- Create and bind the client multicast socket. 
-- Enable reuseaddr and join the multicast group.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ClientMicRecvThread
-- DATE:       13/04/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  DWORD WINAPI ClientMcastThread(LPVOID lpParameter)
--                 LPVOID lpParameter: The thread parameters (unused)
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- Creates a thread that handles the processing of WSARecvFrom events from the
-- microphone udp socket and passes them to a worker routing to complete the 
-- socket I/O processing.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ClientMicRecvThread(LPVOID lpParameter)
{
    MicrophoneDialog *md = (MicrophoneDialog*)lpParameter;
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
        else if (!micD->isRecording){
            qDebug() << "error socket closed";
            closesocket(UdpSocket);
            ExitThread(3);
        }
        else
        {
            // A bad error occurred: stop processing!
            // If we were also processing an event
            // object, this could be an index to the event array.
            qDebug() << "error socket closed";
            closesocket(UdpSocket);
            ExitThread(3);
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ClientMicRecvWorkerRoutine
-- DATE:       12/04/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void CALLBACK ClientMicRecvWorkerRoutine(DWORD Error, 
--                  DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--                  DWORD Error: The error code
--                  DWORD BytesTransferred: The number of bytes received
--                  LPSAOVERLAPPED Overlapped: The overlapped structure
--                  DWORD InFLags: The flags
--                  
-- RETURNS:    void
--
-- NOTES:
-- Completion routine that gets called when the event from WSARecvFrom event is triggered.
-- Processes incoming UDP datagrams from the microphone session.
-- Places another WSARecvFrom call with this completion routine as its completion routine.
----------------------------------------------------------------------------------------------------------------------*/
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
    processMicIO(SI->Buffer, BytesTransferred);

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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ClientMcastThread
-- DATE:       23/03/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  DWORD WINAPI ClientMcastThread(LPVOID lpParameter)
--                 LPVOID lpParameter: The thread parameters (unused)
-- RETURNS:    DWORD: The exit code of the thread
--
-- NOTES:
-- Creates a thread that handles the processing of WSARecvFrom events from the
-- multicast socket and passes them to a worker routing to complete the socket
-- I/O processing.
----------------------------------------------------------------------------------------------------------------------*/
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

    while (bm->_isPlaying)
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
    qDebug() << "exiting client mcast thread";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   ClientMcastWorkerRoutine
-- DATE:       12/04/2016
-- REVISIONS:  
-- DESIGNER:   Joseph Tam-Huang
-- PROGRAMMER: Joseph Tam-Huang
--
-- INTERFACE:  void CALLBACK ClientMcastWorkerRoutine(DWORD Error, 
--                  DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--                  DWORD Error: The error code
--                  DWORD BytesTransferred: The number of bytes received
--                  LPSAOVERLAPPED Overlapped: The overlapped structure
--                  DWORD InFLags: The flags
--                  
-- RETURNS:    void
--
-- NOTES:
-- Completion routine that gets called when the event from WSARecvFrom event is triggered.
-- Processes incoming UDP datagrams from the multicast session.
-- Places another WSARecvFrom call with this completion routine as its completion routine.
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   processIO
-- DATE:       15/03/2016
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  void processIO(char* data, DWORD len)
--                  char* data: The data to process
--                  DWORD len: The lenght of the data to process
--                  
-- RETURNS:    void
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
void processIO(char* data, DWORD len)
{
    //qDebug() << data;
    if (bm->_pb->getSpaceAvailable() > len) {
        bm->_pb->write(data, len);
    }
}

void processMicIO(char* data, DWORD len)
{
    //QByteArray* qba = new QByteArray(data, len);
    micD->audioOutputDevice->write(data,len);

}

void clientCleanup()
{
    qDebug() << "client cleanup called";
    //closesocket(TcpSocket);
    //closesocket(UdpSocket);
    //closesocket(cMcastStruct.Sock);
    WSACleanup();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:   downloadFile
-- DATE:       11/04/2016
-- REVISIONS:  
-- DESIGNER:   
-- PROGRAMMER: 
--
-- INTERFACE:  void downloadFile(const char* filename)
--                  const char* filename: The name of the file to download
--                  
-- RETURNS:    void
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
void downloadFile(const char* filename){
    int size;
    std::vector<std::string> fns = split(filename, ',');
    size = std::stoi(fns[1]);
    getFileFromServer(TcpSocket, fns[0].c_str(), size);
}

DWORD WINAPI sendThread(LPVOID lpParameter){

    MicrophoneDialog* md = (MicrophoneDialog *) lpParameter;
    QByteArray qba;
    qint64 len = 0;
    char temp[BUF_LEN];
   //md->audioOutputDevice = md->audioOutput->start();


    while(md->isRecording){
        qba = md->audioInputDevice->readAll();
        int nRet;
        //md->audioOutputDevice->write(qba);
        if (qba.length() > 0) {
            nRet = sendto(UdpSocket, qba, qba.length(), 0, (SOCKADDR *)&(peerAddr), sizeof(sockaddr_in));
            if (nRet < 0)
            {
                qDebug() << "sendto failed" << WSAGetLastError();
            }
        }
    }
    return 1;
}
