#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <mutex>
#include "Trace.h"

// Used for throwing exception when queue is empty
struct Empty_Message_Queue : std::exception
{
	const char* what() const throw()
	{
		return "Message queue is empty after Pop()\n";
	}
};



/***********************************
** FIFO Thread Safe Circular Queue
************************************/
template<typename T>
class MessageQueue
{
public:
	MessageQueue()
		: headIdx(-1), tailIdx(-1)
	{
	}
	MessageQueue(const MessageQueue &in)
	{
		std::lock_guard<std::mutex> lock(this->m);
		this->headIdx = in.headIdx;
		this->tailIdx = in.tailIdx;
		for (int i = 0; i < maxSize; i++)
		{
			this->queue[i] = in.queue[i];
		}
	}
	MessageQueue & operator=(const MessageQueue &) = delete;
	~MessageQueue()
	{
	}

	bool Empty()
	{
		std::lock_guard<std::mutex> lock(this->m);
		if (this->tailIdx == -1 && this->headIdx == -1)
		{
			return true;
		}
		return false;
	}
	/*****************************
	** Add element to end of queue
	******************************/
	void PushBack(T value)
	{
		std::unique_lock<std::mutex> lock(this->m, std::defer_lock);
		// If queue is empty
		if (Empty())
		{
			lock.lock();
			// Index is set to first position
			this->headIdx = 0;
			this->tailIdx = 0;
		}
		else
		{
			lock.lock();
			// Increment tail position
			// If tailIdx+1 = 40 then tailIdx+1 mod maxSize will equal 0
			// this will bring tailIdx to first index of queue
			this->tailIdx = (this->tailIdx + 1) % maxSize;
		}
		// Insert item to end of queue
		this->queue[this->tailIdx] = value;
	}
	/********************************************
	** Return first element in queue as reference
	** and remove the element from front of queue
	********************************************/
	void Pop(T &retVal)
	{
		// If queue is empty
		if (Empty())
		{
			// Throw exception
			throw Empty_Message_Queue();
		}
		// Else if only one element in queue
		else if(this->tailIdx == this->headIdx)
		{
			std::unique_lock<std::mutex> lock(this->m);
			// Return item at the head of queue
			retVal = this->queue[this->headIdx];
			// Set indices to -1 to indicate empty queue
			this->headIdx = -1;
			this->tailIdx = -1;
			lock.unlock();
		}
		// Else more than one element in queue
		else
		{
			std::unique_lock<std::mutex> lock(this->m);
			// Return item at the head of queue
			retVal = this->queue[this->headIdx];
			// Increment head of queue
			// If headIdx+1 = 40 then headIdx+1 mod maxSize will equal 0
			// this will bring headIdx to first index of queue
			this->headIdx = (this->headIdx + 1) % maxSize;
			lock.unlock();
		}
	}

private:
	// Protection
	mutable std::mutex m;

	// Data
	T queue[40];
	int headIdx; // Index for front of queue
	int tailIdx; // Index for end of queue
	const int maxSize = 40; // max size = 2x number of waveout threads (just in case)
};


#endif // !MESSAGEQUEUE_H
