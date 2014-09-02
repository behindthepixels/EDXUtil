#include "EDXPrerequisites.h"

#include "SIMD/SSE.h"

IntSSE TopLeftEdge(const Vec2i_SSE& v1, const Vec2i_SSE& v2)
{
	return ((v2.y > v1.y) | ((v1.y == v2.y) & (v1.x > v2.x)));
}

int main()
{
	Vec2i_SSE v1 = Vec2i_SSE(IntSSE(1, 2, 3, 4), IntSSE(4, 3, 2, 1));
	Vec2i_SSE v2 = Vec2i_SSE(IntSSE(4, 3, 2, 1), IntSSE(1, 2, 3, 4));

	IntSSE ret = TopLeftEdge(v1, v2);
	IntSSE a;
	a += ret;

	BoolSSE inside = (IntSSE(0, 1, -1, -1) | IntSSE(0, 0, -1, -1) | IntSSE(-1, 1, -1, -2)) >= IntSSE(Math::EDX_ZERO);

	return 0;
}