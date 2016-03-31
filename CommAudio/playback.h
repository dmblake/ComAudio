#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bass.h"




class Playback
{
public:
    Playback(int size);
    ~Playback();
    int read(char * buf, int len);
    int write(const char * buf, int len);
    BASS_FILEPROCS* getFP();
    DWORD getSize();
private:
    char * _data;
    int _writeBytesAvailable;
    int _readPtr;
    int _writePtr;
    int _size;
    BASS_FILEPROCS fp;
    /*
    void CALLBACK fileCloseProc(void *user);
    QWORD CALLBACK fileOpenProc(void *user);
    DWORD CALLBACK fileReadProc(void *buffer, DWORD length, void *user);
    BOOL CALLBACK fileSeekProc(QWORD offset, void* user);
    */
};

#endif // PLAYBACK_H
