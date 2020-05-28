
#include <map>
#include "voroapprox.h"
#include "rasterizer.h"
#include "../xlog.h"

VoroApprox::VoroApprox() : _dt(NULL), _voro(NULL)
{ }

VoroApprox::~VoroApprox()
{
	if (_dt)
	{
		delete _dt;
		_dt = NULL;
	}

	if (_voro)
	{
		delete _voro;
		_voro = NULL;
	}
}

void VoroApprox::set_image(const unsigned char *image, int width, int height, int channel)
{
	if (_voro)
	{
		delete _voro;
		_voro = NULL;
	}

	_params.image = image;
	_params.width = width;
	_params.height = height;
	_params.channel = channel;

	_params.ratio = double(height) / width;
	_params.pixWidth = 2.0 / width;
	_params.pixArea = _params.pixWidth * _params.pixWidth;
}

void VoroApprox::random_init(int vnb)
{
	if (!_params.image)
		return;
	
	srand((unsigned int)time(NULL));

	_sites.clear();
	_sites.resize(2 * vnb);

	for (int i = 0; i < vnb; ++i)
	{
		_sites[2 * i] = double(rand()) / RAND_MAX * 2.0 - 1.0;
		_sites[2 * i + 1] = (double(rand()) / RAND_MAX * 2.0 - 1.0) * _params.ratio;
	}

	compute_voronoi();
}

void VoroApprox::greedy_init(int vnb)
{
	int initN = 3;
	random_init(initN);

	assign_pixels();
	compute_polynomials();
	compute_energies();

	std::multimap<double, int, std::greater<double>> mMap;
	for (int i = 0; i < initN; ++i)
	{
		mMap.insert(std::make_pair(_energies[i], i));
	}

	while (_sites.size() < 2 * vnb && !mMap.empty())
	{
		auto top = mMap.begin();
		int v = top->second;
		mMap.erase(top);

		const MyPolygonCell *cell = _voro->cell(v);
		assert(cell);

		const double *maxP = NULL;
		double maxDist = 0;

		for (int i = cell->face_begin(0); i < cell->face_end(0); ++i)
		{
			const double *p = cell->point(i);
			double dx = p[0] - _sites[2 * v];
			double dy = p[1] - _sites[2 * v + 1];
			double dist = dx * dx + dy * dy;

			if (dist > maxDist)
			{
				maxDist = dist;
				maxP = p;
			}
		}

		_sites.push_back(maxP[0]);
		_sites.push_back(maxP[1]);

		DelaunayTriangulation2D::Vertex_handle newVH = _dt->add_vertex(maxP);
		int newVID = newVH->index();

		std::vector<int> updateList;
		DelaunayTriangulation2D::Vertex_circulator vvit = _dt->incident_vertices(newVH);
		DelaunayTriangulation2D::Vertex_circulator vvend = vvit;
		CGAL_For_all(vvit, vvend)
		{
			if (_dt->is_infinite(vvit))
				continue;

			updateList.push_back(vvit->index());
			_voro->compute(_dt, vvit->index());
		}

		for (auto it = updateList.begin(); it != updateList.end(); ++it)
		{
			for (auto mit = mMap.lower_bound(_energies[*it]); mit != mMap.upper_bound(_energies[*it]); ++mit)
			{
				if (mit->second == *it)
				{
					mMap.erase(mit);
					break;
				}
			}
		}

		updateList.push_back(newVID);

		_voro->cells().push_back(MyPolygonCell());
		_voro->compute(_dt, newVID);

		_pixels.push_back(PixelSet());
		_polynomials.push_back(MyPolynomial());
		_energies.push_back(0);

		for (auto it = updateList.begin(); it != updateList.end(); ++it)
		{
			cell = _voro->cell(*it);

			std::vector<double> polygon;
			cell->face_polygon(0, polygon);
			Rasterizer::rasterize(&polygon[0], (int)polygon.size() / 2, _params.width, _params.height, _pixels[*it]);

			_polynomials[*it].compute_factors(
				_params.image,
				_params.width,
				_params.height,
				_params.channel,
				&_pixels[*it]);

			_energies[*it] = _polynomials[*it].compute_energy(
				_params.image,
				_params.width,
				_params.height,
				_params.channel,
				&_pixels[*it],
				_params.Lp);

			mMap.insert(std::make_pair(_energies[*it], *it));
		}
	}
}

