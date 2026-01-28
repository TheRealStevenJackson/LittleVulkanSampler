#pragma once
#include <string>
#include <functional>

struct GLFWwindow;

namespace platform {
struct IInputProvider;
}

struct WindowDesc {
	std::string title = "LittleVulkanEngine";
	uint32_t width = 1280;
	uint32_t height = 720;
	bool fullstreen = false;
};

class Window {
public:
	using ResizeCallback = std::function<void(uint32_t, uint32_t)>;

	explicit Window(const WindowDesc& desc);
	~Window();

	void pollEvents();
	bool shouldClose() const;

	GLFWwindow* getHandle() const { return glfw_window_; };
	uint32_t width() const { return width_; }
	uint32_t height() const { return height_; }

	void setResizeCallback(ResizeCallback cb);
	void setInputReceiver(platform::IInputProvider* receiver);

private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	GLFWwindow* glfw_window_ = nullptr;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	bool should_close_ = false;
	ResizeCallback on_resize_;
	platform::IInputProvider* input_receiver_ = nullptr;
};