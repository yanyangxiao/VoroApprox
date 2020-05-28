
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef POINTS_RENDER_H
#define POINTS_RENDER_H

#include "shader_program.h"

namespace xyy
{
	static char *PointsRenderVertexShaderSource2 =
		"#version 330\n"
		"\n"
		"layout (location = 0) in vec2 vPos;\n"
		"\n"
		"out vec2 baseZW;\n"
		"out vec2 offsetZW;\n"
		"\n"
		"uniform mat4 u_modelMat;\n"
		"uniform mat4 u_viewMat;\n"
		"uniform mat4 u_projectionMat;\n"
		"uniform float u_pointSize;\n"
		"uniform ivec2 u_windowSize;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 viewSpacePos = u_viewMat * u_modelMat * vec4(vPos, 0, 1);\n"
		"    gl_Position = u_projectionMat * viewSpacePos;\n"
		"    gl_PointSize = u_pointSize;\n"
		"    \n"
		"    float a = u_windowSize.x * (u_projectionMat[1][3] * gl_Position.x - u_projectionMat[1][0] * gl_Position.w);\n"
		"    float b = u_windowSize.y * (u_projectionMat[1][3] * gl_Position.y - u_projectionMat[1][1]  * gl_Position.w);\n"
		"    float c = u_pointSize * u_projectionMat[1][3] * gl_Position.w;\n"
		"    float alpha = u_pointSize * gl_Position.w * gl_Position.w / (sqrt(a * a + b * b) - c);\n"
		"    \n"
		"    baseZW = gl_Position.zw;\n"
		"    offsetZW = u_projectionMat[2].zw * alpha;\n"
		"}";

	static char *PointsRenderVertexShaderSource3 =
		"#version 330\n"
		"\n"
		"layout (location = 0) in vec3 vPos;\n"
		"\n"
		"out vec2 baseZW;\n"
		"out vec2 offsetZW;\n"
		"\n"
		"uniform mat4 u_modelMat;\n"
		"uniform mat4 u_viewMat;\n"
		"uniform mat4 u_projectionMat;\n"
		"uniform float u_pointSize;\n"
		"uniform ivec2 u_windowSize;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 viewSpacePos = u_viewMat * u_modelMat * vec4(vPos, 1);\n"
		"    gl_Position = u_projectionMat * viewSpacePos;\n"
		"    gl_PointSize = u_pointSize;\n"
		"    \n"
		"    float a = u_windowSize.x * (u_projectionMat[1][3] * gl_Position.x - u_projectionMat[1][0] * gl_Position.w);\n"
		"    float b = u_windowSize.y * (u_projectionMat[1][3] * gl_Position.y - u_projectionMat[1][1]  * gl_Position.w);\n"
		"    float c = u_pointSize * u_projectionMat[1][3] * gl_Position.w;\n"
		"    float alpha = u_pointSize * gl_Position.w * gl_Position.w / (sqrt(a * a + b * b) - c);\n"
		"    \n"
		"    baseZW = gl_Position.zw;\n"
		"    offsetZW = u_projectionMat[2].zw * alpha;\n"
		"}";

	static char *PointsRenderFragmentShaderSource =
		"#version 330\n"
		"\n"
		"out vec4 fragColor;\n"
		"\n"
		"in vec2 baseZW;\n"
		"in vec2 offsetZW;\n"
		"\n"
		"uniform vec4 u_pointColor;\n"
		"uniform vec4 u_lightColor;\n"
		"uniform vec3 u_lightDirection;\n"
		"uniform vec3 u_viewDirection;\n"
		"uniform bool u_hasLight;\n"
		"\n"
		"float ambient = 0.3;\n"
		"float shininess = 128.0;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec3 fragNormal;\n"
		"    fragNormal.xy = gl_PointCoord * 2.0 - vec2(1.0);\n" // default upper-left (0, 0)
		"    fragNormal.y *= -1;\n"
		"    float norm = dot(fragNormal.xy, fragNormal.xy);\n"
		"    if (norm > 1.0)\n"
		"        discard;\n"
		"    \n"
		"    fragNormal.z = sqrt(1.0 - norm);\n"
		"    \n"
		"    vec2 zw = baseZW + offsetZW * fragNormal.z;\n"
		"    float NDCz = zw.x / zw.y;\n"
		"    gl_FragDepth = 0.5 * (gl_DepthRange.diff * NDCz + gl_DepthRange.far + gl_DepthRange.near);\n"
		"    \n"
		"    if (u_hasLight)\n"
		"    {\n"
		"		 vec3 invertLightDir = normalize(-u_lightDirection);\n"
		"		 vec3 invertViewDir = normalize(-u_viewDirection);\n"
		"		 \n"
		"		 float diffuse = max(0.0, dot(invertLightDir, fragNormal));\n"
		"		 \n"
		"		 vec3 halfVec = normalize(invertViewDir + invertLightDir);\n"
		"		 float specular = pow(max(dot(fragNormal, halfVec), 0.0), shininess);\n"
		"		 \n"
		"		 fragColor = vec4(u_pointColor.rgb * (ambient + diffuse) + specular * u_lightColor.rgb, 1.0);\n"
		"    }\n"
		"    else\n"
		"        fragColor = u_pointColor;\n"
		"}";
	
