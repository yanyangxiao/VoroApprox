
#include <algorithm>
#include "rasterizer.h"

namespace xyy
{
	void Rasterizer::locate_point(int width, int height, const double *p, int &i, int &j)
	{
		double ratio = double(height) / width;

		double x = 0.5 * (p[0] + 1.0) / 1.0;
		double y = 0.5 * (p[1] + ratio) / ratio;

		x = (std::max)(0.0, x);
		x = (std::min)(1.0, x);
		y = (std::max)(0.0, y);
		y = (std::min)(1.0, y);
		
		i = (int)floor(x * double(width));
		j = (int)floor(y * double(height));

		if (i == width) --i;
		if (j == height) --j;
	}

	void Rasterizer::rasterize(const double *polygon, int vnb, int width, int height, PixelSet &pixels)
	{
		pixels.clear();

		if (vnb < 3)
			return;

		double signed_area = (polygon[2] - polygon[0]) * (polygon[5] - polygon[1]) - (polygon[4] - polygon[0]) * (polygon[3] - polygon[1]);
		bool ccw = (signed_area < 0.0);

		std::vector<int> x_;
		std::vector<int> y_;
		pixels.ymin = height;
		pixels.ymax = -1;
		for (int v = 0; v < vnb; ++v)
		{
			int i = -1, j = -1;
			locate_point(width, height, &polygon[2 * v], i, j);

			x_.push_back(i);
			y_.push_back(j);

			pixels.ymin = std::min(pixels.ymin, j);
			pixels.ymax = std::max(pixels.ymax, j);
		}

		int* x_left = new int[height];
		int* x_right = new int[height];
		for (int j = pixels.ymin; j <= pixels.ymax; ++j)
		{
			x_left[j] = width - 1;
			x_right[j] = 0;
		}
		
		rasterize(ccw, x_, y_, x_left, x_right);
		
		for (int j = pixels.ymin; j <= pixels.ymax; ++j)
		{
			pixels.left.push_back(x_left[j]);
			pixels.right.push_back(x_right[j]);
		}

		delete[] x_left;
		x_left = NULL;
		delete[] x_right;
		x_right = NULL;
	}

	void Rasterizer::rasterize(bool ccw, const std::vector<int> &x_, const std::vector<int> &y_, int *x_left, int *x_right)
	{
		int N = (int)x_.size();

		for (int i = 0; i < N; ++i)
		{
			int j = (i + 1) % N;
			int x1 = x_[i];
			int y1 = y_[i];
			int x2 = x_[j];
			int y2 = y_[j];
			if (y1 == y2)
			{
				if (x2 > x1)
				{
					x_left[y1] = std::min(x_left[y1], x1);
					x_right[y1] = std::max(x_right[y1], x2);
				}
				else
				{
					x_left[y1] = std::min(x_left[y1], x2);
					x_right[y1] = std::max(x_right[y1], x1);
				}
				continue;
			}

			bool is_left = (y2 < y1) ^ ccw;

			// Bresenham algo.
			int dx = x2 - x1;
			int dy = y2 - y1;
			int sx = dx > 0 ? 1 : -1;
			int sy = dy > 0 ? 1 : -1;
			dx *= sx;
			dy *= sy;
			int x = x1;
			int y = y1;

			int* line_x = is_left ? x_left : x_right;
			if ((is_left && sx > 0) || (!is_left && sx < 0))
				line_x[y] = x;

			int e = dy - dx;
			while ((sy > 0 && y < y2) || (sy < 0 && y > y2))
			{
				while (e < 0 || (e == 0 && ((is_left && sx > 0) || (!is_left && sx < 0))))
				{
					x += sx;
					e += 2 * dy;
				}
				if ((!is_left && sx > 0) || (is_left && sx < 0))
					line_x[y] = x;
				y += sy;
				e -= 2 * dx;
				if ((is_left && sx > 0) || (!is_left && sx < 0))
					line_x[y] = x;
			}
			if ((!is_left && sx > 0) || (is_left && sx < 0))
				line_x[y2] = x2;
		}
	}
}

