#pragma once

#include <chrono>
#include <optional>

class Clock {
public:
	Clock();
	~Clock();

	void tick();
	double deltaTime() const { return mDelta; }

private:
	std::optional<std::chrono::steady_clock::time_point> mLast;
	double mDelta;
};
