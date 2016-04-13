#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include "shared.h"

class CircularBuffer
{
public:
    CircularBuffer(int size);
    ~CircularBuffer();
    DWORD read(char * buf, DWORD len);
    DWORD write(const char * buf, DWORD len);
    DWORD getSize();
    DWORD getSpaceAvailable();
    DWORD getDataAvailable();
    DWORD clear();
protected:
    bool _reset = false;
private:
    char * _data;
    DWORD _writeBytesAvailable;
    DWORD _readPtr;
    DWORD _writePtr;
    DWORD _size;
};

#endif // CIRCULARBUFFER_H
