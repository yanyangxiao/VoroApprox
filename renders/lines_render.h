
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef LINES_RENDER_H
#define LINES_RENDER_H

#include "shader_program.h"

namespace xyy
{
	static char *LinesRenderVertexShaderSource2 =
		"#version 330\n"
		"\n"
		"layout (location = 0) in vec2 vPos;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(vPos, 0.0, 1.0);\n"
		"}\n";

	static char *LinesRenderVertexShaderSource3 =
		"#version 330\n"
		"\n"
		"layout (location = 0) in vec3 vPos;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(vPos, 1.0);\n"
		"}\n";

	static char *LinesRenderGeometryShaderSource =
		"#version 330\n"
		"\n"
		"layout (lines) in;\n"
		"layout (triangle_strip, max_vertices = 4) out;\n"
		"\n"
		"out GSOUT_FSIN\n"
		"{\n"
		"    vec2 fragCoord;\n"
		"    vec2 sourceFragCoord;\n"
		"    vec2 targetFragCoord;\n"
		"    float radius;\n"
		"    vec2 baseZW;\n"
		"    vec2 offsetZW;\n"
		"} gsOut;\n"
		"\n"
		"uniform mat4 u_modelMat;\n"
		"uniform mat4 u_viewMat;\n"
		"uniform mat4 u_projectionMat;\n"
		"uniform float u_lineWidth;\n"
		"uniform ivec2 u_windowSize;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	 mat4 mv = u_viewMat * u_modelMat;\n"
		"    vec4 viewPosA = mv * gl_in[0].gl_Position;\n"
		"	 vec4 viewPosB = mv * gl_in[1].gl_Position;\n"
		"    vec4 clipPosA = u_projectionMat * viewPosA;\n"
		"    vec4 clipPosB = u_projectionMat * viewPosB;\n"
		"    \n"
		"    vec4 direction = normalize(vec4((viewPosB - viewPosA).xy, 0, 0));\n"
		"    vec4 normal = normalize(vec4(-direction.y, direction.x, 0, 0));\n"
		"    \n"
		"    float alpha[2];\n"
		"    \n"
		"    float a1 = u_windowSize.x * (u_projectionMat[1][3] * clipPosA.x - u_projectionMat[1][0] * clipPosA.w);\n"
		"    float b1 = u_windowSize.y * (u_projectionMat[1][3] * clipPosA.y - u_projectionMat[1][1] * clipPosA.w);\n"
		"    float c1 = u_lineWidth * u_projectionMat[1][3] * clipPosA.w;\n"
		"    alpha[0] = u_lineWidth * clipPosA.w * clipPosA.w / (sqrt(a1 * a1 + b1 * b1) - c1);\n"
		"    \n"
		"    float a2 = u_windowSize.x * (u_projectionMat[1][3] * clipPosB.x - u_projectionMat[1][0] * clipPosB.w);\n"
		"    float b2 = u_windowSize.y * (u_projectionMat[1][3] * clipPosB.y - u_projectionMat[1][1] * clipPosB.w);\n"
		"    float c2 = u_lineWidth * u_projectionMat[1][3] * clipPosB.w;\n"
		"    alpha[1] = u_lineWidth * clipPosB.w * clipPosB.w / (sqrt(a2 * a2 + b2 * b2) - c2);\n"
		"    \n"
		"    vec4 p[4];\n"
		"    p[0] = viewPosA - 0.5 * alpha[0] * direction - 0.5 * alpha[0] * normal;\n"
		"    p[1] = viewPosB + 0.5 * alpha[1] * direction - 0.5 * alpha[1] * normal;\n"
		"    p[2] = viewPosA - 0.5 * alpha[0] * direction + 0.5 * alpha[0] * normal;\n"
		"    p[3] = viewPosB + 0.5 * alpha[1] * direction + 0.5 * alpha[1] * normal;\n"
		"    \n"
		"    clipPosA /= clipPosA.w;\n"
		"    clipPosB /= clipPosB.w;\n"
		"    vec2 fragCoordA = 0.5 * u_windowSize * (clipPosA.xy + vec2(1.0));\n"
		"    vec2 fragCoordB = 0.5 * u_windowSize * (clipPosB.xy + vec2(1.0));\n"
		"    \n"
		"    for (int i = 0; i < 4; ++i)\n"
		"    {\n"
		"        vec4 q = u_projectionMat * p[i];\n"
		"        gl_Position = q;\n"
		"        q /= q.w;\n"
		"        gsOut.fragCoord = 0.5 * u_windowSize * (q.xy + vec2(1.0));\n"
		"        gsOut.sourceFragCoord = fragCoordA;\n"
		"        gsOut.targetFragCoord = fragCoordB;\n"
		"        gsOut.radius = 0.5 * u_lineWidth;\n"
		"        gsOut.baseZW = gl_Position.zw;\n"
		"        gsOut.offsetZW = u_projectionMat[2].zw * alpha[i % 2];\n"
		"        EmitVertex();\n"
		"    }\n"
		"    \n"
		"    EndPrimitive();\n"
		"}\n";

