#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif // !GLEW_STATIC
#include <GL/glew.h>     // include before GLFW (gl.h)
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"
#include "LightingSystem.h"
#include "GLFWInputBroadcaster.h"

#include "VectorFieldGenerator.h"

#define MS_PER_UPDATE 0.0333333333f
#define CAST_RAY_LEN 1000.f

class Engine : public BroadcastSystem::Listener
{
public:
	std::vector<std::string> m_vstrArgs;

	GLFWwindow* m_pWindow;
	LightingSystem* m_pLightingSystem;

	VectorFieldGenerator* m_pVFG;

	// Constants
	const int m_iWidth = 1280;
	const int m_iHeight = 800;
	const float m_fStepSize = 1.f / 120.f;

	float m_fDeltaTime;	// Time between current frame and last frame
	float m_fLastTime; // Time of last frame

	Camera  *m_pCamera;
	std::vector<Shader*> m_vpShaders;
	Shader *m_pShaderLighting, *m_pShaderNormals, *m_pLineShader;
	
private:
	glm::mat4 m_mat4WorldRotation;
	bool m_bGL;
	bool m_bSphereAdvectorsOnly;

	float m_fDeltaT;
	float m_fAdvectionTime;
	float m_fSphereRadius;

	std::string m_strSavePath;

public:
	Engine(int argc, char* argv[]);
	~Engine();

	bool init();

	void mainLoop();

	void update(float dt);

	void render();

	// Inherited from BroadcastSystem
	void receiveEvent(Object * obj, const int event, void * data);

private:
	bool initGL();

	GLFWwindow* init_gl_context(std::string winName);
	
	void init_lighting();

	void init_camera();

	void init_shaders();

	void generateField();
};