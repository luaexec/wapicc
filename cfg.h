#pragma once
#include "includes.h"
#include <d3d9.h>

class value_t {
public:
	template <typename t = float>
	t get( ) {
		return (t)m_value;
	}

	template <typename t = float>
	void set( t in ) {
		m_value = (double)in;
	}

	Color get_color( int _a = -1 ) {
		int a = ( (D3DCOLOR)m_value >> 24 ) & 0xff;
		int r = ( (D3DCOLOR)m_value >> 16 ) & 0xff;
		int g = ( (D3DCOLOR)m_value >> 8 ) & 0xff;
		int b = (D3DCOLOR)m_value & 0xff;

		if (a == 0 && r == 0 && g == 0 && b == 0) {
			a = 255;
			r = 255;
			g = 255;
			b = 255;
		}

		Color ret( r, g, b, _a != -1 ? _a : a );
		return ret;
	}

	void set_color( Color in ) {
		DWORD color = D3DCOLOR_RGBA( in.r( ), in.g( ), in.b( ), in.a( ) );

		m_value = (double)color;
	}

	template <typename t = float>
	value_t( t _tmp ) {
		m_value = (double)_tmp;
	}

	value_t( Color _tmp ) {
		DWORD color = D3DCOLOR_RGBA( _tmp.r( ), _tmp.g( ), _tmp.b( ), _tmp.a( ) );

		m_value = (double)color;
	}

	value_t( ) {
		m_value = 0.0;
	}

private:
	double m_value = 0.0;
};

extern std::map< std::string, value_t > config;

namespace cfg_t {
	void init( );

	inline static std::unordered_map<std::string, bool> m_hotkey_states{};

	bool get_hotkey( std::string str, std::string mode );

	void save( std::string name = "default.ini", bool to_clipboard = false );
	void load( std::string name = "default.ini", bool from_clipboard = false );

	inline bool m_init;
	inline std::string m_path;
};