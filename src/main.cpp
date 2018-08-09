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

bool processArgs(int argc, char * argv[]);
void printUsage();
bool generateField();

int main(int argc, char * argv[]) 
{
	if (processArgs(argc, argv))
	{
		m_pVFG = new VectorFieldGenerator();

		if (generateField())
			m_pVFG->save(m_strSavePath, !m_bSilent);

		delete m_pVFG;
	}

    return EXIT_SUCCESS;
}

bool processArgs(int argc, char * argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);
		m_vstrArgs.push_back(arg);

		if (arg.compare("--help") == 0 || arg.compare("-h") == 0)
		{
			printUsage();
			return false;
		}

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
	}

	return true;
}

void printUsage()
{
	std::cout << std::endl;
	std::cout << "VecFieldGen is a utility to generate flow fields using control points and a Gaussian Radial basis Function (RBF). The steady-state flow field is generated in R^3 and bounded within [-1,1] before being rediscretized for output as a flow grid at the specified grid resolution." << std::endl;
	std::cout << "usage: VecFieldGen [options]" << std::endl;
	std::cout << "\t" << "options:" << std::endl;
	std::cout << "\t" << "\t" << "--help, -h" << "\t\t" << "Displays this help text" << std::endl;
	std::cout << "\t" << "\t" << "--silent" << "\t\t" << "Do not print status to stdout" << std::endl;
	std::cout << "\t" << "\t" << "--checksphere" << "\t\t" << "Perform a particle advection test through through a sphere of a given radius over a specified time interval. Default: disabled" << std::endl;
	std::cout << "\t" << "\t" << "--onlyadvects" << "\t\t" << "Flowgrid produced must successfully advect a particle through the sphere. This flag should only be used in conjunction with random control points. Default: disabled" << std::endl;
	std::cout << "\t" << "\t" << "-radius <float>" << "\t\t" << "Set the radius of the particle advection test sphere. Default: 0.5" << std::endl;
	std::cout << "\t" << "\t" << "-duration <float>" << "\t" << "Set the time duration for the particle advection test. Default: 10.0" << std::endl;
	std::cout << "\t" << "\t" << "-dt <float>" << "\t\t" << "Set the time step for the forward Euler particle advection test. Default: 0.01111111" << std::endl;
	std::cout << "\t" << "\t" << "-cpcount <int>" << "\t\t" << "Number of random control points to generate. Default: 6" << std::endl;
	std::cout << "\t" << "\t" << "-cpfile <file>" << "\t\t" << "Path to file containing custom control points. Disables random control point generation. Default: disabled. File format (one per line): posX,posY,posZ,dirX,dirY,dirZ" << std::endl;
	std::cout << "\t" << "\t" << "-grid <int>" << "\t\t" << "Set the 3D flow grid resolution. Default: 32" << std::endl;
	std::cout << "\t" << "\t" << "-gaussian <float>" << "\t" << "Set the Gaussian shape for the radial basis function. Default: 1.2" << std::endl;
	std::cout << "\t" << "\t" << "-outfile <file>" << "\t\t" << "Path and name of the output file. Default: flowgrid.fg" << std::endl;
	std::cout << std::endl;
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
				if (!m_bSilent) printf("Cannot parse control point: %s\n", line.c_str());
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