/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#include "Stopwatch.h"
#include <mutex>
#include <math.h>

using namespace std;

double accum = 0;
Stopwatch sw;
mutex m;

double Stopwatch::GlobalTime() {
	lock_guard<mutex> L(m);
	accum += sw.Lap();
	return accum;
}

#ifndef _WIN32
Stopwatch::Stopwatch() {
	clock_getres(CLOCK_MONOTONIC, &freq);
	scale = freq.tv_nsec / 1000000000.0;
	Clear();
}

void Stopwatch::Clear() {
	clock_gettime(CLOCK_MONOTONIC, &last);
}

double Stopwatch::Lap() {
	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	unsigned long long delta = (now.tv_sec - last.tv_sec) * 1000000000ull;
	delta += now.tv_nsec - last.tv_nsec;
	last = now;
	return scale * delta;
}
#else
Stopwatch::Stopwatch() {
	QueryPerformanceFrequency(&freq);
	scale = 1.0 / (double)freq.QuadPart;
	Clear();
}

void Stopwatch::Clear() {
	QueryPerformanceCounter(&last);
}

double Stopwatch::Lap() {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	unsigned long long delta = now.QuadPart - last.QuadPart;
	last = now;
	return scale * delta;
}
#endif
