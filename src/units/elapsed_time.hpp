#pragma once

#include "time.hpp"

namespace xen {
class ElapsedTime {
private:
	Time start_time;
	Time interval;

public:
	explicit ElapsedTime(const Time &interval = -1s);

	uint32_t get_elapsed();

	const Time &get_start_time() const { return start_time; }
	void set_start_time(const Time &startTime) { this->start_time = startTime; }

	const Time &get_interval() const { return interval; }
	void set_interval(const Time &interval) { this->interval = interval; }
};
}