#ifndef STATUS_H
#define STATUS_H

enum ThreadStatus
{
	ALIVE = 0x01,
	DONE = 0x02,
	PLAYING = 0x04
};

enum ContainerStatus
{
	FULL = 0x01,
	EMPTY = 0x02
};

#endif // !STATUS_H

