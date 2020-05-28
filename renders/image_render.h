
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef IMAGE_RENDER_H
#define IMAGE_RENDER_H

#include "shader_program.h"

namespace xyy
{
	static char* ImageRenderVertexShaderSource =
		"#version 330\n"
		"uniform mat4 u_modelMat;\n"
		"uniform mat4 u_viewMat;\n"
		"uniform mat4 u_projectionMat;\n"
		"layout (location = 0) in vec2 inPosition;\n"
		"layout (location = 1) in vec2 inTexcoord;\n"
		"out vec2 varyTexcoord;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = u_projectionMat * u_viewMat * u_modelMat * vec4(inPosition, 0.0, 1.0);\n"
		"    varyTexcoord = inTexcoord;\n"
		"}";

	static char* ImageRenderFragmentShaderSource =
		"#version 330\n"
		"in vec2 varyTexcoord;\n"
		"out vec4 fragColor;\n"
		"uniform sampler2D inTexture;\n"
		"void main()\n"
		"{\n"
		"    fragColor = texture(inTexture, varyTexcoord);\n"
		"}";

	class ImageRender
	{
	protected:
		ShaderProgram *_program;
		GLuint         _VAO;
		GLuint         _VBO;
		unsigned int   _vnb;
		GLuint         _texture;

	public:
		ImageRender();
		~ImageRender();

		void clear_data();
		void set_data(const unsigned char* data, int width, int height, int channel);
		void set_mvp_matrix(const float *modelMat, const float *viewMat, const float *projectionMat);
		void set_model_matrix(const float *modelMat);
		void set_view_matrix(const float *viewMat);
		void set_projection_matrix(const float *projectionMat);
		void draw();
	};
}



#endif
