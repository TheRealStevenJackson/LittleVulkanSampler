#pragma once

#include <platform/common/IInputProvider.h>

namespace core {

/** Interface for receiving input from InputManager. Register with InputManager to get callbacks. */
struct IInputReceiver {
	virtual ~IInputReceiver() = default;

	virtual void onInputEvent(const platform::InputEvent& ev) { (void)ev; }
	virtual void onCursorPosition(double x, double y) { (void)x; (void)y; }
	virtual void onScroll(double xOffset, double yOffset) { (void)xOffset; (void)yOffset; }
};

} // namespace core
