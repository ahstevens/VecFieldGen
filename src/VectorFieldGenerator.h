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

	void init(unsigned int nControlPoints, unsigned int gridResolution);
	
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

	std::vector<std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>>> m_v3DGridPairs;
	Icosphere *m_pSphere;

private:
	bool interpolateAlglib(int resolution);
	bool interpolate(int resolution, float gaussianShape = 1.f);
};

