#pragma once

namespace EDX
{
	template<unsigned int N, class T>
	class Vec
	{
	public:
		T v[N];

		Vec()
		{
		}
		__forceinline Vec(const T val)
		{
			for (auto i = 0; i < N; i++)
				v[i] = val;
		}
		__forceinline Vec(const Vec& vCopyFrom)
		{
			for (auto i = 0; i < N; i++)
				v[i] = vCopyFrom[i];
		}
		template<class T1>
		__forceinline Vec& operator = (const Vec<N, T1>& vOther)
		{
			for (auto i = 0; i < N; i++)
				v[i] = T(vOther[i]);
			return *this;
		}

		__forceinline const T& operator [] (const size_t idx) const { return v[idx]; }
		__forceinline		T& operator [] (const size_t idx)		{ return v[idx]; }
	};
}