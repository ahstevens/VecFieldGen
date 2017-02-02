#ifndef SHADER_H
#define SHADER_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif // !GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    GLuint m_nProgram;

public:
    // Constructor generates the shader on the fly from a file
    Shader(const GLchar* vertexData, const GLchar* fragmentData, const GLchar* geometryData = nullptr, bool dataAreFilepaths = false)
		: m_nProgram(0)
    {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;

		if (!dataAreFilepaths)
		{
			vertexCode = vertexData;
			fragmentCode = fragmentData;
			geometryCode = geometryData ? geometryData : std::string();
		}
		else
		{			
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;
			std::ifstream gShaderFile;

			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				// Open files
				vShaderFile.open(vertexData);
				fShaderFile.open(fragmentData);
				std::stringstream vShaderStream, fShaderStream;
				// Read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// Convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
				// If geometry shader path is present, also load a geometry shader
				if (geometryData != nullptr)
				{
					gShaderFile.open(geometryData);
					std::stringstream gShaderStream;
					gShaderStream << gShaderFile.rdbuf();
					gShaderFile.close();
					geometryCode = gShaderStream.str();
				}
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
		}

		compileGLShader(vertexCode.c_str(), fragmentCode.c_str(), geometryData ? geometryCode.c_str() : nullptr);
    }

    // Uses the current shader
    void use() { glUseProgram(this->m_nProgram); }

    // Turn off shaders
    static void off() { glUseProgram(0); }

private:
	void compileGLShader(const GLchar* vShaderCode, const GLchar* fShaderCode, const GLchar* gShaderCode = nullptr)
	{
		// Shader IDs
		GLuint vertex, fragment, geometry;

		// Create vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		// Create fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		// Create geometry shader (if provided)
		if (gShaderCode != nullptr)
		{
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}

		// Create GLSL program and attach the shaders to it
		this->m_nProgram = glCreateProgram();
		glAttachShader(this->m_nProgram, vertex);
		glAttachShader(this->m_nProgram, fragment);
		if (gShaderCode != nullptr)
		{
			glAttachShader(this->m_nProgram, geometry);
		}
		glLinkProgram(this->m_nProgram);
		checkCompileErrors(this->m_nProgram, "PROGRAM");

		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (gShaderCode != nullptr)
			glDeleteShader(geometry);
	}

    void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if(type == "PROGRAM")
		{
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
            }
		}
		else
		{
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
            }
		}
	}
};

#endif