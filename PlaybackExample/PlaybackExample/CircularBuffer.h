#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class CircularBuffer
{
private:
	char * _data;
	int _size;
	int _readPtr;
	int _writePtr;
	int _writeBytesAvailable;
public:
	CircularBuffer(int bytesRequested);
	~CircularBuffer();
	int Write(char * buf, int len);
	int Read(char * buf, int len);
};