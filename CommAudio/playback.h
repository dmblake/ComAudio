#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bass.h"
#include "circularbuffer.h"




class Playback
{
public:
    Playback(int size);
    ~Playback();
    BASS_FILEPROCS* getFP();
private:
    CircularBuffer* _cb;
    BASS_FILEPROCS fp;   
};

#endif // PLAYBACK_H
