#include "GamepadReader.h"
#include "common/IInputProvider.h"

#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <iostream>

namespace platform {

const char* getGamepadButtonName(int button) {
	switch (button) {
		case 0:  return "A";
		case 1:  return "B";
		case 2:  return "X";
		case 3:  return "Y";
		case 4:  return "LeftBumper";
		case 5:  return "RightBumper";
		case 6:  return "Back";
		case 7:  return "Start";
		case 8:  return "Guide";
		case 9:  return "LeftThumb";
		case 10: return "RightThumb";
		case 11: return "DpadUp";
		case 12: return "DpadRight";
		case 13: return "DpadDown";
		case 14: return "DpadLeft";
		default: return "Unknown";
	}
}

const char* getGamepadAxisName(int axis) {
	switch (axis) {
		case 0: return "LeftStick";
		case 1: return "LeftStick";
		case 2: return "RightStick";
		case 3: return "RightStick";
		case 4: return "LeftTrigger";
		case 5: return "RightTrigger";
		default: return "Unknown";
	}
}

} // namespace platform

float GamepadReader::normalizeAxis(float raw) {
	const float v = std::max(-1.f, std::min(1.f, raw));
	if (std::abs(v) <= AXIS_DEADZONE)
		return 0.f;
	const float sign = (v > 0.f) ? 1.f : -1.f;
	const float normalized = sign * (std::abs(v) - AXIS_DEADZONE) / (1.f - AXIS_DEADZONE);
	return std::max(-1.f, std::min(1.f, normalized));
}

void GamepadReader::poll() {
	if (!input_receiver_)
		return;

	for (int jid = 0; jid < MAX_JOYSTICKS; ++jid) {
		bool present = glfwJoystickPresent(jid) == GLFW_TRUE && glfwJoystickIsGamepad(jid) == GLFW_TRUE;
		if (!present) {
			if (prev_present_[jid]) {
				// Gamepad disconnected; clear stored state so reconnection is clean
				std::memset(prev_buttons_[jid], 0, sizeof(prev_buttons_[jid]));
				std::memset(prev_axes_[jid], 0, sizeof(prev_axes_[jid]));
			}
			prev_present_[jid] = false;
			continue;
		}

		prev_present_[jid] = true;

		GLFWgamepadstate state;
		if (glfwGetGamepadState(jid, &state) != GLFW_TRUE)
			continue;

		// Button events: compare with previous state
		for (int b = 0; b < GAMEPAD_BUTTON_COUNT; ++b) {
			unsigned char now = state.buttons[b];
			unsigned char prev = prev_buttons_[jid][b];
			prev_buttons_[jid][b] = now;

			if (now == prev)
				continue;

			const char* actionStr = (now == GLFW_PRESS) ? "Press" : "Release";
			std::cout << "[GamepadReader] GamepadButton jid=" << jid << " button=" << platform::getGamepadButtonName(b) << " action=" << actionStr << "\n";

			platform::InputEvent ev;
			ev.type = platform::InputEventType::GamepadButton;
			ev.gamepadButton.jid = jid;
			ev.gamepadButton.button = b;
			ev.gamepadButton.action = (now == GLFW_PRESS) ? platform::InputAction::Press : platform::InputAction::Release;
			input_receiver_->onInputEvent(ev);
		}

		// Axis events: emit when value changes beyond deadzone. Use GLFW axis constants so Left/Right stick axes are unambiguous.
		const int leftX = GLFW_GAMEPAD_AXIS_LEFT_X;
		const int leftY = GLFW_GAMEPAD_AXIS_LEFT_Y;
		const int rightX = GLFW_GAMEPAD_AXIS_RIGHT_X;
		const int rightY = GLFW_GAMEPAD_AXIS_RIGHT_Y;

		for (int a = 0; a < GAMEPAD_AXIS_COUNT; ++a) {
			float raw = state.axes[a];
			float prevRaw = prev_axes_[jid][a];
			prev_axes_[jid][a] = raw;

			if (std::abs(raw - prevRaw) <= AXIS_CHANGE_THRESHOLD)
				continue;

			float normalized = normalizeAxis(raw);
			float valueX = 0.f, valueY = 0.f;
			if (a == leftX || a == leftY) {
				valueX = normalizeAxis(state.axes[leftX]);
				valueY = normalizeAxis(state.axes[leftY]);
			} else if (a == rightX || a == rightY) {
				// 8Bitdo SN30 Pro: axis 2 = Right X, axis 3 = Right Y (GLFW names them rightX/rightY but physical order is swapped). Axis 2 can mirror axis 1 (Left Y); mask so LeftStick up/down doesn't appear on RightStick.
				const float rawAxis2 = state.axes[rightX];
				const float rawAxis3 = state.axes[rightY];
				const float rawLeftY = state.axes[leftY];
				valueX = (std::abs(rawAxis2 - rawLeftY) > AXIS_CHANGE_THRESHOLD)
					? normalizeAxis(rawAxis2)
					: 0.f;
				valueY = normalizeAxis(rawAxis3);
			} else {
				valueX = normalized;
				valueY = 0.f;
			}

			std::cout << "[GamepadReader] GamepadAxis jid=" << jid << " axis=" << platform::getGamepadAxisName(a) << " xAxis=" << valueX << " yAxis=" << valueY << "\n";

			platform::InputEvent ev;
			ev.type = platform::InputEventType::GamepadAxis;
			ev.gamepadAxis.jid = jid;
			ev.gamepadAxis.axis = a;
			ev.gamepadAxis.value = normalized;
			ev.gamepadAxis.valueX = valueX;
			ev.gamepadAxis.valueY = valueY;
			input_receiver_->onInputEvent(ev);
		}
	}
}
