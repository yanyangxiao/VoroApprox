
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "pixelset.h"

namespace xyy
{
	class Rasterizer
	{
	public:
		static void locate_point(int w, int h, const double *p, int &i, int &j);
		static void rasterize(const double *polygon, int vnb, int width, int height, PixelSet &pixels);

	protected:
		static void rasterize(bool ccw, const std::vector<int> &x_, const std::vector<int> &y_, int *x_left, int *x_right);
	};
}



#endif
