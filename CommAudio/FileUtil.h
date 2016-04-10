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

void sendMessage(SOCKET sd, const char* msg);
std::string rcvControlMessage(SOCKET sd);
std::string listAllFiles(std::string extension);
void sendFile(SOCKET sd, const char* filename);
void rcvFile(SOCKET sd, char* fname, int size);
void handleControlMessages(SOCKET sd);
void getListFromServer(SOCKET sd);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
