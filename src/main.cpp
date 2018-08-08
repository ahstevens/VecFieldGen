#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "VectorFieldGenerator.h"

std::vector<std::string> m_vstrArgs;

VectorFieldGenerator* m_pVFG = NULL;

bool m_bCustomCPs = false;
bool m_bCheckSphereAdvection = false;
bool m_bSphereAdvectorsOnly = false;
bool m_bSilent = false;

int m_nRandomCPs = 6u;
int m_nGridResolution = 32u;

float m_fGaussianShape = 1.2f;
float m_fDeltaT = 1.f / 90.f;
float m_fAdvectionTime = 10.f;
float m_fSphereRadius = 0.5f;
//float m_fSphereRadius = 0.1f * sqrt(3); // radius from Forsberg paper

std::string m_strSavePath("flowgrid.fg");
std::string m_strCPPath;

bool generateField();

int main(int argc, char * argv[]) 
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);

		if (arg.compare("--silent") == 0)
			m_bSilent = true;

		if (arg.compare("--checksphere") == 0)
		{
			m_bCheckSphereAdvection = true;
		}

		if (arg.compare("--onlyadvects") == 0)
		{
			m_bSphereAdvectorsOnly = true;
			m_bCheckSphereAdvection = true;
		}

		if (arg.compare("-outfile") == 0)
			m_strSavePath = std::string(argv[i + 1]);

		if (arg.compare("-cpfile") == 0)
		{
			m_bCustomCPs = true;
			m_strCPPath = std::string(argv[i + 1]);
		}

		if (arg.compare("-cpcount") == 0)
		{
			m_nRandomCPs = std::stoi(std::string(argv[i + 1]));
		}

		if (arg.compare("-grid") == 0)
		{
			m_nGridResolution = std::stoi(std::string(argv[i + 1]));
		}

		if (arg.compare("-gaussian") == 0)
		{
			m_fGaussianShape = std::stof(std::string(argv[i + 1]));
		}

		if (arg.compare("-radius") == 0)
		{
			m_bCheckSphereAdvection = true;
			m_fSphereRadius = std::stof(std::string(argv[i + 1]));
		}

		if (arg.compare("-duration") == 0)
		{
			m_bCheckSphereAdvection = true;
			m_fAdvectionTime = std::stof(std::string(argv[i + 1]));
		}

		if (arg.compare("-dt") == 0)
		{
			m_bCheckSphereAdvection = true;
			m_fDeltaT = std::stof(std::string(argv[i + 1]));
		}

		m_vstrArgs.push_back(arg);
	}

	if (generateField())
		m_pVFG->save(m_strSavePath, !m_bSilent);

	delete m_pVFG;

    return EXIT_SUCCESS;
}

std::vector<std::string> split(std::string target, std::string delim)
{
	std::vector<std::string> v;
	if (!target.empty()) {
		std::string::size_type start = 0;
		do {
			size_t x = target.find(delim, start);
			if (x == std::string::npos)
				break;

			v.push_back(target.substr(start, x - start));
			start += delim.size();
		} while (true);

		v.push_back(target.substr(start));
	}
	return v;
}

bool loadCustomCPs()
{
	using namespace std::experimental::filesystem::v1;

	std::ifstream cpFile;

	if (!m_bSilent) printf("opening: %s\n", m_strCPPath.c_str());

	cpFile.open(m_strCPPath);

	int cpCount = 0;

	if (cpFile.is_open())
	{
		std::string line;
		std::vector<float> cpvals;

		while (std::getline(cpFile, line))
		{
			std::vector<float> vals;

			std::stringstream ss(line);

			float tmpVal;

			while (ss >> tmpVal)
			{
				vals.push_back(tmpVal);
				if (ss.peek() == ',')
					ss.ignore();
			}

			if (vals.size() != 6)
			{
				if (!m_bSilent) printf("Cannot parse control point: %s\n", line);
				continue;
			}

			glm::vec3 pos(vals[0], vals[1], vals[2]);
			glm::vec3 dir(vals[3], vals[4], vals[5]);

			m_pVFG->setControlPoint(pos, dir);
			cpCount++;
		}
	}
	else
	{
		if (!m_bSilent) printf("Unable to open control point input file %s\n", m_strCPPath.c_str());
		return false;
	}
	
	cpFile.close();
	
	if (!m_bSilent) printf("Ingested %i control points from %s\n", cpCount, m_strCPPath.c_str());

	if (cpCount == 0)
		return false;

	return true;
}

bool generateField()
{
	m_pVFG = new VectorFieldGenerator();

	m_pVFG->setGridResolution(m_nGridResolution);
	m_pVFG->setGaussianShape(m_fGaussianShape);

	if (m_strCPPath.empty())
		m_pVFG->createRandomControlPoints(m_nRandomCPs);
	else
		if (!loadCustomCPs())
		{
			if (!m_bSilent) std::cout << "ERROR: Could not load custom control points from file " << m_strCPPath << std::endl;
			return false;
		}
	
	m_pVFG->generate();

	if (m_bCheckSphereAdvection)
	{
		float t, d, td;
		glm::vec3 exitPt;
		bool advected = m_pVFG->checkSphereAdvection(m_fDeltaT, m_fAdvectionTime, glm::vec3(0.f), m_fSphereRadius, t, d, td, exitPt);

		while (!m_bCustomCPs && m_bSphereAdvectorsOnly && !advected)
		{
			if (!m_bSilent) std::cout << "Regenerating vector field because particle failed to advect through sphere (r = " << m_fSphereRadius << ") in " << m_fAdvectionTime << " seconds" << std::endl;
			m_pVFG->createRandomControlPoints(m_nRandomCPs);
			m_pVFG->generate();
			advected = m_pVFG->checkSphereAdvection(m_fDeltaT, m_fAdvectionTime, glm::vec3(0.f), m_fSphereRadius, t, d, td, exitPt);
		}

		if (!m_bSilent)
		{
			if (advected)
			{
				std::cout << "Particle successfully advected in " << t << " seconds (" << t / m_fDeltaT << " time steps)" << std::endl;
				std::cout << '\t' << "Particle traveled " << d << " units until advecting through sphere (r = " << m_fSphereRadius << ")" << std::endl;
				std::cout << '\t' << "Particle traveled " << td << " total units in " << m_fAdvectionTime << " seconds" << std::endl << std::endl;
			}
			else
			{
				std::cout << "Particled failed to advect!" << std::endl;
				std::cout << '\t' << "Particle traveled " << td << " total units in " << m_fAdvectionTime << " seconds without advecting through sphere (r = " << m_fSphereRadius << ")" << std::endl << std::endl;
			}
		}
	}

	return true;
}