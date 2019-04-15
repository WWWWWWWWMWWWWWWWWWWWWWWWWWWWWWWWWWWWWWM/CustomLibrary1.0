#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <gl/glew.h>
#include <glm/matrix.hpp>

class Shader
{
public:
	Shader(const GLchar *vertexPath, const GLchar *fragmentPath)
	{
		//Setup filestream
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		//Open
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		//Extract
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		//Close
		vShaderFile.close();
		fShaderFile.close();

		//Move
		vertexCode = std::move(vShaderStream.str());
		fragmentCode = std::move(fShaderStream.str());

		//Assign pointer
		const char *vShaderCode = vertexCode.c_str();
		const char *fShaderCode = fragmentCode.c_str();

		unsigned int vertex, fragment;

		//Create VertexShader and Compile
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		//checkComplieErrors(fragment, "VERTEX");

		//Create FragmentShader and Compile
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		//checkComplieErrors(fragment, "FRAGMENT");

		//Link Shaders
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		//checkComplieErrors(fragment, "LINKER");

		//CleanUp
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	~Shader()
	{
		glDeleteProgram(ID);
	}

	void use()
	{
		glUseProgram(ID);
	}

	//---Uniform Setters---

	void setBool(std::string &&name, const bool &value)
	{
		glUniform1i(_mappedVal_(name)->second, static_cast<int>(value));
	}
	void setInt(const std::string &name, const int &value)
	{
		glUniform1i(_mappedVal_(name)->second, value);
	}
	void setFloat(const std::string &name, const float &value)
	{
		glUniform1f(_mappedVal_(name)->second, value);
	}

	//

	void setVec2(const std::string &name, const glm::vec2 &value)
	{
		glUniform2fv(_mappedVal_(name)->second, 1, &value[0]);
	}

	//

	void setVec3(const std::string &name, const glm::vec3 &value)
	{
		glUniform3fv(_mappedVal_(name)->second, 1, &value[0]);
	}

	//

	void setVec4(const std::string &name, const glm::vec4 &value)
	{
		glUniform4fv(_mappedVal_(name)->second, 1, &value[0]);
	}

	//

	void setMat2(const std::string &name, const glm::mat2 &value)
	{
		glUniformMatrix2fv(_mappedVal_(name)->second, 1, GL_FALSE, &value[0][0]);
	}
	void setMat3(const std::string &name, const glm::mat3 &value)
	{
		glUniformMatrix3fv(_mappedVal_(name)->second, 1, GL_FALSE, &value[0][0]);
	}
	void setMat4(const std::string &name, const glm::mat4 &value)
	{
		glUniformMatrix4fv(_mappedVal_(name)->second, 1, GL_FALSE, &value[0][0]);
	}

	const unsigned int& getID() const { return ID; }

private:
	int ID;
	std::unordered_map<std::string, GLint> m_loc;

	decltype(m_loc.begin()) _mappedVal_(const std::string &name)
	{
		auto val = m_loc.find(name);
		if (val == m_loc.end())
			val = m_loc.emplace(name, glGetUniformLocation(ID, name.c_str())).first;
		return val;
	}

	//Compile ErrorChecker
	/*void checkComplieErrors(GLuint &shader, const std::string &type)
	{
		GLint success;
		GLchar infoLog[512];

		if (type != "LINKER")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (success != GL_TRUE)
			{
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << "Shader " << type << " -> failed to compile.\n" << infoLog;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 512, NULL, infoLog);
				throw Error("Linker failed" + std::string(infoLog));
			}
		}
	}*/
};