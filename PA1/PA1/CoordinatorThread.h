#ifndef COORDINATORTHREAD_H
#define COORDINATORTHREAD_H

#include <windows.h>
#include <mutex>
#include "FileThread.h"
#include "WaveOutThread.h"
#include "Status.h"


class CoordinatorThread
{
public:
	CoordinatorThread();
	CoordinatorThread(const CoordinatorThread &) = delete;
	CoordinatorThread & operator=(const CoordinatorThread &) = delete;
	~CoordinatorThread();

	void CoordMain(FileThread &);
	ThreadStatus PushData(WaveOutThread &);

	ContainerStatus getFrontBuffStatus();

	void SwapBuffers();
	ThreadStatus PullData(FileThread &);


	// Protection
	std::condition_variable cv;
	std::mutex pushMutex;
	std::mutex swapMutex;

	// Data
	LPSTR frontBackBuffer[2];
	LPSTR pActiveBuffer;
	unsigned int fbBufferSize[2];
	unsigned int frontBuffIndex;

	ThreadStatus coordStatus;
	ContainerStatus frontBuffStatus;
};


#endif // !COORDINATORTHREAD_H
