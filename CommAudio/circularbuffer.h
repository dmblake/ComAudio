#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include "bass.h"
#define BUF_LEN 1024

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
    int clear();
protected:
    bool _reset = false;
private:
    char * _data;
    int _writeBytesAvailable;
    int _readPtr;
    int _writePtr;
    int _size;
};

#endif // CIRCULARBUFFER_H
