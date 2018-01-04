MEM_TRACKER_BEGIN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <mutex>
#include "ThreadHelper.h"
#include "FileThread.h"
#include "WaveOutThread.h"
#include "CoordinatorThread.h"
#include "PlaybackThread.h"

int main()
{
	HWAVEOUT hWaveOut; // device handle 
	WAVEFORMATEX wfx;  // look this up in your documentation 
	MMRESULT result;   // for waveOut return values 


	wfx.nSamplesPerSec = 22050; // sample rate 
	wfx.wBitsPerSample = 16;    // number of bits per sample of mono data 
	wfx.nChannels = 2;          // number of channels (i.e. mono, stereo...) 
	wfx.wFormatTag = WAVE_FORMAT_PCM;								// format type 
	wfx.nBlockAlign = (WORD)((wfx.wBitsPerSample >> 3) * wfx.nChannels);	// block size of data 
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		// for buffer estimation 
	wfx.cbSize = 0;

	WavePlayerMessenger waveMessenger;

	result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)PlaybackThread::waveOutProc, (DWORD_PTR)&waveMessenger, CALLBACK_FUNCTION);
	if (result != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "unable to open WAVE_MAPPER device\n");
		ExitProcess(1);
	}

	ThreadHelper::SetMainThreadName("MAIN THREAD");

	// -------------------------------------------
	// Start file thread
	// -------------------------------------------

	FileThread *pFileThread = new FileThread();
	std::thread fileThread(&FileThread::FileMain, pFileThread);
	ThreadHelper::SetThreadName(fileThread, "File Thread");

	// -------------------------------------------
	// Start coordinator thread
	// -------------------------------------------

	CoordinatorThread *pCoordThread = new CoordinatorThread();
	std::thread coordThread(&CoordinatorThread::CoordMain, pCoordThread, std::ref(*pFileThread));
	ThreadHelper::SetThreadName(coordThread, "Coordinator Thread");

	// -------------------------------------------
	// Start playback thread
	// -------------------------------------------

	PlaybackThread *pPlaybackThread = new PlaybackThread();
	std::thread playbackThread(&PlaybackThread::PlaybackMain, pPlaybackThread, std::ref(hWaveOut), std::ref(*pCoordThread), std::ref(waveMessenger));
	ThreadHelper::SetThreadName(playbackThread, "Playback Thread");

	// join threads
	playbackThread.join();
	coordThread.join();
	fileThread.join();

	// free the data
	delete pCoordThread;
	delete pFileThread;
	delete pPlaybackThread;

	// ---------------------------------------------------
	// Close Wave Out Handle
	// ---------------------------------------------------

	waveOutClose(hWaveOut);

	waveMessenger.waitForClosed();

	return 0;
}