void VoroApprox::set_sites(const double *sites, int n)
{
	_sites.clear();
	_sites.resize(n * 2);

	memcpy(&_sites[0], sites, sizeof(double) * n * 2);

	compute_voronoi();
}

void VoroApprox::compute_voronoi()
{
	if (_sites.empty() || !_params.image)
		return;

	if (!_dt)
		_dt = new DelaunayTriangulation2D;

	_dt->set_vertices(&_sites[0], (int)_sites.size() / 2);

	if (!_voro)
	{
		_voro = new MyVoronoi;

		double rect[8] = { -1, -_params.ratio, 1, -_params.ratio, 1, _params.ratio, -1, _params.ratio };
		_voro->add_domain(rect, 4);
	}

	_voro->compute(_dt);
}

void VoroApprox::assign_pixels()
{
	if (!_voro)
		return;

	int vnb = _voro->cells_number();

	_pixels.clear();
	_pixels.resize(vnb);

	for (int i = 0; i < vnb; ++i)
	{
		const MyPolygonCell *cell = _voro->cell(i);
		if (!cell)
			continue;

		std::vector<double> polygon;
		cell->face_polygon(0, polygon);
		Rasterizer::rasterize(&polygon[0], (int)polygon.size() / 2, _params.width, _params.height, _pixels[i]);
	}
}

void VoroApprox::compute_polynomials()
{
	if (!_voro || !_params.image || _pixels.empty())
		return;

	_polynomials.clear();
	_polynomials.resize(_pixels.size(), MyPolynomial(_params.degree));

	int vnb = _voro->cells_number();
	for (int i = 0; i < vnb; ++i)
	{
		_polynomials[i].compute_factors(
			_params.image,
			_params.width,
			_params.height,
			_params.channel,
			&_pixels[i]);
	}
}

double VoroApprox::compute_energies()
{
	if (!_voro || !_params.image || _pixels.empty() || _polynomials.empty())
		return 1e10;

	double sum = 0.0;

	int vnb = _voro->cells_number();
	_energies.clear();
	_energies.resize(vnb, 0.0);
	for (int i = 0; i < vnb; ++i)
	{
		_energies[i] = _polynomials[i].compute_energy(
			_params.image,
			_params.width,
			_params.height,
			_params.channel,
			&_pixels[i],
			_params.Lp);

		sum += _energies[i];
	}

	return sum;
}

void VoroApprox::compute_gradients(double *g, int n)
{
	if (!_params.image || !_voro)
		return;

	assert(n == _voro->cells_number());

	for (int v = 0; v < n; ++v)
	{
		g[2 * v] = 0.0;
		g[2 * v + 1] = 0.0;

		const MyPolygonCell *cell = _voro->cell(v);
		if (!cell)
			continue;

		for (int i = cell->face_begin(0); i < cell->face_end(0); ++i)
		{
			int nv = cell->point_flag(i);
			if (nv < 0)
				continue;

			int next = cell->next_around_face(0, i);
			const double *source = cell->point(i);
			const double *target = cell->point(next);

			double result[2] = { 0.0, 0.0 };
			compute_gradient(
				&_sites[2 * v],
				&_polynomials[v],
				&_sites[2 * nv],
				&_polynomials[nv],
				source,
				target,
				result);

			g[2 * v] += result[0];
			g[2 * v + 1] += result[1];
		}
	}
}

