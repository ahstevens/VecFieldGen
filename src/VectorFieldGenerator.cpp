#include "VectorFieldGenerator.h"

#include <iostream>

#include "DebugDrawer.h"

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

	m_matControlPointKernel = Eigen::MatrixXf(nControlPoints, nControlPoints);
	m_vCPXVals = Eigen::VectorXf(nControlPoints);
	m_vCPYVals = Eigen::VectorXf(nControlPoints);
	m_vCPZVals = Eigen::VectorXf(nControlPoints);
		
	for (unsigned int i = 0u; i < nControlPoints; ++i)
	{
		// Create control point
		ControlPoint cp;
		cp.pos = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));
		cp.dir = glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG));

		m_vControlPoints.push_back(cp);

		// Draw debug line for control point
		DebugDrawer::getInstance().drawLine(cp.pos, cp.pos + cp.dir, glm::vec3(0.f));
		
		m_vCPXVals(i) = cp.dir.x;
		m_vCPYVals(i) = cp.dir.y;
		m_vCPZVals(i) = cp.dir.z;

		m_matControlPointKernel(i,i) = 1.f;
		for (int j = static_cast<int>(i) - 1; j >= 0; --j)
		{
			float r = glm::length(cp.pos - m_vControlPoints[j].pos);
			float gaussian = exp(-(1.2 * 1.2 * r * r));
			m_matControlPointKernel(i, j) = m_matControlPointKernel(j, i) = gaussian;
		}
	}

	std::cout << m_matControlPointKernel << std::endl;
	
	Eigen::MatrixXf invMat = m_matControlPointKernel.inverse();

	m_vLambdaX = invMat * m_vCPXVals;
	m_vLambdaY = invMat * m_vCPYVals;
	m_vLambdaZ = invMat * m_vCPZVals;

	interpolate(gridResolution, 1.2f);
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
				for (int m = 0; m < m_vControlPoints.size(); ++m)
				{
					float r = glm::length(point - m_vControlPoints[m].pos);
					float gaussian = exp(-(gaussianShape * gaussianShape * r * r));
					float tmpX = m_vLambdaX[m] * gaussian;
					float tmpY = m_vLambdaY[m] * gaussian;
					float tmpZ = m_vLambdaZ[m] * gaussian;

					outVec.x += m_vControlPoints[m].dir.x * tmpX;
					outVec.y += m_vControlPoints[m].dir.y * tmpY;
					outVec.z += m_vControlPoints[m].dir.z * tmpZ;
				}

				DebugDrawer::getInstance().drawLine(point, point + outVec, (outVec + 1.f) / 2.f);

				row.push_back(std::pair<glm::vec3, glm::vec3>(point, outVec));
			}
			frame.push_back(row);
		}
		m_v3DGridPairs.push_back(frame);
	}

	return true;
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
			m_pSphere->m_vec3DiffColor = (cp.dir + 1.f) / 2.f;

			m_pSphere->draw(s);
		}
	}

	// DRAW VECTOR FIELD
	//{
	//	m_pSphere->setScale(0.005f);

	//	for (auto const &frame : m_v3DGridPairs)
	//	{
	//		for (auto const &row : frame)
	//		{
	//			for (auto const &gridPair : row)
	//			{
	//				m_pSphere->setPosition(gridPair.first);
	//				m_pSphere->m_vec3DiffColor = (gridPair.second + 1.f) / 2.f;

	//				m_pSphere->draw(s);

	//				//m_pSphere->setPosition(gridPair.first + 0.1f * gridPair.second);
	//				//m_pSphere->m_vec3DiffColor = (gridPair.second + 1.f) / 2.f;

	//				//m_pSphere->draw(s);
	//			}
	//		}
	//	}
	//}
}
