#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include "bass.h"

class CircularBuffer
{
public:
    CircularBuffer(int size);
    ~CircularBuffer();
    int read(char * buf, int len);
    int write(const char * buf, int len);
    DWORD getSize();
    int getSpaceAvailable();
    int getDataAvailable();
private:
    char * _data;
    int _writeBytesAvailable;
    int _readPtr;
    int _writePtr;
    int _size;
};

#endif // CIRCULARBUFFER_H
