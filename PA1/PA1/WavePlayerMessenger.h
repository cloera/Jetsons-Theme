#ifndef WAVEPLAYERMESSENGER_H
#define WAVEPLAYERMESSENGER_H

#include <mutex>

class WavePlayerMessenger
{
public:
	WavePlayerMessenger();
	WavePlayerMessenger(const WavePlayerMessenger &) = delete;
	WavePlayerMessenger & operator=(const WavePlayerMessenger &) = delete;
	~WavePlayerMessenger();
	
	void incrWaveNum();
	void decrWaveNum();

	void signalClosed();
	void signalDone();

	void waitForClosed();
	void waitForDone();
	void waitForWaveReady();

	bool getDone();
	bool getClosed();
	int  getWaveNum();

private:
	std::mutex m;
	std::condition_variable cv;

	volatile int  waveNum;
	volatile bool donePlaying;
	volatile bool closed;
	bool pad1;
	bool pad2;
};

#endif // !WAVEPLAYERMESSENGER_H
