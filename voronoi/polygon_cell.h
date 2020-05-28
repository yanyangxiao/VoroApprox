
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef POLYGON_CELL_H
#define POLYGON_CELL_H

#include <vector>

namespace xyy
{
	/**
	* can process multiple polygons in a cell
	*/
	template <typename Real = double, typename Flag = int>
	class PolygonCell
	{
	private:
		std::vector<int>    _faces;
		std::vector<Real>   _points;
		std::vector<Flag>   _flags;

	public:
		PolygonCell()
		{
			_faces.push_back(0);
		}

		PolygonCell(const PolygonCell &rhs)
		{
			_faces = rhs._faces;
			_points = rhs._points;
			_flags = rhs._flags;
		}

		~PolygonCell()
		{
		}

		void clear()
		{
			_faces.clear();
			_points.clear();
			_flags.clear();

			_faces.push_back(0);
		}

		PolygonCell& operator=(const PolygonCell &rhs)
		{
			_faces = rhs._faces;
			_points = rhs._points;
			_flags = rhs._flags;

			return *this;
		}

		int faces_number() const
		{
			return (int)_faces.size() - 1;
		}

		int points_number() const
		{
			return (int)_flags.size();
		}

		void begin_face()
		{ }

		void add_point(const Real *p, Flag flag)
		{
			_points.push_back(p[0]);
			_points.push_back(p[1]);
			_flags.push_back(flag);
		}

		void end_face()
		{
			_faces.push_back(points_number());
		}

		int face_begin(int f) const
		{
			return _faces[f];
		}

		int face_end(int f) const
		{
			return _faces[f + 1];
		}

		int face_size(int f) const
		{
			return face_end(f) - face_begin(f);
		}

		Real* point(int i)
		{
			return &_points[2 * i];
		}

		const Real* point(int i) const
		{
			return &_points[2 * i];
		}

		Flag point_flag(int i) const
		{
			return _flags[i];
		}

		void set_point_flag(int i, Flag flag)
		{
			_flags[i] = flag;
		}

		int next_around_face(int f, int i) const
		{
			return (i + 1 == face_end(f) ? face_begin(f) : i + 1);
		}

		int prev_around_face(int f, int i) const
		{
			return (i == face_begin(f) ? face_end(f) - 1 : i - 1);
		}

		void face_polygon(int f, std::vector<Real> &polygon) const
		{
			polygon.clear();
			polygon.resize(2 * face_size(f));
			int start = face_begin(f);
			int end = face_end(f);
			std::copy(_points.begin() + 2 * start, _points.begin() + 2 * end, polygon.begin());
			//for (int s = facet_begin(f), i = 0; s < facet_end(f); ++s, ++i)
			//{
			//	polygon[2 * i] = _points[2 * s];
			//	polygon[2 * i + 1] = _points[2 * s + 1];
			//}
		}

		void face_center(int f, Real *cent) const
		{
			int vnb = face_size(f);

			assert(vnb > 2);

			cent[0] = Real(0.0);
			cent[1] = Real(0.0);

			for (int i = face_begin(f); i < face_end(f); ++i)
			{
				cent[0] += _points[2 * i];
				cent[1] += _points[2 * i + 1];
			}

			cent[0] /= vnb;
			cent[1] /= vnb;
		}

		Real face_area(int f) const
		{
			Real area = Real(0.0);

			int s = face_begin(f);
			const Real *p0 = point(s);

			int nexts = next_around_face(f, s);
			int nextnexts = next_around_face(f, nexts);

			const Real *p1 = point(nexts);
			Real a[2] = { p1[0] - p0[0], p1[1] - p0[1] };

			for (int i = nextnexts; i != s; i = next_around_face(f, i))
			{
				const Real *p2 = point(i);
				Real b[2] = { p2[0] - p0[0], p2[1] - p0[1] };

				area += Real(0.5) * (a[0] * b[1] - b[0] * a[1]);

				a[0] = b[0];
				a[1] = b[1];
			}

			return area;
		}
	};
}

#endif
