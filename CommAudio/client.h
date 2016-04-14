#ifndef CLIENT_H
#define CLIENT_H

#include <QBuffer>
#include "shared.h"
#include "mainwindow.h"
#include "microphonedialog.h"
#include "network.h"
#include "bass.h"
#include "buffermanager.h"


struct ThreadSockStruct
{
  SOCKET Sock;
  SOCKADDR_IN peerAddr;
};

void startFileTransfer();

void downloadFile(const char* filename);
void startMicrophone(const char * ipaddress, MicrophoneDialog *md, BufferManager* bm);
DWORD WINAPI sendThread(LPVOID lpParameter);
DWORD WINAPI receiveThread(LPVOID lpParameter);
std::vector<std::string> updateServerFiles();
bool setupTcpSocket(QString ipaddr);
bool setUdpSocket();
bool setupClientMulticastSocket();
void clientCleanup();
void startClientMulticastSession(BufferManager* bm);
void CALLBACK ClientMcastWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void CALLBACK ClientMicRecvWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientMcastThread(LPVOID lpParameter);
DWORD WINAPI ClientMicRecvThread(LPVOID lpParameter);
bool fillPeerAddrStruct(const char* peerIp);
void processIO(char* data, DWORD len);
void processMicIO(char* data, DWORD len);
DWORD WINAPI PlaybackThreadProc(LPVOID lpParamater);
DWORD WINAPI PlaybackFileProc(LPVOID param);
void setFilename(std::string str);
extern MainWindow *mw;

#endif // CLIENT_H
