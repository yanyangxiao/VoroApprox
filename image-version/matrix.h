
/**
* A class for representing square matrices of arbitrary dimension.
*/

#ifndef MATRIX_H
#define MATRIX_H

#include <assert.h>

namespace xyy
{
	static double det(const double *mat, int n)
	{
		if (n == 1)
			return mat[0];

		if (n == 2)
			return mat[0] * mat[3] - mat[1] * mat[2];

		double *sub = new double[(n - 1) * (n - 1)];
		int mov = 0;
		double sum = 0.0;

		for (int arow = 0; arow < n; ++arow)
		{
			for (int brow = 0; brow < n - 1; ++brow)
			{
				mov = arow > brow ? 0 : 1;
				for (int i = 0; i < n - 1; ++i)
				{
					sub[brow * (n - 1) + i] = mat[(brow + mov) * n + i + 1];
				}
			}

			int flag = (arow % 2 == 0 ? 1 : -1);
			sum += flag * mat[arow * n] * det(sub, n - 1);
		}

		delete[] sub;

		return sum;
	}

	template <int DIM> 
	class Matrix
	{
	private:
		double _coeff[DIM][DIM];

	public:
		Matrix();
		void load_zero();
		void load_identity();

		double& operator()(int i, int j);
		const double& operator()(int i, int j) const;

		Matrix<DIM>& operator+=(const Matrix<DIM> &rhs);
		Matrix<DIM>& operator-=(const Matrix<DIM> &rhs);
		Matrix<DIM>& operator*=(double rhs);
		Matrix<DIM>& operator/=(double rhs);

		Matrix<DIM> operator+(const Matrix<DIM> &op2) const;
		Matrix<DIM> operator-(const Matrix<DIM> &op2) const;
		Matrix<DIM> operator*(const Matrix<DIM> &op2) const;

		double determinant() const;
		Matrix<DIM> inverse() const;
		Matrix<DIM> transpose() const;

		// Routines for interfacing with Fortran, OpenGL etc...

		const double* data() const 
		{ 
			return &(_coeff[0][0]); 
		}
		double* data() 
		{ 
			return &(_coeff[0][0]); 
		}
	};

	//_______________________________________________________________________

	template <int DIM> 
	inline Matrix<DIM>::Matrix() 
	{
		load_identity();
	}

	template <int DIM> 
	inline double& Matrix<DIM>::operator()(int i, int j) 
	{
		assert(i >= 0 && i < DIM);
		assert(j >= 0 && j < DIM);
		return _coeff[i][j];
	}

	template <int DIM> 
	inline const double& Matrix<DIM>::operator()(int i, int j) const 
	{
		assert(i >= 0 && i < DIM);
		assert(j >= 0 && j < DIM);
		return _coeff[i][j];
	}

	template <int DIM> 
	inline Matrix<DIM>& Matrix<DIM>::operator+=(const Matrix<DIM> &rhs)
	{
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] += rhs._coeff[i][j];
			}
		}

		return *this;
	}

	template <int DIM> 
	inline Matrix<DIM>& Matrix<DIM>::operator-=(const Matrix<DIM> &rhs)
	{
		for (int i = 0; i < DIM; ++i) 
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] -= rhs._coeff[i][j];
			}
		}

		return *this;
	}

	template <int DIM> 
	inline Matrix<DIM>& Matrix<DIM>::operator*=(double rhs) 
	{
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] *= rhs;
			}
		}

		return *this;
	}

	template <int DIM> 
	inline Matrix<DIM>& Matrix<DIM>::operator/=(double rhs) 
	{
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] /= rhs;
			}
		}

		return *this;
	}

	template <int DIM>
	inline Matrix<DIM> Matrix<DIM>::operator+(const Matrix<DIM> &op2) const 
	{
		Matrix<DIM> result = *this;
		result += op2;
		return result;
	}

	template <int DIM> 
	inline Matrix<DIM> Matrix<DIM>::operator-(const Matrix<DIM> &op2) const 
	{
		Matrix<DIM> result = *this;
		result -= op2;
		return result;
	}

	template <int DIM> 
	inline Matrix<DIM> Matrix<DIM>::operator*(const Matrix<DIM> &op2) const 
	{
		Matrix<DIM> result;
		result.load_zero();

		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				for (int k = 0; k < DIM; ++k)
				{
					result._coeff[i][j] += _coeff[i][k] * op2._coeff[k][j];
				}
			}
		}

		return result;
	}

	//_______________________________________________________________________

	template <int DIM> 
	void Matrix<DIM>::load_zero() 
	{
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] = double(0);
			}
		}
	}

	template <int DIM> 
	void Matrix<DIM>::load_identity() 
	{
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				_coeff[i][j] = (i == j) ? double(1) : double(0);
			}
		}
	}

	template <int DIM>
	double Matrix<DIM>::determinant() const
	{
		return det(data(), DIM);
	}

	template <int DIM>
	Matrix<DIM>	Matrix<DIM>::inverse() const 
	{
		double val, val2;
		int i, j, k, ind;
		Matrix<DIM> tmp = (*this);
		Matrix<DIM> result;

		result.load_identity();

		for (i = 0; i != DIM; ++i)
		{
			val = tmp(i, i);			/* find pivot */
			ind = i;
			for (j = i + 1; j != DIM; ++j)
			{
				if (fabs(tmp(j, i)) > fabs(val))
				{
					ind = j;
					val = tmp(j, i);
				}
			}

			if (ind != i)
			{
				for (j = 0; j != DIM; ++j)
				{
					val2 = result(i, j);
					result(i, j) = result(ind, j);
					result(ind, j) = val2;           /* swap columns */
					val2 = tmp(i, j);
					tmp(i, j) = tmp(ind, j);
					tmp(ind, j) = val2;
				}
			}

			assert(val != 0.0);

			for (j = 0; j != DIM; ++j)
			{
				tmp(i, j) /= val;
				result(i, j) /= val;
			}

			for (j = 0; j != DIM; ++j)
			{
				if (j == i)
					continue;                       /* eliminate column */

				val = tmp(j, i);
				for (k = 0; k != DIM; ++k)
				{
					tmp(j, k) -= tmp(i, k) * val;
					result(j, k) -= result(i, k) * val;
				}
			}
		}

		return result;
	}

	template <int DIM>
	Matrix<DIM> Matrix<DIM>::transpose() const 
	{
		Matrix<DIM> result;
		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
			{
				result(i, j) = (*this)(j, i);
			}
		}

		return result;
	}

	template <int DIM> 
	Matrix<DIM> operator*(double op1, const Matrix<DIM> &op2)
	{
		Matrix<DIM> result = op2;

		for (int i = 0; i < DIM; ++i)
		{
			for (int j = 0; j < DIM; ++j)
				result(i, j) *= op1;
		}

		return result;
	}

	template <int N> 
	void mult(const Matrix<N> &M, const double *x, double *y) 
	{
		for (int i = 0; i < N; ++i)
		{
			y[i] = 0;
			for (int j = 0; j < N; ++j)
			{
				y[i] += M(i, j) * x[j];
			}
		}
	}

	template <int N> 
	bool solve(const Matrix<N> &M, const double *b, double *x)
	{
		if (fabs(M.determinant()) < DBL_EPSILON)
			return false;

		Matrix<N> M_inv = M.inverse();
		mult(M_inv, b, x);

		return true;
	}
}

#endif
