/*
BASS simple playback test
Copyright (c) 1999-2012 Un4seen Developments Ltd.
*/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <bass.h>
#include "CircularBuffer.h"

HWND win = NULL;

HSTREAM *strs = NULL;
int strc = 0;
HMUSIC *mods = NULL;
int modc = 0;
HSAMPLE *sams = NULL;
int samc = 0;

void CALLBACK fileCloseProc(void *user)
{
}

QWORD CALLBACK fileOpenProc(void *user)
{
	return 0;
}

DWORD CALLBACK fileReadProc(void *buffer, DWORD length, void *user)
{
	return ((CircularBuffer*)user)->Read((char*)buffer, length);
}

BOOL CALLBACK fileSeekProc(QWORD offset, void* user)
{
	return true;
}

BASS_FILEPROCS fp = { fileCloseProc, fileOpenProc, fileReadProc, fileSeekProc };
// display error messages
void Error(const char *es)
{
	char mes[200];
	sprintf(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
	MessageBox(win, mes, 0, 0);
}

DWORD CALLBACK readFromFile(HSTREAM handle, void* buf, DWORD len, void* user)
{
	/*
	FILE *file = (FILE*)user;
	DWORD c = fread(buf, 1, len, file); // read the file into the buffer
	if (feof(file)) c |= BASS_STREAMPROC_END; // end of the file/stream
	return c;
	*/
	CircularBuffer *cb = (CircularBuffer*)user;
	char * b = (char*)buf;
	DWORD ret = cb->Read(b, len);
	return ret;
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		MessageBox(0, "An incorrect version of BASS.DLL was loaded", 0, MB_ICONERROR);
		return 0;
	}
	

	OutputDebugString("Before Dialog\n");
	// display the window
	//DialogBox(hInstance, MAKEINTRESOURCE(1000), NULL, &dialogproc);

	HSTREAM str = 0;

	CircularBuffer rb(500000);

	HANDLE hf = CreateFile("C:\\Users\\Administrator\\Desktop\\Famous.mp3", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	char buf[1000000];
	DWORD bytesRead = 0;

	while (ReadFile(hf, buf, 5000, &bytesRead, 0)) {
		if (rb.Write(buf, bytesRead) == 0)
			break;
	}
	

	if (!BASS_Init(-1, 44100, 0, win, NULL))
		Error("Can't initialize device");
	
	if (!(str = BASS_StreamCreateFileUser (STREAMFILE_BUFFER, BASS_STREAM_BLOCK | BASS_STREAM_RESTRATE | BASS_ASYNCFILE, &fp, &rb)))
		Error("Can't open stream");
	BASS_ChannelSetAttribute(str, BASS_ATTRIB_VOL, 1.0);
	if (!BASS_ChannelPlay(str, TRUE))
		Error("Can't open stream");

	DWORD totalBytesRead;
	while (1) {
		while (ReadFile(hf, buf, 1, &bytesRead, 0)) {
			while (rb.Write(buf, bytesRead) == 0) {}
			
		}
	}
	
	/*
	if (str = BASS_StreamCreateFile(TRUE, buf, 0, 500000, 0)) {
		strc++;
		strs = (HSTREAM*)malloc(sizeof(*strs));
		strs[strc - 1] = str;

		if (!BASS_ChannelPlay(strs[0], TRUE)) // play the stream (continue from current position)
			Error("Can't play stream");
	}
	else
		Error("Can't open stream");
		*/
	
//	Sleep(500000);

	OutputDebugString("After Dialog\n");


	return 0;
}
