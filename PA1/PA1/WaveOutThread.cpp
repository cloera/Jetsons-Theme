#include <assert.h>
#include <mutex>
#include <condition_variable>
#include "WaveOutThread.h"

#define UNUSED_VARIABLE(x) { (void)x; }

WaveOutThread::WaveOutThread() 
{
	// Create 2K Waveout memory block
	unsigned int waveBufferSize = (unsigned int)(2 * 1024);
	this->pWaveoutBuff = new char[waveBufferSize];

	// Create new Wave Header and zero it out
	this->pHeader = new WAVEHDR();
	memset(this->pHeader, 0x0, sizeof(WAVEHDR));

	// Set buffer and buffer size to Wave Header
	this->pHeader->dwBufferLength = waveBufferSize;
	this->pHeader->lpData = this->pWaveoutBuff;

	// Set flag to false Waveout buffer cannot be written without data
	this->canPlay = false;
	this->isLive = true;

	this->pad1 = false;
	this->pad2 = false;
}

WaveOutThread::~WaveOutThread()
{
	delete this->pHeader;
	delete this->pWaveoutBuff;
}


void WaveOutThread::WaveOutMain(HWAVEOUT &hWaveOut)
{
	MMRESULT result;

	while (this->isLive)
	{
		result = writeHeader(hWaveOut);

		if (result != MMSYSERR_NOERROR)
		{
			printf("WaveOut Write Error\n    Result: %x\n", result);
		}
	}
}


void WaveOutThread::prepareHeader(HWAVEOUT hWaveOut, unsigned int waveThreadNum)
{
	// Store buffer number in pHeader to be used in waveOutProc() callback
	this->pHeader->dwUser = (DWORD_PTR) waveThreadNum;

	// Prepare Wave Header once
	MMRESULT res = waveOutPrepareHeader(hWaveOut, this->pHeader, sizeof(WAVEHDR));
	if (res != MMSYSERR_NOERROR)
	{
		printf("WaveOut Prepare Error\n\t Result: %x  waveBuff[%d]\n", res, waveThreadNum);
	}
}

void WaveOutThread::copyWaveBlock(LPSTR pMemBuff, unsigned int bufferSize)
{
	// copy data into buffer
	std::unique_lock<std::mutex> lock(this->writeMutex);
	memcpy_s(this->pHeader->lpData, (unsigned int)(2 * 1024), pMemBuff, bufferSize);
}

MMRESULT WaveOutThread::writeHeader(HWAVEOUT &hWaveOut)
{
	std::unique_lock<std::mutex> lock(this->playMutex);
	MMRESULT result = waveOutWrite(hWaveOut, this->pHeader, sizeof(WAVEHDR));
	this->canPlay = false;
	cv.wait(lock, [=]() { return this->canPlay; });

	return result;
}

void WaveOutThread::unprepareHeader(HWAVEOUT &hWaveOut)
{
	// Zero out memory
	memset(this->pWaveoutBuff, 0x0, (unsigned int)(2 * 1024));
	memset(this->pHeader, 0x0, sizeof(WAVEHDR));

	// Escape from main loop thread
	this->isLive = false;
	this->canPlay = true;
	cv.notify_one();

	waveOutUnprepareHeader(hWaveOut, this->pHeader, sizeof(WAVEHDR));
}


void WaveOutThread::play()
{
	// Set flag to true so Wave Header can be written
	this->canPlay = true;
	// Signal cv.wait() in writeHeader()
	cv.notify_one();
}


