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
}

VectorFieldGenerator::~VectorFieldGenerator()
{
	if (m_pSphere)
		delete m_pSphere;
}

void VectorFieldGenerator::init(unsigned int nControlPoints, unsigned int gridResolution)
{	
	if (m_vControlPoints.size() > 0u)
	{
		m_vControlPoints.clear();
		m_v3DGridPairs.clear();
		DebugDrawer::getInstance().flushLines();
	}

	DebugDrawer::getInstance().drawBox(glm::vec3(-1.f), glm::vec3(1.f), glm::vec3(1.f));
	DebugDrawer::getInstance().drawTransform(1.f);
		
	for (unsigned int i = 0u; i < nControlPoints; ++i)
	{
		ControlPoint cp;
		cp.pos = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));
		cp.dir = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));

		DebugDrawer::getInstance().drawLine(cp.pos, cp.pos + cp.dir, glm::vec3(0.f));
		
		m_vControlPoints.push_back(cp);
	}

	//interpolateAlglib(gridResolution);
	interpolate(gridResolution, 1.2f);
}

void VectorFieldGenerator::draw(const Shader & s)
{
	DebugDrawer::getInstance().setTransformDefault();

	// DRAW CONTROL POINTS
	{
		m_pSphere->setScale(0.025f);

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

	// DRAW VECTOR FIELD
	{
		m_pSphere->setScale(0.005f);

		for (auto const &frame : m_v3DGridPairs)
		{
			for (auto const &row : frame)
			{
				for (auto const &gridPair : row)
				{
					m_pSphere->setPosition(gridPair.first);
					m_pSphere->m_vec3DiffColor = (gridPair.second + 1.f) / 2.f;

					m_pSphere->draw(s);

					//m_pSphere->setPosition(gridPair.first + 0.1f * gridPair.second);
					//m_pSphere->m_vec3DiffColor = (gridPair.second + 1.f) / 2.f;

					//m_pSphere->draw(s);

					DebugDrawer::getInstance().drawLine(gridPair.first, gridPair.first + gridPair.second, (gridPair.second + 1.f) / 2.f);
				}
			}
		}
	}
}

bool VectorFieldGenerator::interpolateAlglib(int resolution)
{
	std::vector<std::vector<double>> cpMat;

	for (auto const &cp : m_vControlPoints)
	{
		std::vector<double> cpInfo;
		cpInfo.push_back(static_cast<double>(cp.pos.x));
		cpInfo.push_back(static_cast<double>(cp.pos.y));
		cpInfo.push_back(static_cast<double>(cp.pos.z));
		cpInfo.push_back(static_cast<double>(cp.dir.x));
		cpInfo.push_back(static_cast<double>(cp.dir.y));
		cpInfo.push_back(static_cast<double>(cp.dir.z));

		cpMat.push_back(cpInfo);
	}

	alglib::rbfmodel model;
	alglib::rbfcreate(3, 3, model); // 3-dimensional space with 3-component vectors as output
	alglib::real_2d_array xy0;
	xy0.setcontent(m_vControlPoints.size(), 6, &(cpMat[0][0]));

	alglib::rbfsetpoints(model, xy0);

	alglib::rbfsetzeroterm(model);

	alglib::rbfreport rep;
	try
	{
		alglib::rbfbuildmodel(model, rep);
	}
	catch (alglib::ap_error e)
	{
		printf("error msg: %s\n", e.msg.c_str());
		return false;
	}

	m_v3DGridPairs.clear();

	for (unsigned int i = 0; i < resolution; ++i)
	{
		std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>> frame;
		for (unsigned int j = 0; j < resolution; ++j)
		{
			std::vector<std::pair<glm::vec3, glm::vec3>> row;
			for (unsigned int k = 0; k < resolution; ++k)
			{
				glm::vec3 point;
				point.x = -1.f + k * (2.f / (resolution - 1));
				point.y = -1.f + j * (2.f / (resolution - 1));
				point.z = -1.f + i * (2.f / (resolution - 1));

				alglib::real_1d_array x;
				double tmp[3];
				tmp[0] = static_cast<double>(point.x);
				tmp[1] = static_cast<double>(point.y);
				tmp[2] = static_cast<double>(point.z);
				x.setcontent(3, tmp);
				alglib::real_1d_array y;

				alglib::rbfcalc(model, x, y);

				glm::vec3 outVec;
				outVec.x = static_cast<float>(y.getcontent()[0]);
				outVec.y = static_cast<float>(y.getcontent()[1]);
				outVec.z = static_cast<float>(y.getcontent()[2]);

				row.push_back(std::pair<glm::vec3, glm::vec3>(point, outVec));
			}
			frame.push_back(row);
		}
		m_v3DGridPairs.push_back(frame);
	}

	return true;
}

bool VectorFieldGenerator::interpolate(int resolution, float gaussianShape)
{
	m_v3DGridPairs.clear();

	for (unsigned int i = 0; i < resolution; ++i)
	{
		std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>> frame;
		for (unsigned int j = 0; j < resolution; ++j)
		{
			std::vector<std::pair<glm::vec3, glm::vec3>> row;
			for (unsigned int k = 0; k < resolution; ++k)
			{
				glm::vec3 point;
				point.x = -1.f + k * (2.f / (resolution - 1));
				point.y = -1.f + j * (2.f / (resolution - 1));
				point.z = -1.f + i * (2.f / (resolution - 1));

				glm::vec3 outVec(0.f);
				for (auto const &cp : m_vControlPoints)
				{
					float r = glm::length(cp.pos - point);
					float tmp = exp(-(gaussianShape * r * r));

					outVec += cp.dir * tmp;
				}

				row.push_back(std::pair<glm::vec3, glm::vec3>(point, outVec));
			}
			frame.push_back(row);
		}
		m_v3DGridPairs.push_back(frame);
	}

	return true;
}
