#ifndef PLAYBACKTHREAD_H
#define PLAYBACKTHREAD_H

#include <windows.h>
#include "CoordinatorThread.h"
#include "WaveOutThread.h"
#include "WavePlayerMessenger.h"
#include "Status.h"

class PlaybackThread
{
public:
	PlaybackThread();
	PlaybackThread(const PlaybackThread &) = delete;
	PlaybackThread & operator=(const PlaybackThread &) = delete;
	~PlaybackThread();

	void PlaybackMain(HWAVEOUT hWaveOut, CoordinatorThread & pCoordThread, WavePlayerMessenger & waveMessenger);
	void PlaybackLoop(CoordinatorThread & pCoordThread, WavePlayerMessenger & waveMessenger);

	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

private:
	// Data
	WaveOutThread *waveOutThreads[20];
};

#endif // !PLAYBACKTHREAD_H

