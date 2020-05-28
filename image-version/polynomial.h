
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include <math.h>
#include <Eigen/Eigen>

#include "pixelset.h"

namespace xyy
{
	template <typename Real>
	class Polynomial
	{
		int tab[3] = { 1, 3, 6 };

	private:
		int                 _degree;
		std::vector<Real>   _coeff;

	public:
		Polynomial(int d = 1)
			: _degree(d)
		{ }

		Polynomial(const Polynomial &rhs)
		{
			_degree = rhs._degree;
			_coeff = rhs._coeff;
		}

		Polynomial& operator= (const Polynomial &rhs)
		{
			_degree = rhs._degree;
			_coeff = rhs._coeff;

			return *this;
		}

		int degree() const	{ return _degree; }
		std::vector<Real>& coefficients() const { return _coeff; }

		void compute_factors(
			const unsigned char *image, 
			int width, 
			int height, 
			int channel,
			const PixelSet* pixels);

		Real evaluate(int c, Real x, Real y) const;
		Real evaluate(int c, const Real* p) const
		{
			return evaluate(c, p[0], p[1]);
		}

		Real compute_energy(
			const unsigned char *image,
			int width,
			int height,
			int channel,
			const PixelSet* pixels,
			int Lp = 2) const;

	protected:
		void compute_constant_factors(
			const unsigned char *image,
			int width,
			int height,
			int channel,
			const PixelSet* pixels);

		void compute_linear_factors(
			const unsigned char *image,
			int width,
			int height,
			int channel,
			const PixelSet* pixels);

		void compute_quadratic_factors(
			const unsigned char *image,
			int width,
			int height,
			int channel,
			const PixelSet* pixels);
	};

	template <typename Real>
	void Polynomial<Real>::compute_factors(
		const unsigned char *image,
		int width,
		int height,
		int channel,
		const PixelSet* pixels)
	{
		if (!image || !pixels)
			return;

		switch (_degree)
		{
		case 1:
			compute_linear_factors(image, width, height, channel, pixels);
			break;
		case 2:
			compute_quadratic_factors(image, width, height, channel, pixels);
			break;
		default:
			compute_constant_factors(image, width, height, channel, pixels);
			break;
		}
	}

	template <typename Real>
	void Polynomial<Real>::compute_constant_factors(
		const unsigned char *image,
		int width,
		int height,
		int channel,
		const PixelSet* pixels)
	{
		_coeff.clear();
		_coeff.resize(channel, Real(0.0));

		int count = 0;
		for (int j = pixels->ymin; j <= pixels->ymax; ++j)
		{
			int lineStart = j * width;

			int loc = j - pixels->ymin;
			for (int i = pixels->left[loc]; i <= pixels->right[loc]; ++i)
			{
				int pixID = lineStart + i;
				const unsigned char* pixColor = &image[channel * pixID];

				for (int c = 0; c < channel; ++c)
				{
					_coeff[c] += pixColor[c];
				}

				++count;
			}
		}

		if (count > 0)
		{
			for (int c = 0; c < channel; ++c)
			{
				_coeff[c] /= count;
			}
		}
	}

	template <typename Real>
	void Polynomial<Real>::compute_linear_factors(
		const unsigned char *image,
		int width,
		int height,
		int channel,
		const PixelSet* pixels)
	{
		_coeff.clear();
		_coeff.resize(channel * 3);

		Real ratio = Real(height) / width;
		Real pixWidth = Real(2.0) / width;
		Real pixArea = pixWidth * pixWidth;

		std::vector<Eigen::Matrix3d> matA(channel);
		std::vector<Eigen::Vector3d> vecB(channel);
		for (int c = 0; c < channel; ++c)
		{
			matA[c].setZero();
			vecB[c].setZero();
		}

		Real temp[3];
		for (int j = pixels->ymin; j <= pixels->ymax; ++j)
		{
			Real y = pixWidth * (j + Real(0.5)) - ratio;
			int lineStart = j * width;

			int loc = j - pixels->ymin;
			for (int i = pixels->left[loc]; i <= pixels->right[loc]; ++i)
			{
				Real x = pixWidth * (i + Real(0.5)) - Real(1.0);

				int pixID = lineStart + i;
				const unsigned char* pixColor = &image[channel * pixID];

				temp[2] = pixArea;
				temp[0] = x * temp[2];
				temp[1] = y * temp[2];

				for (int c = 0; c < channel; ++c)
				{
					for (int k = 0; k < 3; ++k)
					{
						vecB[c](k) += pixColor[c] * temp[k];

						matA[c](0, k) += x * temp[k];
						matA[c](1, k) += y * temp[k];
						matA[c](2, k) += temp[k];
					}
				}
			}
		}

		for (int c = 0; c < channel; ++c)
		{
			if (matA[c].determinant() != Real(0.0))
			{
				Eigen::Vector3d vecX = matA[c].colPivHouseholderQr().solve(vecB[c]);
				_coeff[c * 3] = vecX(0);
				_coeff[c * 3 + 1] = vecX(1);
				_coeff[c * 3 + 2] = vecX(2);
			}
			else
			{ // back to constant
				_coeff[c * 3] = Real(0.0);
				_coeff[c * 3 + 1] = Real(0.0);
				_coeff[c * 3 + 2] = Real(0.0);
				if (matA[c](2, 2) != Real(0.0))
				{
					_coeff[c * 3 + 2] = vecB[c](2) / matA[c](2, 2);
				}
			}
		}
	}

