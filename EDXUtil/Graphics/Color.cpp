#include "Color.h"

namespace EDX
{
	const Color Color::BLACK(0.0f, 0.0f, 0.0f);
	const Color Color::WHITE(1.0f, 1.0f, 1.0f);
	const Color Color::RED(1.0f, 0.0f, 0.0f);
	const Color Color::GREEN(0.0f, 1.0f, 0.0f);
	const Color Color::BLUE(0.0f, 0.0f, 1.0f);

	Color::Color(const Color4b& c)
		: r(c.r * 0.00390625f)
		, g(c.g * 0.00390625f)
		, b(c.b * 0.00390625f)
		, a(c.a * 0.00390625f)
	{
	}

	namespace Math
	{
		Color Pow(const Color& color, float pow)
		{
			Color ret;
			ret.r = Math::Pow(color.r, pow);
			ret.g = Math::Pow(color.g, pow);
			ret.b = Math::Pow(color.b, pow);
			ret.a = color.a;
			return ret;
		}

		Color4b Pow(const Color4b& color, float pow)
		{
			return Pow(Color(color), pow);
		}
	}
}