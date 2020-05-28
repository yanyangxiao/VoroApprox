
#include "render.h"

Render::Render() : _imageRender(NULL), _approxRender(NULL), _sitesRender(NULL), _voroRender(NULL)
{
	_imageRender = new ImageRender;
	_approxRender = new ImageRender;
	_sitesRender = new PointsRender<2>;
	_sitesRender->set_point_size(10.0f);

	_voroRender = new LinesRender<2>;
	_voroRender->set_line_width(5.0f);
}

Render::~Render()
{
	if (_imageRender)
	{
		delete _imageRender;
		_imageRender = NULL;
	}

	if (_approxRender)
	{
		delete _approxRender;
		_approxRender = NULL;
	}

	if (_sitesRender)
	{
		delete _sitesRender;
		_sitesRender = NULL;
	}

	if (_voroRender)
	{
		delete _voroRender;
		_voroRender = NULL;
	}
}

void Render::set_image(const unsigned char *data, int w, int h, int c)
{
	if (_imageRender)
	{
		_imageRender->set_data(data, w, h, c);
	}
}

void Render::set_approx(const unsigned char *data, int w, int h, int c)
{
	if (_approxRender)
	{
		_approxRender->set_data(data, w, h, c);
	}
}

void Render::set_sites(const float *points, int n)
{
	if (_sitesRender)
	{
		_sitesRender->set_data(points, n);
	}
}

void Render::set_voronoi(const float *corners, int vnb, const int *edges, int enb)
{
	if (_voroRender)
	{
		_voroRender->set_data(corners, vnb, edges, enb);
	}
}

void Render::set_mvp_matrix(const float *modelMat, const float *viewMat, const float *projectionMat)
{
	if (_imageRender)
	{
		_imageRender->set_mvp_matrix(modelMat, viewMat, projectionMat);
	}

	if (_approxRender)
	{
		_approxRender->set_mvp_matrix(modelMat, viewMat, projectionMat);
	}

	if (_sitesRender)
	{
		_sitesRender->set_mvp_matrix(modelMat, viewMat, projectionMat);
	}

	if (_voroRender)
	{
		_voroRender->set_mvp_matrix(modelMat, viewMat, projectionMat);
	}
}

void Render::set_model_matrix(const float *modelMat)
{
	if (_imageRender)
	{
		_imageRender->set_model_matrix(modelMat);
	}

	if (_approxRender)
	{
		_approxRender->set_model_matrix(modelMat);
	}

	if (_sitesRender)
	{
		_sitesRender->set_model_matrix(modelMat);
	}

	if (_voroRender)
	{
		_voroRender->set_model_matrix(modelMat);
	}
}

void Render::set_view_matrix(const float *viewMat)
{
	if (_imageRender)
	{
		_imageRender->set_view_matrix(viewMat);
	}

	if (_approxRender)
	{
		_approxRender->set_view_matrix(viewMat);
	}

	if (_sitesRender)
	{
		_sitesRender->set_view_matrix(viewMat);
	}

	if (_voroRender)
	{
		_voroRender->set_view_matrix(viewMat);
	}
}

void Render::set_projection_matrix(const float *projectionMat)
{
	if (_imageRender)
	{
		_imageRender->set_projection_matrix(projectionMat);
	}

	if (_approxRender)
	{
		_approxRender->set_projection_matrix(projectionMat);
	}

	if (_sitesRender)
	{
		_sitesRender->set_projection_matrix(projectionMat);
	}

	if (_voroRender)
	{
		_voroRender->set_projection_matrix(projectionMat);
	}
}

void Render::set_window_size(int w, int h)
{
	int size[2] = { w, h };
	if (_sitesRender)
	{
		_sitesRender->set_window_size(size);
	}

	if (_voroRender)
	{
		_voroRender->set_window_size(size);
	}
}

void Render::draw_image()
{
	if (_imageRender)
	{
		_imageRender->draw();
	}
}

void Render::draw_approx()
{
	if (_approxRender)
	{
		_approxRender->draw();
	}
}

void Render::draw_sites()
{
	if (_sitesRender)
	{
		_sitesRender->draw();
	}
}

void Render::draw_voronoi()
{
	if (_voroRender)
	{
		_voroRender->draw();
	}
}