	template <typename Real>
	void Polynomial<Real>::compute_quadratic_factors(
		const unsigned char *image,
		int width,
		int height,
		int channel,
		const PixelSet* pixels)
	{
		_coeff.clear();
		_coeff.resize(channel * 6);

		Real ratio = Real(height) / width;
		Real pixWidth = Real(2.0) / width;
		Real pixArea = pixWidth * pixWidth;

		Eigen::MatrixXd matA[3];
		Eigen::VectorXd vecB[3];

		for (int c = 0; c < channel; c++)
		{
			matA[c] = Eigen::MatrixXd::Zero(6, 6);
			vecB[c] = Eigen::VectorXd::Zero(6);
		}

		Real temp[6];
		for (int j = pixels->ymin; j <= pixels->ymax; ++j)
		{
			Real y = pixWidth * (j + Real(0.5)) - ratio;
			int lineStart = j * width;

			int loc = j - pixels->ymin;
			for (int i = pixels->left[loc]; i <= pixels->right[loc]; ++i)
			{
				Real x = pixWidth * (i + Real(0.5)) - Real(1.0);

				int pixID = lineStart + i;
				const unsigned char* pixColor = &image[channel * pixID];

				temp[5] = pixArea;
				temp[0] = x * x * temp[5];
				temp[1] = x * y * temp[5];
				temp[2] = y * y * temp[5];
				temp[3] = x * temp[5];
				temp[4] = y * temp[5];

				for (int c = 0; c < channel; ++c)
				{
					for (int k = 0; k < 6; ++k)
					{
						vecB[c](k) += pixColor[c] * temp[k];

						matA[c](0, k) += x * x * temp[k];
						matA[c](1, k) += x * y * temp[k];
						matA[c](2, k) += y * y * temp[k];
						matA[c](3, k) += x * temp[k];
						matA[c](4, k) += y * temp[k];
						matA[c](5, k) += temp[k];
					}
				}
			}
		}

		for (int c = 0; c < channel; ++c)
		{
			if (matA[c].determinant() != Real(0.0))
			{
				Eigen::VectorXd vecX = matA[c].colPivHouseholderQr().solve(vecB[c]);
				for (int k = 0; k < 6; ++k)
				{
					_coeff[c * 6 + k] = vecX(k);
				}
			}
			else
			{ // back to linear
				_coeff[c * 6] = Real(0.0);
				_coeff[c * 6 + 1] = Real(0.0);
				_coeff[c * 6 + 2] = Real(0.0);

				Eigen::Matrix3d matAN;
				matAN(0, 0) = matA[c](0, 5);
				matAN(0, 1) = matA[c](1, 5);
				matAN(0, 2) = matA[c](3, 5);
				matAN(1, 1) = matA[c](2, 5);
				matAN(0, 2) = matA[c](4, 5);
				matAN(2, 2) = matA[c](5, 5);
				matAN(1, 0) = matAN(0, 1);
				matAN(2, 0) = matAN(0, 2);
				matAN(2, 1) = matAN(1, 2);

				if (matAN.determinant() != Real(0.0))
				{
					Eigen::Vector3d vecBN, vecXN;
					for (int k = 0; k < 3; ++k)
						vecBN(k) = vecB[c](k + 3);

					vecXN = matAN.colPivHouseholderQr().solve(vecBN);

					for (int k = 0; k < 3; ++k)
						_coeff[c * 6 + k + 3] = vecXN(k);
				}
				else
				{ // back to constant
					_coeff[c * 6 + 3] = Real(0.0);
					_coeff[c * 6 + 4] = Real(0.0);
					_coeff[c * 6 + 5] = Real(0.0);
					if (matA[c](5, 5) != Real(0.0))
					{
						_coeff[c * 6 + 5] = vecB[c](5) / matA[c](5, 5);
					}
				}
			}
		}
	}

	template <typename Real>
	Real Polynomial<Real>::evaluate(int c, Real x, Real y) const
	{
		Real result = Real(0.0);

		switch (_degree)
		{
		case 0:
			result = _coeff[c];
			break;
		case 1:
			result = _coeff[c * 3] * x + _coeff[c * 3 + 1] * y + _coeff[c * 3 + 2];
			break;
		case 2:
			result = _coeff[c * 6] * x * x + _coeff[c * 6 + 1] * x * y + _coeff[c * 6 + 2] * y * y
				+ _coeff[c * 6 + 3] * x + _coeff[c * 6 + 4] * y + _coeff[c * 6 + 5];
			break;
		}

		return result;
	}

	template <typename Real>
	Real Polynomial<Real>::compute_energy(
		const unsigned char *image,
		int width,
		int height,
		int channel,
		const PixelSet* pixels,
		int Lp = 2) const
	{
		if (!image || !pixels)
			return Real(0.0);

		Real ratio = Real(height) / width;
		Real pixWidth = Real(2.0) / width;
		Real pixArea = pixWidth * pixWidth;

		Real result = Real(0.0);
		for (int j = pixels->ymin; j <= pixels->ymax; ++j)
		{
			Real y = pixWidth * (j + Real(0.5)) - ratio;
			int lineStart = j * width;

			int loc = j - pixels->ymin;
			for (int i = pixels->left[loc]; i <= pixels->right[loc]; ++i)
			{
				Real x = pixWidth * (i + Real(0.5)) - Real(1.0);

				int pixID = lineStart + i;
				const unsigned char* pixColor = &image[channel * pixID];

				Real tempEnergy = 0.0;
				for (int c = 0; c < channel; ++c)
				{
					Real approxVal = evaluate(c, x, y);
					Real pixVal = (Real)pixColor[c];
					Real absError = abs(pixVal - approxVal);
					tempEnergy += std::pow(absError, Lp);
				}

				result += tempEnergy * pixArea;
			}
		}

		return result;
	}
}

#endif