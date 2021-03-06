#pragma once
#include "dirent.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <errno.h>
#include <string>
#include <sstream>
#include "shared.h"

int sendMessage(SOCKET sd, const char* msg);
void getFileFromServer(SOCKET sd, const char* msg, int size);
std::string rcvControlMessage(SOCKET sd);
std::string listAllFiles(std::string extension);
void sendFile(SOCKET sd, const char* filename);
void rcvFile(SOCKET sd, const char* fname, int size);
void handleControlMessages(SOCKET sd);
std::string getListFromServer(SOCKET sd);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
