#pragma once

namespace EDX
{
	template<unsigned int N, class T>
	class Vec
	{
	public:
		T v[N];

		__forceinline const T& operator [] (const size_t idx) const { assert(idx < 1); return v[idx]; }
		__forceinline		T& operator [] (const size_t idx)		{ assert(idx < 1); return v[idx]; }
	};
}