#include "WavePlayerMessenger.h"

WavePlayerMessenger::WavePlayerMessenger()
{
	this->waveNum = 0;
	this->donePlaying = false;
	this->closed = false;
	this->pad1 = false;
	this->pad2 = false;
}

WavePlayerMessenger::~WavePlayerMessenger()
{
}

void WavePlayerMessenger::incrWaveNum()
{
	++waveNum;
}

void WavePlayerMessenger::decrWaveNum()
{
	--waveNum;
	cv.notify_one(); // Notify for waitForWaveReady()
}

void WavePlayerMessenger::signalClosed()
{
	std::unique_lock<std::mutex> lock(this->m);
	this->closed = true;
	lock.unlock();
	cv.notify_one();
}

void WavePlayerMessenger::signalDone()
{
	std::unique_lock<std::mutex> lock(this->m);
	this->donePlaying = true;
	lock.unlock();
	cv.notify_one();
}


void WavePlayerMessenger::waitForClosed()
{
	std::unique_lock<std::mutex> lock(this->m);
	cv.wait(lock, [=]() { return this->getClosed(); });
}

void WavePlayerMessenger::waitForDone()
{
	std::unique_lock<std::mutex> lock(this->m);
	cv.wait(lock, [=]() { return this->getDone(); });
}

void WavePlayerMessenger::waitForWaveReady()
{
	std::unique_lock<std::mutex> lock(this->m);
	// wait until at least 1 out of the 20 waveout buffers are free
	cv.wait(lock, [=]() { return this->getWaveNum() < 20; });
}

bool WavePlayerMessenger::getDone()
{
	return this->donePlaying;
}

bool WavePlayerMessenger::getClosed()
{
	return this->closed;
}

int WavePlayerMessenger::getWaveNum()
{
	return this->waveNum;
}
