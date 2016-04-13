#include "circularbuffer.h"

CircularBuffer::CircularBuffer(int size) {
    if (size <= 0)
        return;
    _data = (char *) malloc(size);
    if (_data == 0)
        return;
    _size = size;
    _readPtr = _writePtr = 0;
    _writeBytesAvailable = size;
    memset(_data, 0, size);
}

CircularBuffer::~CircularBuffer() {
    if (_data != 0)
        free(_data);
}

DWORD CircularBuffer::read(char * buf, DWORD len) {
    if (buf == 0 || len <= 0 || _writeBytesAvailable == _size || _size == 0)
        {
            return 0;
        }

        DWORD maxRead = _size - _writeBytesAvailable;

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

DWORD CircularBuffer::write(const char * buf, DWORD len) {
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

DWORD CircularBuffer::getSize() {
    return _size;
}

DWORD CircularBuffer::getSpaceAvailable() {
    return _writeBytesAvailable;
}

DWORD CircularBuffer::getDataAvailable() {
    return (_size - _writeBytesAvailable);
}

// clears buffer
// returns amount of space available
DWORD CircularBuffer::clear() {
    memset(_data, 0, _size);
    _writePtr = _readPtr = 0;
    _writeBytesAvailable = _size;
    return _writeBytesAvailable;
}