	static char *LinesRenderFragmentShaderSource =
		"#version 330\n"
		"\n"
		"out vec4 fragColor;\n"
		"\n"
		"in GSOUT_FSIN\n"
		"{\n"
		"    vec2 fragCoord;\n"
		"    vec2 sourceFragCoord;\n"
		"    vec2 targetFragCoord;\n"
		"    float radius;\n"
		"    vec2 baseZW;\n"
		"    vec2 offsetZW;\n"
		"} fsIn;\n"
		"\n"
		"uniform vec4 u_lineColor;\n"
		"uniform vec4 u_lightColor;\n"
		"uniform vec3 u_lightDirection;\n"
		"uniform vec3 u_viewDirection;\n"
		"uniform bool u_hasLight;\n"
		"\n"
		"float ambient = 0.3;\n"
		"float shininess = 128;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec2 a = fsIn.fragCoord.xy - fsIn.sourceFragCoord;\n"
		"    vec2 b = fsIn.fragCoord.xy - fsIn.targetFragCoord;\n"
		"    vec2 c = fsIn.targetFragCoord - fsIn.sourceFragCoord;\n"
		"    \n"
		"    if (dot(a, c) < 0.0 && dot(a, a) > fsIn.radius)\n"
		"        discard;\n"
		"    \n"
		"    if (dot(b, -c) < 0.0 && dot(b, b) > fsIn.radius)\n"
		"        discard;\n"
		"    \n"
		"    vec3 fragNormal;\n"
		"    if (dot(a, c) < 0.0)\n"
		"    {\n"
		"        fragNormal.xy = a;\n"
		"        fragNormal.z = sqrt(fsIn.radius * fsIn.radius - dot(a, a));\n"
		"        fragNormal = normalize(fragNormal);\n"
		"    }\n"
		"    else if (dot(b, -c) < 0.0)\n"
		"    {\n"
		"        fragNormal.xy = b;\n"
		"        fragNormal.z = sqrt(fsIn.radius * fsIn.radius - dot(b, b));\n"
		"        fragNormal = normalize(fragNormal);\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        vec2 d = normalize(vec2(-c.y, c.x));\n"
		"        fragNormal.xy = d * dot(d, a);\n"
		"        fragNormal.z = sqrt(fsIn.radius * fsIn.radius - dot(fragNormal.xy, fragNormal.xy));\n"
		"        fragNormal = normalize(fragNormal);\n"
		"    }\n"
		"    \n"
		"    vec2 zw = fsIn.baseZW + fsIn.offsetZW * fragNormal.z;\n"
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
		"		 fragColor = vec4(u_lineColor.rgb * (ambient + diffuse) + specular * u_lightColor.rgb, 1.0);\n"
		"    }\n"
		"    else\n"
		"        fragColor = u_lineColor;\n"
		"}";

	template <int DIM>
	class LinesRender
	{
	protected:
		ShaderProgram *_program;
		GLuint         _VAO;
		GLuint         _VBO;
		int            _vnb;

		GLuint         _EBO;
		int            _enb;

	public:
		LinesRender();
		~LinesRender();
		void clear_data();

		void set_data(const float *lines, int n);
		void set_data(const float *sources, const float *targets, int n);
		void set_data(const float *points, int vnb, const int *indices, int enb);

		void set_model_matrix(const float *mat);
		void set_view_matrix(const float *mat);
		void set_projection_matrix(const float *mat);
		void set_mvp_matrix(const float *m, const float *v, const float *p);
		void set_window_size(const int size[2]);

		void set_light_color(const float light[4]);
		void set_light_direction(const float lightDir[3]);
		void set_view_direction(const float viewDir[3]);

		void set_light(bool on);

		void set_line_width(float width);
		void set_line_color(const float color[4]);

		void draw();
	};

