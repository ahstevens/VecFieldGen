#pragma once

#include <vector>
#include <random>

#include <glm/glm.hpp>

#include <Eigen/Dense>

#include "Icosphere.h"

class VectorFieldGenerator
{
public:	
	VectorFieldGenerator();
	~VectorFieldGenerator();

	void init(unsigned int nControlPoints, unsigned int gridResolution);
	
	void draw(const Shader &s);

	void save();

private:
	struct ControlPoint {
		glm::vec3 pos;
		glm::vec3 dir;
	};

private:	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribuion;

	std::vector<ControlPoint> m_vControlPoints;

	float m_fGaussianShape;
	Eigen::MatrixXf m_matControlPointKernel;
	Eigen::VectorXf m_vCPXVals, m_vCPYVals, m_vCPZVals;
	Eigen::VectorXf m_vLambdaX, m_vLambdaY, m_vLambdaZ;
	std::vector<std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>>> m_v3DGridPairs;
	Icosphere *m_pSphere;

private:
	void createControlPoints(unsigned int nControlPoints);
	void makeGrid(unsigned int resolution, float gaussianShape = 1.f);
	glm::vec3 interpolate(glm::vec3 pt);
	void advectParticles(std::vector<glm::vec3> seedPoints, float dt, float totalTime);
	bool checkSphereAdvection(float dt, float totalTime, glm::vec3 sphereCenter, float sphereRadius, float &timeToAdvect, float &distanceToAdvect, float &totalDistance);
	float gaussianBasis(float r, float eta);
};

