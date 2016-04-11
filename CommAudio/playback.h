#ifndef PLAYBACK_H
#define PLAYBACK_H

#include "circularbuffer.h"




class Playback : public CircularBuffer
{
public:
    Playback(int size);
    ~Playback();
    BASS_FILEPROCS* getFP();
    DWORD playFromFile(const char * filename);
    static DWORD startThread(LPVOID instance);
    bool setFilename(const char * fn);
private:
 //   CircularBuffer* _cb;
    BASS_FILEPROCS fp;
    char * filename; // holds a file to read from
};

#endif // PLAYBACK_H
