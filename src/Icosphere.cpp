#include "Icosphere.h"
#include <list>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Icosphere::Icosphere(int recursionLevel, glm::vec3 diffuseColor, glm::vec3 specularColor)
	: m_iIndex(0)
	, m_vec3DiffColor(diffuseColor)
	, m_vec3SpecColor(specularColor)
	, m_vec3EmisColor(0.f)
{
	recalculate(recursionLevel);
	initGL();
}

Icosphere::~Icosphere(void)
{
	m_vvec3Vertices.clear();
	m_vuiIndices.clear();
	m_mapMiddlePointIndexCache.clear();
}

void Icosphere::recalculate(int recursionLevel)
{
	m_vvec3Vertices.clear();
	m_vuiIndices.clear();
	m_mapMiddlePointIndexCache.clear();
	m_iIndex = 0;

	// create 12 vertices of a icosahedron
	float t = (1.f + sqrt(5.f)) / 2.f;

	addVertex(glm::vec3(-1.f, t, 0.f));
	addVertex(glm::vec3(1.f, t, 0.f));
	addVertex(glm::vec3(-1.f, -t, 0.f));
	addVertex(glm::vec3(1.f, -t, 0.f));

	addVertex(glm::vec3(0.f, -1.f, t));
	addVertex(glm::vec3(0.f, 1.f, t));
	addVertex(glm::vec3(0.f, -1.f, -t));
	addVertex(glm::vec3(0.f, 1.f, -t));

	addVertex(glm::vec3(t, 0.f, -1.f));
	addVertex(glm::vec3(t, 0.f, 1.f));
	addVertex(glm::vec3(-t, 0.f, -1.f));
	addVertex(glm::vec3(-t, 0.f, 1.f));


	// create 20 triangles of the icosahedron
	std::list<TriangleIndices> faces;

	// 5 faces around point 0
	faces.push_back(TriangleIndices(0, 11, 5));
	faces.push_back(TriangleIndices(0, 5, 1));
	faces.push_back(TriangleIndices(0, 1, 7));
	faces.push_back(TriangleIndices(0, 7, 10));
	faces.push_back(TriangleIndices(0, 10, 11));

	// 5 adjacent faces 
	faces.push_back(TriangleIndices(1, 5, 9));
	faces.push_back(TriangleIndices(5, 11, 4));
	faces.push_back(TriangleIndices(11, 10, 2));
	faces.push_back(TriangleIndices(10, 7, 6));
	faces.push_back(TriangleIndices(7, 1, 8));

	// 5 faces around point 3
	faces.push_back(TriangleIndices(3, 9, 4));
	faces.push_back(TriangleIndices(3, 4, 2));
	faces.push_back(TriangleIndices(3, 2, 6));
	faces.push_back(TriangleIndices(3, 6, 8));
	faces.push_back(TriangleIndices(3, 8, 9));

	// 5 adjacent faces 
	faces.push_back(TriangleIndices(4, 9, 5));
	faces.push_back(TriangleIndices(2, 4, 11));
	faces.push_back(TriangleIndices(6, 2, 10));
	faces.push_back(TriangleIndices(8, 6, 7));
	faces.push_back(TriangleIndices(9, 8, 1));


	// refine triangles
	for (int i = 0; i < recursionLevel; i++)
	{
		std::list<TriangleIndices> faces2;
		for (auto &tri : faces)
		{
			// replace triangle by 4 triangles
			int a = getMiddlePoint(tri.v1, tri.v2);
			int b = getMiddlePoint(tri.v2, tri.v3);
			int c = getMiddlePoint(tri.v3, tri.v1);

			faces2.push_back(TriangleIndices(tri.v1, a, c));
			faces2.push_back(TriangleIndices(tri.v2, b, a));
			faces2.push_back(TriangleIndices(tri.v3, c, b));
			faces2.push_back(TriangleIndices(a, b, c));
		}
		faces = faces2;
	}

	// done, now add triangles to mesh
	for (auto &tri : faces)
	{
		this->m_vuiIndices.push_back(tri.v1);
		this->m_vuiIndices.push_back(tri.v2);
		this->m_vuiIndices.push_back(tri.v3);
	}
}

std::vector<glm::vec3> Icosphere::getVertices(void) { return m_vvec3Vertices; }

std::vector<unsigned int> Icosphere::getIndices(void) { return m_vuiIndices; }

std::vector<glm::vec3> Icosphere::getUnindexedVertices(void)
{
	std::vector<glm::vec3> flatVerts;

	for (size_t i = 0; i < m_vuiIndices.size(); ++i)
	{
		flatVerts.push_back(m_vvec3Vertices[m_vuiIndices[i]]);
	}

	return flatVerts;
}

// add vertex to mesh, fix position to be on unit sphere, return index
int Icosphere::addVertex(glm::vec3 p)
{
	m_vvec3Vertices.push_back(glm::normalize(p));
	return m_iIndex++;
}

// return index of point in the middle of p1 and p2
int Icosphere::getMiddlePoint(int p1, int p2)
{
    // first check if we have it already
    bool firstIsSmaller = p1 < p2;
    int64_t smallerIndex = firstIsSmaller ? p1 : p2;
    int64_t greaterIndex = firstIsSmaller ? p2 : p1;
    int64_t key = (smallerIndex << 32) + greaterIndex;

	// look to see if middle point already computed
    if (this->m_mapMiddlePointIndexCache.find(key) != this->m_mapMiddlePointIndexCache.end())
    	return this->m_mapMiddlePointIndexCache[key];

    // not in cache, calculate it
	glm::vec3 point1 = this->m_vvec3Vertices[p1];
	glm::vec3 point2 = this->m_vvec3Vertices[p2];
	glm::vec3 middle = (point1 + point2) / 2.f;

    // add vertex makes sure point is on unit sphere
    int i = addVertex(middle); 

    // store it, return index
    this->m_mapMiddlePointIndexCache[key] = i;
    return i;
}

void Icosphere::initGL()
{
	// Create buffers/arrays
	if (!this->m_glVAO) glGenVertexArrays(1, &this->m_glVAO);
	if (!this->m_glVBO) glGenBuffers(1, &this->m_glVBO);
	if (!this->m_glEBO) glGenBuffers(1, &this->m_glEBO);

	std::vector<Vertex> buffer;

	for (auto const &v : m_vvec3Vertices)
	{
		Vertex temp;
		temp.pos = v;
		temp.norm = v;
		buffer.push_back(temp);
	}

	glBindVertexArray(this->m_glVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(Vertex), &buffer[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vuiIndices.size() * sizeof(GLuint), &m_vuiIndices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, norm));

	glBindVertexArray(0);
}

void Icosphere::draw(Shader s)
{
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.diffuse"), m_vec3DiffColor.r, m_vec3DiffColor.g, m_vec3DiffColor.b);
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.specular"), m_vec3SpecColor.r, m_vec3SpecColor.g, m_vec3SpecColor.b);
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.emissive"), m_vec3EmisColor.r, m_vec3EmisColor.g, m_vec3EmisColor.b);
	glUniform1f(glGetUniformLocation(s.m_nProgram, "material.shininess"), 32.0f);

	glm::mat4 model = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_mat3Rotation) * glm::scale(glm::mat4(), m_vec3Scale);

	glUniformMatrix4fv(glGetUniformLocation(s.m_nProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	// Draw mesh
	glBindVertexArray(this->m_glVAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_vuiIndices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
