/*
SOURCE FILE: CircularBuffer.cpp
DATE: March 15, 2016
Revision: v1
Designer: Dylan Blake
Programmer: Dylan Blake
Functions:

Notes:
	Handles the circular buffer for receiving data and playing back audio.*/

#include "CircularBuffer.h"

/*
Function: CircularBuffer::CircularBuffer
Data: March 15, 2016
Revision: v1
Designer: Dylan Blake
Programmer: Dylan Blake
Interface: CircularBuffer(int bytesRequested)
				bytesRequested : the size of the circular buffer to instantiate
Notes:
	Constructor for the circular buffer. Can fail if there is not enough memory.
*/
CircularBuffer::CircularBuffer(int bytesRequested)
{
	if (bytesRequested < 1) return;
	_data = (char*)malloc(bytesRequested);
	if (_data == 0) return;

	memset(_data, 0, bytesRequested);
	_size = bytesRequested;
	_readPtr = _writePtr = 0;
	_writeBytesAvailable = bytesRequested;
}

/*
Function: CircularBuffer::~CircularBuffer
Date: March 15, 2016
Revision: v1
Designer: Dylan Blake
Programmer: Dylan Blake
Interface: ~CircularBuffer()
Notes:
	Destructor. Frees the memory allocated.
*/
CircularBuffer::~CircularBuffer()
{
	if (_data == 0) return;
	free(_data);
}

/*
Function: CircularBuffer::Write
Date: March 15, 2016
Revision: v1
Designer: Dylan Blake
Programmer: Dylan Blake
Interface: Write(char * buf, int len)
				buf : buffer to read from
				len : number of bytes to read
Returns: the number of bytes written to the circular buffer.
Notes:
	Writes len number of bytes from buf to the circular buffer.
	Returns 0 upon failure.
*/
int CircularBuffer::Write(char * buf, int len)
{
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
Function: CircularBuffer::Read
Date: March 15, 2016
Revision: v1
Designer: Dylan Blake
Programmer: Dylan Blake
Interface: Read(char * buf, int len)
			buf : buffer to store data into
			len : number of bytes to read from the circular buffer
Returns: number of bytes read
Notes:
	Reads len bytes from the circular buffer and stores it to the memory
	pointed to by buf.
	Returns 0 upon failure.
*/
int CircularBuffer::Read(char * buf, int len)
{
	if (buf == 0 || len <= 0 || _writeBytesAvailable == _size)
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
		memcpy(buf, _data + _readPtr, before_wrap);
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

int CircularBuffer::GetAvailable()
{
	return (_size - _writeBytesAvailable);
}