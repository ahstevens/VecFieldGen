#pragma once
#include <vector>
#include <algorithm>
#include <iostream>

#include "GLFWInputBroadcaster.h"


GLFWInputBroadcaster::GLFWInputBroadcaster()
{
}

GLFWInputBroadcaster& GLFWInputBroadcaster::getInstance()
{
	static GLFWInputBroadcaster instance;
	return instance;
}

void GLFWInputBroadcaster::init(GLFWwindow * window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	memset(m_arrbActiveKeys, 0, sizeof m_arrbActiveKeys);
	m_bFirstMouse = true;
	m_bMousePressed = false;
	m_fLastMouseX = 0;
	m_fLastMouseY = 0;
}

bool GLFWInputBroadcaster::keyPressed(const int glfwKeyCode)
{
	return m_arrbActiveKeys[glfwKeyCode];
}

bool GLFWInputBroadcaster::mousePressed()
{
	return m_bMousePressed;
}

void GLFWInputBroadcaster::poll()
{
	glfwPollEvents();
}

// Is called whenever a key is pressed/released via GLFW
void GLFWInputBroadcaster::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		return;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			getInstance().m_arrbActiveKeys[key] = true;
			getInstance().notify(NULL, BroadcastSystem::EVENT::KEY_PRESS, &key);
		}
		else if (action == GLFW_REPEAT)
		{
			getInstance().notify(NULL, BroadcastSystem::EVENT::KEY_REPEAT, &key);
		}
		else if (action == GLFW_RELEASE)
		{
			getInstance().m_arrbActiveKeys[key] = false;
			getInstance().notify(NULL, BroadcastSystem::EVENT::KEY_UNPRESS, &key);
		}
	}
}

void GLFWInputBroadcaster::mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		getInstance().m_bMousePressed = true;
		getInstance().notify(NULL, BroadcastSystem::EVENT::MOUSE_CLICK, &button);
	}
	else if (action == GLFW_RELEASE)
	{
		getInstance().m_bMousePressed = false;
		getInstance().notify(NULL, BroadcastSystem::EVENT::MOUSE_UNCLICK, &button);
	}
}

void GLFWInputBroadcaster::mouse_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (getInstance().m_bFirstMouse)
	{
		getInstance().m_fLastMouseX = static_cast<GLfloat>(xpos);
		getInstance().m_fLastMouseY = static_cast<GLfloat>(ypos);
		getInstance().m_bFirstMouse = false;
	}

	GLfloat xoffset = static_cast<GLfloat>(xpos) - getInstance().m_fLastMouseX;
	GLfloat yoffset = getInstance().m_fLastMouseY - static_cast<GLfloat>(ypos);  // Reversed since y-coordinates go from bottom to left

	getInstance().m_fLastMouseX = static_cast<GLfloat>(xpos);
	getInstance().m_fLastMouseY = static_cast<GLfloat>(ypos);

	float offset[2] = { static_cast<float>(xoffset), static_cast<float>(yoffset) };

	getInstance().notify(NULL, BroadcastSystem::EVENT::MOUSE_MOVE, &offset);
}

void GLFWInputBroadcaster::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	float offset = static_cast<float>(yoffset);
	getInstance().notify(NULL, BroadcastSystem::EVENT::MOUSE_SCROLL, &offset);
}
