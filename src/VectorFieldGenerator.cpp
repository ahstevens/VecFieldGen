#include "VectorFieldGenerator.h"

#include <iostream>
#include <random>


VectorFieldGenerator::VectorFieldGenerator()
{
}

VectorFieldGenerator::~VectorFieldGenerator()
{
}

void VectorFieldGenerator::init(unsigned int nControlPoints)
{	
	std::mt19937 rng; // Mersenne Twister
	rng.seed(std::random_device()());

	std::uniform_real_distribution<float> dist(-1.f, 1.f);

	for (unsigned int i = 0u; i < nControlPoints; ++i)
	{
		m_vvec3ControlPoints.push_back(glm::vec3(dist(rng), dist(rng), dist(rng)));
	}
}

std::vector<glm::vec3> VectorFieldGenerator::getControlPoints()
{
	return m_vvec3ControlPoints;
}
