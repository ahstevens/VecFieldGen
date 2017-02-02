#pragma once

#include <vector>

#include <glm/glm.hpp>

class VectorFieldGenerator
{
public:	
	VectorFieldGenerator();
	~VectorFieldGenerator();

	void init(unsigned int nControlPoints);

	std::vector<glm::vec3> getControlPoints();

private:	
	std::vector<glm::vec3> m_vvec3ControlPoints;
};

