
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef VORONOI_2D_H
#define VORONOI_2D_H

#include <stack>
#include <unordered_set>
#include <unordered_map>
#include "dual_segment.h"
#include "polygon_cell.h"
#include "utility.h"

namespace xyy
{
	template <typename Delaunay, typename Real = double>
	class Voronoi2D
	{
		typedef DualSegment<Real, int> DualSeg;
		typedef PolygonCell<Real, int> PolyCell;

		struct StackItem
		{
			int vertexID;
			int segmentID;

			StackItem(int v, int s)
				: vertexID(v), segmentID(s)
			{
			}

			StackItem(const StackItem &rhs)
				: vertexID(rhs.vertexID), segmentID(rhs.segmentID)
			{
			}

			StackItem& operator= (const StackItem &rhs)
			{
				vertexID = rhs.vertexID;
				segmentID = rhs.segmentID;

				return *this;
			}
		};

	protected:
		PolyCell                              _domain;
		std::vector<PolyCell>                 _cells;

		std::stack<StackItem>                 _stack;
		std::vector<bool>                     _marks;
		std::vector<std::unordered_set<int>>  _visit;

		std::vector<std::vector<DualSeg*>>    _duals;
		std::vector<std::vector<DualSeg*>>    _borders;

	public:
		Voronoi2D();
		~Voronoi2D();

		void clear_domain() { _domain.clear(); }
		void add_domain(const Real *polygon, int n);
		void compute(const Delaunay *dt);
		void compute(const Delaunay *dt, int v);

		int cells_number() const { return (int)_cells.size(); }
		std::vector<PolyCell>& cells() { return _cells; }
		const PolyCell* cell(int v) const { return &_cells[v]; }

	protected:
		// stack
		void visit(int v, int s);
		bool is_visited(int v, int s);
		void stack_push(int v, int s, bool check = true);

		void clip(int v, int bf, int bs);
		void fill_cell(const Delaunay *dt, int v);

		void clip(int bf, int bs, std::vector<DualSeg*> &duals, std::vector<DualSeg*> &borders);
	};

	template <typename Delaunay, typename Real>
	Voronoi2D<Delaunay, Real>::Voronoi2D()
	{
	}

