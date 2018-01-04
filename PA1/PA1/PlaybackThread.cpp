#include "PlaybackThread.h"
#include "WavePlayerMessenger.h"

PlaybackThread::PlaybackThread()
{
	const unsigned int threadCount = (unsigned int)20;
	for (unsigned int i = 0; i < threadCount; i++)
	{
		waveOutThreads[i] = new WaveOutThread();
	}

}

PlaybackThread::~PlaybackThread()
{
	for (int i = 0; i < 20; i++)
	{
		delete waveOutThreads[i];
	}
}

void PlaybackThread::PlaybackMain(HWAVEOUT hWaveOut, CoordinatorThread & pCoordThread, WavePlayerMessenger & waveMessenger)
{

	// -------------------------------------------
	// Prepare WaveOutThreads
	// -------------------------------------------

	const unsigned int threadCount = (unsigned int)20;
	for (int i = 0; i < threadCount; i++)
	{
		waveOutThreads[i]->prepareHeader(hWaveOut, (unsigned int)i);
	}

	// ---------------------------------------------------
	// Fill the buffers and begin
	// ---------------------------------------------------

	for (unsigned int i = 0; i < threadCount; i++)
	{
		// copy data into buffer
		pCoordThread.PushData(*waveOutThreads[i]);
		waveMessenger.incrWaveNum();
	}
	
	std::thread waveThreads[20];
	for (unsigned int i = 0; i < threadCount; i++)
	{
		waveThreads[i] = std::thread(&WaveOutThread::WaveOutMain, waveOutThreads[i], std::ref(hWaveOut));
	}

	// ---------------------------------------------------
	// Playback loop
	// ---------------------------------------------------

	PlaybackLoop(pCoordThread, waveMessenger);

	// ---------------------------------------------------
	// Spin until done
	// ---------------------------------------------------

	waveMessenger.waitForDone();

	// ---------------------------------------------------
	// Unprepare buffers and free them
	// ---------------------------------------------------

	for (unsigned int i = 0; i < threadCount; i++)
	{
		waveOutThreads[i]->unprepareHeader(hWaveOut);
	}

	for (unsigned int i = 0; i < threadCount; i++)
	{
		waveThreads[i].join();
	}
}

void PlaybackThread::PlaybackLoop(CoordinatorThread & pCoordThread, WavePlayerMessenger & waveMessenger)
{
	int j = 0;
	ThreadStatus statusCoordinator;
	const unsigned int threadCount = (unsigned int)20;
	while (pCoordThread.coordStatus == ThreadStatus::ALIVE)
	{
		// Spin until one buffer is free
		waveMessenger.waitForWaveReady();

		// Send waveOutThread to Coordinator Thread to filled with data
		statusCoordinator = pCoordThread.PushData(*waveOutThreads[j]);
		// If statusCoordinator is DONE then it was final push
		if (statusCoordinator == ThreadStatus::DONE)
		{
			// early break
			break;
		}
		waveMessenger.incrWaveNum();
		// Play buffer
		waveOutThreads[j]->play();

		j++;
		if (j > threadCount - 1)
		{
			j = 0;
		}
	}
}



#define UNUSED_VARIABLE(x) { (void)x; }
void CALLBACK PlaybackThread::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	UNUSED_VARIABLE(dwParam2);
	UNUSED_VARIABLE(hWaveOut);

	WavePlayerMessenger *pWavePlayer = (WavePlayerMessenger *)dwInstance;
	WAVEHDR *waveHdr = 0;

	switch (uMsg)
	{
	case WOM_DONE:
		// with this case, dwParam1 is the wavehdr
		waveHdr = (WAVEHDR *)dwParam1;
		pWavePlayer->decrWaveNum();

		if (pWavePlayer->getWaveNum() <= 0)
		{
			pWavePlayer->signalDone();
		}
		break;

	case WOM_CLOSE:
		pWavePlayer->signalClosed();
		printf("WOM_CLOSE:\n");
		break;

	case WOM_OPEN:
		printf("WOM_OPEN:\n");
		break;

	default:
		assert(false);
	}


}

