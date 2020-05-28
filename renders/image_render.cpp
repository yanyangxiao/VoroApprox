
#include "image_render.h"

namespace xyy
{
	ImageRender::ImageRender()
		: _program(NULL),
		_VAO(0),
		_VBO(0),
		_vnb(0),
		_texture(0)
	{
		_program = new ShaderProgram(ImageRenderVertexShaderSource, ImageRenderFragmentShaderSource);
	}

	ImageRender::~ImageRender()
	{
		clear_data();

		if (_program)
		{
			delete _program;
			_program = NULL;
		}
	}

	void ImageRender::clear_data()
	{
		if (_texture)
		{
			glDeleteTextures(1, &_texture);
			_texture = 0;
		}
	}

	void ImageRender::set_data(const unsigned char* data, int width, int height, int channel)
	{
		clear_data();
		if (!_program || !data || width < 1)
			return;

		float ratio = float(height) / width;
		float rect[] =
		{   -1.0f, -ratio, 0.0f, 0.0f,
			1.0f, -ratio, 1.0f, 0.0f,
			1.0f, ratio, 1.0f, 1.0f,
			-1.0f, -ratio, 0.0f, 0.0f,
			1.0f, ratio, 1.0f, 1.0f,
			-1.0f, ratio, 0.0f, 1.0f
		};
		_vnb = 6;

		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (channel == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else if (channel == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (channel == 1)
		{
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_R8, GL_UNSIGNED_BYTE, data);
			int N = width * height;
			unsigned char *rgb = new unsigned char[N * 3];

			for (int i = 0; i < N; ++i)
			{
				rgb[3 * i] = data[i];
				rgb[3 * i + 1] = data[i];
				rgb[3 * i + 2] = data[i];
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);

			delete[] rgb;
			rgb = NULL;
		}
		glGenerateMipmap(GL_TEXTURE_2D);

		glGenVertexArrays(1, &_VAO);
		glBindVertexArray(_VAO);

		// vbo
		glGenBuffers(1, &_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void ImageRender::set_mvp_matrix(const float *modelMat, const float *viewMat, const float *projectionMat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_modelMat", modelMat);
			_program->set_mat4f("u_viewMat", viewMat);
			_program->set_mat4f("u_projectionMat", projectionMat);
		}
	}

	void ImageRender::set_model_matrix(const float *modelMat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_modelMat", modelMat);
		}
	}

	void ImageRender::set_view_matrix(const float *viewMat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_viewMat", viewMat);
		}
	}

	void ImageRender::set_projection_matrix(const float *projectionMat)
	{
		if (_program)
		{
			_program->use();
			_program->set_mat4f("u_projectionMat", projectionMat);
		}
	}

	void ImageRender::draw()
	{
		if (_program && _texture && _VAO)
		{
			_program->use();

			glBindVertexArray(_VAO);
			glBindTexture(GL_TEXTURE_2D, _texture);
			glDrawArrays(GL_TRIANGLES, 0, _vnb);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}
	}
}