	template <typename Delaunay, typename Real>
	Voronoi2D<Delaunay, Real>::~Voronoi2D()
	{
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::add_domain(const Real *polygon, int n)
	{
		int count = _domain.points_number();

		_domain.begin_face();

		for (int i = 0; i < n; ++i)
		{
			_domain.add_point(&polygon[2 * i], count);
			++count;
		}

		_domain.end_face();
	}

	// stack
	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::visit(int v, int s)
	{
		_visit[v].insert(s);
		_marks[s] = true;
	}

	template <typename Delaunay, typename Real>
	bool Voronoi2D<Delaunay, Real>::is_visited(int v, int s)
	{
		return (_visit[v].find(s) != _visit[v].end());
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::stack_push(int v, int s, bool check)
	{
		if (check && is_visited(v, s))
			return;

		visit(v, s);
		_stack.push(StackItem(v, s));
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::compute(const Delaunay *dt)
	{
		if (!dt)
			return;

		int vnb = dt->vertices_number();

		_marks.clear();
		_marks.resize(_domain.points_number(), false);
		_visit.clear();
		_visit.resize(vnb, std::unordered_set<int>());

		_cells.clear();
		_cells.resize(vnb);
		_duals.clear();
		_duals.resize(vnb, std::vector<DualSeg*>());
		_borders.clear();
		_borders.resize(vnb, std::vector<DualSeg*>());

		int fnb = _domain.faces_number();
		for (int f = 0; f < fnb; ++f)
		{
			for (int s = _domain.face_begin(f);	s < _domain.face_end(f); ++s)
			{
				if (_marks[s])
					continue;

				const Real *bSegSource = _domain.point(s);
				int v = dt->nearest_vertex_index(bSegSource);
				stack_push(v, s);

				while (!_stack.empty())
				{
					int tempv = _stack.top().vertexID;
					int temps = _stack.top().segmentID;
					_stack.pop();

					if (_duals[tempv].empty())
						dt->compute_dual(tempv, _duals[tempv]);

					clip(tempv, f, temps);
				}
			}
		}

		for (int i = 0; i < vnb; ++i)
		{
			fill_cell(dt, i);
		}
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::clip(int v, int bf, int bs)
	{
		if (_duals[v].empty())
			return;

		int nexts = _domain.next_around_face(bf, bs);
		const Real *bSegSource = _domain.point(bs);
		const Real *bSegTarget = _domain.point(nexts);

		Real q1[2] = { bSegSource[0], bSegSource[1] };
		Real q2[2] = { bSegTarget[0], bSegTarget[1] };
		Real qvec[2] = { q2[0] - q1[0], q2[1] - q1[1] };

		DualSeg *result = new DualSeg(q1, q2);
		result->set_flag(-bs - 1);

		bool isBorderSegmentOutsideCell = false;
		bool hasIntersection = false;

		int secCount = 0;
		int snb = (int)_duals[v].size();
		Real ip[2] = { 0.0 };

		for (int i = 0; i < snb; ++i)
		{
			if (secCount == 2)
				break;

			const Real *p1 = _duals[v][i]->source();
			const Real *p2 = _duals[v][i]->target();
			Real segvec[2] = { p2[0] - p1[0], p2[1] - p1[1] };
			Real crossval = cross(segvec, qvec);
			bool has = false;
			if (crossval >= 0)
			{
				// p1 is on q1q2
				if (on_segment(q1, q2, p1))
					continue;
				else if (on_segment(q1, q2, p2))
				{
					_duals[v][i]->set_next_segment(result);
					result->set_prev_segment(_duals[v][i]);

					q1[0] = p2[0];
					q1[1] = p2[1];

					has = true;
				}
				else if (has_intersection_between_segments(p1, p2, q1, q2, ip))
				{
					DualSeg *nextDualSegment = _duals[v][i]->next_segment();

					DualSeg *clippedSegment = new DualSeg(*(_duals[v][i]));
					clippedSegment->set_source(ip);
					clippedSegment->set_prev_segment(NULL);
					_duals[v].push_back(clippedSegment);

					if (nextDualSegment)
						nextDualSegment->set_prev_segment(clippedSegment);

					_duals[v][i]->set_target(ip);
					_duals[v][i]->set_next_segment(result);
					result->set_prev_segment(_duals[v][i]);

					q1[0] = ip[0];
					q1[1] = ip[1];

					has = true;
				}
			}
			else
			{
				// p1 is on q1q2
				if (on_segment(q1, q2, p2))
					continue;
				else if (on_segment(q1, q2, p1))
				{
					_duals[v][i]->set_prev_segment(result);
					result->set_next_segment(_duals[v][i]);

					q2[0] = p1[0];
					q2[1] = p1[1];

					has = true;
				}
				else if (has_intersection_between_segments(p1, p2, q1, q2, ip))
				{
					DualSeg *prevDualSegment = _duals[v][i]->prev_segment();

					DualSeg *clippedSegment = new DualSeg(*(_duals[v][i]));
					clippedSegment->set_target(ip);
					clippedSegment->set_next_segment(NULL);
					_duals[v].push_back(clippedSegment);

					if (prevDualSegment)
						prevDualSegment->set_next_segment(clippedSegment);

					_duals[v][i]->set_source(ip);
					_duals[v][i]->set_prev_segment(result);
					result->set_next_segment(_duals[v][i]);

					q2[0] = ip[0];
					q2[1] = ip[1];

					has = true;
				}
			}

			if (has)
			{
				++secCount;
				hasIntersection = true;

				int nvid = _duals[v][i]->flag();
				if (nvid > -1)
					stack_push(nvid, bs);
			}
			else if (!hasIntersection && !isBorderSegmentOutsideCell)
			{
				Real q2source[2] = { p1[0] - q1[0], p1[1] - q1[1] };
				Real q2target[2] = { p2[0] - q1[0], p2[1] - q1[1] };

				if (cross(q2source, q2target) < 0.0)
					isBorderSegmentOutsideCell = true;
			}
		}

		if (fabs(q1[0] - q2[0]) < DBL_EPSILON && fabs(q1[1] - q2[1]) < DBL_EPSILON)
		{
			DualSeg *prev = result->prev_segment();
			DualSeg *next = result->next_segment();

			if (prev)
				prev->set_next_segment(next);
			if (next)
				next->set_prev_segment(prev);

			delete result;
			result = NULL;
			return;
		}

		if (hasIntersection)
		{
			if (_borders[v].empty())
				_borders[v].resize(_domain.points_number(), NULL);

			result->set_source(q1);
			result->set_target(q2);
			_borders[v][bs] = result;

			return;
		}

		// no intersection
		if (isBorderSegmentOutsideCell)
		{
			delete result;
			result = NULL;
			return;
		}

		if (_borders[v].empty())
			_borders[v].resize(_domain.points_number(), NULL);

		result->set_source(q1);
		result->set_target(q2);
		_borders[v][bs] = result;

		int prevs = _domain.prev_around_face(bf, bs);

		stack_push(v, nexts);
		stack_push(v, prevs);
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::fill_cell(const Delaunay *dt, int v)
	{
		_cells[v].clear();

		if (!_borders[v].empty())
		{
			std::unordered_map<DualSeg*, bool> in;
			for (int f = 0; f < _domain.faces_number(); ++f)
			{
				for (int i = _domain.face_begin(f); i < _domain.face_end(f); ++i)
				{
					DualSeg *s = _borders[v][i];
					if (s == NULL)
						continue;

					if (in[s])
						continue;

					_cells[v].begin_face();

					DualSeg *temp = s;
					do
					{
						_cells[v].add_point(temp->source(), temp->flag());
						in[temp] = true;

						DualSeg *next = temp->next_segment();
						if (!next)
						{
							int nexti = _domain.next_around_face(f, -temp->flag() - 1);
							assert(nexti > -1);
							next = _borders[v][nexti];
						}

						temp = next;

					} while (temp != s);

					_cells[v].end_face();
				}
			}

			int nb = (int)_duals[v].size();
			for (int i = 0; i < nb; ++i)
			{
				delete _duals[v][i];
				_duals[v][i] = NULL;
			}
			_duals[v].clear();

			for (auto it = _borders[v].begin(); it != _borders[v].end(); ++it)
			{
				delete *it;
			}
			_borders[v].clear();

			return;
		}

		if (!_duals[v].empty())
		{
			_cells[v].begin_face();

			int nb = (int)_duals[v].size();
			for (int i = 0; i < nb; ++i)
			{
				_cells[v].add_point(_duals[v][i]->source(), _duals[v][i]->flag());

				delete _duals[v][i];
				_duals[v][i] = NULL;
			}
			_duals[v].clear();

			_cells[v].end_face();
		}
		else
			dt->compute_dual(v, _cells[v]);
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::compute(const Delaunay *dt, int v)
	{
		if (!dt)
			return;

		std::vector<DualSeg*> duals;
		dt->compute_dual(v, duals);

		std::vector<DualSeg*> borders;

		int fnb = _domain.faces_number();
		for (int f = 0; f < fnb; ++f)
		{
			for (int s = _domain.face_begin(f); s < _domain.face_end(f); ++s)
			{
				clip(f, s, duals, borders);
			}
		}

		_cells[v].clear();

		if (!borders.empty())
		{
			std::unordered_map<DualSeg*, bool> in;
			for (int f = 0; f < _domain.faces_number(); ++f)
			{
				for (int i = _domain.face_begin(f); i < _domain.face_end(f); ++i)
				{
					DualSeg *s = borders[i];
					if (s == NULL)
						continue;

					if (in[s])
						continue;

					_cells[v].begin_face();

					DualSeg *temp = s;
					do
					{
						_cells[v].add_point(temp->source(), temp->flag());
						in[temp] = true;

						DualSeg *next = temp->next_segment();
						if (!next)
						{
							int nexti = _domain.next_around_face(f, -temp->flag() - 1);
							assert(nexti > -1);
							next = borders[nexti];
						}

						temp = next;

					} while (temp != s);

					_cells[v].end_face();
				}
			}

			int nb = (int)duals.size();
			for (int i = 0; i < nb; ++i)
			{
				delete duals[i];
				duals[i] = NULL;
			}
			duals.clear();

			for (auto it = borders.begin(); it != borders.end(); ++it)
			{
				if (*it)
					delete *it;
			}
			borders.clear();

			return;
		}

		_cells[v].begin_face();

		int nb = (int)duals.size();
		for (int i = 0; i < nb; ++i)
		{
			_cells[v].add_point(duals[i]->source(), duals[i]->flag());

			delete duals[i];
			duals[i] = NULL;
		}
		duals.clear();

		_cells[v].end_face();
	}

	template <typename Delaunay, typename Real>
	void Voronoi2D<Delaunay, Real>::clip(
		int bf, int bs, 
		std::vector<DualSeg*> &duals, 
		std::vector<DualSeg*> &borders)
	{
		if (duals.empty())
			return;

		int nexts = _domain.next_around_face(bf, bs);
		const Real *bSegSource = _domain.point(bs);
		const Real *bSegTarget = _domain.point(nexts);

		Real q1[2] = { bSegSource[0], bSegSource[1] };
		Real q2[2] = { bSegTarget[0], bSegTarget[1] };
		Real qvec[2] = { q2[0] - q1[0], q2[1] - q1[1] };

		DualSeg *result = new DualSeg(q1, q2);
		result->set_flag(-bs - 1);

		bool isBorderSegmentOutsideCell = false;
		bool hasIntersection = false;

		int secCount = 0;
		int snb = (int)duals.size();
		Real ip[2] = { 0.0 };

		for (int i = 0; i < snb; ++i)
		{
			if (secCount == 2)
				break;

			const Real *p1 = duals[i]->source();
			const Real *p2 = duals[i]->target();
			Real segvec[2] = { p2[0] - p1[0], p2[1] - p1[1] };
			Real crossval = cross(segvec, qvec);
			bool has = false;
			if (crossval >= 0)
			{
				// p1 is on q1q2
				if (on_segment(q1, q2, p1))
					continue;
				else if (on_segment(q1, q2, p2))
				{
					duals[i]->set_next_segment(result);
					result->set_prev_segment(duals[i]);

					q1[0] = p2[0];
					q1[1] = p2[1];

					has = true;
				}
				else if (has_intersection_between_segments(p1, p2, q1, q2, ip))
				{
					DualSeg *nextDualSegment = duals[i]->next_segment();

					DualSeg *clippedSegment = new DualSeg(*(duals[i]));
					clippedSegment->set_source(ip);
					clippedSegment->set_prev_segment(NULL);
					duals.push_back(clippedSegment);

					if (nextDualSegment)
						nextDualSegment->set_prev_segment(clippedSegment);

					duals[i]->set_target(ip);
					duals[i]->set_next_segment(result);
					result->set_prev_segment(duals[i]);

					q1[0] = ip[0];
					q1[1] = ip[1];

					has = true;
				}
			}
			else
			{
				// p1 is on q1q2
				if (on_segment(q1, q2, p2))
					continue;
				else if (on_segment(q1, q2, p1))
				{
					duals[i]->set_prev_segment(result);
					result->set_next_segment(duals[i]);

					q2[0] = p1[0];
					q2[1] = p1[1];

					has = true;
				}
				else if (has_intersection_between_segments(p1, p2, q1, q2, ip))
				{
					DualSeg *prevDualSegment = duals[i]->prev_segment();

					DualSeg *clippedSegment = new DualSeg(*(duals[i]));
					clippedSegment->set_target(ip);
					clippedSegment->set_next_segment(NULL);
					duals.push_back(clippedSegment);

					if (prevDualSegment)
						prevDualSegment->set_next_segment(clippedSegment);

					duals[i]->set_source(ip);
					duals[i]->set_prev_segment(result);
					result->set_next_segment(duals[i]);

					q2[0] = ip[0];
					q2[1] = ip[1];

					has = true;
				}
			}

			if (has)
			{
				++secCount;
				hasIntersection = true;
			}
			else if (!hasIntersection && !isBorderSegmentOutsideCell)
			{
				Real q2source[2] = { p1[0] - q1[0], p1[1] - q1[1] };
				Real q2target[2] = { p2[0] - q1[0], p2[1] - q1[1] };

				if (cross(q2source, q2target) < 0.0)
					isBorderSegmentOutsideCell = true;
			}
		}

		if (fabs(q1[0] - q2[0]) < DBL_EPSILON && fabs(q1[1] - q2[1]) < DBL_EPSILON)
		{
			DualSeg *prev = result->prev_segment();
			DualSeg *next = result->next_segment();

			if (prev)
				prev->set_next_segment(next);
			if (next)
				next->set_prev_segment(prev);

			delete result;
			result = NULL;
			return;
		}

		if (hasIntersection)
		{
			if (borders.empty())
				borders.resize(_domain.points_number(), NULL);

			result->set_source(q1);
			result->set_target(q2);
			borders[bs] = result;

			return;
		}

		// no intersection
		if (isBorderSegmentOutsideCell)
		{
			delete result;
			result = NULL;
			return;
		}

		if (borders.empty())
			borders.resize(_domain.points_number(), NULL);

		result->set_source(q1);
		result->set_target(q2);
		borders[bs] = result;
	}
}

#endif
