#ifndef WAVEOUTTHREAD_H
#define WAVEOUTTHREAD_H

#include <windows.h>
#include <mutex>


class WaveOutThread
{
public:
	WaveOutThread();
	WaveOutThread(const WaveOutThread &) = delete;
	WaveOutThread & operator=(const WaveOutThread &) = delete;
	~WaveOutThread();

	void WaveOutMain(HWAVEOUT &hWaveOut);

	void prepareHeader(HWAVEOUT hWaveOut, unsigned int waveThreadNum);
	void copyWaveBlock(LPSTR pMemBuff, unsigned int bufferSize);
	MMRESULT writeHeader(HWAVEOUT &hWaveOut);
	void unprepareHeader(HWAVEOUT &hWaveOut);

	void play();


	// Protection
	std::mutex writeMutex;
	std::mutex playMutex;
	std::condition_variable cv;
	
	// Data
	WAVEHDR *pHeader;
	LPSTR pWaveoutBuff;
	bool canPlay;
	bool isLive;

private:
	bool pad1;
	bool pad2;
};

#endif // !WAVEOUTTHREAD_H

