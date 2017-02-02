#include "VectorFieldGenerator.h"

#include <iostream>

#include "DebugDrawer.h"

#include <alglib/ap.h>
#include <alglib/interpolation.h>

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
	if (m_pSphere)
		delete m_pSphere;
}

void VectorFieldGenerator::init(unsigned int nControlPoints)
{	
	if (m_vControlPoints.size() > 0u)
	{
		m_vControlPoints.clear();
		DebugDrawer::getInstance().flushLines();
	}

	DebugDrawer::getInstance().drawBox(glm::vec3(-1.f), glm::vec3(1.f), glm::vec3(1.f));

	double cpMat[6][6];

	for (unsigned int i = 0u; i < nControlPoints; ++i)
	{
		ControlPoint cp;
		cp.pos = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));
		cp.dir = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));

		DebugDrawer::getInstance().drawLine(cp.pos, cp.pos + cp.dir, glm::vec3(0.f));

		cpMat[i][0] = static_cast<double>(cp.pos.x);
		cpMat[i][1] = static_cast<double>(cp.pos.y);
		cpMat[i][2] = static_cast<double>(cp.pos.z);
		cpMat[i][3] = static_cast<double>(cp.dir.x);
		cpMat[i][4] = static_cast<double>(cp.dir.y);
		cpMat[i][5] = static_cast<double>(cp.dir.z);

		m_vControlPoints.push_back(cp);
	}

	alglib::rbfmodel model;
	alglib::rbfreport rep;

	alglib::rbfcreate(3, 3, model);
	alglib::real_2d_array xy0;
	xy0.setcontent(6, 6, *cpMat);
	
	alglib::rbfbuildmodel(model, rep);

	double coords[8 * 3] = {
		-1., -1., -1.,
		-1., -1., 1.,
		-1., 1., -1.,
		1., -1., -1.,
		1., 1., 1.,
		-1., 1., 1.,
		1., -1., 1.,
		1., 1., -1.
	};
	alglib::real_1d_array samplePoints;
	samplePoints.setcontent(8 * 3, coords);
	alglib::real_1d_array outVecs;

	alglib::rbfcalc(model, samplePoints, outVecs);
	
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
