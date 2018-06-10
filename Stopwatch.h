/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#ifdef _WIN32
#include <wtypes.h>
#define STOPWATCH_BACKEND LARGE_INTEGER
#else
#include <time.h>
#define STOPWATCH_BACKEND timespec
#endif

class Stopwatch {
public:
	//! \brief Seconds since program was started
	static double GlobalTime();

	//! \brief Instantiate a new Stopwatch object and begin accumulating time from zero.
	Stopwatch();

	//! \brief Clear internal time accumulator.
	void Clear();

	/*! \brief Retreive and clear internal time accumulator.
	 *  \return Accumulated time in seconds.
	 */
	double Lap();

private:
	double scale;
	STOPWATCH_BACKEND freq, last;
};

