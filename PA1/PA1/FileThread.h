#ifndef FILETHREAD_H
#define FILETHREAD_H

#include <windows.h>
#include <mutex>
#include "Status.h"

class FileThread {
public:
	FileThread();
	FileThread(const FileThread &in) = delete;
	FileThread & operator=(const FileThread &) = delete;
	~FileThread();
	
	void FileMain();
	void LoadFile(const char* filename);
	ThreadStatus TransferBuffer(LPSTR &FrontBackBuffer, unsigned int &FBBufferSize);

	LPSTR getBlock();
	unsigned int getFileHeaderSize();
	void incrementBlockPointer(unsigned int bytesToMove);


	// Protection
	std::condition_variable cv;
	std::mutex m;

	// Data
	LPSTR pOriginalFileData; // pointer to the original block
	LPSTR pFileData; // pointer to increment 
	unsigned int fileHeaderSize; // holds the size of the block

	ContainerStatus  buffStatus;
	ThreadStatus  fileStatus;
};

#endif // !FILETHREAD_H

