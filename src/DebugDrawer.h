#pragma once

#include <vector>

#include <string>

// GL Includes
#define GLEW_STATIC      // use static GLEW libs
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class DebugDrawer
{
public:
	// Singleton instance access
	static DebugDrawer& getInstance()
	{
		static DebugDrawer instance;
		return instance;
	}

	// Set the debug drawer's transform using a GLM 4x4 matrix
	// NOTE: Set this before drawing if you want to specify non-world-space coordinates
	// (e.g., set it to an object's model transformation matrix before drawing the model in local space)
	void setTransform(glm::mat4 &m)
	{
		m_mat4Transform = m;
	}

	// Set the debug drawer's transform using an OpenGL-style float[16] array
	// NOTE: Set this before drawing if you want to specify non-world-space coordinates
	// (e.g., set it to an object's model transformation matrix before drawing the model in local space)
	void setTransform(float *m)
	{
		m_mat4Transform = glm::make_mat4(m);
	}	
	
	void setTransform(const float *m)
	{
		m_mat4Transform = glm::make_mat4(m);
	}

	// Reset the debug drawer's drawing transform so that it draws in world space
	void setTransformDefault()
	{
		m_mat4Transform = glm::mat4();
	}

	void setVPMatrix(glm::mat4 &m)
	{
		m_mat4VP = m;
	}

	// Draw a line using the debug drawer. To draw in a different coordinate space, use setTransform()
	void drawLine(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &col = glm::vec3(1.f))
	{
		glm::vec3 from_xformed = glm::vec3(m_mat4Transform * glm::vec4(from, 1.f));
		glm::vec3 to_xformed = glm::vec3(m_mat4Transform * glm::vec4(to, 1.f));

		m_vVertices.push_back(DebugVertex(from_xformed, col));
		m_vVertices.push_back(DebugVertex(to_xformed, col));

		m_bNeedsRefresh = true;
	}

	void drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &color)
	{
		drawLine(v0, v1, color);
		drawLine(v1, v2, color);
		drawLine(v2, v0, color);

		m_bNeedsRefresh = true;
	}

	void drawTransform(float orthoLen)
	{
		glm::vec3 start(0.f);
		drawLine(start, start + glm::vec3(glm::vec4(orthoLen, 0.f, 0.f, 0.f)), glm::vec3(0.7f, 0.f, 0.f));
		drawLine(start, start + glm::vec3(glm::vec4(0.f, orthoLen, 0.f, 0.f)), glm::vec3(0.f, 0.7f, 0.f));
		drawLine(start, start + glm::vec3(glm::vec4(0.f, 0.f, orthoLen, 0.f)), glm::vec3(0.f, 0.f, 0.7f));

		m_bNeedsRefresh = true;
	}

	void drawArc(float radiusX, float radiusY, float minAngle, float maxAngle, const glm::vec3 &color, bool drawSect, float stepDegrees = float(10.f))
	{
		glm::vec3 center(0.f);

		const glm::vec3 vx(1.f, 0.f, 0.f);
		glm::vec3 vy(0.f, 1.f, 0.f);
		float step = glm::radians(stepDegrees);
		int nSteps = (int)glm::abs((maxAngle - minAngle) / step);
		if (!nSteps) nSteps = 1;
		glm::vec3 prev = center + radiusX * vx * glm::cos(glm::radians(minAngle)) + radiusY * vy * glm::sin(glm::radians(minAngle));
		if (drawSect)
		{
			drawLine(center, prev, color);
		}
		for (int i = 1; i <= nSteps; i++)
		{
			float angle = minAngle + (maxAngle - minAngle) * float(i) / float(nSteps);
			glm::vec3 next = center + radiusX * vx * glm::cos(glm::radians(angle)) + radiusY * vy * glm::sin(glm::radians(angle));
			drawLine(prev, next, color);
			prev = next;
		}
		if (drawSect)
		{
			drawLine(center, prev, color);
		}

		m_bNeedsRefresh = true;
	}

	void drawSphere(float radius, float stepDegrees, glm::vec3 &color)
	{
		float minTh = -glm::half_pi<float>();
		float maxTh = glm::half_pi<float>();
		float minPs = -glm::half_pi<float>();
		float maxPs = glm::half_pi<float>();
		glm::vec3 center(0.f, 0.f, 0.f);
		glm::vec3 up(0.f, 1.f, 0.f);
		glm::vec3 axis(1.f, 0.f, 0.f);
		drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
		drawSpherePatch(center, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);

		m_bNeedsRefresh = true;
	}

	virtual void drawBox(const glm::vec3 &bbMin, const glm::vec3 &bbMax, const glm::vec3 &color)
	{
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMin[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMax[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMax[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMin[1], bbMin[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMin[2]), glm::vec3(bbMin[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMin[2]), glm::vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMin[2]), glm::vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMin[2]), glm::vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMin[1], bbMax[2]), glm::vec3(bbMax[0], bbMin[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMin[1], bbMax[2]), glm::vec3(bbMax[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMax[0], bbMax[1], bbMax[2]), glm::vec3(bbMin[0], bbMax[1], bbMax[2]), color);
		drawLine(glm::vec3(bbMin[0], bbMax[1], bbMax[2]), glm::vec3(bbMin[0], bbMin[1], bbMax[2]), color);

		m_bNeedsRefresh = true;
	}

	// Render the mesh
	void render()
	{
		if (m_bNeedsRefresh)
			_refreshVBO();

		if (m_vVertices.size() < 2)
			return;

		glUseProgram(m_pShader->m_nProgram);

		glUniformMatrix4fv(m_glViewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(m_mat4VP));
		
		// Draw mesh
		glBindVertexArray(this->m_glVAO);
		glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vVertices.size()));
		glBindVertexArray(0);

		glUseProgram(0);
	}

	void flushLines()
	{
		m_vVertices.clear();

		m_bNeedsRefresh = true;
	}

