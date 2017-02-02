#pragma once
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>

#include "BroadcastSystem.h"


class GLFWInputBroadcaster : public BroadcastSystem::Broadcaster
{
public:
	static GLFWInputBroadcaster& getInstance();

	void init(GLFWwindow* window);

	bool keyPressed(const int glfwKeyCode);

	bool mousePressed();

	void poll();

private:
	GLFWInputBroadcaster();

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_button_callback(GLFWwindow* window,int x, int y, int z);
	static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	bool m_arrbActiveKeys[1024];
	bool m_bFirstMouse, m_bMousePressed;
	float m_fLastMouseX, m_fLastMouseY;

	GLFWInputBroadcaster(GLFWInputBroadcaster const&) = delete; // no copies of singletons (C++11)
	void operator=(GLFWInputBroadcaster const&) = delete; // no assigning of singletons (C++11)
};
