#ifndef CLIENT_H
#define CLIENT_H

#include "shared.h"
#include "mainwindow.h"
#include "network.h"
#include "playback.h"
#include "bass.h"
void downloadFile(const char* filename);
void startMicrophone(const char* ipaddress);
DWORD WINAPI sendThread(LPVOID lpParameter);
DWORD WINAPI receiveThread(LPVOID lpParameter);
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
DWORD WINAPI PlaybackFileProc(LPVOID param);
void setFilename(std::string str);
extern CircularBuffer* networkBuffer;
extern Playback* playbackBuffer;
extern MainWindow *mw;
#endif // CLIENT_H
