#include "VectorFieldGenerator.h"

#include <iostream>

#include "DebugDrawer.h"

VectorFieldGenerator::VectorFieldGenerator()
	: m_pSphere(NULL)
{
	m_RNG.seed(std::random_device()());

	m_Distribuion = std::uniform_real_distribution<float>(-1.f, 1.f);

	m_pSphere = new Icosphere(4);
	m_pSphere->setScale(0.025f);
}

VectorFieldGenerator::~VectorFieldGenerator()
{
}

void VectorFieldGenerator::init(unsigned int nControlPoints)
{	
	if (m_vControlPoints.size() > 0u)
	{
		m_vControlPoints.clear();
		DebugDrawer::getInstance().flushLines();
	}

	for (unsigned int i = 0u; i < nControlPoints; ++i)
	{
		ControlPoint cp;
		cp.pos = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));
		cp.dir = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));

		DebugDrawer::getInstance().drawLine(cp.pos, cp.pos + cp.dir, glm::vec3(0.f));

		m_vControlPoints.push_back(cp);
	}

	DebugDrawer::getInstance().drawBox(glm::vec3(-1.f), glm::vec3(1.f), glm::vec3(1.f));
}

void VectorFieldGenerator::draw(const Shader & s)
{
	DebugDrawer::getInstance().setTransformDefault();

	for (auto const &cp : m_vControlPoints)
	{
		m_pSphere->setPosition(cp.pos);
		m_pSphere->m_vec3DiffColor = glm::vec3(1.f);

		m_pSphere->draw(s);

		m_pSphere->setPosition(cp.pos + cp.dir);
		m_pSphere->m_vec3DiffColor = glm::vec3(0.f);

		m_pSphere->draw(s);
	}
}
