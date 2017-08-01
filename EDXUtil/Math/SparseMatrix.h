#pragma once

#include "Containers/Array.h"

#include <ppl.h>
using namespace concurrency;

namespace EDX
{
	//============================================================================
	// Dynamic compressed sparse row matrix.

	template<class T>
	struct DynamicSparseMatrix
	{
		int n; // dimension
		Array<Array<int>> index; // for each row, a list of all column indices (sorted)
		Array<Array<T>> value; // values corresponding to index

		explicit DynamicSparseMatrix(int n_ = 0, int expected_nonzeros_per_row = 7)
			: n(n_)
		{
			index.Resize(n);
			value.Resize(n);

			for (int i = 0; i < n; ++i)
			{
				index[i].Reserve(expected_nonzeros_per_row);
				value[i].Reserve(expected_nonzeros_per_row);
			}
		}

		void Clear(void)
		{
			n = 0;
			index.Clear();
			value.Clear();
		}

		void Zero(void)
		{
			for (int i = 0; i < n; ++i)
			{
				index[i].Clear();
				value[i].Clear();
			}
		}

		void Resize(int n_)
		{
			n = n_;
			index.Resize(n);
			value.Resize(n);
		}

		T operator()(int i, int j) const
		{
			for (int k = 0; k < index[i].Size(); ++k)
			{
				if (index[i][k] == j)
					return value[i][k];
				else if (index[i][k] > j)
					return 0;
			}
			return 0;
		}

		void SetElement(int i, int j, T new_value)
		{
			int k = 0;
			for (; k < index[i].Size(); ++k)
			{
				if (index[i][k] == j)
				{
					value[i][k] = new_value;
					return;
				}
				else if (index[i][k] > j)
				{
					index[i].Insert(j, k);
					value[i].Insert(new_value, k);
					return;
				}
			}
			index[i].Add(j);
			value[i].Add(new_value);
		}

		void AddToElement(int i, int j, T increment_value)
		{
			int k = 0;
			for (; k < index[i].Size(); ++k)
			{
				if (index[i][k] == j)
				{
					value[i][k] += increment_value;
					return;
				}
				else if (index[i][k] > j)
				{
					index[i].Insert(j, k);
					value[i].Insert(increment_value, k);
					return;
				}
			}
			index[i].Add(j);
			value[i].Add(increment_value);
		}

		// assumes indices is already sorted
		void AddSparseRow(int i, const Array<int>& indices, const Array<T>& values)
		{
			int j = 0, k = 0;
			while (j < indices.Size() && k < index[i].Size())
			{
				if (index[i][k] < indices[j])
				{
					++k;
				}
				else if (index[i][k] > indices[j])
				{
					index[i].Insert(indices[j], k);
					value[i].Insert(values[j], k);
					++j;
				}
				else
				{
					value[i][k] += values[j];
					++j;
					++k;
				}
			}
			for (; j < indices.Size(); ++j)
			{
				index[i].Add(indices[j]);
				value[i].Add(values[j]);
			}
		}

		//// assumes matrix has symmetric structure - so the indices in row i tell us which columns to delete i from
		//void symmetric_remove_row_and_column(int i)
		//{
		//	for (int a = 0; a < index[i].Size(); ++a) {
		//		int j = index[i][a]; // 
		//		for (int b = 0; b < index[j].Size(); ++b) {
		//			if (index[j][b] == i) {
		//				erase(index[j], b);
		//				erase(value[j], b);
		//				break;
		//			}
		//		}
		//	}
		//	index[i].Resize(0);
		//	value[i].Resize(0);
		//}
	};

	typedef DynamicSparseMatrix<float> DynamicSparseMatrixf;
	typedef DynamicSparseMatrix<double> DynamicSparseMatrixd;

	// perform result=matrix*x
	template<class T>
	void Multiply(const DynamicSparseMatrix<T>& matrix, const Array<T>& x, Array<T>& result)
	{
		Assert(matrix.n == x.Size());
		result.Resize(matrix.n);
		parallel_for(0, (int)matrix.n, [&](int i)
		{
			result[i] = 0;
			for (int j = 0; j < matrix.index[i].Size(); ++j)
			{
				result[i] += matrix.value[i][j] * x[matrix.index[i][j]];
			}
		});
	}

	// perform result=result-matrix*x
	template<class T>
	void MultiplyAndSubtract(const DynamicSparseMatrix<T>& matrix, const Array<T>& x, Array<T>& result)
	{
		Assert(matrix.n == x.Size());
		result.Resize(matrix.n);
		parallel_for(0, (int)matrix.n, [&](int i)
		{
			for (int j = 0; j < matrix.index[i].Size(); ++j)
			{
				result[i] -= matrix.value[i][j] * x[matrix.index[i][j]];
			}
		});
	}

	//============================================================================
	// Static version of DynamicSparseMatrix. This is not a good structure for dynamically
	// modifying the matrix, but can be significantly faster for matrix-vector
	// multiplies due to better data locality.

	template<class T>
	struct SparseMatrix
	{
		int n; // dimension
		Array<T> value; // nonzero values row by row
		Array<int> colindex; // corresponding column indices
		Array<int> rowstart; // where each row starts in value and colindex (and last entry is one past the end, the number of nonzeros)

		explicit SparseMatrix(int n_ = 0)
			: n(n_), value({ T(0) }), colindex({ 0 }), rowstart({ 0, 0 })
		{}

		void Clear(void)
		{
			n = 0;
			value.Clear();
			colindex.Clear();
			rowstart.Clear();
		}

		void Resize(int n_)
		{
			n = n_;
			rowstart.Resize(n + 1);
		}

		void ConstructFromMatrix(const DynamicSparseMatrix<T>& matrix)
		{
			Resize(matrix.n);
			rowstart[0] = 0;
			for (int i = 0; i < n; ++i) {
				rowstart[i + 1] = rowstart[i] + matrix.index[i].Size();
			}
			value.Resize(rowstart[n]);
			colindex.Resize(rowstart[n]);
			int j = 0;
			for (int i = 0; i < n; ++i) {
				for (int k = 0; k < matrix.index[i].Size(); ++k) {
					value[j] = matrix.value[i][k];
					colindex[j] = matrix.index[i][k];
					++j;
				}
			}
		}
	};

	typedef SparseMatrix<float> SparseMatrixf;
	typedef SparseMatrix<double> SparseMatrixd;

	// perform result=matrix*x
	template<class T>
	void Multiply(const SparseMatrix<T>& matrix, const Array<T>& x, Array<T>& result)
	{
		Assert(matrix.n == x.Size());
		result.Resize(matrix.n);
		parallel_for(0, (int)matrix.n, [&](int i)
		{
			result[i] = 0;
			for (int j = matrix.rowstart[i]; j < matrix.rowstart[i + 1]; ++j)
			{
				result[i] += matrix.value[j] * x[matrix.colindex[j]];
			}
		});
	}

	// perform result=result-matrix*x
	template<class T>
	void MultiplyAndSubtract(const SparseMatrix<T>& matrix, const Array<T>& x, Array<T>& result)
	{
		Assert(matrix.n == x.Size());
		result.Resize(matrix.n);
		parallel_for(0, (int)matrix.n, [&](int i)
		{
			for (int j = matrix.rowstart[i]; j < matrix.rowstart[i + 1]; ++j)
			{
				result[i] -= matrix.value[j] * x[matrix.colindex[j]];
			}
		});
	}
}