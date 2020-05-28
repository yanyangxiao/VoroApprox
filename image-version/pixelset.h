
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef PIXELSET_H
#define PIXELSET_H

#include <vector>

namespace xyy
{
	struct PixelSet
	{
		std::vector<int> left;
		std::vector<int> right;
		int ymin;
		int ymax;

		PixelSet()
			: ymin(1000000), ymax(-1)
		{
		}

		PixelSet& operator= (const PixelSet& rhs)
		{
			ymin = rhs.ymin;
			ymax = rhs.ymax;
			left = rhs.left;
			right = rhs.right;

			return *this;
		}

		void clear()
		{
			ymin = 1000000;
			ymax = -1;
			left.clear();
			right.clear();
		}
	};
}

#endif