void VoroApprox::compute_gradient(
	const double *A,
	const MyPolynomial *polynomialA,
	const double *B,
	const MyPolynomial *polynomialB,
	const double *source,
	const double *target,
	double *result) const
{
	result[0] = 0.0;
	result[1] = 0.0;

	double dx = source[0] - target[0];
	double dy = source[1] - target[1];
	double length = std::sqrt(dx * dx + dy * dy);
	int n = int(length / _params.pixWidth + 0.5);
	//n *= 5;

	double ds = length / n;
	int n2 = 2 * n;
	for (int s = 1; s < n2; s += 2)
	{
		double lamda = double(s) / n2;
		double p[2];
		p[0] = source[0] * (1.0 - lamda) + target[0] * lamda;
		p[1] = source[1] * (1.0 - lamda) + target[1] * lamda;

		int i = -1, j = -1;
		locate_point(p, i, j);

		int pixID = j * _params.width + i;
		const unsigned char* pixColor = &_params.image[_params.channel * pixID];

		double energyA = 0.0, energyB = 0.0;
		for (int c = 0; c < _params.channel; ++c)
		{
			double diff = double(pixColor[c]) - polynomialA->evaluate(c, p);
			energyA += diff * diff;

			diff = double(pixColor[c]) - polynomialB->evaluate(c, p);
			energyB += diff * diff;
		}

		double energyDiff = energyA - energyB;
		double vec[2] = { p[0] - A[0], p[1] - A[1] };

		result[0] += energyDiff * vec[0] * ds;
		result[1] += energyDiff * vec[1] * ds;
	}

	dx = B[0] - A[0];
	dy = B[1] - A[1];
	length = std::sqrt(dx * dx + dy * dy);
	result[0] /= length;
	result[1] /= length;
}

void VoroApprox::locate_point(const double *p, int &i, int &j) const
{
	double x = 0.5 * (p[0] + 1.0) / 1.0;
	double y = 0.5 * (p[1] + _params.ratio) / _params.ratio;

	x = (std::max)(0.0, x);
	x = (std::min)(1.0, x);
	y = (std::max)(0.0, y);
	y = (std::min)(1.0, y);

	i = (int)floor(x * double(_params.width));
	j = (int)floor(y * double(_params.height));

	if (i == _params.width) --i;
	if (j == _params.height) --j;
}

void VoroApprox::optimize(int degree, int iteration, double stepScale /* = 0.3*/)
{
	if (!_params.image || !_voro)
		return;

	_params.degree = degree;

	int vnb = _voro->cells_number();
	std::vector<double> steps(vnb, 0.0);
	for (int i = 0; i < vnb; ++i)
	{
		const MyPolygonCell *cell = _voro->cell(i);
		if (!cell)
			continue;

		steps[i] = std::sqrt(cell->face_area(0)) * stepScale;
	}
	
	assign_pixels();
	compute_polynomials();
	double sumEnergy = compute_energies();
	xlog("init energy = %f", sumEnergy);
	
	double sigma = 0.5;
	std::vector<double> gradient(2 * vnb, 0.0);
	for (int it = 0; it < iteration; ++it)
	{
		compute_gradients(&gradient[0], vnb);

		double ri = double(it) / double(iteration - it);
		
		for (int v = 0; v < vnb; ++v)
		{
			double gnorm = 0.0;
			gnorm += gradient[2 * v] * gradient[2 * v];
			gnorm += gradient[2 * v + 1] * gradient[2 * v + 1];
			gnorm = std::sqrt(gnorm);

			if (gnorm == 0.0)
				continue;

			gradient[2 * v] /= gnorm;
			gradient[2 * v + 1] /= gnorm;

			double delta = steps[v] * std::pow(sigma, ri);

			_sites[2 * v] -= delta * gradient[2 * v];
			_sites[2 * v + 1] -= delta * gradient[2 * v + 1];

			// clamp
			if (_sites[2 * v] < -1.0) _sites[2 * v] = -1.0;
			if (_sites[2 * v] > 1.0)  _sites[2 * v] = 1.0;
			if (_sites[2 * v + 1] < -_params.ratio) _sites[2 * v + 1] = -_params.ratio;
			if (_sites[2 * v + 1] > _params.ratio) _sites[2 * v + 1] = _params.ratio;
		}
		
		compute_voronoi();
		assign_pixels();
		compute_polynomials();
		sumEnergy = compute_energies();
		xlog("it = %d, energy = %f", it + 1, sumEnergy);
	}
}

