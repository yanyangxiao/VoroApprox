
#include "delaunay2.h"

bool DelaunayTriangulation2D::set_vertices(const double *pos, int vnb)
{
	clear();
	_vertices.clear();
	_vertices.reserve(vnb);

	bool ok = true;

	int index = 0;
	Vertex_handle vh = 0;
	Face_handle fh = 0;
	for (int i = 0; i < vnb; ++i)
	{
		vh = insert(Point(pos[2 * i], pos[2 * i + 1]), fh);
		if (vh == 0)
		{
			ok = false;
			continue;
		}

		fh = vh->face();

		vh->set_index(index);
		_vertices.push_back(vh);

		++index;
	}

	return ok;
}

DelaunayTriangulation2D::Vertex_handle DelaunayTriangulation2D::add_vertex(const double *p)
{
	Vertex_handle vh = insert(Point(p[0], p[1]));
	if (vh != 0)
	{
		vh->set_index((int)_vertices.size());
		_vertices.push_back(vh);
	}

	return vh;
}