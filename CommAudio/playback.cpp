#include "playback.h"


void CALLBACK fileCloseProc(void *user)
{
    ((Playback*)user)->setPlaying(false);
    //((Playback*)user)->clear();
}

QWORD CALLBACK fileOpenProc(void *user)
{
    ((Playback*)user)->setPlaying(true);
    return 0;
}

DWORD CALLBACK fileReadProc(void *buffer, DWORD length, void *user)
{
    return ((Playback*)user)->read((char*)buffer, length);
}

BOOL CALLBACK fileSeekProc(QWORD offset, void* user)
{
    // this function could be used for file seeking
    // but i ain't doing that yet
    return true;
}


Playback::Playback(int size) : CircularBuffer(size){
    fp.close = fileCloseProc;
    fp.length = fileOpenProc;
    fp.read = fileReadProc;
    fp.seek = fileSeekProc;
}

Playback::~Playback() {

}

BASS_FILEPROCS* Playback::getFP() {
    return &fp;
}

// call it in a thread
DWORD Playback::playFromFile(const char * filename) {
    HANDLE hFile;
    wchar_t * wideStr;
    int space = this->getSpaceAvailable();
    char buf[BUF_LEN];
    DWORD bytesRead = 0;
    size_t lenC = mbsrtowcs(NULL, &filename, 0, NULL);
    wchar_t * wideFile = new wchar_t[lenC+1]();
    lenC = mbsrtowcs(wideFile, &filename, lenC+1, NULL);
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
    while (!_reset && ReadFile(hFile, buf, BUF_LEN, &bytesRead, 0)) {
        if (bytesRead == 0) {
            CloseHandle(hFile);
            return 0;
        }
        // wait for space
        space = this->getSpaceAvailable();
        while (space < bytesRead) {
            space = this->getSpaceAvailable();
        }
        this->write(buf, bytesRead);
    }
    _reset = false;
    CloseHandle(hFile);
    return 0;
}

bool Playback::isPlaying() {
    return _playing;
}

void Playback::setPlaying(bool val) {
    _playing = val;
}

DWORD Playback::startThread(LPVOID instance) {
    Playback* pb = (Playback*)instance;
    // filename must be set by calling function before this will successfully start a thread
    if (pb->filename != 0 && strlen(pb->filename) > 0) {
        qDebug() << "Starting thread";
        return pb->playFromFile(pb->filename);
    }
    return 0;
}

bool Playback::setFilename(const char * fn) {
    if (fn != 0) {
        filename = (char*)malloc(strlen(fn));
        if (filename == 0) {
            // error malloc
            return false;
        }
        strcpy(filename, fn);
        return true;
    }
    return false;
}
