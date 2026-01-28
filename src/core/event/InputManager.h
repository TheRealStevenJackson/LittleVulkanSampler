#pragma once

#include <platform/common/IInputProvider.h>
#include <vector>

#include "IInputReceiver.h"
#include "InputEventQueue.h"

namespace core {

class InputManager : public platform::IInputProvider {
public:
	void onInputEvent(const platform::InputEvent& ev) override;
	void onCursorPosition(double x, double y) override;
	void onScroll(double xOffset, double yOffset) override;

	void registerReceiver(IInputReceiver* receiver);
	void unregisterReceiver(IInputReceiver* receiver);

	void update();

	InputEventQueue& eventQueue() { return queue_; }
	const InputEventQueue& eventQueue() const { return queue_; }

private:
	InputEventQueue queue_;
	std::vector<IInputReceiver*> receivers_;
};

} // namespace core
