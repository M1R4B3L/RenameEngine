#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	void Stop();
	void Resume();

	Uint32 Read();

private:

	bool	running;
	Uint32	startedAt;
	Uint32	stoppedAt;
};

#endif //__TIMER_H__