private:
	struct DebugVertex {
		glm::vec3 pos;
		glm::vec3 col;

		DebugVertex(glm::vec3 p, glm::vec3 c)
			: pos(p)
			, col(c)
		{}
	};

	GLuint m_glVAO, m_glVBO;
	GLint m_glViewProjectionMatrixLocation;
	std::vector<DebugVertex> m_vVertices;
	glm::mat4 m_mat4Transform;
	glm::mat4 m_mat4VP;

	Shader *m_pShader;

	bool m_bNeedsRefresh;

	static DebugDrawer *s_instance;

	// CTOR
	DebugDrawer()
		: m_pShader(NULL)
		, m_bNeedsRefresh(false)
	{
		_createShader();
		_initGL();
	}

	void drawSpherePatch(const glm::vec3 &center, const glm::vec3 &up, const glm::vec3 &axis, float radius,
		float minTh, float maxTh, float minPs, float maxPs, const glm::vec3 &color, float stepDegrees = float(10.f), bool drawCenter = true)
	{
		glm::vec3 vA[74];
		glm::vec3 vB[74];
		glm::vec3 *pvA = vA, *pvB = vB, *pT;
		glm::vec3 npole = center + up * radius;
		glm::vec3 spole = center - up * radius;
		glm::vec3 arcStart;
		float step = glm::radians(stepDegrees);
		const glm::vec3& kv = up;
		const glm::vec3& iv = axis;
		glm::vec3 jv = glm::cross(kv, iv);
		bool drawN = false;
		bool drawS = false;
		if (minTh <= -glm::half_pi<float>())
		{
			minTh = -glm::half_pi<float>() + step;
			drawN = true;
		}
		if (maxTh >= glm::half_pi<float>())
		{
			maxTh = glm::half_pi<float>() - step;
			drawS = true;
		}
		if (minTh > maxTh)
		{
			minTh = -glm::half_pi<float>() + step;
			maxTh = glm::half_pi<float>() - step;
			drawN = drawS = true;
		}
		int n_hor = (int)((maxTh - minTh) / step) + 1;
		if (n_hor < 2) n_hor = 2;
		float step_h = (maxTh - minTh) / float(n_hor - 1);
		bool isClosed = false;
		if (minPs > maxPs)
		{
			minPs = -glm::pi<float>() + step;
			maxPs = glm::pi<float>();
			isClosed = true;
		}
		else if ((maxPs - minPs) >= glm::pi<float>() * float(2.f))
		{
			isClosed = true;
		}
		else
		{
			isClosed = false;
		}
		int n_vert = (int)((maxPs - minPs) / step) + 1;
		if (n_vert < 2) n_vert = 2;
		float step_v = (maxPs - minPs) / float(n_vert - 1);
		for (int i = 0; i < n_hor; i++)
		{
			float th = minTh + float(i) * step_h;
			float sth = radius * glm::sin(th);
			float cth = radius * glm::cos(th);
			for (int j = 0; j < n_vert; j++)
			{
				float psi = minPs + float(j) * step_v;
				float sps = glm::sin(psi);
				float cps = glm::cos(psi);
				pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
				if (i)
				{
					drawLine(pvA[j], pvB[j], color);
				}
				else if (drawS)
				{
					drawLine(spole, pvB[j], color);
				}
				if (j)
				{
					drawLine(pvB[j - 1], pvB[j], color);
				}
				else
				{
					arcStart = pvB[j];
				}
				if ((i == (n_hor - 1)) && drawN)
				{
					drawLine(npole, pvB[j], color);
				}

				if (drawCenter)
				{
					if (isClosed)
					{
						if (j == (n_vert - 1))
						{
							drawLine(arcStart, pvB[j], color);
						}
					}
					else
					{
						if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1))))
						{
							drawLine(center, pvB[j], color);
						}
					}
				}
			}
			pT = pvA; pvA = pvB; pvB = pT;
		}
	}

	void _initGL()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->m_glVAO);
		glGenBuffers(1, &this->m_glVBO);

		glBindVertexArray(this->m_glVAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)0);
		// Vertex Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (GLvoid*)offsetof(DebugVertex, col));

		glBindVertexArray(0);
	}
	
	bool _createShader()
	{
		std::string vBuffer, fBuffer, gBuffer;

		vBuffer.append("#version 410\n");
		vBuffer.append("layout(location = 0) in vec3 v3Position;\n");
		vBuffer.append("layout(location = 1) in vec3 v3ColorIn;\n");
		vBuffer.append("uniform mat4 matVP;\n");
		vBuffer.append("out vec4 v4Color;\n");
		vBuffer.append("void main()\n");
		vBuffer.append("{\n");
		vBuffer.append("	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n");
		vBuffer.append("	gl_Position = matVP * vec4(v3Position, 1.0);\n");
		vBuffer.append("}\n");

		fBuffer.append("#version 410\n");
		fBuffer.append("in vec4 v4Color;\n");
		fBuffer.append("out vec4 outputColor;\n");
		fBuffer.append("void main()\n");
		fBuffer.append("{\n");
		fBuffer.append("   outputColor = v4Color;\n");
		fBuffer.append("}\n");

		m_pShader = new Shader(vBuffer.c_str(), fBuffer.c_str());

		m_glViewProjectionMatrixLocation = glGetUniformLocation(m_pShader->m_nProgram, "matVP");
		if (m_glViewProjectionMatrixLocation == -1)
		{
			printf("Unable to find view projection matrix uniform in debug shader\n");
			return false;
		}

		return m_pShader->m_nProgram != 0;
	}

	void _refreshVBO()
	{
		if (!m_bNeedsRefresh)
			return;

		glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
		glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * sizeof(DebugVertex), m_vVertices.data(), GL_STREAM_DRAW);

		m_bNeedsRefresh = false;
	}

// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	DebugDrawer(DebugDrawer const&) = delete;
	void operator=(DebugDrawer const&) = delete;
};
