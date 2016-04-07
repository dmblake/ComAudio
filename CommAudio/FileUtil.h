#include "dirent.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <errno.h>
#include <string>
#include "shared.h"

void sendControlMessage(SOCKET sd, char* msg);
std::string listAllFiles(std::string extension);
void sendFile(SOCKET sd, char* filename);
void rcvFile(SOCKET sd, char* fname, int size);
