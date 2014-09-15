#pragma once

namespace EDX
{
	template<unsigned int N, class T>
	class Vec
	{
	public:
		T v[N];

		__forceinline const T& operator [] (const size_t idx) const { return v[idx]; }
		__forceinline		T& operator [] (const size_t idx)		{ return v[idx]; }
	};
}