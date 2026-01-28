#pragma once

namespace platform {

/** Action for key and mouse button events. */
enum class InputAction {
	Release = 0,
	Press   = 1,
	Repeat  = 2,
};

struct KeyEvent {
	int key;
	InputAction action;
	int mods;
};

struct MouseButtonEvent {
	int button;
	InputAction action;
	int mods;
};

enum class InputEventType { Key, MouseButton };

struct InputEvent {
	InputEventType type;
	union {
		KeyEvent key;
		MouseButtonEvent mouseButton;
	};
};

/** Interface for receiving window input (keyboard, mouse). */
struct IInputProvider {
	virtual ~IInputProvider() = default;

	/** Key or mouse button event. Use ev.type to discriminate. */
	virtual void onInputEvent(const InputEvent& ev) { (void)ev; }

	/** Cursor position in window coordinates. */
	virtual void onCursorPosition(double x, double y) { (void)x; (void)y; }

	/** Scroll offsets. */
	virtual void onScroll(double xOffset, double yOffset) { (void)xOffset; (void)yOffset; }
};

} // namespace platform