	template <int DIM>
	LinesRender<DIM>::LinesRender()
		: _program(NULL), _VAO(0), _VBO(0), _vnb(0), _EBO(0), _enb(0)
	{
		if (DIM == 2)
			_program = new ShaderProgram(
				LinesRenderVertexShaderSource2,
				LinesRenderFragmentShaderSource,
				LinesRenderGeometryShaderSource);
		else if (DIM == 3)
			_program = new ShaderProgram(
				LinesRenderVertexShaderSource3, 
				LinesRenderFragmentShaderSource, 
				LinesRenderGeometryShaderSource);

		float lineColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
		set_line_color(lineColor);
		set_line_width(2.0f);

		// light
		float lightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		set_light_color(lightColor);
		float lightDir[3] = { 0.0f, 0.0f, -1.0f };
		set_light_direction(lightDir);
		float viewDir[3] = { 0.0f, 0.0f, -1.0f };
		set_view_direction(viewDir);

		set_light(false);

		// windowSize
		int windowSize[2] = { 1000, 1000 };
		set_window_size(windowSize);
	}

	template <int DIM>
	LinesRender<DIM>::~LinesRender()
	{
		clear_data();

		if (_program)
		{
			delete _program;
			_program = NULL;
		}
	}

	template <int DIM>
	void LinesRender<DIM>::clear_data()
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

		if (_EBO)
		{
			glDeleteBuffers(1, &_EBO);
			_EBO = 0;
		}

		_enb = 0;
	}

	template <int DIM>
	void LinesRender<DIM>::set_data(const float *lines, int n)
	{
		clear_data();

		if (!_program || !lines || n < 1)
			return;
		
		glGenVertexArrays(1, &_VAO);
		glGenBuffers(1, &_VBO);

		_vnb = n * 2;

		glBindVertexArray(_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, _VBO);		
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _vnb * DIM, lines, GL_STATIC_DRAW);
		glVertexAttribPointer(0, DIM, GL_FLOAT, GL_FALSE, sizeof(float) * DIM, (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}

	template <int DIM>
	void LinesRender<DIM>::set_data(const float *sources, const float *targets, int n)
	{
		clear_data();

		if (!_program || !sources || !targets || n < 2)
			return;

		int size = n * DIM * 2;
		float *lines = new float[size];
		for (unsigned int i = 0; i < n; ++i)
		{
			for (unsigned int k = 0; k < DIM; ++k)
				lines[i * DIM * 2 + k] = sources[i * DIM + k];
			for (unsigned int k = 0; k < DIM; ++k)
				lines[i * DIM * 2 + DIM + k] = targets[i * DIM + k];
		}

		set_data(lines, n);

		delete[] lines;
	}

	template <int DIM>
	void LinesRender<DIM>::set_data(const float *points, int vnb, const int *indices, int enb)
	{
		clear_data();

		if (!_program || !points || vnb < 2 || !indices || enb < 1)
			return;

		_enb = enb;

		glGenVertexArrays(1, &_VAO);
		glGenBuffers(1, &_VBO);
		glGenBuffers(1, &_EBO);
		
		glBindVertexArray(_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vnb * DIM, points, GL_STATIC_DRAW);
		glVertexAttribPointer(0, DIM, GL_FLOAT, GL_FALSE, sizeof(float) * DIM, (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _enb * 2, indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		// EBO must be unbound after VAO unbinding
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	template <int DIM>
	void LinesRender<DIM>::set_model_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_modelMat", mat);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_view_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_viewMat", mat);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_projection_matrix(const float *mat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_projectionMat", mat);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_mvp_matrix(const float *m, const float *v, const float *p)
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
	void LinesRender<DIM>::set_window_size(const int size[2])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec2i("u_windowSize", size);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_light_color(const float light[4])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec4f("u_lightColor", light);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_light_direction(const float lightDir[3])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec3f("u_lightDirection", lightDir);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_view_direction(const float viewDir[3])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec3f("u_viewDirection", viewDir);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_light(bool on)
	{
		if (_program)
		{
			_program->use();
			_program->set_bool("u_hasLight", on);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_line_width(float size)
	{
		if (_program)
		{
			_program->use();
			_program->set_float("u_lineWidth", size);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::set_line_color(const float color[4])
	{
		if (_program)
		{
			_program->use();
			_program->set_vec4f("u_lineColor", color);
		}
	}

	template <int DIM>
	void LinesRender<DIM>::draw()
	{
		if (_program && _VAO)
		{
			_program->use();
			glBindVertexArray(_VAO);

			if (_EBO)
				glDrawElements(GL_LINES, _enb * 2, GL_UNSIGNED_INT, 0);
			else
				glDrawArrays(GL_LINES, 0, _vnb);

			glBindVertexArray(0);
		}
	}
}

#endif
