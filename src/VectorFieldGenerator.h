#pragma once

#include <vector>
#include <random>

#include <glm/glm.hpp>

#include "Icosphere.h"

class VectorFieldGenerator
{
public:	
	VectorFieldGenerator();
	~VectorFieldGenerator();

	void init(unsigned int nControlPoints);
	
	void draw(const Shader &s);

private:
	struct ControlPoint {
		glm::vec3 pos;
		glm::vec3 dir;
	};

private:	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribuion;

	std::vector<ControlPoint> m_vControlPoints;

	Icosphere *m_pSphere;
};

