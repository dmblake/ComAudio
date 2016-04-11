#include "FileUtil.h"

void getFileFromServer(SOCKET sd, const char* fname, int size) {
    std::string msg = "file{";
    msg += fname;
    sendMessage(sd, msg.c_str());
    rcvFile(sd, fname, size);
}

void sendMessage(SOCKET sd, const char* msg) {
    int err = send(sd, msg, BUF_LEN, 0);
    qDebug() << err;
}

std::string getListFromServer(SOCKET sd) {
    sendMessage(sd, "updatelist{");
    return rcvControlMessage(sd);
}

void handleControlMessages(SOCKET sd) {
    qDebug() << "handle control messages";
    while(true) {
        std::string msg = rcvControlMessage(sd);
        qDebug() << msg.c_str();
        std::vector<std::string> splitmsg = split(msg, '{');

        if (splitmsg[0] == "updatelist") {
            qDebug() << "updatelist";
            std::string list = listAllFiles(".wav");
            list += listAllFiles(".mp3");
            sendMessage(sd, list.c_str());
        } else if (splitmsg[0] == "file") {
            qDebug() << "file";
            sendFile(sd, splitmsg[1].c_str());

        } else {
            qDebug() << "no match";
        }
    }
}

std::string rcvControlMessage(SOCKET sd) {
	char * bp;
	char * rbuf = (char *) malloc(BUF_LEN);
	int bytes_to_read;
	int n;

	bp = rbuf;
    bytes_to_read = BUF_LEN;

	// client makes repeated calls to recv until no more data is expected to arrive.
    while ((n = recv(sd, bp, bytes_to_read, 0)) < BUF_LEN) {
		bp += n;
		bytes_to_read -= n;
		if (n == 0)
			break;
	}
    qDebug() << rbuf;
	std::string msg = std::string(rbuf);
	return msg;
}

std::string listAllFiles(std::string extension) {
	DIR           *d;
	struct dirent *dir;
	d = opendir(".");

	std::string finalList = "";

	if (d)
	{
	    while ((dir = readdir(d)) != NULL)
	    {
	    	if (dir->d_type == DT_REG) {
	    		std::string fname = dir->d_name;
	    		if (fname.find(extension, (fname.length() - extension.length())) != std::string::npos) {
	    			finalList += fname;
	    			finalList += ",";

	    			// file size
	    			long size;
	    			std::filebuf *pbuf;
					std::ifstream sourcestr;

	    			sourcestr.open(fname, std::ios::in | std::ios::binary);

					// get pointer to associated buffer object
					pbuf = sourcestr.rdbuf();

					// get file size using buffer's members
					size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
					sourcestr.close();

					finalList += std::to_string(size);

	    			finalList += "\n";
	    		}
	    	}
	    }

	    closedir(d);
	}

	return finalList;
}

void sendFile(SOCKET sd, const char* filename) {
	std::filebuf *pbuf;
	std::ifstream sourcestr;
	long size;
	char * bbuffer;

	sourcestr.open(filename, std::ios::in | std::ios::binary);

	// get pointer to associated buffer object
	pbuf = sourcestr.rdbuf();

	// get file size using buffer's members
	size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
	pbuf->pubseekpos(0, std::ios::in);

	// allocate memory to contain file data
	bbuffer = (char*)malloc(sizeof(char)*size);

	// get file data  
	pbuf->sgetn(bbuffer, size);

	sourcestr.close();

	send(sd, bbuffer, size, 0);
}

void rcvFile(SOCKET sd, const char* fname, int size) {
	char * bp;
	char * rbuf = (char *) malloc(size);
	int bytes_to_read;
	int n;

	bp = rbuf;
	bytes_to_read = size;

	// client makes repeated calls to recv until no more data is expected to arrive.
	while ((n = recv(sd, bp, bytes_to_read, 0)) < size) {
		bp += n;
		bytes_to_read -= n;
		if (n == 0)
			break;
	}

	FILE * pFile;
	pFile = fopen(fname, "wb");

	if (pFile != NULL) {
		fwrite(rbuf, sizeof(char), size, pFile);
		fclose(pFile);
	} else {
		printf("NULL");
	}
}

// puts result in a preconstructed vector
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

// returns a new vector
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


