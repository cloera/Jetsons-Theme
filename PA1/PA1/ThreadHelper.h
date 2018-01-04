#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

#include "WindowsWrapper.h"

class ThreadHelper
{
public:
	static unsigned long long GetCurrentID();
	static unsigned long long GetCurrentID(std::thread &t);
	static void SetThreadName(std::thread &t0, char* threadName);
	static void SetMainThreadName(char* threadName);
	static const char * GetThreadName();
};

#endif