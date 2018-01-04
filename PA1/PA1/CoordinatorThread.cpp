#include "CoordinatorThread.h"
#include "Trace.h"

CoordinatorThread::CoordinatorThread()
{
	unsigned int size = (unsigned int) 512 * 1024;
	this->frontBackBuffer[0] = new char[size];
	this->frontBackBuffer[1] = new char[size];

	memset(this->frontBackBuffer[0], 0, size);
	memset(this->frontBackBuffer[1], 0, size);

	this->fbBufferSize[0] = 0;
	this->fbBufferSize[1] = 0;

	this->frontBuffIndex = 0;
	this->pActiveBuffer = 0;
	this->coordStatus = ThreadStatus::ALIVE;
	this->frontBuffStatus = ContainerStatus::EMPTY;
}

CoordinatorThread::~CoordinatorThread()
{
	delete this->frontBackBuffer[0];
	delete this->frontBackBuffer[1];
}

void CoordinatorThread::CoordMain(FileThread & fileThread)
{
	ThreadStatus fileStatus;
	while (fileThread.fileStatus == ThreadStatus::ALIVE)
	{
		// Pull data from file thread and fill back buffer
		fileStatus = PullData(fileThread);

		// Wait for front buffer to be empty
		std::unique_lock<std::mutex> lock(this->pushMutex);
		cv.wait(lock, [=]() { return this->frontBuffStatus == ContainerStatus::EMPTY; });

		// Swap front and back buffers
		SwapBuffers();
	}
	// Wait for final buffer to fill
	std::unique_lock<std::mutex> lock(this->pushMutex);
	cv.wait(lock, [=]() { return this->frontBuffStatus == ContainerStatus::EMPTY; });

	// Set flags to break out of while loop in PlaybackLoop()
	// and wait() in PushData()
	this->coordStatus = ThreadStatus::DONE;
	this->frontBuffStatus = ContainerStatus::FULL;

	// Notify PushData() to finish process
	this->cv.notify_one();
}

ThreadStatus CoordinatorThread::PullData(FileThread & fileThread)
{
	unsigned int backIndex = 0x1 ^ this->frontBuffIndex;
	ThreadStatus status = fileThread.TransferBuffer(this->frontBackBuffer[backIndex], this->fbBufferSize[backIndex]);
	printf("Pulling data *******************\n");
	return status;
}

ThreadStatus CoordinatorThread::PushData(WaveOutThread & waveoutThread)
{
	// If front buffer is empty then notify and exit
	if (this->fbBufferSize[this->frontBuffIndex] <= 0)
	{
		this->frontBuffStatus = ContainerStatus::EMPTY;
		cv.notify_one();
		
		std::unique_lock<std::mutex> lock(this->swapMutex);
		this->cv.wait(lock, [=]() { return this->frontBuffStatus == ContainerStatus::FULL; });
	}
	if (this->coordStatus == ThreadStatus::ALIVE)
	{
		unsigned int waveBuffSize = (2 * 1024);
		LPSTR pBuff;

		std::unique_lock<std::mutex> lock2(this->pushMutex);
		// Is there LESS THAN 2K worth of data pushing?
		if (this->fbBufferSize[this->frontBuffIndex] < waveBuffSize)
		{
			// Copy LESS THAN 2K of data to waveout buffer
			waveBuffSize = this->fbBufferSize[this->frontBuffIndex];
		}
		// Store pActiveBuffer in temp variables
		pBuff = this->pActiveBuffer;

		// Increment active buffer pointer
		this->pActiveBuffer += waveBuffSize;
		// Decrement front buffer size
		this->fbBufferSize[this->frontBuffIndex] -= waveBuffSize;
		Trace::out("FB Buff Size: %d\n", this->fbBufferSize[this->frontBuffIndex]);
		lock2.unlock();

		waveoutThread.copyWaveBlock(pBuff, (unsigned int)waveBuffSize);

		lock2.lock();
		return this->coordStatus;
	}
	return this->coordStatus;
}

void CoordinatorThread::SwapBuffers()
{
	unsigned int backIndex = 0x1 ^ this->frontBuffIndex;

	std::unique_lock<std::mutex> lock(this->swapMutex);

	// Move pActiveBuffer to new front buffer
	this->pActiveBuffer = this->frontBackBuffer[backIndex];
	// Move front buffer size
	this->fbBufferSize[this->frontBuffIndex] = this->fbBufferSize[backIndex];
	// Flip bit for new front buffer index
	this->frontBuffIndex = backIndex;
	// Set front buffer flag
	this->frontBuffStatus = ContainerStatus::FULL;

	lock.unlock();
	this->cv.notify_one();

	printf("Swapping buffers\n");
}

ContainerStatus CoordinatorThread::getFrontBuffStatus()
{
	return this->frontBuffStatus;
}
