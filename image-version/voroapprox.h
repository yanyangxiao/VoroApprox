
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef POLYNOMIAL_APPROXIMATION_ON_VORONOI_H
#define POLYNOMIAL_APPROXIMATION_ON_VORONOI_H

#include "delaunay2.h"
#include "voronoi2.h"
#include "pixelset.h"
#include "polynomial.h"

using namespace xyy;

class VoroApprox
{
	struct Parameters
	{
		const unsigned char *image = NULL;
		int width;
		int height;
		int channel;
		double ratio;
		double pixWidth;
		double pixArea;

		int degree = 1;

		int Lp = 2;
	};

	typedef PolygonCell<double, int> MyPolygonCell;
	typedef Voronoi2D<DelaunayTriangulation2D> MyVoronoi;
	typedef Polynomial<double> MyPolynomial;

protected:
	Parameters                _params;
	std::vector<double>       _sites;
	DelaunayTriangulation2D  *_dt;
	MyVoronoi                *_voro;

	std::vector<PixelSet>     _pixels;
	std::vector<MyPolynomial> _polynomials;
	std::vector<double>       _energies;

public:
	VoroApprox();
	~VoroApprox();

	void set_degree(int d) { _params.degree = d; }

	void set_image(const unsigned char *image, int width, int height, int channel);

	void random_init(int vnb);
	void greedy_init(int vnb);
	void set_sites(const double *sites, int n);

	void compute_voronoi();
	void assign_pixels();
	void compute_polynomials();
	double compute_energies();
	void compute_gradients(double *g, int n);

	void optimize(int degree, int iteration, double stepScale = 0.3);

	void approximate(int degree, unsigned char *output, int width, int height, int channel);

	// data access
	std::vector<double>& sites() { return _sites; }
	void sites_data(std::vector<float> &sites);
	void voronoi_data(std::vector<float> &corners, std::vector<int> &edges);

protected:
	void compute_gradient(
		const double *A,
		const MyPolynomial *polynomialA,
		const double *B,
		const MyPolynomial *polynomialB,
		const double *source,
		const double *target,
		double *result) const;
	void locate_point(const double *p, int &i, int &j) const;
};

#endif
