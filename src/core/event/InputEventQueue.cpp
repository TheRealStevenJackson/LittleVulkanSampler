#include "InputEventQueue.h"

namespace core {

void InputEventQueue::push(const platform::InputEvent& ev) {
	queue_.push(ev);
}

void InputEventQueue::pop() {
	queue_.pop();
}

const platform::InputEvent& InputEventQueue::front() const {
	return queue_.front();
}

bool InputEventQueue::empty() const {
	return queue_.empty();
}

size_t InputEventQueue::size() const {
	return queue_.size();
}

} // namespace core
