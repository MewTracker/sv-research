#pragma once

#include <windows.h>
#include <cstdint>

class Stopwatch
{
public:
	Stopwatch()
	{
		QueryPerformanceFrequency(&frequency);
		start_time.QuadPart = 0;
		end_time.QuadPart = 0;
	}

	void start()
	{
		QueryPerformanceCounter(&start_time);
	}

	void stop()
	{
		QueryPerformanceCounter(&end_time);
	}

	uint64_t microseconds()
	{
		LARGE_INTEGER elapsed_microseconds;
		elapsed_microseconds.QuadPart = end_time.QuadPart - start_time.QuadPart;
		elapsed_microseconds.QuadPart *= 1000000;
		elapsed_microseconds.QuadPart /= frequency.QuadPart;
		return elapsed_microseconds.QuadPart;
	}

	uint64_t milliseconds()
	{
		return microseconds() / 1000;
	}

private:
	LARGE_INTEGER frequency, start_time, end_time;
};
