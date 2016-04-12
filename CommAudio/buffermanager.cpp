#include "buffermanager.h"

void CALLBACK fileClose(void *user)
{
    //((BufferManager*)user)->_isPlaying = false;
    //((BufferManager*)user)->_pb->clear();
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
    DWORD bytesRead = 0;
    BufferManager* bm = (BufferManager*)param;
    char buf[BUF_LEN];
    int space = bm->_pb->getSpaceAvailable();
    // convert char * filename to wide string
    std::string temp(bm->_filename);
    char * ctemp = new char[temp.length()+1];
    strcpy(ctemp, temp.c_str());
    size_t lenC = mbsrtowcs(NULL, (const char **)&ctemp, 0, NULL);
    wchar_t * wideFile = new wchar_t[lenC+1]();
    lenC = mbsrtowcs(wideFile, (const char **)&ctemp, lenC+1, NULL);
    // finish converting
    // set EOF to false
    bm->_eof = false;
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
        // finished reading file
        if (bytesRead == 0) {
            CloseHandle(hFile);
            bm->_eof = true;
            qDebug() << "Exiting read thread: bytesRead = 0";
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
    qDebug() << "Exiting read thread";
    return 0;
}

DWORD BufferManager::play(LPVOID param) {
    BufferManager * bm = (BufferManager*)param;
    while (bm->_isPlaying) {
        // start stream
        if (bm->_str == 0 && (bm->_pb->getDataAvailable() > BUF_LEN*3)) {

            if (!(bm->_str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
            }
            if (!BASS_ChannelPlay(bm->_str, FALSE)) {
                qDebug() << "Failed to play" << BASS_ErrorGetCode();
            }
        }
        // if data is in the buffer, start any stopped stream
        if (bm->_isPlaying && (bm->_pb->getDataAvailable() > BUF_LEN*3 )) {
            int act = BASS_ChannelIsActive(bm->_str);
            switch (act) {
            case BASS_ACTIVE_PAUSED:
                BASS_ChannelPause(bm->_str);
                break;
            case BASS_ACTIVE_PLAYING:
                break;
            case BASS_ACTIVE_STALLED:
                break;
            case BASS_ACTIVE_STOPPED:
                // play
                if (!(bm->_str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                    qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
                }
                if (!BASS_ChannelPlay(bm->_str, FALSE)) {
                    qDebug() << "Failed to play" << BASS_ErrorGetCode();
                }
                break;
            case -1:
                qDebug() << "Error in BASS status";
            }
        }
        // if we've hit EOF, see if we can play anything else, then exit
        if (bm->_eof) {
            if (BASS_ChannelIsActive(bm->_str) == BASS_ACTIVE_STOPPED) {
                if (!(bm->_str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                }
                if (!BASS_ChannelPlay(bm->_str, FALSE)) {
                    bm->_isPlaying = false;
                } else {
                    do {
                        // do nothing until we're not playing
                    } while (BASS_ChannelIsActive(bm->_str) == BASS_ACTIVE_PLAYING);
                        // then stop
                        bm->_isPlaying = false;
                }
            }
        }
    }
    // clean up while exiting
    if (BASS_ChannelIsActive(bm->_str) == BASS_ACTIVE_PLAYING) {
        BASS_ChannelStop(bm->_str);
    }
    qDebug() << "Exiting play thread";
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
    if (bm->_isServer && bm->_filename != 0 && strlen(bm->_filename) > 0) {
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

void BufferManager::stop() {
    switch(BASS_ChannelIsActive(_str)) {
    case BASS_ACTIVE_STOPPED:
        // fall down
    case BASS_ACTIVE_PAUSED:
        // fall down
    case BASS_ACTIVE_PLAYING:
        BASS_ChannelStop(_str);
        _isPlaying = false;
        _str = 0;
        _pb->clear();
        break;
    }
}

void BufferManager::pause() {
    switch (BASS_ChannelIsActive(_str)) {
    case BASS_ACTIVE_PAUSED:
        // fall through
    case BASS_ACTIVE_PLAYING:
        BASS_Pause();
        break;
    }
}

void BufferManager::resume() {
    if (BASS_ChannelIsActive(_str)== BASS_ACTIVE_PAUSED) {
        BASS_Start();
    }
}

// mute and unmute
void BufferManager::mute() {
    if (BASS_GetVolume() > 0) {
        BASS_SetVolume(0);
    } else {
        BASS_SetVolume(1);
    }
}

