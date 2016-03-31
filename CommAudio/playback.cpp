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
    return ((Playback*)user)->read((char*)buffer, length);
}

BOOL CALLBACK fileSeekProc(QWORD offset, void* user)
{
    return true;
}


Playback::Playback(int size) {
    if (size <= 0)
        return;
    _data = (char *) malloc(size);
    if (_data == 0)
        return;
    _size = size;
    _readPtr = _writePtr = 0;
    _writeBytesAvailable = size;
    fp = { fileCloseProc, fileOpenProc, fileReadProc, fileSeekProc };
    memset(_data, 0, size);
}

Playback::~Playback() {
    if (_data != 0)
        free(_data);
}

int Playback::read(char * buf, int len) {
    if (buf == 0 || len <= 0 || _writeBytesAvailable == _size || _size == 0)
        {
            return 0;
        }

        int maxRead = _size - _writeBytesAvailable;

        if (len > maxRead)
        {
            len = maxRead;
        }

        if (len > _size - _readPtr)
        {
            int before_wrap = _size - _readPtr;
            memcpy(buf,_data + _readPtr, before_wrap);
            memcpy(buf + before_wrap, _data, len - before_wrap);
        }
        else
        {
            memcpy(buf, _data + _readPtr, len);
        }

        _writeBytesAvailable += len;
        _readPtr = (_readPtr + len) % _size;
        return len;
}

int Playback::write(const char * buf, int len) {
    if (buf == 0 || len <= 0 || _writeBytesAvailable == 0)
        {
            return 0;
        }

        if (len > _writeBytesAvailable)
        {
            len = _writeBytesAvailable;
        }
        if (len > _size - _writePtr)
        {
            int before_wrap = _size - _writePtr;
            memcpy(_data + _writePtr, buf, before_wrap);
            memcpy(_data, buf + before_wrap, len - before_wrap);
        }
        else
        {
            memcpy(_data + _writePtr, buf, len);
        }
        _writePtr = (_writePtr + len) % _size;
        _writeBytesAvailable -= len;

        return len;
}

BASS_FILEPROCS* Playback::getFP() {
    return &fp;
}

DWORD Playback::getSize() {
    return _size;
}
