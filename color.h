#pragma once

class Color {
private:
	// easy reinterpret.
	union {
		struct {
			uint8_t m_r;
			uint8_t m_g;
			uint8_t m_b;
			uint8_t m_a;
		};

		uint32_t m_rgba;
	};

public:
	// ctors.
	__forceinline Color( ) : m_r{ 0 }, m_g{ 0 }, m_b{ 0 }, m_a{ 0 }, m_rgba{} {}
	__forceinline Color( int r, int g, int b, int a = 255 ) : m_r{ (uint8_t)r }, m_g{ (uint8_t)g }, m_b{ (uint8_t)b }, m_a{ (uint8_t)a } {}
	__forceinline Color( uint32_t rgba ) : m_rgba{ rgba } {}

	static Color hsl_to_rgb( float h, float s, float l ) {
		float q;

		if (l < 0.5f)
			q = l * ( s + 1.f );

		else
			q = l + s - ( l * s );

		float p = 2 * l - q;

		float rgb[3];
		rgb[0] = h + ( 1.f / 3.f );
		rgb[1] = h;
		rgb[2] = h - ( 1.f / 3.f );

		for (int i = 0; i < 3; ++i) {
			if (rgb[i] < 0)
				rgb[i] += 1.f;

			if (rgb[i] > 1)
				rgb[i] -= 1.f;

			if (rgb[i] < ( 1.f / 6.f ))
				rgb[i] = p + ( ( q - p ) * 6 * rgb[i] );
			else if (rgb[i] < 0.5f)
				rgb[i] = q;
			else if (rgb[i] < ( 2.f / 3.f ))
				rgb[i] = p + ( ( q - p ) * 6 * ( ( 2.f / 3.f ) - rgb[i] ) );
			else
				rgb[i] = p;
		}

		return {
			int( rgb[0] * 255.f ),
			int( rgb[1] * 255.f ),
			int( rgb[2] * 255.f )
		};
	}

	typedef struct {
		double r;
		double g;
		double b;
	} rgb;

	typedef struct {
		double h;
		double s;
		double v;
	} hsv;

	static Color hsv_to_rgb( double h, double s, double v )
	{
		double      hh, p, q, t, ff;
		long        i;
		rgb         out;

		if (s <= 0.0) {
			out.r = v;
			out.g = v;
			out.b = v;
			return Color( out.r * 255.f, out.g * 255.f, out.b * 255.f );
		}
		hh = h;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = v * ( 1.0 - s );
		q = v * ( 1.0 - ( s * ff ) );
		t = v * ( 1.0 - ( s * ( 1.0 - ff ) ) );

		switch (i) {
		case 0:
			out.r = v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = v;
			break;
		case 5:
		default:
			out.r = v;
			out.g = p;
			out.b = q;
			break;
		}
		return Color( int( out.r * 255.f ), int( out.g * 255.f ), int( out.b * 255.f ) );
	}

	static hsv rgb_to_hsv( int r, int g, int b )
	{
		rgb in = { (float)( r / 255 ), (float)( g / 255 ) , (float)( b / 255 ) };
		hsv         out;
		double      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min < in.b ? min : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max > in.b ? max : in.b;

		out.v = max;
		delta = max - min;
		if (delta < 0.00001)
		{
			out.s = 0;
			out.h = 0;
			return out;
		}
		if (max > 0.0) {
			out.s = ( delta / max );
		}
		else {
			out.s = 0.0;
			out.h = NAN;
			return out;
		}
		if (in.r >= max)
			out.h = ( in.g - in.b ) / delta;
		else
			if (in.g >= max)
				out.h = 2.0 + ( in.b - in.r ) / delta;
			else
				out.h = 4.0 + ( in.r - in.g ) / delta;

		out.h *= 60.0;

		if (out.h < 0.0)
			out.h += 360.0;

		return out;
	}

	// member accessors.
	__forceinline uint8_t& r( ) { return m_r; }
	__forceinline uint8_t& g( ) { return m_g; }
	__forceinline uint8_t& b( ) { return m_b; }
	__forceinline uint8_t& a( ) { return m_a; }
	__forceinline uint32_t& rgba( ) { return m_rgba; }

	Color alpha( int _a ) {
		return Color( r( ), g( ), b( ), _a );
	}

	Color blend( Color _tmp, float fraction ) {
		float r_d = r( ) - _tmp.r( ), g_d = g( ) - _tmp.g( ), b_d = b( ) - _tmp.b( ), a_d = a( ) - _tmp.a( );

		return Color( r( ) - ( r_d * fraction ), g( ) - ( g_d * fraction ), b( ) - ( b_d * fraction ), a( ) - ( a_d * fraction ) );
	}

	// operators.
	__forceinline operator uint32_t( ) { return m_rgba; }
};

namespace colors {
	static Color white{ 255, 255, 255, 255 };
	static Color black{ 0, 0, 0, 255 };
	static Color yellowgreen{ 182,231,23,255 };
	static Color red{ 255, 0, 0, 255 };
	static Color burgundy{ 0xff2d00b3 };
	static Color light_blue{ 95, 174, 227, 255 };
	static Color orange{ 243, 156, 18, 255 };
	static Color green{ 149, 184, 6, 255 };
	static Color purple{ 180, 60, 120, 255 };
	static Color transparent_green{ 0, 255, 0, 200 };
	static Color transparent_yellow{ 255, 255, 0, 200 };
	static Color transparent_red{ 255, 0, 0, 200 };
	static Color accent{ 149, 181, 230, 255 };
}