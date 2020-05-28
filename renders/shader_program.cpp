
#include "shader_program.h"
#include "../xlog.h"

namespace xyy
{
	ShaderProgram::ShaderProgram()
		: _vs(NULL), _fs(NULL), _gs(NULL), _shaderProgram(0)
	{
	}

	ShaderProgram::ShaderProgram(const char *vs, const char *fs, const char *gs /* = NULL*/)
		: _vs(vs), _fs(fs), _gs(gs), _shaderProgram(0)
	{
		GLuint vertexShader = init_vertex_shader(vs);
		GLuint fragmentShader = init_fragment_shader(fs);
		GLuint geometryShader = 0;
		if (gs)
			geometryShader = init_geometry_shader(gs);

		if (vertexShader && fragmentShader)
		{
			init_shader_program(vertexShader, fragmentShader, geometryShader);
		}
	}

	ShaderProgram::ShaderProgram(const ShaderProgram &rhs)
	{
		ShaderProgram(rhs._vs, rhs._fs, rhs._gs);
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(_shaderProgram);
		_shaderProgram = 0;
	}

	ShaderProgram& ShaderProgram::operator= (const ShaderProgram &rhs)
	{
		if (this != &rhs)
		{
			GLuint vertexShader = init_vertex_shader(rhs._vs);
			GLuint fragmentShader = init_fragment_shader(rhs._fs);
			GLuint geometryShader = 0;
			if (rhs._gs)
				geometryShader = init_geometry_shader(rhs._gs);

			if (vertexShader && fragmentShader)
			{
				init_shader_program(vertexShader, fragmentShader, geometryShader);
			}
		}

		return *this;
	}

	GLuint ShaderProgram::init_vertex_shader(const char *vs)
	{
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vs, NULL);
		glCompileShader(vertexShader);

		// check
		int success;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{ // failed
			char infoLog[512];
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);

			glDeleteShader(vertexShader);
			vertexShader = 0;

			xlog("failed, %s", infoLog);
		}

		return vertexShader;
	}

	GLuint ShaderProgram::init_fragment_shader(const char *fs)
	{
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fs, NULL);
		glCompileShader(fragmentShader);

		// check
		int success;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[512];
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);

			glDeleteShader(fragmentShader);
			fragmentShader = 0;

			xlog("failed, %s", infoLog);
		}

		return fragmentShader;
	}

	GLuint ShaderProgram::init_geometry_shader(const char *gs)
	{
		GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &gs, NULL);
		glCompileShader(geometryShader);

		// check
		int success;
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[512];
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);

			glDeleteShader(geometryShader);
			geometryShader = 0;

			xlog("failed, %s", infoLog);
		}

		return geometryShader;
	}

	void ShaderProgram::init_shader_program(GLuint vs, GLuint fs, GLuint gs)
	{
		_shaderProgram = glCreateProgram();
		glAttachShader(_shaderProgram, vs);
		glAttachShader(_shaderProgram, fs);
		if (gs)
			glAttachShader(_shaderProgram, gs);

		glLinkProgram(_shaderProgram);
		int success;
		glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[512];
			glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);

			glDeleteProgram(_shaderProgram);
			_shaderProgram = 0;

			xlog("failed, %s", infoLog);
		}

		glDeleteShader(vs);
		vs = 0;
		glDeleteShader(fs);
		fs = 0;
		if (gs)
		{
			glDeleteShader(gs);
			gs = 0;
		}
	}

	void ShaderProgram::use()
	{
		glUseProgram(_shaderProgram);
	}

	bool ShaderProgram::valid() const
	{
		return _shaderProgram != 0;
	}

	GLint ShaderProgram::uniform_location(const char* name) const
	{
		GLint loc = glGetUniformLocation(_shaderProgram, name);
		if (loc == -1)
			xlog("WARNING: did not find uniform %s", name);

		return loc;
	}

	void ShaderProgram::set_bool(const char* name, bool val)
	{
		glUniform1i(uniform_location(name), (int)val);
	}

	void ShaderProgram::set_int(const char* name, int val)
	{
		glUniform1i(uniform_location(name), val);
	}

	void ShaderProgram::set_float(const char* name, float val)
	{
		glUniform1f(uniform_location(name), val);
	}

	void ShaderProgram::set_vec2i(const char* name, const int *vec)
	{
		glUniform2i(uniform_location(name), vec[0], vec[1]);
	}

	void ShaderProgram::set_vec2f(const char* name, const float *vec)
	{
		glUniform2f(uniform_location(name), vec[0], vec[1]);
	}

	void ShaderProgram::set_vec3i(const char* name, const int *vec)
	{
		glUniform3i(uniform_location(name), vec[0], vec[1], vec[2]);
	}

	void ShaderProgram::set_vec3f(const char* name, const float *vec)
	{
		glUniform3f(uniform_location(name), vec[0], vec[1], vec[2]);
	}

	void ShaderProgram::set_vec4i(const char* name, const int *vec)
	{
		glUniform4i(uniform_location(name), vec[0], vec[1], vec[2], vec[3]);
	}

	void ShaderProgram::set_vec4f(const char* name, const float *vec)
	{
		glUniform4f(uniform_location(name), vec[0], vec[1], vec[2], vec[3]);
	}

	void ShaderProgram::set_mat2f(const char* name, const float *mat)
	{
		glUniformMatrix2fv(uniform_location(name), 1, GL_FALSE, mat);
	}

	void ShaderProgram::set_mat3f(const char* name, const float *mat)
	{
		glUniformMatrix3fv(uniform_location(name), 1, GL_FALSE, mat);
	}

	void ShaderProgram::set_mat4f(const char* name, const float *mat)
	{
		glUniformMatrix4fv(uniform_location(name), 1, GL_FALSE, mat);
	}
}
