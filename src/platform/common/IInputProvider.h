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

/** Gamepad button indices; values match GLFW_GAMEPAD_BUTTON_* for direct comparison. */
enum class GamepadButton : int {
	A = 0,
	B = 1,
	X = 2,
	Y = 3,
	LeftBumper = 4,
	RightBumper = 5,
	Back = 6,
	Start = 7,
	Guide = 8,
	LeftThumb = 9,
	RightThumb = 10,
	DpadUp = 11,
	DpadRight = 12,
	DpadDown = 13,
	DpadLeft = 14,
};

/** Gamepad axis indices; values match GLFW_GAMEPAD_AXIS_*. */
enum class GamepadAxis : int {
	LeftStickX = 0,
	LeftStickY = 1,
	RightStickX = 2,
	RightStickY = 3,
	LeftTrigger = 4,
	RightTrigger = 5,
};

/** Human-readable name for a gamepad button index, or "Unknown" if out of range. */
const char* getGamepadButtonName(int button);

/** Human-readable name for a gamepad axis index, or "Unknown" if out of range. */
const char* getGamepadAxisName(int axis);

/** Gamepad button: jid = GLFW_JOYSTICK_1..16, button = GamepadButton / GLFW_GAMEPAD_BUTTON_*. */
struct GamepadButtonEvent {
	int jid;
	int button;
	InputAction action;
};

/** Gamepad axis: jid = GLFW_JOYSTICK_1..16, axis = GamepadAxis / GLFW_GAMEPAD_AXIS_*. value = primary axis; valueX/valueY = stick (x,y) pair for Left/Right stick, or valueX=value, valueY=0 for triggers. All in [-1, 1]. */
struct GamepadAxisEvent {
	int jid;
	int axis;
	float value;
	float valueX;
	float valueY;
};

enum class InputEventType { Key, MouseButton, GamepadButton, GamepadAxis };

struct InputEvent {
	InputEventType type;
	union {
		KeyEvent key;
		MouseButtonEvent mouseButton;
		GamepadButtonEvent gamepadButton;
		GamepadAxisEvent gamepadAxis;
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
