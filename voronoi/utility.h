
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <algorithm>

namespace xyy
{
	template <typename Real>
	int quadrant2d(Real x, Real y)
	{
		return x >= 0.0 ? (y >= 0.0 ? 0 : 3) : (y >= 0.0 ? 1 : 2);
	}

	template <typename Real>
	Real dot(
		Real x1, Real y1,
		Real x2, Real y2)
	{
		return x1 * x2 + y1 * y2;
	}

	template <typename Real>
	Real dot(const Real *a, const Real *b)
	{
		return a[0] * b[0] + a[1] * b[1];
	}

	template <typename Real>
	Real cross(
		Real x1, Real y1,
		Real x2, Real y2)
	{
		return x1 * y2 - x2 * y1;
	}

	template <typename Real>
	Real cross(const Real *a, const Real *b)
	{
		return a[0] * b[1] - b[0] * a[1];
	}

	template <typename Real>
	Real signed_area(
		Real x1, Real y1,
		Real x2, Real y2,
		Real x3, Real y3)
	{
		Real p1p2[2] = { x2 - x1, y2 - y1 };
		Real p1p3[2] = { x3 - x1, y3 - y1 };
		return 0.5 * cross(p1p2, p1p3);
	}

	template <typename Real>
	Real signed_area(
		const Real *x1,
		const Real *x2,
		const Real *x3)
	{
		Real p1p2[2] = { x2[0] - x1[0], x2[1] - x1[1] };
		Real p1p3[2] = { x3[0] - x1[0], x3[1] - x1[1] };
		return 0.5 * cross(p1p2, p1p3);
	}

	template <typename Real>
	Real area(
		Real x1, Real y1,
		Real x2, Real y2,
		Real x3, Real y3)
	{
		return fabs(signed_area(x1, y1, x2, y2, x3, y3));
	}

	template <typename Real>
	Real area(const Real *a, const Real *b, const Real *c)
	{
		return fabs(signed_area(a, b, c));
	}

	template <typename Real>
	Real side(Real ax, Real ay, Real bx, Real by, Real px, Real py)
	{
		Real dir1[2] = { ax - px, ay - py };
		Real dir2[2] = { bx - px, by - py };
		return cross(dir1, dir2);
	}

	template <typename Real>
	Real side(const Real *a, const Real *b, const Real *p)
	{
		Real dir1[2] = { a[0] - p[0], a[1] - p[1] };
		Real dir2[2] = { b[0] - p[0], b[1] - p[1] };
		return cross(dir1, dir2);
	}

	/**
	* relationship between point and segment
	* @return: >0: not on segment, =0: coincide with endpoint of segment, <0: inside segment
	*/
	template <typename Real>
	Real locate_point_on_segment(
		Real x, Real y,
		Real ax, Real ay,
		Real bx, Real by)
	{
		Real pa[2] = { ax - x, ay - y };
		Real pb[2] = { bx - x, by - y };

		return dot(pa, pb);
	}

	template <typename Real>
	Real locate_point_on_segment(
		const Real *x,
		const Real *a,
		const Real *b)
	{
		return locate_point_on_segment(x[0], x[1], a[0], a[1], b[0], b[1]);
	}

	/**
	* relationship between point and polygon using improved arc length method
	* @requirement: the polygon has direction
	* @return: -1: outside, 0: on, 1: inside
	*/
	template <typename Real>
	int locate_point_on_polygon2d(
		Real x,
		Real y,
		const Real *polygon,
		int nb)
	{
		// corner size less than 3, unbounded polygon!
		if (nb < 3)
		{
			return -1;
		}

		Real prevx = polygon[2 * (nb - 1)] - x;
		Real prevy = polygon[2 * (nb - 1) + 1] - y;
		if (prevx == 0.0 && prevy == 0.0) // the intersection is polygon corner
			return 0;

		int prevQuad = quadrant2d(prevx, prevy);

		int sum = 0;
		for (int i = 0; i < nb; ++i)
		{
			// translate to the coordinate system origined by tested point
			Real xi = polygon[2 * i] - x;
			Real yi = polygon[2 * i + 1] - y;
			if (xi == 0.0 && yi == 0.0) // the intersection is polygon corner
				return 0;

			Real crossVal = prevx * yi - xi * prevy;
			// the intersection is polygon corner
			if (crossVal == 0.0 && prevx * xi <= 0.0 && prevy * yi <= 0.0)
				return 0;

			int quad = quadrant2d(xi, yi);

			if (quad == (prevQuad + 1) % 4) // next quadrant
				sum += 1;
			else if (quad == (prevQuad + 3) % 4) // previous quadrant
				sum -= 1;
			else if (quad == (prevQuad + 2) % 4) // cross quadrant
			{
				if (crossVal > 0.0)
					sum += 2;
				else
					sum -= 2;
			}

			prevx = xi;
			prevy = yi;
			prevQuad = quad;
		}

		if (sum) // sum > 0 means the tested point is inside
			return 1;

		return -1;
	}

	template <typename Real>
	int locate_point_on_polygon2d(
		Real x,
		Real y,
		const std::vector<Real> &polygon)
	{
		return locate_point_on_polygon2d(x, y, &polygon[0], (int)polygon.size() / 2);
	}

