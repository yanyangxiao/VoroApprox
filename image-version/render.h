
#ifndef RENDER_H
#define RENDER_H

#include "image_render.h"
#include "points_render.h"
#include "lines_render.h"

using namespace xyy;

class Render
{
protected:
	ImageRender     *_imageRender;
	ImageRender     *_approxRender;
	PointsRender<2> *_sitesRender;
	LinesRender<2>  *_voroRender;	

public:
	Render();
	~Render();
	
	void set_image(const unsigned char *data, int w, int h, int c);
	void set_approx(const unsigned char *data, int w, int h, int c);
	void set_sites(const float *points, int n);
	void set_voronoi(const float *corners, int vnb, const int *edges, int enb);

	void set_mvp_matrix(const float *modelMat, const float *viewMat, const float *projectionMat);
	void set_model_matrix(const float *modelMat);
	void set_view_matrix(const float *viewMat);
	void set_projection_matrix(const float *projectionMat);

	void set_window_size(int w, int h);
	void set_point_size(float size) { if (_sitesRender) _sitesRender->set_point_size(size); }
	void set_line_width(float size) { if (_voroRender) _voroRender->set_line_width(size); }

	void draw_image();
	void draw_approx();
	void draw_sites();
	void draw_voronoi();
};

#endif