	template <int DIM>
    class PointsRender
    {
	protected:
		ShaderProgram   *_program;
		GLuint           _VAO;
		GLuint           _VBO;
		int              _vnb;	

	public:
		PointsRender();
		~PointsRender();

		void clear_data();
		void set_data(const float *points, int n);

		void set_model_matrix(const float *mat);
		void set_view_matrix(const float *mat);
		void set_projection_matrix(const float *mat);
		void set_mvp_matrix(const float *m, const float *v, const float *p);
		void set_window_size(const int size[2]);

		void set_light_color(const float light[4]);
		void set_light_direction(const float lightDir[3]);
		void set_view_direction(const float viewDir[3]);

		void set_light(bool on);

		void set_point_size(float size);
		void set_point_color(const float color[4]);

		void draw();
    };

	template <int DIM>
	PointsRender<DIM>::PointsRender()
		: _program(NULL), _VAO(0), _VBO(0), _vnb(0)
	{
		glEnable(GL_PROGRAM_POINT_SIZE);

		if (DIM == 2)
			_program = new ShaderProgram(PointsRenderVertexShaderSource2, PointsRenderFragmentShaderSource);
		else if (DIM == 3)
			_program = new ShaderProgram(PointsRenderVertexShaderSource3, PointsRenderFragmentShaderSource);

		float pointColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		set_point_color(pointColor);
		set_point_size(5.0f);
		
		// light
		float lightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		set_light_color(lightColor);
		float lightDir[3] = { 0.0f, 0.0f, -1.0f };
		set_light_direction(lightDir);
		float viewDir[3] = { 0.0f, 0.0f, -1.0f };
		set_view_direction(viewDir);

		set_light(true);

		// windowSize
		int windowSize[2] = { 1000, 1000 };
		set_window_size(windowSize);
	}

	template <int DIM>
	PointsRender<DIM>::~PointsRender()
	{
		clear_data();

		if (_program)
		{
			delete _program;
			_program = NULL;
		}
	}

	template <int DIM>
	void PointsRender<DIM>::clear_data()
	{
		if (_VBO)
		{
			glDeleteBuffers(1, &_VBO);
			_VBO = 0;
		}

		if (_VAO)
		{
			glDeleteVertexArrays(1, &_VAO);
			_VAO = 0;
		}

		_vnb = 0;
	}

	template <int DIM>
	void PointsRender<DIM>::set_data(const float *points, int n)
	{
		clear_data();

		if (!_program || !points || n < 1)
			return;

		glGenVertexArrays(1, &_VAO);
		glGenBuffers(1, &_VBO);

		_vnb = n;

		glBindVertexArray(_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, _VBO);		
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _vnb * DIM, points, GL_STATIC_DRAW);
		glVertexAttribPointer(0, DIM, GL_FLOAT, GL_FALSE, sizeof(float) * DIM, (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}
	
	template <int DIM>
	void PointsRender<DIM>::set_model_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_modelMat", mat);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_view_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_viewMat", mat);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_projection_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_projectionMat", mat);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_mvp_matrix(const float *m, const float *v, const float *p)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_modelMat", m);
			_program->set_mat4f("u_viewMat", v);
			_program->set_mat4f("u_projectionMat", p);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_window_size(const int size[2])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec2i("u_windowSize", size);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_light_color(const float light[4])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec4f("u_lightColor", light);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_light_direction(const float lightDir[3])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec3f("u_lightDirection", lightDir);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_view_direction(const float viewDir[3])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec3f("u_viewDirection", viewDir);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_light(bool on)
	{
		if (_program)
		{
			_program->use();
			_program->set_bool("u_hasLight", on);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_point_size(float size)
	{
		if (_program)
		{
			_program->use();
			_program->set_float("u_pointSize", size);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::set_point_color(const float color[4])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec4f("u_pointColor", color);
		}
	}

	template <int DIM>
	void PointsRender<DIM>::draw()
	{
		if (_program && _VAO)
		{
			_program->use();
			glBindVertexArray(_VAO);
			glDrawArrays(GL_POINTS, 0, _vnb);
			glBindVertexArray(0);
		}
	}
}

#endif
