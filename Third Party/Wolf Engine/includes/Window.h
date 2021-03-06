#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <functional>

namespace Wolf
{	
	class Window
	{
	public:
		Window(std::string appName, int width, int height, void* systemManagerInstance,
			std::function<void(void*, int, int)> resizeCallback);
		~Window();

		void cleanup();

		GLFWwindow* getWindow() const { return m_window; }

	private:
		static void onWindowResized(GLFWwindow* window, int width, int height)
		{
			if (width == 0 || height == 0) return;

			Window* windowManager = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			windowManager->callResizeCallback(width, height);
		}

	public:
		inline void callResizeCallback(int width, int height) { m_resizeCallback(m_systemManagerInstance, width, height); }

	private:
		GLFWwindow* m_window;

		void* m_systemManagerInstance = nullptr;
		std::function<void(void*, int, int)> m_resizeCallback;
	};
}
