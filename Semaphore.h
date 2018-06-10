/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#include <mutex>
#include <condition_variable>

struct Semaphore {
	std::mutex m;
	std::condition_variable cond;
	int waiting, count;

	Semaphore() {
		waiting = 0;
		count = 0;
	}
	void Signal(int num = 0) {
		std::unique_lock<std::mutex> lock(m);
		if (num < 1) num += waiting;
		count += num;
		cond.notify_all();
	}

	void Wait() {
		std::unique_lock<std::mutex> lock(m);
		waiting++;
		while (!count) cond.wait(lock);
		waiting--;
		count--;
	}

	bool TryWait() {
		std::unique_lock<std::mutex> lock(m);
		if (!count) return false;
		count--;
		return true;
	}
};

