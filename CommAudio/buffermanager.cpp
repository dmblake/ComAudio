/*
 * Source file: buffermanager.cpp
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Functions:
 *  BufferManager(int len, bool server);
    static DWORD play(LPVOID param);
    DWORD startPlayThread(LPVOID param);
    void stop();
    void pause();
    void resume();
    static DWORD loadFromFile(LPVOID param);
    DWORD BufferManager::startReadThread(LPVOID instance);
    bool BufferManager::setFilename(const char * fn);
    BASS_FILEPROCS* getFP();
    void mute();
 *
 * Notes:
 * Handles the circular buffers, providing methods for clients to start threads
 * Also handles feeding data into the BASS library via callback functions.
 *
 * This class was a major revision to how playback was handled.
 * The majority of these functions are moved from various other parts of the program.
 * For example, the threaded playback function was originally in client.cpp.
 * All of the functionality has been moved to this class for ease of access.
 */

#include "buffermanager.h"


/*
 * Function: fileClose
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: void fileClose(void *user)
 *              void * user : user data passed to the function
 * Returns void
 * Notes:
 *  BASS library call back function when a file is closed/stream is ended
 */
void CALLBACK fileClose(void *user)
{
    //((BufferManager*)user)->_isPlaying = false;
    //((BufferManager*)user)->_pb->clear();
}

/*
 * Function: fileOPen
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: QWORD fileClose(void *user)
 *              void * user : user data passed to the function
 * Returns QWORD size of file; always 0 for streams
 * Notes:
 *  BASS library call back function when a stream is created.
 *  Note that the return value is irrelevant for streams (could be used when reading from file)
 */
QWORD CALLBACK fileOpen(void *user)
{
    return 0;
}

/*
 * Function: fileRead
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD fileClose(void *buffer, DWORD length, void *user)
 *              void * buffer : buffer to move data into
 *              DWORD length : amount of data to read
 *              void * user : a BufferManager object for access to buffers
 * Returns number of bytes read
 * Notes:
 *  BASS library call back function when BASS needs data for the audio stream.
 */
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

/*
 * Function: fileSeek
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: BOOL fileClose(QWORD offset, void *user)
 *              QWORD offset : offset to move in
 *              void * user : user data passed to the function
 * Returns true always
 * Notes:
 *  BASS library call back function when trying to seek through a file.
 * Unused with streams
 */
BOOL CALLBACK fileSeek(QWORD offset, void* user)
{
    // this function could be used for file seeking
    // but i ain't doing that yet
    return true;
}


/*
 * Function: BufferManager
 * Date: 04/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: BufferManager(int len, bool server)
 *              int len : size of circular buffers
 *              bool server : whether the buffers belong to a server
 * Returns: constructor, no returns
 * Notes:
 *  Creates the circular buffers for use in networking and audio playback.
 */
BufferManager::BufferManager(int len, bool server) :
    _pb(new CircularBuffer(len)), _net(new CircularBuffer(len)), _isServer(server), _isPlaying(false), _isSending(false)
{
    if (!BASS_Init(-1, 44100, 0, 0, 0)) {
        qDebug() << "Failed to init bass " << BASS_ErrorGetCode();
    }

    _fp.close = fileClose;
    _fp.length = fileOpen;
    _fp.read = fileRead;
    _fp.seek = fileSeek;

}

/*
 * Function: loadFromFile
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD loadFromFile(LPVOID param)
 *              LPVOID param : pointer to a buffer manager object
 * Returns 0
 * Notes:
 *  Threaded function; reads from a file into the playback buffer specified
 * by the BufferManager pointer passed in.
 * File is specified by state within the same BufferManager.
 * Suitable for both local playback and for sending over the net.
 */
