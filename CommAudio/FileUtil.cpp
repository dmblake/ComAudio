#include "FileUtil.h"

void sendControlMessage(SOCKET sd, char* msg) {
	send(sd, msg, BUF_LEN, 0);
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

	// fix this

	return "";
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
	    			finalList += ", ";

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

void sendFile(SOCKET sd, char* filename) {
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

void rcvFile(SOCKET sd, char* fname, int size) {
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
