#pragma once

#include "Engine.h"

#include <glm/gtc/type_ptr.hpp>

#include "DebugDrawer.h"

#define GRID_RES 32u

Engine::Engine(int argc, char* argv[])
	: m_pWindow(NULL)
	, m_pLightingSystem(NULL)
	, m_pVFG(NULL)
	, m_fDeltaTime(0.f)
	, m_fLastTime(0.f)
	, m_pCamera(NULL)
	, m_pShaderLighting(NULL)
	, m_pShaderNormals(NULL)
{
	for (int i = 0; i < argc; ++i)
		m_vstrArgs.push_back(std::string(argv[i]));
}

Engine::~Engine()
{
}

void Engine::receiveEvent(Object * obj, const int event, void * data)
{
	if (event == BroadcastSystem::EVENT::KEY_PRESS || event == BroadcastSystem::EVENT::KEY_REPEAT)
	{
		int key;
		memcpy(&key, data, sizeof(key));

		if (key == GLFW_KEY_R)
			m_pVFG->init(6u, GRID_RES);

		if (key == GLFW_KEY_RIGHT)
			m_mat4WorldRotation = glm::rotate(m_mat4WorldRotation, glm::radians(1.f), glm::vec3(0.f, 1.f, 0.f));
		if (key == GLFW_KEY_LEFT)
			m_mat4WorldRotation = glm::rotate(m_mat4WorldRotation, glm::radians(-1.f), glm::vec3(0.f, 1.f, 0.f));
	}

	if (event == BroadcastSystem::EVENT::MOUSE_UNCLICK)
	{
		int button;
		memcpy(&button, data, sizeof(button));

		if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			glm::vec3 rayFrom = m_pCamera->getPosition();
			glm::vec3 rayTo = rayFrom + m_pCamera->getOrientation()[2] * CAST_RAY_LEN;
			glm::vec3 payload[2] = { rayFrom, rayTo };
		}
	}
}

bool Engine::init()
{
	// Load GLFW 
	glfwInit();

	m_pWindow = init_gl_context("OpenGL Seaweed Viewer");

	if (!m_pWindow)
		return false;

	GLFWInputBroadcaster::getInstance().init(m_pWindow);
	GLFWInputBroadcaster::getInstance().attach(this);  // Register self with input broadcaster

	init_lighting();
	init_camera();
	init_shaders();
		
	m_pVFG = new VectorFieldGenerator();
	m_pVFG->init(6u, GRID_RES);

	return true;
}

void Engine::mainLoop()
{
	m_fLastTime = static_cast<float>(glfwGetTime());

	// Main Rendering Loop
	while (!glfwWindowShouldClose(m_pWindow)) {
		// Calculate deltatime of current frame
		float newTime = static_cast<float>(glfwGetTime());
		m_fDeltaTime = newTime - m_fLastTime;
		m_fLastTime = newTime;

		// Poll input events first
		GLFWInputBroadcaster::getInstance().poll();

		update(m_fDeltaTime);

		render();

		// Flip buffers and render to screen
		glfwSwapBuffers(m_pWindow);
	}

	glfwTerminate();
}

void Engine::update(float dt)
{
	m_pCamera->update(dt);

	for (auto &pl : m_pLightingSystem->pLights)
	{
		float step = 180.f * dt;
		pl.position = glm::vec3(glm::rotate(glm::mat4(), glm::radians(step), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(pl.position, 1.f));
		pl.diffuse = (glm::normalize(pl.position) + glm::vec3(1.f)) / glm::vec3(2.f);
		pl.specular = (glm::normalize(pl.position) + glm::vec3(1.f)) / glm::vec3(2.f);
	}

	// Create camera transformations
	glm::mat4 view = m_pCamera->getViewMatrix();
	glm::mat4 projection = glm::perspective(
		glm::radians(m_pCamera->getZoom()),
		static_cast<float>(m_iWidth) / static_cast<float>(m_iHeight),
		0.1f,
		1000.0f
		);

	for (auto &shader : m_vpShaders)
	{
		shader->use();

		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "worldRotation"), 1, GL_FALSE, glm::value_ptr(m_mat4WorldRotation));
		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		if (shader == m_pShaderLighting)
			m_pLightingSystem->update(view, shader);
	}

	DebugDrawer::getInstance().setVPMatrix(projection * view * m_mat4WorldRotation);

	Shader::off();
}

