#pragma once

#include <core/event/IInputReceiver.h>
#include <core/event/InputManager.h>
#include <platform/common/IInputProvider.h>

#include <atomic>

namespace engine {

class Controller : public core::IInputReceiver {
public:
	explicit Controller(core::InputManager& manager);
	~Controller();

	void onInputEvent(const platform::InputEvent& ev) override;
	void onCursorPosition(double x, double y) override;
	void onScroll(double xOffset, double yOffset) override;

	/** Left stick state in [-1, 1], updated from gamepad axis events. */
	std::atomic<float> leftStickX{0.f};
	std::atomic<float> leftStickY{0.f};

	/** Right stick state in [-1, 1], updated from gamepad axis events. */
	std::atomic<float> rightStickX{0.f};
	std::atomic<float> rightStickY{0.f};

private:
	core::InputManager* manager_ = nullptr;
};

} // namespace engine
