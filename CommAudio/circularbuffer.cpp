/*
 * Source file: circularbuffer.cpp
 * Date: March 21, 2016
 * Revision:v2 -- added clear functionality
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Functions:
 *  CircularBuffer(int size);
    ~CircularBuffer();
    DWORD read(char * buf, DWORD len);
    DWORD write(const char * buf, DWORD len);
    DWORD getSize();
    DWORD getSpaceAvailable();
    DWORD getDataAvailable();
    DWORD clear();
 *
 * Notes:
 * Circular buffer class, to hold both playback and networking buffers.
 */

#include "circularbuffer.h"

/*
 * Function: CircularBuffer
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: CircularBuffer(int size)
 *              int size : size of buffer to instantiate
 * Returns constructor (nothing)
 * Notes:
 *  Creates a CircularBuffer of "size" bytes
 */
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

/*
 * Function: read
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD read(char * buf, DWORD len)
 *              char * buf : pointer to data to copy to
 *              DWORD len : length of data to read
 * Returns number of bytes copied to buf
 * Notes:
 *  Reads up to "len" bytes from the buffer into "buf".
 * If "len" bytes are not available, reads as much as possible.
 */
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

/*
 * Function: write
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD write(const char * buf, DWORD len)
 *              const char * buf : pointer to data to copy from
 *              DWORD len : length of data to read
 * Returns number of bytes copied to the circular buffer
 * Notes:
 *  Reads up to "len" bytes from "buf" into the buffer
 * If "len" bytes are not free in the buffer, reads as much as possible from "buf".
 */
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

/*
 * Function: getSize
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD getSize()
 * Returns total size of buffer
 * Notes:
 *  Getter for the size of the buffer.
 */
DWORD CircularBuffer::getSize() {
    return _size;
}

/*
 * Function: getSpaceAvailable()
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD getSpaceAvailable()
 * Returns total number of bytes available for writing
 * Notes:
 *  useful for checking if a buffer is not full, or has space available before writing.
 * attempting to write a buffer that is full will do nothing and possibly result in loss of data
 */
DWORD CircularBuffer::getSpaceAvailable() {
    return _writeBytesAvailable;
}

/*
 * Function: getDataAvailable()
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD getDataAvailable()
 * Returns total number of bytes ready for reading
 * Notes:
 *  useful for checking whether a certain amount of data is available within the buffer
 */
DWORD CircularBuffer::getDataAvailable() {
    return (_size - _writeBytesAvailable);
}

/*
 * Function: clear
 * Date: March 21, 2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD clear()
 * Returns total amount of free space in buffer
 * Notes:
 *  For resetting a buffer. Data inside is not discarded, but will be overwritten
 * by future write calls.
 */
DWORD CircularBuffer::clear() {
    memset(_data, 0, _size);
    _writePtr = _readPtr = 0;
    _writeBytesAvailable = _size;
    return _writeBytesAvailable;
}

