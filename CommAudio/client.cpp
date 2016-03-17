#include "client.h"

SOCKET TcpSocket;
SOCKET UdpSocket;
SOCKET ClientMulticastSocket;
struct ip_mreq ClientMreq;
struct sockaddr_in serverAddr;
struct sockaddr_in peerAddr;

extern struct sockaddr_in myAddr;

void startFileTransfer()
{
    if (!setupTcpSocket())
    {
        qDebug() << "Failed to setup TCP Socket";
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
        qWarning() << "Failed to create TCP Socket";
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

    if (connect(TcpSocket, (struct sockaddr*)&serverAddr, sizeof(sockaddr_in)) == -1)
    {
        qDebug() << "Failed to connect to the server";
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
 * Creates and binds the UDP Socket.
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
    myAddr.sin_port = MIC_PORT;

    if (bind(UdpSocket, (PSOCKADDR)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "bind() udp failed with error" << WSAGetLastError();
        closesocket(UdpSocket);
        return false;
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
    if ((ClientMulticastSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        qDebug() << "Failed to create Client Multicast Socket: " << WSAGetLastError();
        closesocket((ClientMulticastSocket));
        return false;
    }

    // Change the port in the myAddr struct to the multicast port
    myAddr.sin_port = MCAST_PORT;

    if (bind(ClientMulticastSocket, (struct sockaddr*)&myAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        qDebug() << "Failed to bind Client Multicast Socket: " << WSAGetLastError();
        return false;
    }

    // Setting the local IP address of interface and the multicast address group
    ClientMreq.imr_interface.s_addr = INADDR_ANY;
    ClientMreq.imr_multiaddr.s_addr = inet_addr(MCAST_IP);


    // Joining the Multicast group
    if (setsockopt(ClientMulticastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ClientMreq, sizeof(ClientMreq)) == SOCKET_ERROR)
    {
        qDebug() << "setsockopt(IP_ADD_MEMBERSHIP) failed: " << WSAGetLastError();
        closesocket(ClientMulticastSocket);
        return false;
    }
    return true;
}

void clientCleanup()
{
    qDebug() << "client cleanup called";
    closesocket(TcpSocket);
    closesocket(UdpSocket);
    closesocket(ClientMulticastSocket);
    WSACleanup();
}