void VoroApprox::approximate(int degree, unsigned char *output, int width, int height, int channel)
{
	if (!_params.image || !_voro || !output || channel > _params.channel)
		return;

	memset(output, 0, sizeof(unsigned char) * width * height * channel);

	_params.degree = degree;

	assign_pixels();
	compute_polynomials();

	int vnb = _voro->cells_number();

	_pixels.clear();
	_pixels.resize(vnb);

	for (int v = 0; v < vnb; ++v)
	{
		const MyPolygonCell *cell = _voro->cell(v);
		if (!cell)
			continue;

		std::vector<double> polygon;
		cell->face_polygon(0, polygon);
		Rasterizer::rasterize(&polygon[0], (int)polygon.size() / 2, width, height, _pixels[v]);
	}

	std::vector<double> approx(width * height * channel, 0.0);
	std::vector<int> counts(width * height, 0);

	double ratio = double(height) / width;
	double pixWidth = double(2.0) / width;

	for (int v = 0; v < vnb; ++v)
	{
		for (int j = _pixels[v].ymin; j <= _pixels[v].ymax; ++j)
		{
			double y = pixWidth * (j + 0.5) - ratio;
			int lineStart = j * width;

			int loc = j - _pixels[v].ymin;
			for (int i = _pixels[v].left[loc]; i <= _pixels[v].right[loc]; ++i)
			{
				double x = pixWidth * (i + 0.5) - 1.0;
				int pixID = lineStart + i;

				for (int c = 0; c < channel; ++c)
				{
					approx[pixID * channel + c] += _polynomials[v].evaluate(c, x, y);
				}

				++counts[pixID];
			}
		}
	}

	for (int j = 0; j < height; ++j)
	{
		double y = pixWidth * (j + 0.5) - ratio;
		int lineStart = j * width;

		for (int i = 0; i < width; ++i)
		{
			double x = pixWidth * (i + 0.5) - 1.0;
			int pixID = lineStart + i;

			if (counts[pixID] < 1)
			{
				continue;
			}

			for (int c = 0; c < channel; ++c)
			{
				double avg = approx[pixID * channel + c] / counts[pixID];
				if (avg > 255) avg = 255;
				if (avg < 0) avg = 0;

				output[pixID * channel + c] = unsigned char(avg);
			}
		}
	}
}

void VoroApprox::sites_data(std::vector<float> &sites)
{
	sites.clear();

	for (auto it = _sites.begin(); it != _sites.end(); ++it)
		sites.push_back(float(*it));
}

void VoroApprox::voronoi_data(std::vector<float> &corners, std::vector<int> &edges)
{
	corners.clear();
	edges.clear();

	if (!_voro)
		return;

	int start = 0;
	int vnb = _voro->cells_number();

	double shrink = 1.0;

	for (int i = 0; i < vnb; ++i)
	{
		const MyPolygonCell *cell = _voro->cell(i);

		for (int f = 0; f < cell->faces_number(); ++f)
		{
			int fsize = cell->face_size(f);
			for (int v = cell->face_begin(f), j = 0; v < cell->face_end(f); ++v, ++j)
			{
				const double *p = cell->point(v);
				corners.push_back(float(p[0] * shrink + _sites[2 * i] * (1.0 - shrink)));
				corners.push_back(float(p[1] * shrink + _sites[2 * i + 1] * (1.0 - shrink)));

				int k = (j + 1) % fsize;
				edges.push_back(start + j);
				edges.push_back(start + k);
			}

			start += fsize;
		}
	}
}