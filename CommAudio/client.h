#ifndef CLIENT_H
#define CLIENT_H

#include "shared.h"
#include "mainwindow.h"
#include "network.h"
#include "playback.h"
#include "bass.h"
#include "buffermanager.h"
void downloadFile(const char* filename);
std::vector<std::string> updateServerFiles();
bool setupTcpSocket(QString ipaddr);
bool setUdpSocket();
bool setupClientMulticastSocket();
void clientCleanup();
void startClientMulticastSession(BufferManager* bm);
void CALLBACK ClientMcastWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientMcastThread(LPVOID lpParameter);
void processIO(char* data, DWORD len);
DWORD WINAPI PlaybackThreadProc(LPVOID lpParamater);
DWORD WINAPI PlaybackFileProc(LPVOID param);
void setFilename(std::string str);
extern MainWindow *mw;
#endif // CLIENT_H
