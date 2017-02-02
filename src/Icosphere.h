#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Object.h"

class Icosphere : public Object
{
public:	
	Icosphere(int recursionLevel, glm::vec3 diffuseColor = glm::vec3(1.f), glm::vec3 specularColor = glm::vec3(1.f));
	~Icosphere(void);

	void Icosphere::recalculate(int recursionLevel);

	std::vector<glm::vec3> getVertices(void);
	std::vector<unsigned int> getIndices(void);

	std::vector<glm::vec3> getUnindexedVertices(void);

private:	
    struct TriangleIndices
    {
        int v1;
        int v2;
        int v3;

        TriangleIndices(int v1, int v2, int v3)
        {
            this->v1 = v1;
            this->v2 = v2;
            this->v3 = v3;
        }
    };

	int addVertex(glm::vec3 p);
	int getMiddlePoint(int p1, int p2);

	std::vector<glm::vec3> m_vvec3Vertices;
	std::vector<unsigned int> m_vuiIndices;
	
    int m_iIndex;
    std::unordered_map<int64_t, int> m_mapMiddlePointIndexCache;


public:
	glm::vec3 m_vec3DiffColor, m_vec3SpecColor, m_vec3EmisColor;
	void initGL();
	void draw(Shader s);

private:
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
	};

	GLuint m_glVAO, m_glVBO, m_glEBO;
};

