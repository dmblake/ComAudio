#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <QDebug>
#include "shared.h"
#include "circularbuffer.h"


class BufferManager
{
public:
    BufferManager(int len, bool server);
    static DWORD play(LPVOID param);
    DWORD startPlayThread(LPVOID param);
    CircularBuffer * _pb;
    CircularBuffer * _net;
    bool _isServer;
    bool _isPlaying;
    static DWORD loadFromFile(LPVOID param);
    DWORD BufferManager::startReadThread(LPVOID instance);
    bool BufferManager::setFilename(const char * fn);
    BASS_FILEPROCS* getFP();
private:
    char * _filename;
    BASS_FILEPROCS _fp;

};

#endif // BUFFERMANAGER_H
