//hank revision

#include "client.h"
#include "mainwindow.h"
#include "FileUtil.h"
SOCKET TcpSocket;
SOCKET UdpSocket;
SOCKET ClientMulticastSocket;
struct ip_mreq ClientMreq;
struct sockaddr_in serverAddr;
struct sockaddr_in peerAddr;
struct sockaddr_in clientMcastAddr;
HANDLE hMulticastThread;
HANDLE hPlaybackThread;
McastStruct cMcastStruct;
extern struct sockaddr_in mcastAddr;
extern struct sockaddr_in myAddr;

SOCKET ClientListenSocket;
MainWindow* mw;
Playback* playbackBuffer;
CircularBuffer* networkBuffer;
bool playing = false;

void startClient()
{
    playbackBuffer = new Playback(MAX_BUF);
    networkBuffer = new CircularBuffer(MAX_BUF);


    if (!BASS_Init(-1, 44100, 0, 0, 0)) {
        qDebug() << "Failed to init bass " << BASS_ErrorGetCode();
        mw->printToListView("Failed to init BASS");
    }

}

// handles when you press the play button
// temp function
void playback()
{
    playing = true;
    hPlaybackThread = CreateThread(NULL, 0, PlaybackThreadProc, NULL, 0, NULL);

}

DWORD WINAPI PlaybackThreadProc(LPVOID lpParamater) {
    HSTREAM str = 0;
    char tmp[BUF_LEN];
    int datalen= 0;
    int netdata = 0;
    int playspace = 0;
    Playback *pb = playbackBuffer;
    memset(tmp, 0, BUF_LEN);


    // main playback loop
    while (playing) {
        // if there is space in the playback buffer and data in the network buffer
        netdata = networkBuffer->getDataAvailable();
        playspace = playbackBuffer->getSpaceAvailable();
        if (playspace > 0 && netdata > 0) {
            // set amount of data to copy
            datalen = (netdata > BUF_LEN) ? BUF_LEN : netdata;
            networkBuffer->read(tmp, datalen);
            playbackBuffer->write(tmp, datalen);
            // if stream is not created, do so now
            // the check on data available is to ensure the entire header has been read in
            if (str == 0 && playbackBuffer->getDataAvailable() > 20000) {
                if (!(str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, playbackBuffer->getFP(), playbackBuffer))) {
                    qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
                    mw->printToListView("Failed to create stream");
                }

                // auto start, move this away!
                if (!BASS_ChannelPlay(str, FALSE)) {
                    qDebug() << "Failed to play" << BASS_ErrorGetCode();
                    mw->printToListView(("Failed to play stream"));
                }
            } // end init stream
        } // end adding to data
        if (playbackBuffer->getDataAvailable() > 20000) {
            int act = BASS_ChannelIsActive(str);
            switch (BASS_ChannelIsActive(str)) {
            case BASS_ACTIVE_PAUSED:
                break;
            case BASS_ACTIVE_PLAYING:
                break;
            case BASS_ACTIVE_STALLED:
                break;
            case BASS_ACTIVE_STOPPED:
                // play
                if (!(str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, playbackBuffer->getFP(), playbackBuffer))) {
                    qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
                    mw->printToListView("Failed to create stream");
                }
                // auto start, move this away!
                if (!BASS_ChannelPlay(str, FALSE)) {
                    qDebug() << "Failed to play" << BASS_ErrorGetCode();
                    mw->printToListView(("Failed to play stream"));
                }
                break;
            case -1:
                qDebug() << "Error in BASS status";

            }
        }

        /*else if (BASS_ChannelIsActive(str) == BASS_ACTIVE_PLAYING && playbackBuffer->getDataAvailable() < 20000){
            // stop
            if (!BASS_ChannelPause(str)) {
                qDebug() << "Failed to stop" << BASS_ErrorGetCode();
             }
        }
        */
    } // end while loop
    return 1;
} // end thread proc

void startFileTransfer()
{
    std::string str = getListFromServer(TcpSocket);
    QString qstr = QString::fromStdString(str);
    qDebug() << qstr;
}

void startClientMulticastSession()
{
    qDebug() << "startClientMulticastSession called";
    WSAEVENT ThreadEvent;
    startClient();

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
    playback();
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
        WSACleanup();
        return false;
    }

    qDebug() << "Connected: Server Name: " << hp->h_name;

    // Close the socket right way for testing
    //closesocket(TcpSocket);
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

    SocketInfo->Socket = cMcastStruct.Sock;
    ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    SocketInfo->DataBuf.len = BUF_LEN;
    SocketInfo->DataBuf.buf = SocketInfo->Buffer;

    mw->printToListView("TEstiNG");


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
    if (networkBuffer->getSpaceAvailable() > len) {
        networkBuffer->write(data, len);
    }
}

bool setupClientListenSocket()
{
    qDebug()<< "setupClientListenSocket called";
    createTcpSocket(&ClientListenSocket);

    // set reuseaddr
    setsockopt(ClientListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&tFlag, sizeof(BOOL));

    // Change the port in the myAddr struct to the default port
    myAddr.sin_port = htons(MIC_PORT);

    // Bind the listen socket
    if (bind(ClientListenSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind the listen socket";
        return false;
    }

    // Setup the ListenSocket to listen for incoming connections
    // with a backlog size 5
    if (listen(ClientListenSocket, 5))
    {
        qDebug() << "listen() failed";
        return false;
    }

    /*
    if (CreateThread(NULL, 0, AcceptSocketThread, NULL, 0, NULL) == NULL)
    {
        qDebug() << "AcceptSocket Thread could not be created";
    }*/

    return true;
}

/* */
DWORD WINAPI ClientAcceptSocketThread(LPVOID lpParameter)
{
    qDebug() << "inside accept socket thread";
    SOCKET AcceptSocket;
    while(TRUE)
    {
        createAcceptSocket(&ClientListenSocket, &AcceptSocket);

        //handle mic stuff

        //close accept socket after passing a copy to the thread
        closesocket(AcceptSocket);
    }
}

void clientCleanup()
{
    qDebug() << "client cleanup called";
    closesocket(TcpSocket);
    closesocket(UdpSocket);
    closesocket(cMcastStruct.Sock);
    WSACleanup();
}