DWORD BufferManager::loadFromFile(LPVOID param) {
    HANDLE hFile;
    wchar_t * wideStr;
    DWORD bytesRead = 0;
    BufferManager* bm = (BufferManager*)param;
    char buf[BUF_LEN];
    DWORD space = bm->_pb->getSpaceAvailable();
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

/*
 * Function: play(LPVOID param)
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD play(LPVOID param)
 *              LPVOID param : pointer to a BufferManager object
 * Returns 0
 * Notes:
 *  Threaded function; attempts to read from a playback buffer into the BASS library.
 * Ends when the _isPlaying flagged is signalled, usually through the MainWindow interface
 */
DWORD BufferManager::play(LPVOID param) {
    BufferManager * bm = (BufferManager*)param;
    while (bm->_isPlaying) {
        // start stream
        if (bm->_str == 0 && (bm->_pb->getDataAvailable() > BUF_LEN * 20)) {

            if (!(bm->_str = BASS_StreamCreateFileUser(STREAMFILE_BUFFER, BASS_STREAM_BLOCK, bm->getFP(), bm))) {
                //qDebug() << "Failed to create stream" << BASS_ErrorGetCode();
            }
             else if (!BASS_ChannelPlay(bm->_str, FALSE)) {
                //qDebug() << "Failed to play" << BASS_ErrorGetCode();
            }else {
                qDebug() << "Started playing";
            }
        }
        // if data is in the buffer, start any stopped stream
        if (bm->_isPlaying && (bm->_pb->getDataAvailable() > BUF_LEN * 20)) {
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
                else if (!BASS_ChannelPlay(bm->_str, FALSE)) {
                    qDebug() << "Failed to play" << BASS_ErrorGetCode();
                } else {
                    qDebug() << "Started playing";
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

/*
 * Function: getFP()
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: BASS_FILEPROCS* getFP()
 * Returns a pointer to a structure of function pointers
 * Notes:
 *  BASS_FILEPROCS is a structure to hold a set of user-defined functions
 *  for the BASS library to callback in various situations (ie. file open, close, reading and seeking)
 */
BASS_FILEPROCS* BufferManager::getFP() {
    return &_fp;
}

/*
 * Function: startPlayThread(LPVOID param)
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD startPlayThread(LPVOID param)
 *              LPVOID param : unused
 * Returns 1 upon successful thread creation, 0 otherwise
 * Notes:
 *  Allows clients to start playback from the buffers.
 * Because calling threaded functions that belong to a class requires the function to be static,
 * this allows a client to invoke a thread from an object by passing the "this" pointer
 * to the created thread.
 */
DWORD BufferManager::startPlayThread(LPVOID param) {
    HANDLE hThread = CreateThread(0, 0, BufferManager::play, this, 0, 0);
    qDebug() << "starting play thread";
    return (hThread != INVALID_HANDLE_VALUE ? 1 : 0);
}

/*
 * Function: startReadThread(LPVOID instance)
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: DWORD startReadThread(LPVOID instance)
 *              LPVOID param : unused
 * Returns 1 upon successful thread creation, 0 otherwise
 * Notes:
 *  Allows clients to start file reading into the buffers.
 * Because calling threaded functions that belong to a class requires the function to be static,
 * this allows a client to invoke a thread from an object by passing the "this" pointer
 * to the created thread.
 */
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

/*
 * Function: setFilename
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: bool setFilename(const char * fn)
 *              const char * fn : filename to store
 * Returns true if filename successfully copied, false otherwise
 * Notes:
 *  sets the filename in the buffer manager for future reference
 * by file reading functions.
 */
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

/*
 * Function: stop
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: void stop()
 * Returns void
 * Notes:
 *  stops playback and sending, also resets the stream and clears the playback buffer
 */
void BufferManager::stop() {
    switch(BASS_ChannelIsActive(_str)) {
    case BASS_ACTIVE_STOPPED:
        // fall down
    case BASS_ACTIVE_PAUSED:
        // fall down
    case BASS_ACTIVE_PLAYING:
        BASS_ChannelStop(_str);
        _isPlaying = false;
        _isSending = false;
        _str = 0;
        _pb->clear();
        //_net->clear();   // this causes a race condition where you might send junk data; clear _net in server.cpp mcast thread
        break;
    }
}

/*
 * Function: pause
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: void pause()
 * Returns void
 * Notes:
 *  pauses the BASS library
 */
void BufferManager::pause() {
    switch (BASS_ChannelIsActive(_str)) {
    case BASS_ACTIVE_PAUSED:
        // fall through
    case BASS_ACTIVE_PLAYING:
        BASS_Pause();
        break;
    }
}

/*
 * Function: resume
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: void resume()
 * Returns void
 * Notes:
 *  resumes the BASS library
 */
void BufferManager::resume() {
    if (BASS_ChannelIsActive(_str)== BASS_ACTIVE_PAUSED) {
        BASS_Start();
    }
}

/*
 * Function: mute
 * Date: 4/12/2016
 * Revision: v1
 * Designer: Dylan Blake
 * Programmer: Dylan Blake
 * Interface: void mute()
 * Returns void
 * Notes:
 *  mutes or unmutes the BASS audio
 */
void BufferManager::mute() {
    if (BASS_GetVolume() > 0) {
        BASS_SetVolume(0);
    } else {
        BASS_SetVolume(0.8);
    }
}

