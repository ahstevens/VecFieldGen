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

	float dt = 1.f / 90.f;
	float time = 10.f;
	float r = 0.1f * sqrt(3);
	float t, d, td;

	bool advected = checkSphereAdvection(dt, time, r, t, d, td);

	if (advected)
	{
		std::cout << "Particle successfully advected in " << t << " seconds (" << t / dt << " time steps)" << std::endl;
		std::cout << '\t' << "Particle traveled " << d << " units until advecting through sphere (r = " << r << ")" << std::endl;
		std::cout << '\t' << "Particle traveled " << td << " total units in " << time << " seconds" << std::endl << std::endl;
	}
	else
	{
		std::cout << "Particled failed to advect!" << std::endl;
		std::cout << '\t' << "Particle traveled " << td << " total units in " << time << " seconds without advecting through sphere (r = " << r << ")" << std::endl << std::endl;
	}
	
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
		DebugDrawer::getInstance().drawLine(cp.pos, cp.pos + cp.dir, (cp.dir + 1.f) / 2.f);
		
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

glm::vec3 lineSphereIntersection(glm::vec3 linePoint0, glm::vec3 linePoint1, glm::vec3 circleCenter, double circleRadius)
{
	// http://www.codeproject.com/Articles/19799/Simple-Ray-Tracing-in-C-Part-II-Triangles-Intersec

	float cx = circleCenter.x;
	float cy = circleCenter.y;
	float cz = circleCenter.z;

	float px = linePoint0.x;
	float py = linePoint0.y;
	float pz = linePoint0.z;

	float vx = linePoint1.x - px;
	float vy = linePoint1.y - py;
	float vz = linePoint1.z - pz;

	float A = vx * vx + vy * vy + vz * vz;
	float B = 2.f * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
	float C = px * px - 2.f * px * cx + cx * cx + py * py - 2.f * py * cy + cy * cy +
		pz * pz - 2.f * pz * cz + cz * cz - circleRadius * circleRadius;

	// discriminant
	float D = B * B - 4.f * A * C;

	if (D < 0.f)
		return glm::vec3(0.f);

	float t1 = (-B - sqrtf(D)) / (2.f * A);

	glm::vec3 solution1(linePoint0.x * (1.f - t1) + t1 * linePoint1.x,
		linePoint0.y * (1.f - t1) + t1 * linePoint1.y,
		linePoint0.z * (1.f - t1) + t1 * linePoint1.z);

	if (D == 0.f)
		return solution1;

	float t2 = (-B + sqrtf(D)) / (2.f * A);
	glm::vec3 solution2(linePoint0.x * (1.f - t2) + t2 * linePoint1.x,
		linePoint0.y * (1.f - t2) + t2 * linePoint1.y,
		linePoint0.z * (1.f - t2) + t2 * linePoint1.z);

	// prefer a solution that's on the line segment itself

	if (abs(t1 - 0.5f) < abs(t2 - 0.5f))
		return solution1;

	return solution2;
}

bool VectorFieldGenerator::checkSphereAdvection(
	float dt,
	float totalTime,
	float sphereRadius,
	float &timeToAdvectSphere,
	float &distanceToAdvectSphere,
	float &totalAdvectionDistance
)
{
	bool advected = false;
	timeToAdvectSphere = 0.f;
	float distanceCounter = distanceToAdvectSphere = 0.f;
	glm::vec3 pt(0.f); // start at the center of the field
	for (float i = 0.f; i < totalTime; i += dt)
	{
		// advect point by one timestep to get new point
		glm::vec3 newPt = pt + dt * interpolate(pt);
		
		DebugDrawer::getInstance().drawLine(pt, newPt, glm::normalize(newPt - pt));

		distanceCounter += glm::length(newPt - pt);

		if (glm::length(newPt) >= sphereRadius && !advected)
		{
			advected = true;
			timeToAdvectSphere = i;
			distanceToAdvectSphere = distanceCounter;

			glm::vec3 exitPt = lineSphereIntersection(pt, newPt, glm::vec3(0.f), sphereRadius);

			glm::vec3 x = glm::cross(glm::normalize(exitPt), glm::vec3(0.f, 1.f, 0.f));
			glm::vec3 y = glm::cross(x, glm::normalize(exitPt));

			float crossSize = 0.05f;

			DebugDrawer::getInstance().drawLine(glm::vec3(0.f), exitPt, glm::vec3(1.f, 0.f, 0.f));
			DebugDrawer::getInstance().drawLine(exitPt - crossSize * x, exitPt + crossSize * x, glm::vec3(1.f, 0.f, 0.f));
			DebugDrawer::getInstance().drawLine(exitPt - crossSize * y, exitPt + crossSize * y, glm::vec3(1.f, 0.f, 0.f));

			//break;
		}

		pt = newPt;
	}

	totalAdvectionDistance = distanceCounter;

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
