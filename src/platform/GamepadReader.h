#pragma once

namespace platform {
struct IInputProvider;
}

/** Polls GLFW gamepad state and forwards button/axis changes to IInputProvider::onInputEvent. */
class GamepadReader {
public:
	GamepadReader() = default;
	~GamepadReader() = default;

	void setInputReceiver(platform::IInputProvider* receiver) { input_receiver_ = receiver; }

	/** Call each frame (e.g. after glfwPollEvents). Emits events for button press/release and axis changes. */
	void poll();

private:
	static constexpr int MAX_JOYSTICKS = 16;
	static constexpr int GAMEPAD_BUTTON_COUNT = 15;
	static constexpr int GAMEPAD_AXIS_COUNT = 6;
	/** Minimum raw change to emit an axis event. */
	static constexpr float AXIS_CHANGE_THRESHOLD = 0.001f;
	/** Raw values in [-DEADZONE, DEADZONE] are output as 0; remainder is rescaled to [-1, 1]. */
	static constexpr float AXIS_DEADZONE = 0.15f;

	/** Returns value in [-1, 1] with deadzone applied and range rescaled. */
	static float normalizeAxis(float raw);

	platform::IInputProvider* input_receiver_ = nullptr;
	unsigned char prev_buttons_[MAX_JOYSTICKS][GAMEPAD_BUTTON_COUNT] = {};
	float prev_axes_[MAX_JOYSTICKS][GAMEPAD_AXIS_COUNT] = {};
	bool prev_present_[MAX_JOYSTICKS] = {};
};
