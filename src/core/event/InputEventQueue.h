#pragma once

#include <platform/common/IInputProvider.h>
#include <queue>

namespace core {

class InputEventQueue {
public:
	void push(const platform::InputEvent& ev);
	void pop();
	const platform::InputEvent& front() const;
	bool empty() const;
	size_t size() const;

private:
	std::queue<platform::InputEvent> queue_;
};

} // namespace core