	/**
	* has intersection between two segments
	*/
	template <typename Real>
	bool has_intersection_between_segments(
		Real p1x, Real p1y,
		Real p2x, Real p2y,
		Real q1x, Real q1y,
		Real q2x, Real q2y,
		Real &x, Real &y,
		Real threshold = 2e-10)
	{
		// test 1
		if (std::min(p1x, p2x) > std::max(q1x, q2x) ||
			std::min(q1x, q2x) > std::max(p1x, p2x) ||
			std::min(p1y, p2y) > std::max(q1y, q2y) ||
			std::min(q1y, q2y) > std::max(p1y, p2y))
			return false;

		// test2
		// q1q2 are same side of p1p2
		Real p1q1[2] = { q1x - p1x, q1y - p1y };
		Real p1p2[2] = { p2x - p1x, p2y - p1y };
		Real p1q2[2] = { q2x - p1x, q2y - p1y };

		Real cross1 = p1q1[0] * p1p2[1] - p1p2[0] * p1q1[1];
		if (fabs(cross1) < threshold)
			cross1 = 0.0;

		Real cross2 = p1q2[0] * p1p2[1] - p1p2[0] * p1q2[1];
		if (fabs(cross2) < threshold)
			cross2 = 0.0;

		if (cross1 * cross2 > 0.0) // same side
			return false;

		// p1p2 are same side of q1q2
		Real q1p1[2] = { p1x - q1x, p1y - q1y };
		Real q1q2[2] = { q2x - q1x, q2y - q1y };
		Real q1p2[2] = { p2x - q1x, p2y - q1y };

		cross1 = q1p1[0] * q1q2[1] - q1q2[0] * q1p1[1];
		if (fabs(cross1) < threshold)
			cross1 = 0.0;

		cross2 = q1p2[0] * q1q2[1] - q1q2[0] * q1p2[1];
		if (fabs(cross2) < threshold)
			cross2 = 0.0;

		if (cross1 * cross2 > 0.0) // same side
			return false;

		Real s1 = area(p1x, p1y, q1x, q1y, p2x, p2y);
		Real s2 = area(p1x, p1y, p2x, p2y, q2x, q2y);
		Real sum = s1 + s2;

		x = (q2x * s1 + q1x * s2) / sum;
		y = (q2y * s1 + q1y * s2) / sum;

		return true;
	}

	template <typename Real>
	bool has_intersection_between_segments(
		const Real *p1, const Real *p2,
		const Real *q1, const Real *q2,
		Real *x, Real threshold = 2e-10)
	{
		return has_intersection_between_segments(p1[0], p1[1], p2[0], p2[1], q1[0], q1[1], q2[0], q2[1], x[0], x[1], threshold);
	}

	/**
	* has intersection between two segments
	*/
	template <typename Real>
	bool has_intersection_except_endpoints_between_segments(
		Real p1x, Real p1y,
		Real p2x, Real p2y,
		Real q1x, Real q1y,
		Real q2x, Real q2y,
		Real &x, Real &y)
	{
		// test 1
		if (std::min(p1x, p2x) > std::max(q1x, q2x) ||
			std::min(q1x, q2x) > std::max(p1x, p2x) ||
			std::min(p1y, p2y) > std::max(q1y, q2y) ||
			std::min(q1y, q2y) > std::max(p1y, p2y))
			return false;

		// test2
		// p1p1 are same side of q1q2
		Real q1p1[2] = { p1x - q1x, p1y - q1y };
		Real q1q2[2] = { q2x - q1x, q2y - q1y };
		Real q1p2[2] = { p2x - q1x, p2y - q1y };

		Real cross1 = q1p1[0] * q1q2[1] - q1q2[0] * q1p1[1];
		Real cross2 = q1p2[0] * q1q2[1] - q1q2[0] * q1p2[1];
		if (cross1 * cross2 >= 0.0) // same side
			return false;

		// q1q2 are same side of p1p2
		Real p1q1[2] = { q1x - p1x, q1y - p1y };
		Real p1p2[2] = { p2x - p1x, p2y - p1y };
		Real p1q2[2] = { q2x - p1x, q2y - p1y };

		cross1 = p1q1[0] * p1p2[1] - p1p2[0] * p1q1[1];
		cross2 = p1q2[0] * p1p2[1] - p1p2[0] * p1q2[1];
		if (cross1 * cross2 >= 0.0) // same side
			return false;

		Real s1 = area(p1x, p1y, q1x, q1y, p2x, p2y);
		Real s2 = area(p1x, p1y, p2x, p2y, q2x, q2y);
		Real sum = s1 + s2;

		x = (q2x * s1 + q1x * s2) / sum;
		y = (q2y * s1 + q1y * s2) / sum;

		return true;
	}

	template <typename Real>
	bool has_intersection_except_endpoints_between_segments(
		const Real *p1, const Real *p2,
		const Real *q1, const Real *q2,
		Real *x)
	{
		return has_intersection_except_endpoints_between_segments(p1[0], p1[1], p2[0], p2[1], q1[0], q1[1], q2[0], q2[1], x[0], x[1]);
	}

	template <typename Real>
	bool on_segment(Real p1x, Real p1y, Real p2x, Real p2y, Real qx, Real qy, Real threshold = 2e-10)
	{
		Real p1q[2] = { qx - p1x, qy - p1y };
		Real p2q[2] = { qx - p2x, qy - p2y };
		Real crossVal = p1q[0] * p2q[1] - p1q[1] * p2q[0];
		if (fabs(crossVal) > threshold)
			return false;
		
		Real dotVal = p1q[0] * p2q[0] + p1q[1] * p2q[1];
		return dotVal < 0.0;
	}

	template <typename Real>
	bool on_segment(const Real *p1, const Real *p2, const Real *q, Real threshold = 2e-10)
	{
		return on_segment(p1[0], p1[1], p2[0], p2[1], q[0], q[1], threshold);
	}
}

#endif
