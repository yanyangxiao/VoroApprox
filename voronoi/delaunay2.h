
#ifndef DELAUNAY_TRIANGULATION_2D_H
#define DELAUNAY_TRIANGULATION_2D_H

#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/Triangulation_face_base_2.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include "dual_segment.h"
#include "polygon_cell.h"

#define TRIANGULATION_2D_INFINITE_DOUBLE 1e10
#define TRIANGULATION_2D_INFINITE_INT    1000000

template <typename K, typename Vbb>
class My_Delaunay_Triangulation_Vertex2D : public Vbb
{
public:
	typedef typename K::FT FT;
	typedef typename K::Point_2     Point;
	typedef typename K::Vector_2    Vector;
	typedef typename K::Segment_2   Segment;

	typedef typename Vbb::Triangulation_data_structure TDS;
	typedef typename TDS::Face_handle   Face_handle;
	typedef typename TDS::Vertex_handle Vertex_handle;

	template < typename TDS2 >
	struct Rebind_TDS
	{
		typedef typename Vbb::template Rebind_TDS<TDS2>::Other Vb2;
		typedef My_Delaunay_Triangulation_Vertex2D<K, Vb2> Other;
	};

protected:
	int _index;

public:
	My_Delaunay_Triangulation_Vertex2D()
		: Vbb(), _index(-1)
	{
	}

	My_Delaunay_Triangulation_Vertex2D(const Point& p)
		: Vbb(p), _index(-1)
	{
	}

	My_Delaunay_Triangulation_Vertex2D(const Point& p, Face_handle f)
		: Vbb(p, f), _index(-1)
	{
	}

	My_Delaunay_Triangulation_Vertex2D(const My_Delaunay_Triangulation_Vertex2D &rhs)
		: Vbb(rhs), _index(rhs._index)
	{

	}

	~My_Delaunay_Triangulation_Vertex2D()
	{
	}

	My_Delaunay_Triangulation_Vertex2D& operator= (const My_Delaunay_Triangulation_Vertex2D &rhs)
	{
		if (this != &rhs)
		{
			Vbb::operator= (rhs);
			_index = rhs._index;
		}

		return *this;
	}

	inline int index() const { return _index; }
	inline void set_index(int i) { _index = i; }
};

// kernel
typedef CGAL::Exact_predicates_inexact_constructions_kernel                                    Kernel;

// Vertex
typedef CGAL::Triangulation_vertex_base_2<Kernel>                                              CGAL_Delaunay_Triangulation_Vertex_Base_2;
typedef My_Delaunay_Triangulation_Vertex2D<Kernel, CGAL_Delaunay_Triangulation_Vertex_Base_2>  Delaunay_Triangulation_Vertex2D;

// Triangulation
typedef CGAL::Triangulation_data_structure_2<Delaunay_Triangulation_Vertex2D>                  Delaunay_Triangulation_Data_Structure_2;
typedef CGAL::Delaunay_triangulation_2<Kernel, Delaunay_Triangulation_Data_Structure_2>        CGAL_Delaunay_Triangulation2D;

using namespace xyy;

class DelaunayTriangulation2D : public CGAL_Delaunay_Triangulation2D
{
private:
	typedef CGAL_Delaunay_Triangulation2D      superclass;
	typedef DelaunayTriangulation2D            thisclass;

public:
	typedef typename thisclass::Geom_traits    Kernel;
	typedef typename Kernel::Point_2           Point;
	typedef typename Kernel::Vector_2          Vector;
	typedef typename Kernel::Ray_2             Ray;
	typedef typename Kernel::Line_2            Line;
	typedef typename Kernel::Segment_2         Segment;
	typedef typename Kernel::Triangle_2        Triangle;

	typedef typename thisclass::Vertex                   Vertex;
	typedef typename thisclass::Vertex_handle            Vertex_handle;
	typedef typename thisclass::Vertex_iterator          Vertex_iterator;
	typedef typename thisclass::Vertex_circulator        Vertex_circulator;
	typedef typename thisclass::Finite_vertices_iterator Finite_vertices_iterator;

	typedef typename thisclass::Edge                  Edge;
	typedef typename thisclass::Edge_iterator         Edge_iterator;
	typedef typename thisclass::Edge_circulator       Edge_circulator;
	typedef typename thisclass::Finite_edges_iterator Finite_edges_iterator;

	typedef typename thisclass::Face                  Face;
	typedef typename thisclass::Face_handle           Face_handle;
	typedef typename thisclass::Face_iterator         Face_iterator;
	typedef typename thisclass::Face_circulator       Face_circulator;
	typedef typename thisclass::Finite_faces_iterator Finite_faces_iterator;

protected:
	std::vector<Vertex_handle> _vertices;

public:
	int vertices_number() const
	{
		return (int)_vertices.size();
	}