void Engine::render()
{
	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_MULTISAMPLE);

	// Background Fill Color
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& shader : m_vpShaders)
	{
		shader->use();

		m_pLightingSystem->draw(*shader);

		m_pVFG->draw(*shader);
	}

	DebugDrawer::getInstance().render();

	Shader::off();
}

GLFWwindow* Engine::init_gl_context(std::string winName)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* mWindow = glfwCreateWindow(m_iWidth, m_iHeight, winName.c_str(), nullptr, nullptr);
	// Check for Valid Context
	if (mWindow == nullptr)
		return nullptr;

	// Create Context and Load OpenGL Functions
	glfwMakeContextCurrent(mWindow);

	// GLFW Options
	//glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture cursor and disable it

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	glewInit();
	fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
	GLenum err = glGetError(); // clear GL_INVALID_ENUM error from glewInit

	// Define the viewport dimensions
	glViewport(0, 0, m_iWidth, m_iHeight);

	return mWindow;
}

// Initialize the lighting system
void Engine::init_lighting()
{
	m_pLightingSystem = new LightingSystem();
	GLFWInputBroadcaster::getInstance().attach(m_pLightingSystem);

	// Directional light
	m_pLightingSystem->addDirectLight(glm::vec3(1.f)
		, glm::vec3(0.1f)
		, glm::vec3(1.f)
		, glm::vec3(1.f)
	);

	// Positions of the point lights
	//m_pLightingSystem->addPointLight(glm::vec3(5.f, 0.f, 5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(5.f, 0.f, -5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(-5.f, 0.f, 5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(-5.f, 0.f, -5.f));

	// Spotlight
	//m_pLightingSystem->addSpotLight();
}

void Engine::init_camera()
{
	m_pCamera = new Camera(glm::vec3(0.f, 0.f, 5.f));
	GLFWInputBroadcaster::getInstance().attach(m_pCamera);
}

void Engine::init_shaders()
{
	// Build and compile our shader program
	m_pShaderLighting = m_pLightingSystem->generateLightingShader();
	m_vpShaders.push_back(m_pShaderLighting);

	std::string vBuffer, fBuffer, gBuffer;

	vBuffer.append("#version 330 core\n");
	vBuffer.append("layout(location = 0) in vec3 position;\n");
	vBuffer.append("layout(location = 1) in vec3 normal;\n");
	vBuffer.append("out VS_OUT{\n");
	vBuffer.append("	vec3 normal;\n");
	vBuffer.append("} vs_out;\n");
	vBuffer.append("uniform mat4 projection;\n");
	vBuffer.append("uniform mat4 view;\n");
	vBuffer.append("uniform mat4 model;\n");
	vBuffer.append("void main()\n");
	vBuffer.append("{\n");
	vBuffer.append("	gl_Position = projection * view * model * vec4(position, 1.0f);\n");
	vBuffer.append("	mat3 normalMatrix = mat3(transpose(inverse(view * model)));\n");
	vBuffer.append("	vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * normal, 1.0)));\n");
	vBuffer.append("}");

	gBuffer.append("#version 330 core\n");
	gBuffer.append("layout(triangles) in;\n");
	gBuffer.append("layout(line_strip, max_vertices = 6) out;\n");
	gBuffer.append("in VS_OUT{\n");
	gBuffer.append("	vec3 normal;\n");
	gBuffer.append("} gs_in[];\n");
	gBuffer.append("const float MAGNITUDE = 1.f;\n");
	gBuffer.append("void GenerateLine(int index)\n");
	gBuffer.append("{\n");
	gBuffer.append("	gl_Position = gl_in[index].gl_Position;\n");
	gBuffer.append("	EmitVertex();\n");
	gBuffer.append("	gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE;\n");
	gBuffer.append("	EmitVertex();\n");
	gBuffer.append("	EndPrimitive();\n");
	gBuffer.append("}\n");
	gBuffer.append("void main()\n");
	gBuffer.append("{\n");
	gBuffer.append("	GenerateLine(0); // First vertex normal\n");
	gBuffer.append("	GenerateLine(1); // Second vertex normal\n");
	gBuffer.append("	GenerateLine(2); // Third vertex normal\n");
	gBuffer.append("}");	
		
	fBuffer.append("#version 330 core\n");
	fBuffer.append("out vec4 color;\n");
	fBuffer.append("void main()\n");
	fBuffer.append("{\n");
	fBuffer.append("	color = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n");
	fBuffer.append("}");

	m_pShaderNormals = new Shader(vBuffer.c_str(), fBuffer.c_str(), gBuffer.c_str());
	//m_vpShaders.push_back(m_pShaderNormals);
}