
#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h>

namespace xyy
{
	class ShaderProgram
	{
	protected:
		const char *_vs;
		const char *_fs;
		const char *_gs;

		GLuint      _shaderProgram;

	public:
		ShaderProgram();
		ShaderProgram(const char *vs, const char *fs, const char *gs = NULL);
		ShaderProgram(const ShaderProgram &rhs);
		~ShaderProgram();
		ShaderProgram& operator= (const ShaderProgram &rhs);

		void use();
		bool valid() const;

		void set_bool(const char *name, bool val);
		void set_int(const char *name, int val);
		void set_float(const char *name, float val);
		void set_vec2i(const char *name, const int *vec);
		void set_vec2f(const char *name, const float *vec);
		void set_vec3i(const char *name, const int *vec);
		void set_vec3f(const char *name, const float *vec);
		void set_vec4i(const char *name, const int *vec);
		void set_vec4f(const char *name, const float *vec);
		void set_mat2f(const char *name, const float *mat);
		void set_mat3f(const char *name, const float *mat);
		void set_mat4f(const char *name, const float *mat);

	protected:
		GLuint init_vertex_shader(const char *vs);
		GLuint init_fragment_shader(const char *fs);
		GLuint init_geometry_shader(const char *gs);
		void init_shader_program(GLuint vs, GLuint fs, GLuint gs);

		GLint uniform_location(const char *name) const;
	};
}

#endif
