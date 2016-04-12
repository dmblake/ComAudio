/*---------------------------------------------------------------------------------------
--  SOURCE FILE:    FileUtil.cs
--
--  PROGRAM:        CommAudio
--
--  FUNCTIONS:
--		int sendMessage(SOCKET sd, const char* msg);
--		void getFileFromServer(SOCKET sd, const char* msg, int size);
--		std::string rcvControlMessage(SOCKET sd);
--		std::string listAllFiles(std::string extension);
--		void sendFile(SOCKET sd, const char* filename);
--		void rcvFile(SOCKET sd, const char* fname, int size);
--		void handleControlMessages(SOCKET sd);
--		std::string getListFromServer(SOCKET sd);
--		std::vector<std::string> split(const std::string &s, char delim);
--		std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
--
--  DATE:           April 11, 2016
--
--  REVISIONS: (none)
--
--  DESIGNERS:      Hank Lo
--
--  PROGRAMMER:     Hank Lo
--
--  NOTES:
--  This class contains utility functions for file transfer related operations.
---------------------------------------------------------------------------------------*/
#include "FileUtil.h"

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: getFileFromServer
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: void getFileFromServer(SOCKET sd, const char* fname, int size)
    --		SOCKET sd: A tcp socket connecting the client to the server
    --		const char* fname: The filename of the file you want to get from the server
    --		int size: the size of the file you're requesting
    --
    -- RETURNS: void
    --
    -- NOTES:
    -- Call this function to request a file from the server - this function sends a request to get a particular file
    -- from the server, and receives the file afterwards. The file received is named the same as the file requested.
    ---------------------------------------------------------------------------------------------------------------------*/
void getFileFromServer(SOCKET sd, const char* fname, int size) {
    std::string msg = "file{";
    msg += fname;
    sendMessage(sd, msg.c_str());
    rcvFile(sd, fname, size);
}

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: sendMessage
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: int sendMessage(SOCKET sd, const char* msg)
    --		SOCKET sd: A connected tcp socket
    --		const char* msg: the message you want to send.
    --
    -- RETURNS: an int representing the number of bytes sent; -1 means failure, but everything else is good.
    --
    -- NOTES:
    -- Call this function to send a message over the connected socket.
    ---------------------------------------------------------------------------------------------------------------------*/
int sendMessage(SOCKET sd, const char* msg) {
    int err = send(sd, msg, BUF_LEN, 0);
    return err;
}

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: getListFromServer
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: std::string getListFromServer(SOCKET sd)
    --		SOCKET sd: A connected TCP socket representing the server.
    --
    -- RETURNS: a standard string, the list of files + filesizes from the server
    --
    -- NOTES:
    -- This function sends a message to the server asking for a list update; the server responds with a string in the format
    -- of: "filename,filesize\nfilename2,filesize2\n...filenameN,filesizeN\n". This function returns that string received,
    -- or an empty string if the message failed to send (eg: socket connection was bad).
    ---------------------------------------------------------------------------------------------------------------------*/
std::string getListFromServer(SOCKET sd) {
    if (sendMessage(sd, "updatelist{") != -1) {
        return rcvControlMessage(sd);
    } else {
        return "";
    }
}

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: handleControlMessages
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: void handleControlMessages(SOCKET sd)
    -- 	 SOCKET sd: a tcp socket connected to a client
    --
    -- RETURNS: void
    --
    -- NOTES:
    -- This function is meant to be run in a thread on the server; it listens on the provided socket and handles any messages
    -- received. If it receives a message type of "updatelist", it searches for all .wav and .mp3 files on the server, and 
    -- sends that list + the files sizes to the connected client. If it receives a message of type "file", it searches for
    -- and sends the requested file to the connected client.
    ---------------------------------------------------------------------------------------------------------------------*/
void handleControlMessages(SOCKET sd) {
    while(true) {
        std::string msg = rcvControlMessage(sd);
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
        }
    }
}

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: rcvControlMessage
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: std::string rcvControlMessage(SOCKET sd)
    -- 		SOCKET sd: A connected tcp socket.
    --
    -- RETURNS: a standard string representing the message read from the socket.
    --
    -- NOTES:
    -- This function can be used in general by both client and server to receive a message of length shorter than BUF_LEN.
    ---------------------------------------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: listAllFiles
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: std::string listAllFiles(std::string extension)
    --		std::string extension
    --
    -- RETURNS: a standard string representing all the files and their filesizes in the format of:
    -- "filename,filesize\nfilename2,filesize2\n...filenameN,filesizeN\n".
    --
    -- NOTES:
    -- This function can be used to get a list of all .wav and .mp3 files, and their sizes.
    ---------------------------------------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------------------------------------
    -- FUNCTION: sendFile
    --
    -- DATE: April 11, 2016
    --
    -- REVISIONS: (none)
    --
    -- DESIGNER: Hank Lo
    --
    -- PROGRAMMER: Hank Lo
    --
    -- INTERFACE: void sendFile(SOCKET sd, const char* filename)
    --		SOCKET sd: a connected tcp socket
    --		const char* filename: the name of the file we are sending over the socket
    --
    -- RETURNS: void
    --
    -- NOTES:
    -- This function is used to send a file to a connected client using tcp.
    ---------------------------------------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: rcvFile
--
-- DATE: April 11, 2016
--
-- REVISIONS: (none)
--
-- DESIGNER: Hank Lo
--
-- PROGRAMMER: Hank Lo
--
-- INTERFACE: void rcvFile(SOCKET sd, const char* fname, int size)
--		SOCKET sd: a connected tcp socket to the server
--		const char* fname: the name of the file we are trying to receive, and the name we will be saving the file as
--		int size: the size of the file we are trying to receive
--
-- RETURNS: void
--
-- NOTES:
-- This function is called by the client to receive a file from the server. You must provide the filesize received
-- from the server when calling this function.
---------------------------------------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: split
--
-- DATE: April 11, 2016
--
-- REVISIONS: (none)
--
-- DESIGNER: Hank Lo
--
-- PROGRAMMER: Hank Lo
--
-- INTERFACE: std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
--		const std::string &s: The string we want to split up
--		char delim: The delimiter we want to split the string with
--		std::vector<std::string> &elems: The vector where the split up strings will be stored
--
-- RETURNS: a preconstructed std::vector<std::string>, filled by this function.
--
-- NOTES:
-- This function splits a standard string up using the provided character delimiter, and stores the elements into the provided
-- vector of strings.
---------------------------------------------------------------------------------------------------------------------*/
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

/*---------------------------------------------------------------------------------------------------------------------
-- FUNCTION: split
--
-- DATE: April 11, 2016
--
-- REVISIONS: (none)
--
-- DESIGNER: Hank Lo
--
-- PROGRAMMER: Hank Lo
--
-- INTERFACE: std::vector<std::string> split(const std::string &s, char delim)
--		const std::string &s: The string you want to split up
--		char delim: the delimiter you want to split the string with
--
-- RETURNS: a new vector of strings, each part of the vector representing a portion of the original string.
--
-- NOTES:
-- This function takes a string and a delimiter and returns a vector of strings, in which each element is a part of the
-- original string. This function is a wrapper function that calls the other split function so that the user does not
-- have to manually create a vector.
---------------------------------------------------------------------------------------------------------------------*/
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


