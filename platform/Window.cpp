#include "Window.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

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

	glfwSetFramebufferSizeCallback(glfw_window_, framebufferResizeCallback);

	glfwSetWindowUserPointer(glfw_window_, this);
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

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!self) return;

	self->width_ = static_cast<uint32_t>(width);
	self->height_ = static_cast<uint32_t>(height);

	if (self->on_resize_)
		self->on_resize_(self->width_, self->height_);
}