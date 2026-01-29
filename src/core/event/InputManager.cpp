#include "InputManager.h"
#include <algorithm>
#include <iostream>

namespace core {

void InputManager::onInputEvent(const platform::InputEvent& ev) {
	using platform::InputAction;
	using platform::InputEventType;
	const char* actionStr = ev.type == InputEventType::Key
		? (ev.key.action == InputAction::Release ? "Release" : ev.key.action == InputAction::Press ? "Press" : "Repeat")
		: (ev.mouseButton.action == InputAction::Release ? "Release" : ev.mouseButton.action == InputAction::Press ? "Press" : "Repeat");
	//if (ev.type == InputEventType::Key)
	//	std::cout << "[InputManager] Queuing KeyEvent key=" << ev.key.key << " action=" << actionStr << " mods=" << ev.key.mods << "\n";
	//else
	//	std::cout << "[InputManager] Queuing MouseButtonEvent button=" << ev.mouseButton.button << " action=" << actionStr << " mods=" << ev.mouseButton.mods << "\n";
	queue_.push(ev);
}

void InputManager::onCursorPosition(double x, double y) {
	auto copy = receivers_;
	for (IInputReceiver* r : copy)
		r->onCursorPosition(x, y);
}

void InputManager::onScroll(double xOffset, double yOffset) {
	auto copy = receivers_;
	for (IInputReceiver* r : copy)
		r->onScroll(xOffset, yOffset);
}

void InputManager::update() {
	auto copy = receivers_;
	while (!queue_.empty()) {
		platform::InputEvent ev = queue_.front();
		queue_.pop();
		for (IInputReceiver* r : copy)
			r->onInputEvent(ev);
	}
}

void InputManager::registerReceiver(IInputReceiver* receiver) {
	if (receiver && std::find(receivers_.begin(), receivers_.end(), receiver) == receivers_.end())
		receivers_.push_back(receiver);
}

void InputManager::unregisterReceiver(IInputReceiver* receiver) {
	auto it = std::find(receivers_.begin(), receivers_.end(), receiver);
	if (it != receivers_.end())
		receivers_.erase(it);
}

} // namespace core
