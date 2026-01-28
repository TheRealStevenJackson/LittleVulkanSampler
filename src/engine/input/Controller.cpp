#include "Controller.h"
#include <iostream>

namespace engine {

Controller::Controller(core::InputManager& manager) : manager_(&manager) {
	manager_->registerReceiver(this);
}

Controller::~Controller() {
	if (manager_)
		manager_->unregisterReceiver(this);
}

void Controller::onInputEvent(const platform::InputEvent& ev) {
	using platform::InputAction;
	using platform::InputEventType;
	const char* actionStr = ev.type == InputEventType::Key
		? (ev.key.action == InputAction::Release ? "Release" : ev.key.action == InputAction::Press ? "Press" : "Repeat")
		: (ev.mouseButton.action == InputAction::Release ? "Release" : ev.mouseButton.action == InputAction::Press ? "Press" : "Repeat");
	if (ev.type == InputEventType::Key)
		std::cout << "[Controller] KeyEvent key=" << ev.key.key << " action=" << actionStr << " mods=" << ev.key.mods << "\n";
	else
		std::cout << "[Controller] MouseButtonEvent button=" << ev.mouseButton.button << " action=" << actionStr << " mods=" << ev.mouseButton.mods << "\n";
}

void Controller::onCursorPosition(double x, double y) {
	(void)x;
	(void)y;
}

void Controller::onScroll(double xOffset, double yOffset) {
	(void)xOffset;
	(void)yOffset;
}

} // namespace engine
