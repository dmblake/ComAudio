#include "buffermanager.h"

void CALLBACK fileClose(void *user)
{
    /*
    ((BufferManager*)user)->_isPlaying = false;
    ((BufferManager*)user)->_pb->clear();
    */
}

QWORD CALLBACK fileOpen(void *user)
{
    return 0;
}

DWORD CALLBACK fileRead(void *buffer, DWORD length, void *user)
{
    DWORD bytesRead;
    BufferManager * bm = (BufferManager*)user;
    bytesRead = ((BufferManager*)user)->_pb->read((char*)buffer, length);
    if (bm->_isServer) {
        bm->_net->write((char*)buffer, bytesRead);
    }
    return bytesRead;
}

BOOL CALLBACK fileSeek(QWORD offset, void* user)
{
    // this function could be used for file seeking
    // but i ain't doing that yet
    return true;
}



BufferManager::BufferManager(int len, bool server) :
    _pb(new CircularBuffer(len)), _net(new CircularBuffer(len)), _isServer(server), _isPlaying(false)
{
    if (!BASS_Init(-1, 44100, 0, 0, 0)) {
        qDebug() << "Failed to init bass " << BASS_ErrorGetCode();
    }

    _fp.close = fileClose;
    _fp.length = fileOpen;
    _fp.read = fileRead;
    _fp.seek = fileSeek;

}

// loads playback buffer from a file
DWORD BufferManager::loadFromFile(LPVOID param) {
    HANDLE hFile;
    wchar_t * wideStr;
    BufferManager* bm = (BufferManager*)param;
    int space = bm->_pb->getSpaceAvailable();
    char buf[BUF_LEN];
    DWORD bytesRead = 0;
    size_t lenC = mbsrtowcs(NULL, (const char **)&bm->_filename, 0, NULL);
    wchar_t * wideFile = new wchar_t[lenC+1]();
    lenC = mbsrtowcs(wideFile, (const char **)&bm->_filename, lenC+1, NULL);
    // open file
    hFile = CreateFile
            (wideFile,               // file to open
            GENERIC_READ,
            0,
            (LPSECURITY_ATTRIBUTES)NULL,       // share for reading
            OPEN_EXISTING,         // existing file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            (HANDLE)NULL);
    delete wideFile;
    if (hFile == INVALID_HANDLE_VALUE) {
        qDebug() << "Failed to open file " << wideStr;
        CloseHandle(hFile);
        return 0;
    }
    SetFilePointer(hFile, 0, 0, FILE_BEGIN);
    // while space in buffer
    while (bm->_isPlaying && ReadFile(hFile, buf, BUF_LEN, &bytesRead, 0)) {
        if (bytesRead == 0) {
            CloseHandle(hFile);
            return 0;
        }
        // wait for space
        space = bm->_pb->getSpaceAvailable();
        while (space < bytesRead) {
            space = bm->_pb->getSpaceAvailable();
        }
        bm->_pb->write(buf, bytesRead);
    }
    CloseHandle(hFile);
    return 0;
}

DWORD BufferManager::play(LPVOID param) {
    HSTREAM str = 0;
    BufferManager * bm = (BufferManager*)param;
    while (bm->_isPlaying) {
        // start stream
        if (str == 0) {

            if (!(str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
            }
            if (!BASS_ChannelPlay(str, FALSE)) {
                qDebug() << "Failed to play" << BASS_ErrorGetCode();
            }
        }
        // if data is in the buffer, start any stopped stream
        if (bm->_pb->getDataAvailable() > 20000) {
            switch (BASS_ChannelIsActive(str)) {
            case BASS_ACTIVE_PAUSED:
                break;
            case BASS_ACTIVE_PLAYING:
                break;
            case BASS_ACTIVE_STALLED:
                break;
            case BASS_ACTIVE_STOPPED:
                // play
                if (!(str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                    qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
                }
                if (!BASS_ChannelPlay(str, FALSE)) {
                    qDebug() << "Failed to play" << BASS_ErrorGetCode();
                }
                break;
            case -1:
                qDebug() << "Error in BASS status";

            }
        }
    }
    return 0;
}

BASS_FILEPROCS* BufferManager::getFP() {
    return &_fp;
}

DWORD BufferManager::startPlayThread(LPVOID param) {
    HANDLE hThread = CreateThread(0, 0, BufferManager::play, this, 0, 0);
    qDebug() << "starting play thread";
    return (hThread != INVALID_HANDLE_VALUE ? 1 : 0);
}

DWORD BufferManager::startReadThread(LPVOID instance) {
    BufferManager* bm = (BufferManager*)instance;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    // filename must be set by calling function before this will successfully start a thread
    if (bm->_filename != 0 && strlen(bm->_filename) > 0) {
        qDebug() << "Starting read thread";
        hThread = CreateThread(0, 0, BufferManager::loadFromFile, this, 0, 0);
    }
    return (hThread != INVALID_HANDLE_VALUE ? 1 : 0);
}

bool BufferManager::setFilename(const char * fn) {
    if (fn != 0) {
        _filename = (char*)malloc(strlen(fn));
        if (_filename == 0) {
            // error malloc
            return false;
        }
        strcpy(_filename, fn);
        return true;
    }
    return false;
}