	Vertex_handle vertex_handle(int i) const
	{
		return _vertices[i];
	}

	Vertex_handle source_vertex(const Edge &e) const
	{
		return e.first->vertex(thisclass::ccw(e.second));
	}

	Vertex_handle target_vertex(const Edge &e) const
	{
		return e.first->vertex(thisclass::cw(e.second));
	}

	int nearest_vertex_index(const double *query) const
	{
		return nearest_vertex(Point(query[0], query[1]))->index();
	}

	bool set_vertices(const double *pos, int vnb);
	Vertex_handle add_vertex(const double *p);

	template <typename Real, typename Flag>
	void compute_dual(int v, std::vector<DualSegment<Real, Flag>*> &segments) const;

	template <typename Real, typename Flag>
	void compute_dual(int v, PolygonCell<Real, Flag> &cell) const;
};

template <typename Real, typename Flag>
void DelaunayTriangulation2D::compute_dual(int v, std::vector<DualSegment<Real, Flag>*> &segments) const
{
	segments.clear();

	const Point& pt = _vertices[v]->point();	

	int start = -1, end = -1;

	Edge_circulator ecirc = incident_edges(_vertices[v]);
	Edge_circulator eend = ecirc;
	CGAL_For_all(ecirc, eend)
	{
		if (is_infinite(ecirc))
			continue;

		Edge e(*ecirc);
		Vertex_handle nhv = source_vertex(e);
		Point src, tgt;

		Face_handle leftFace = e.first;
		Face_handle rightFace = leftFace->neighbor(e.second);
		bool leftInfinite = is_infinite(leftFace);
		bool rightInfinite = is_infinite(rightFace);
		// can not be both infinite
		if (leftInfinite)
		{
			start = (int)segments.size();

			tgt = dual(rightFace);

			const Point& ps = nhv->point();
			Vector st90(ps.y() - pt.y(), pt.x() - ps.x());
			Ray r(tgt, st90);

			src = r.point(TRIANGULATION_2D_INFINITE_DOUBLE);
		}
		else if (rightInfinite)
		{
			end = (int)segments.size();

			src = dual(leftFace);

			const Point& ps = nhv->point();
			Vector st90(pt.y() - ps.y(), ps.x() - pt.x());
			Ray r(src, st90);
			tgt = r.point(TRIANGULATION_2D_INFINITE_DOUBLE);
		}
		else
		{
			src = dual(leftFace);
			tgt = dual(rightFace);
		}

		DualSegment<Real, Flag> *obj = new DualSegment<Real, Flag>(&src.x(), &tgt.x(), (Flag)nhv->index());
		segments.push_back(obj);
	}

	int snb = (int)segments.size();
	for (int i = 0; i < snb; ++i)
	{
		int j = (i + 1) % snb;
		int k = (i - 1 + snb) % snb;

		if (i != end)
			segments[i]->set_next_segment(segments[j]);
		if (i != start)
			segments[i]->set_prev_segment(segments[k]);
	}
}

template <typename Real, typename Flag>
void DelaunayTriangulation2D::compute_dual(int v, PolygonCell<Real, Flag> &cell) const
{
	cell.clear();
	
	const Point& pt = _vertices[v]->point();
	
	cell.begin_face();

	Edge_circulator ecirc = incident_edges(_vertices[v]);
	Edge_circulator eend = ecirc;
	CGAL_For_all(ecirc, eend)
	{
		if (is_infinite(ecirc))
			continue;

		Edge e(*ecirc);
		Vertex_handle nhv = source_vertex(e);

		Face_handle leftFace = e.first;
		Face_handle rightFace = leftFace->neighbor(e.second);
		bool leftInfinite = is_infinite(leftFace);
		bool rightInfinite = is_infinite(rightFace);
		// can not be both infinite
		if (leftInfinite)
		{
			Point rcw(dual(rightFace));
			const Point& ps = nhv->point();

			Vector st90(ps.y() - pt.y(), pt.x() - ps.x());
			Ray r(rcw, st90);
			Point rp(r.point(TRIANGULATION_2D_INFINITE_DOUBLE));
			cell.add_point(&rp.x(), nhv->index());
		}
		else if (rightInfinite)
		{
			Point lcw(dual(leftFace));
			const Point& ps = nhv->point();

			Vector st90(pt.y() - ps.y(), ps.x() - pt.x());
			Ray r(lcw, st90);
			Point rp(r.point(TRIANGULATION_2D_INFINITE_DOUBLE));

			cell.add_point(&lcw.x(), nhv->index());
			cell.add_point(&rp.x(), -TRIANGULATION_2D_INFINITE_INT);
		}
		else
		{
			Point lcw(dual(leftFace));
			cell.add_point(&lcw.x(), nhv->index());
		}
	}

	cell.end_face();
}

#endif
