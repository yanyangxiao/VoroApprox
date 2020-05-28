
#ifndef TIMER_H
#define TIMER_H

/************************************************************************/
/* ¸ÄÐ´×ÔCMU performanceCounter                                          */
/************************************************************************/

/**************** LINUX/MAC OS X COUNTER *******************/
#if (defined __unix__) || (defined __APPLE__)

#include "stdlib.h"
#include "sys/time.h"

class Timer
{
protected:

	long startCountSec, stopCountSec, startCountMicroSec, stopCountMicroSec;

public:
	Timer()
	{
		// also, reset the starting counter
		start();
	}

	inline void start() // call this before your code block
	{
		struct timeval tv;

		gettimeofday(&tv, NULL);

		startCountSec = tv.tv_sec;
		startCountMicroSec = tv.tv_usec;
	}

	inline void stop() // call this after your code block
	{
		struct timeval tv;

		gettimeofday(&tv, NULL);

		stopCountSec = tv.tv_sec;
		stopCountMicroSec = tv.tv_usec;
	}

	// read elapsed time (units are seconds, accuracy is up to microseconds)
	inline double get_elapsed_time()
	{
		float elapsedTime = 1.0 * (stopCountSec - startCountSec) + 1E-6 * (stopCountMicroSec - startCountMicroSec);
		return elapsedTime;
	}
};

#endif

#ifdef WIN32

/**************** WINDOWS COUNTER *******************/

#include <windows.h>

class Timer
{
protected:
	LARGE_INTEGER timerFrequency;
	LARGE_INTEGER startCount, stopCount;

public:
	Timer()
	{
		// reset the counter frequency
		QueryPerformanceFrequency(&timerFrequency);
		// also, reset the starting counter
		start();
	}

	inline void start() // call this before your code block
	{
		QueryPerformanceCounter(&startCount);
	}

	inline void stop() // call this after your code block
	{
		QueryPerformanceCounter(&stopCount);
	}

	// read elapsed time (units are seconds, accuracy is up to microseconds)
	inline double get_elapsed_time()
	{
		return ((double)(stopCount.QuadPart - startCount.QuadPart)) / ((double)timerFrequency.QuadPart);
	}
};

#endif

#endif

