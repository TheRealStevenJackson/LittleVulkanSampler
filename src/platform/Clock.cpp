#include "Clock.h"

Clock::Clock()
	: mDelta(0.0)
{

}

Clock::~Clock()
{

}

void Clock::tick()
{
	auto now = std::chrono::steady_clock::now();

	if (!mLast.has_value()) {
		mLast = now;
		return;
	}

	mDelta = std::chrono::duration<double>(now - *mLast).count();
	mLast = now;
}