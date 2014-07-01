#include "Color.h"

namespace EDX
{
	const Color Color::BLACK(0.0f, 0.0f, 0.0f);
	const Color Color::WHITE(1.0f, 1.0f, 1.0f);
	const Color Color::RED(1.0f, 0.0f, 0.0f);
	const Color Color::GREEN(0.0f, 1.0f, 0.0f);
	const Color Color::BLUE(0.0f, 0.0f, 1.0f);

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
	}
}