
#include "polynomial.h"
#include "matrix.h"

namespace xyy
{
	Polynomial::Polynomial(int d)
		: _degree(d)
	{
		_coeff.clear();
	}

	Polynomial::Polynomial(const Polynomial &rhs)
		: _degree(rhs._degree)
	{
		_coeff = rhs._coeff;
	}

	Polynomial& Polynomial::operator= (const Polynomial &rhs)
	{
		_degree = rhs._degree;
		_coeff = rhs._coeff;

		return *this;
	}

	void Polynomial::set_degree(int d)
	{
		static int tab[3] = { 1, 3, 6 };
		_degree = d;
		_coeff.clear();
		_coeff.resize(tab[d], 0.0);
	}

	void Polynomial::compute(
		const std::vector<double> &points,
		const std::vector<double> &data,
		const std::vector<double> &areas)
	{
		if (points.empty() ||
			points.size() != 2 * data.size() ||
			data.size() != areas.size())
			return;
		
		switch (_degree)
		{
		case 0:
			compute_constant(data, areas);
			break;
		case 1:
			compute_linear(points, data, areas);
			break;
		case 2:
			compute_quadratic(points, data, areas);
			break;
		default:
			break;
		}
	}

	void Polynomial::compute_constant(
		const std::vector<double> &data,
		const std::vector<double> &areas)
	{
		_coeff.clear();
		_coeff.resize(1, 0.0);

		int nb((int)data.size());

		double sumData(0.0);
		double sumArea(0.0);
		for (int i = 0; i < nb; ++i)
		{
			sumData += double(data[i] * areas[i]);
			sumArea += areas[i];
		}

		if (sumArea != 0.0)
			_coeff[0] = sumData / sumArea;
	}

	void Polynomial::compute_linear(
		const std::vector<double> &points,
		const std::vector<double> &data,
		const std::vector<double> &areas)
	{
		_coeff.clear();
		_coeff.resize(3, 0.0);

		Matrix<3> matA;
		matA.load_zero();
		double vecB[3] = { 0.0, 0.0, 0.0};

		int nb((int)data.size());

		double temp[3] = { 1.0, 1.0, 1.0 };
		for (int i = 0; i < nb; ++i)
		{
			double x(points[2 * i]);
			double y(points[2 * i + 1]);

			temp[2] = areas[i];
			temp[0] = x * temp[2];
			temp[1] = y * temp[2];

			for (int k = 0; k < 3; k++)
			{
				vecB[k] += data[i] * temp[k];

				matA(0, k) += x * temp[k];
				matA(1, k) += y * temp[k];
				matA(2, k) += temp[k];
			}
		}

		if (!solve<3>(matA, vecB, &_coeff[0]))
		{
			if (matA(2, 2) != 0.0)
			{
				_coeff[2] = vecB[2] / matA(2, 2);
			}
		}
	}

	void Polynomial::compute_quadratic(
		const std::vector<double> &points,
		const std::vector<double> &data,
		const std::vector<double> &areas)
	{
		_coeff.clear();
		_coeff.resize(6, 0.0);

		Matrix<6> matA;
		matA.load_zero();
		double vecB[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

		int nb((int)data.size());

		double temp[6] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
		for (int i = 0; i < nb; ++i)
		{
			double x(points[2 * i]);
			double y(points[2 * i + 1]);

			temp[5] = areas[i];
			temp[0] = x * x * temp[5];
			temp[1] = x * y * temp[5];
			temp[2] = y * y * temp[5];
			temp[3] = x * temp[5];
			temp[4] = y * temp[5];

			for (int k = 0; k < 6; ++k)
			{
				vecB[k] += data[i] * temp[k];

				matA(0, k) += x * x * temp[k];
				matA(1, k) += x * y * temp[k];
				matA(2, k) += y * y * temp[k];
				matA(3, k) += x * temp[k];
				matA(4, k) += y * temp[k];
				matA(5, k) += temp[k];
			}
		}

		if (!solve<6>(matA, vecB, &_coeff[0]))
		{
			Matrix<3> matAN;
			matAN(0, 0) = matA(0, 5);
			matAN(0, 1) = matA(1, 5);
			matAN(0, 2) = matA(3, 5);
			matAN(1, 1) = matA(2, 5);
			matAN(0, 2) = matA(4, 5);
			matAN(2, 2) = matA(5, 5);
			matAN(1, 0) = matAN(0, 1);
			matAN(2, 0) = matAN(0, 2);
			matAN(2, 1) = matAN(1, 2);

			if (!solve<3>(matAN, &vecB[3], &_coeff[3]))
			{
				if (matA(5, 5) != 0.0)
				{
					_coeff[5] = vecB[5] / matA(5, 5);
				}
			}
		}
	}

	double Polynomial::evaluate(const double *p) const
	{
		return evaluate(p[0], p[1]);
	}

	double Polynomial::evaluate(double x, double y) const
	{
		double result(0.0);

		switch (_degree)
		{
		case 0:
			result = _coeff[0];
			break;
		case 1:
			result = _coeff[0] * x + _coeff[1] * y + _coeff[2];
			break;
		case 2:
			result = _coeff[0] * x * x + _coeff[1] * x * y + _coeff[2] * y * y
				+ _coeff[3] * x + _coeff[4] * y + _coeff[5];
			break;
		}

		return result;
	}
}