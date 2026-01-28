#pragma once

#include <core/event/IInputReceiver.h>
#include <core/event/InputManager.h>

namespace engine {

class Controller : public core::IInputReceiver {
public:
	explicit Controller(core::InputManager& manager);
	~Controller();

	void onInputEvent(const platform::InputEvent& ev) override;
	void onCursorPosition(double x, double y) override;
	void onScroll(double xOffset, double yOffset) override;

private:
	core::InputManager* manager_ = nullptr;
};

} // namespace engine
