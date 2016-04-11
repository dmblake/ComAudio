#ifndef CLIENT_H
#define CLIENT_H

#include "shared.h"
#include "mainwindow.h"
#include "network.h"
#include "playback.h"
#include "bass.h"
std::vector<std::string> updateServerFiles();
bool setupTcpSocket(QString ipaddr);
bool setUdpSocket();
bool setupClientMulticastSocket();
void clientCleanup();
void startClientMulticastSession();
void CALLBACK ClientMcastWorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI ClientMcastThread(LPVOID lpParameter);
void processIO(char* data, DWORD len);
void startClient();
void playback();
DWORD WINAPI PlaybackThreadProc(LPVOID lpParamater);
extern CircularBuffer* networkBuffer;
extern Playback* playbackBuffer;
extern MainWindow *mw;
#endif // CLIENT_H
