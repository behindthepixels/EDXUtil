#pragma once

#include "../Math/Vec3.h"

namespace EDX
{
	class Color
	{
	public:
		float r, g, b, a;

	public:
		Color()
			: r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
		Color(float fR, float fG, float fB, float fA = 1.0f)
			: r(fR), g(fG), b(fB), a(fA) {}
		Color(float val)
			: r(val), g(val), b(val), a(1.0f) {}

		explicit Color(const Vector3& vVec)
			: r(vVec.x), g(vVec.y), b(vVec.z), a(1.0f) {}

		~Color()
		{
		}

		inline bool IsBlack() const { return *this == BLACK; }
		inline float Luminance() const { return r * 0.212671f + g * 0.715160f + b * 0.072169f; }

		Color operator + (const Color& color) const
		{
			return Color(r + color.r, g + color.g, b + color.b);
		}

		Color& operator += (const Color& color)
		{
			r += color.r; g += color.g; b += color.b; a = 1.0f;
			return *this;
		}
		Color operator - (const Color& color) const
		{
			return Color(r - color.r, g - color.g, b - color.b);
		}

		Color& operator -= (const Color& color)
		{
			r -= color.r; g -= color.g; b -= color.b; a = 1.0f;
			return *this;
		}
		Color operator * (float val) const { return Color(val * r, val * g, val * b); }

		Color operator * (const Color& color) const
		{
			return Color(r * color.r, g * color.g, b * color.b);
		}

		Color& operator *= (float val)
		{
			r *= val; g *= val; b *= val; a = 1.0f;
			return *this;
		}

		Color& operator *= (const Color& color)
		{
			r *= color.r; g *= color.g; b *= color.b; a = 1.0f;
			return *this;
		}

		Color operator / (float val) const
		{
			float fInv = 1.0f / val;
			return Color(r * fInv, g * fInv, b * fInv);
		}

		Color& operator /= (float val)
		{
			float fInv = 1.0f / val;
			r *= fInv; g *= fInv; b *= fInv; a = 1.0f;
			return *this;
		}


		bool operator == (const Color& color) const
		{
			return r == color.r && g == color.g && b == color.b;
		}
		bool operator != (const Color& color) const
		{
			return r != color.r || g != color.g || b != color.b;
		}

		static const Color BLACK;
		static const Color WHITE;

		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;

	};

	inline Color operator * (float f, const Color& color) { return color * f; }

	namespace Math
	{
		Color Pow(const Color& color, float fP);
	}
}