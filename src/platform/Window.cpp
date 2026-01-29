#include "Window.h"
#include "common/IInputProvider.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

Window::Window(const WindowDesc& desc) {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfw_window_ = glfwCreateWindow(
		static_cast<int>(desc.width),
		static_cast<int>(desc.height),
		desc.title.c_str(),
		nullptr,
		nullptr
	);

	if (!glfw_window_)
		throw std::runtime_error("Failed to create GLFW window");

	width_ = desc.width;
	height_ = desc.height;

	glfwSetWindowUserPointer(glfw_window_, this);

	glfwSetFramebufferSizeCallback(glfw_window_, framebufferResizeCallback);
	glfwSetKeyCallback(glfw_window_, keyCallback);
	glfwSetMouseButtonCallback(glfw_window_, mouseButtonCallback);
	glfwSetCursorPosCallback(glfw_window_, cursorPosCallback);
	glfwSetScrollCallback(glfw_window_, scrollCallback);
}

Window::~Window() {
	if (glfw_window_)
		glfwDestroyWindow(glfw_window_);
	glfwTerminate();
}

void Window::pollEvents() {
	glfwPollEvents();

	if (glfwWindowShouldClose(glfw_window_))
		should_close_ = true;
}

// why is this const
bool Window::shouldClose() const {
	return should_close_;
}

void Window::setResizeCallback(ResizeCallback cb) {
	on_resize_ = std::move(cb);
}

void Window::setInputReceiver(platform::IInputProvider* receiver) {
	input_receiver_ = receiver;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self) return;

	self->width_ = static_cast<uint32_t>(width);
	self->height_ = static_cast<uint32_t>(height);

	if (self->on_resize_)
		self->on_resize_(self->width_, self->height_);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)scancode;
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self || !self->input_receiver_) return;

	const char* actionStr = action == GLFW_RELEASE ? "Release" : action == GLFW_PRESS ? "Press" : "Repeat";
	//std::cout << "[Window] KeyEvent key=" << key << " action=" << actionStr << " mods=" << mods << "\n";

	platform::InputEvent ev;
	ev.type = platform::InputEventType::Key;
	ev.key.key = key;
	ev.key.action = static_cast<platform::InputAction>(action);
	ev.key.mods = mods;
	self->input_receiver_->onInputEvent(ev);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self || !self->input_receiver_) return;

	const char* actionStr = action == GLFW_RELEASE ? "Release" : action == GLFW_PRESS ? "Press" : "Repeat";
	//std::cout << "[Window] MouseButtonEvent button=" << button << " action=" << actionStr << " mods=" << mods << "\n";

	platform::InputEvent ev;
	ev.type = platform::InputEventType::MouseButton;
	ev.mouseButton.button = button;
	ev.mouseButton.action = static_cast<platform::InputAction>(action);
	ev.mouseButton.mods = mods;
	self->input_receiver_->onInputEvent(ev);
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self || !self->input_receiver_) return;

	self->input_receiver_->onCursorPosition(xpos, ypos);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self || !self->input_receiver_) return;

	self->input_receiver_->onScroll(xoffset, yoffset);
}