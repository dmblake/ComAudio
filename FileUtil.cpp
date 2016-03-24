#include "FileUtil.h"

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
	    			finalList += "\n";
	    		}
	    	}
	    }

	    closedir(d);
	}

	return finalList;
}

void sendFile(std::string filename) {
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
	    		if (fname == filename){
	    			char * buffer;     //buffer to store file contents
					long size;     //file size
					ifstream file (filename, ios::in|ios::binary|ios::ate);     //open file in binary mode, get pointer at the end of the file (ios::ate)
					size = file.tellg();     //retrieve get pointer position
					file.seekg (0, ios::beg);     //position get pointer at the begining of the file
					buffer = new char [size];     //initialize the buffer
					file.read (buffer, size);     //read file to buffer
					file.close();     //close file

					// send buffer through tcp socket
	    		}
	    	}
	    }

	    closedir(d);
	}
}

int main(void)
{
	std::string tmp = listAllFiles("mp3"); // wav will be default
	printf("%s\n", tmp.c_str());	

	return(0);
}