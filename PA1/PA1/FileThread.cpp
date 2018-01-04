#include <mmsystem.h>
#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include "FileThread.h"

FileThread::FileThread()
{
	this->pOriginalFileData = new char[(512 * 1024)]; // Create new block of memory
	memset(this->pOriginalFileData, 0, (512 * 1024)); // Zero out block

	this->pFileData = this->pOriginalFileData;
	this->fileHeaderSize = 0;  // Will be given size in LoadFile()

	this->buffStatus = ContainerStatus::EMPTY;
	this->fileStatus = ThreadStatus::ALIVE;
};


FileThread::~FileThread()
{
	delete this->pOriginalFileData;
}


void FileThread::FileMain()
{
	char *filename = new char[25];
	for (int i = 0; i < 23; i++)
	{
		// Create string for file location
		sprintf_s(filename, 25, "AudioFiles/wave_%d.wav", i);

		// Load file to buffer and set data size
		this->LoadFile(filename);

		
		// While there is still data in buffer
		while (this->buffStatus == ContainerStatus::FULL)
		{
			std::unique_lock<std::mutex> lock(this->m);
			if (this->buffStatus == ContainerStatus::EMPTY)
			{
				break;
			}
			lock.unlock();

			// Sleep for 20 ms
			Sleep(20);
		}
	}

	// Set flag so CoordinatorThread cannot request data
	this->fileStatus = ThreadStatus::DONE;
	this->cv.notify_all();

	delete filename;
}

void FileThread::LoadFile(const char* filename)
{
	FILE *pFile;
	unsigned int size = 0;

	// Open file and check to see if it was successful
	if (fopen_s(&pFile, filename, "r") != 0)
	{
		printf("Could not read file - %s\n", filename);
		return;
	}

	// get it's size, allocate memory and read the file
	// into memory. don't use this on large files!
	fseek(pFile, 0, SEEK_END);
	size = (unsigned int)ftell(pFile);
	rewind(pFile);

	// Read file and set data to block
	std::unique_lock<std::mutex> lock(this->m); // Lock
	fread(this->pOriginalFileData, sizeof(int), size / sizeof(int), pFile);
	this->pFileData = this->pOriginalFileData;
	this->fileHeaderSize = size;
	this->buffStatus = ContainerStatus::FULL;
	lock.unlock(); // Unlock

	// Notify thread running TransferBuffer
	this->cv.notify_one();

	fclose(pFile);

	printf("Read file: %s\n", filename);
}

ThreadStatus FileThread::TransferBuffer(LPSTR & FrontBackBuffer, unsigned int & FBBufferSize)
{
	// If there is still data to be transfered
	std::unique_lock<std::mutex> lock(this->m);
	this->cv.wait(lock, [=]() { return this->buffStatus == ContainerStatus::FULL ||
										this->fileStatus == ThreadStatus::DONE; });

	if (this->fileStatus == ThreadStatus::ALIVE)
	{
		// Copy entire file buffer to FrontBackBuffer
		memcpy_s(FrontBackBuffer, (unsigned int)(512 * 1024), this->pOriginalFileData, (unsigned int)(512 * 1024));
		FBBufferSize = this->fileHeaderSize;
		this->buffStatus = ContainerStatus::EMPTY;
		// Notify thread running FileMain()
		cv.notify_one();
		return this->fileStatus;
	}
	return this->fileStatus;
}


LPSTR FileThread::getBlock()
{
	return this->pFileData;
}

unsigned int FileThread::getFileHeaderSize()
{
	return this->fileHeaderSize;
}

void FileThread::incrementBlockPointer(unsigned int bytesToMove)
{
	this->pFileData += bytesToMove;
}

