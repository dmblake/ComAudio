#include "playback.h"


void CALLBACK fileCloseProc(void *user)
{
}

QWORD CALLBACK fileOpenProc(void *user)
{
    return 0;
}

DWORD CALLBACK fileReadProc(void *buffer, DWORD length, void *user)
{
    return ((CircularBuffer*)user)->read((char*)buffer, length);
}

BOOL CALLBACK fileSeekProc(QWORD offset, void* user)
{
    return true;
}


Playback::Playback(int size) {
    _cb = new CircularBuffer(size);
    fp = { fileCloseProc, fileOpenProc, fileReadProc, fileSeekProc };
}

Playback::~Playback() {
    if (_cb != 0)
        delete [] _cb;
}

BASS_FILEPROCS* Playback::getFP() {
    return &fp;
}
