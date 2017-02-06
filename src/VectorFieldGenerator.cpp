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
	m_v3DGridPairs.clear();

	DebugDrawer::getInstance().flushLines();

	DebugDrawer::getInstance().drawBox(glm::vec3(-1.f), glm::vec3(1.f), glm::vec3(1.f));

	m_fGaussianShape = 1.2f;

	createControlPoints(nControlPoints);

	makeGrid(gridResolution, m_fGaussianShape);

	std::cout << "Advects from center: " << checkCenterAdvection(0.5f) << std::endl;
}

void VectorFieldGenerator::createControlPoints(unsigned int nControlPoints)
{
	if (m_vControlPoints.size() > 0u)	
		m_vControlPoints.clear();

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
		
		// store each component of the control point vector direction
		m_vCPXVals(i) = m_vControlPoints[i].dir.x;
		m_vCPYVals(i) = m_vControlPoints[i].dir.y;
		m_vCPZVals(i) = m_vControlPoints[i].dir.z;

		// fill in the distance matrix kernel entries for this control point
		m_matControlPointKernel(i, i) = 1.f;
		for (int j = static_cast<int>(i) - 1; j >= 0; --j)
		{
			float r = glm::length(m_vControlPoints[i].pos - m_vControlPoints[j].pos);
			float gaussian = gaussianBasis(r, m_fGaussianShape);
			m_matControlPointKernel(i, j) = m_matControlPointKernel(j, i) = gaussian;
		}
	}

	// solve for lambda coefficients in each (linearly independent) dimension using LU decomposition of distance matrix
	//     -the lambda coefficients are the interpolation weights for each control(/interpolation data) point
	m_vLambdaX = m_matControlPointKernel.fullPivLu().solve(m_vCPXVals);
	m_vLambdaY = m_matControlPointKernel.fullPivLu().solve(m_vCPYVals);
	m_vLambdaZ = m_matControlPointKernel.fullPivLu().solve(m_vCPZVals);
}

void VectorFieldGenerator::makeGrid(int resolution, float gaussianShape)
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
				// calculate grid point position
				glm::vec3 point;
				point.x = -1.f + k * (2.f / (resolution - 1));
				point.y = -1.f + j * (2.f / (resolution - 1));
				point.z = -1.f + i * (2.f / (resolution - 1));

				glm::vec3 res = interpolate(point);

				//DebugDrawer::getInstance().drawLine(point, point + res, (res + 1.f) / 2.f);

				row.push_back(std::pair<glm::vec3, glm::vec3>(point, res));
			}
			frame.push_back(row);
		}
		m_v3DGridPairs.push_back(frame);
	}
}

glm::vec3 VectorFieldGenerator::interpolate(glm::vec3 pt)
{
	// find interpolated 3D vector by summing influence from each CP via radial basis function (RBF)
	glm::vec3 outVec(0.f);
	for (int m = 0; m < m_vControlPoints.size(); ++m)
	{
		float r = glm::length(pt - m_vControlPoints[m].pos);
		float gaussian = gaussianBasis(r, m_fGaussianShape);

		// interpolate each separate (linearly independent) component of our 3D vector to get CP influence
		float interpX = m_vLambdaX[m] * gaussian;
		float interpY = m_vLambdaY[m] * gaussian;
		float interpZ = m_vLambdaZ[m] * gaussian;

		// add CP influence to resultant vector
		outVec += glm::vec3(interpX, interpY, interpZ);
	}

	return outVec;
}

bool VectorFieldGenerator::checkCenterAdvection(float sphereRadius)
{
	float stepSize = 0.01f;
	float totalTime = 100.f;
	bool advected = false;
	glm::vec3 pt(0.f);
	for (float i = 0.f; i < totalTime; i += stepSize)
	{
		glm::vec3 newPt = pt + stepSize * interpolate(pt);
		
		DebugDrawer::getInstance().drawLine(pt, newPt, glm::vec3(1.f, 0.f, 0.f));

		pt = newPt;

		if (glm::length(pt) >= sphereRadius)
		{
			advected = true;
			//break;
		}
	}
	return advected;
}

float VectorFieldGenerator::gaussianBasis(float radius, float eta)
{
	return exp(-(eta * radius * radius));
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
