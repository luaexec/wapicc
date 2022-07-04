#pragma once
#include "includes.h"
#include <optional>

namespace gui {
	inline auto m_pos{ vec2_t( 500.f, 250.f ) };
	inline auto m_size{ vec2_t( 500.f, 400.f ) };
	inline auto m_open{ true };
	inline auto m_anim{ 0.f };
	inline auto m_accent{ Color( 242, 97, 97 ) };
	__forceinline float m_alpha( float _alpha = 255.f ) { return _alpha * m_anim; }

	__forceinline void animate( bool condition, float speed, float& anim ) {
		if (condition) { anim += speed * g_csgo.m_globals->m_frametime; }
		else { anim -= speed * g_csgo.m_globals->m_frametime; }
		anim = std::clamp( anim, 0.f, 1.f );
	}

	__forceinline std::string key_name( const int VirtualKey ) {
		auto Code = MapVirtualKeyA( VirtualKey, MAPVK_VK_TO_VSC );

		int Result;
		char Buffer[128];

		switch (VirtualKey) {
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
		case VK_RCONTROL: case VK_RMENU:
		case VK_LWIN: case VK_RWIN: case VK_APPS:
		case VK_PRIOR: case VK_NEXT:
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE:
		case VK_NUMLOCK:
			Code |= KF_EXTENDED;
		default:
			Result = GetKeyNameTextA( Code << 16, Buffer, 128 );
		}

		if (Result == 0) {
			switch (VirtualKey) {
			case VK_XBUTTON1:
				return "mouse4";
			case VK_XBUTTON2:
				return "mouse5";
			case VK_LBUTTON:
				return "mouse1";
			case VK_MBUTTON:
				return "mouse3";
			case VK_RBUTTON:
				return "mouse2";
			default:
				return "-";
			}
		}

		auto transformer = std::string( Buffer );
		std::transform( transformer.begin( ), transformer.end( ), transformer.begin( ), ::tolower );
		return transformer.substr( 0, 6 );
	}

	struct tab_t {
		std::string title;
		float anim;

		tab_t( std::string _title, float _anim = 0.f ) {
			title = _title; anim = _anim;
		}
	};

	namespace palette {
		inline auto dark = Color( 18, 18, 22 );
		inline auto grey = Color( 22, 22, 27 );
		inline auto med = Color( 26, 26, 31 );
		inline auto light = Color( 30, 30, 36 );
	}

	__forceinline auto tab_area( bool split = false ) {
		int width = ( ( m_size.x - 100 ) - 20 ) / 2;
		int height = ( m_size.y - ( 20 + ( split ? 10 : 0 ) ) ) / ( split ? 2 : 1 );
		return vec2_t( width, height );
	}

	class window_t {
	private:
		float motion_anim{ 0.f };
		std::deque<tab_t> tabs{ };
	public:
		int cur_tab{ 0 };
	public:
		window_t( std::vector<std::string> _tabs ) {
			for (auto t : _tabs)
				add_tab( tab_t( t ) );
		}

		bool begin( std::string title ) {
			animate( m_open, 4.5f, m_anim );

			if (g_input.GetKeyPress( VK_INSERT ))
				m_open = !m_open;

			m_accent = config["menu_accent"].get_color( 255 );

			static vec2_t c_pos{}, c_mpos{};
			static auto c_press = false;
			if (g_input.hovered( m_pos, m_size.x, 10 ) && g_input.GetKeyPress( 0x01 ))
				c_press = true;

			if (c_press) {
				m_pos.x = c_mpos.x + ( ( g_input.m_mouse.x ) - c_pos.x );
				m_pos.y = c_mpos.y + ( ( g_input.m_mouse.y ) - c_pos.y );
				c_press = g_input.GetKeyState( 0x01 );
			}
			else { c_pos = vec2_t( g_input.m_mouse.x, g_input.m_mouse.y ); c_mpos = m_pos; c_press = false; }

			static vec2_t c_size{}, c_m_size{};
			static auto c_spress = false;
			if (g_input.hovered( m_pos + m_size - vec2_t( 8, 8 ), 8, 8 ) && g_input.GetKeyPress( 0x01 ))
				c_spress = true;

			if (c_spress) {
				m_size.x = c_m_size.x + ( ( g_input.m_mouse.x ) - c_size.x );
				m_size.y = c_m_size.y + ( ( g_input.m_mouse.y ) - c_size.y );
				c_spress = g_input.GetKeyState( 0x01 );
				m_size.x = std::clamp( m_size.x, 500.f, 1500.f ); m_size.y = std::clamp( m_size.y, 400.f, 1000.f );
			}
			else { c_size = vec2_t( g_input.m_mouse.x, g_input.m_mouse.y ); c_m_size = m_size; c_spress = false; }

			animate( ( c_spress || c_press ), 4.f, motion_anim );

			render::round_rect( m_pos.x - 1, m_pos.y - 1, m_size.x + 2, m_size.y + 2, 2, ( palette::grey.blend( m_accent, motion_anim ) ).alpha( m_alpha( ) ) );
			render::round_rect( m_pos.x, m_pos.y, m_size.x, m_size.y, 2, palette::dark.alpha( m_alpha( ) ) );

			render::round_rect( m_pos.x + 1, m_pos.y + 1, 100, m_size.y - 2, 2, palette::grey.alpha( m_alpha( ) ) );

			auto i{ 0 };
			for (auto& t : tabs) {
				animate( cur_tab == i, 4.5f, t.anim );
				auto area = render::esp.size( t.title );

				if (cur_tab == i) {
					render::rect_filled( m_pos.x + 10, m_pos.y + 8 + ( 20 * i ), 91, 20, palette::dark.alpha( m_alpha( ) ) );
					render::gradient1337( m_pos.x + 11, m_pos.y + 9 + ( 20 * i ), 91, 18, m_accent.alpha( m_alpha( 55 ) ), colors::black.alpha( m_alpha( 0 ) ) );
				}
				render::esp.string( m_pos.x + 10 + ( 7.f * t.anim ), m_pos.y + 10 + ( 20 * i ), colors::white.blend( m_accent, t.anim ).alpha( m_alpha( ) ), t.title );

				if (g_input.hovered( m_pos.x, m_pos.y + 8 + ( 20 * i ), 100, 20 ) && g_input.GetKeyPress( 0x01 ))
					cur_tab = i;

				i++;
			}

			return m_open && m_anim > 0.f;
		}

		void add_tab( tab_t t ) {
			tabs.push_back( t );
		}
	};

	class checkbox_t {
	private:
		std::string title, var;
		float anim{ 0.f };
	public:
		checkbox_t( std::string _t, std::string _v, value_t def ) {
			title = _t; var = _v;

			bool found = config.find( var ) != config.end( );
			if (!found) {
				config.insert( { var, def } );
			}
		}

		void render( vec2_t& area, bool use ) {
			animate( config[var].get<bool>( ), 5.f, anim );

			render::rect_filled( area.x, area.y, 8, 8, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x, area.y, 8, 8, palette::light.alpha( m_alpha( ) ) );
			render::gradient1337( area.x + 2, area.y + 2, 4, 4, m_accent.alpha( m_alpha( 155 * anim ) ), colors::black.alpha( m_alpha( 0 ) ) );

			render::esp.string( area.x + 15, area.y - 3, colors::white.alpha( m_alpha( ) ), title );

			if (g_input.hovered( area.x, area.y, 12 + render::esp.size( title ).m_width, 15 ) && g_input.GetKeyPress( 0x01 ) && use)
				config[var].set<bool>( !config[var].get<bool>( ) );

			area += vec2_t( 0, 15 );
		}
	};

	class slider_t {
	private:
		std::string title, var, prefix;
		float min, max;
	public:
		bool dragging{ false };
	public:
		slider_t( std::string _t, std::string _v, value_t def, float _min, float _max, std::string _pre = "" ) {
			title = _t; var = _v; min = _min; max = _max; prefix = _pre;

			bool found = config.find( var ) != config.end( );
			if (!found) {
				config.insert( { var, def } );
			}
		}

		void render( vec2_t& area, bool use ) {
			render::esp.string( area.x + 9, area.y, colors::white.alpha( m_alpha( ) ), title );
			render::pixel.string( area.x + 9 + render::esp.size( title ).m_width + 5, area.y + 3, m_accent.alpha( m_alpha( ) ), tfm::format( "%s%s", config[var].get<int>( ), prefix ) );

			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9.f, area.y + 15, size, 8, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x + 9.f, area.y + 15, size, 8, palette::light.alpha( m_alpha( ) ) );

			if (min >= 0.f) {
				render::gradient1337( area.x + 11, area.y + 17, ( ( size - 4.f ) * float( config[var].get<float>( ) / float( max ) ) ), 4, m_accent.alpha( m_alpha( 155 ) ), colors::black.alpha( m_alpha( 0 ) ) );
			}
			else {
				int bar = int( ( ( size / 2 ) - 2.f ) * float( float( abs( config[var].get( ) ) ) / float( max ) ) ), offset = ( config[var].get( ) >= 0 ? 0 : bar );
				render::gradient1337( area.x + 11 + ( ( size / 2 ) - 2.f ) - offset, area.y + 17, bar, 4, m_accent.alpha( m_alpha( 155 ) ), colors::black.alpha( m_alpha( 0 ) ) );
				render::rect_filled( area.x + 8 + ( size / 2.f ), area.y + 16, 1, 6, palette::light.alpha( m_alpha( ) ) );
			}

			float dx = std::clamp( ( g_input.m_mouse.x - ( area.x ) ) / size, 0.f, 1.f );
			if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area - vec2_t( 1, 0 ), size + 2, 7 + 15 ) && use)
				dragging = true;

			if (dragging) {
				int difference = abs( min - max );
				config[var].set<int>( min + ( difference * dx ) );
				dragging = g_input.GetKeyState( 0x01 );
			}
			else { dragging = false; }

			int cla = std::clamp( config[var].get<float>( ), min, max );
			config[var].set<int>( cla );

			area += vec2_t( 0, 30 );
		}
	};

	class dropdown_t {
	private:
		std::string title, var;
		std::vector<std::string> elements;
		float anim{ 0.f };
	public:
		bool open{ false };
	public:
		dropdown_t( std::string _t, std::string _v, value_t def, std::vector<std::string> _e ) {
			title = _t; var = _v; elements = _e;

			bool found = config.find( var ) != config.end( );
			if (!found) {
				config.insert( { var, def } );
			}
		}

		void render( vec2_t& area, bool use ) {
			animate( open, 4.f, anim );

			render::esp.string( area.x + 9, area.y, colors::white.alpha( m_alpha( ) ), title );

			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y + 15, size, 20, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x + 9, area.y + 15, size, 20, palette::light.alpha( m_alpha( ) ) );

			render::esp.string( area.x + 12, area.y + 18, colors::white.alpha( m_alpha( ) ), elements[config[var].get<int>( )] );

			if (g_input.GetKeyPress( 0x01 )) {
				if (( g_input.hovered( area.x + 9, area.y + 15, size, 20 ) && ( use || open ) ) || ( !g_input.hovered( area.x + 9, area.y + 35, size, 10 + ( elements.size( ) * 15 ) ) && open ))
					open = !open;
			}

			area += vec2_t( 0, 42 );
		}

		void finish( vec2_t area ) {
			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y + 35, size, 10 + ( elements.size( ) * 15 ), palette::med.alpha( m_alpha( 255.f * anim ) ) );
			render::rect( area.x + 9, area.y + 35, size, 10 + ( elements.size( ) * 15 ), palette::light.alpha( m_alpha( 255.f * anim ) ) );

			int i{ 0 }, selection{ config[var].get<int>( ) };
			for (auto e : elements) {
				if (selection == i)
					render::gradient1337( area.x + 13, area.y + 41 + ( i * 15 ), size - 2, 13, m_accent.alpha( m_alpha( 55 ) ), colors::black.alpha( m_alpha( 0 ) ) );

				render::esp.string( area.x + 12 + ( selection == i ? 5 * anim : 0 ), area.y + 40 + ( i * 15 ), ( selection == i ? m_accent : colors::white ).alpha( m_alpha( 255.f * anim ) ), e );

				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + 9, area.y + 40 + ( i * 15 ), size, 15 )) {
					config[var].set<int>( i );
					open = false;
				}

				i++;
			}
		}
	};

	struct multitems_t {
		std::string name;
		std::string var;
		value_t val;
		multitems_t( std::string _n, std::string _v, bool _d = false ) {
			name = _n; var = _v; val = value_t( _d );
		}
	};

	class multidropdown_t {
	private:
		std::string title;
		std::vector<multitems_t> items;
		float anim{ 0.f };
	public:
		bool open{ false };
	public:
		multidropdown_t( std::string _t, std::vector<multitems_t> _i ) {
			title = _t; items = _i;

			for (auto i : items) {
				bool found = config.find( i.var ) != config.end( );
				if (!found) {
					config.insert( { i.var, i.val } );
				}
			}
		}

		void render( vec2_t& area, bool use ) {
			animate( open, 4.f, anim );

			render::esp.string( area.x + 9, area.y, colors::white.alpha( m_alpha( ) ), title );

			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y + 15, size, 20, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x + 9, area.y + 15, size, 20, palette::light.alpha( m_alpha( ) ) );

			std::string preview{ "" };
			for (auto e : items) {
				if (config[e.var].get<bool>( ))
					preview.append( preview == "" ? e.name : ( std::string( ", " ).append( e.name ) ) );
			}
			if (preview == "")
				preview = "empty..";

			bool too_big = preview.length( ) > ( size / 7 );
			preview = preview.substr( 0, ( size / 7 ) ).append( too_big ? ".." : "" );

			render::esp.string( area.x + 12, area.y + 18, colors::white.alpha( m_alpha( ) ), preview );

			if (g_input.GetKeyPress( 0x01 )) {
				if (( g_input.hovered( area.x + 9, area.y + 15, size, 20 ) && ( use || open ) ) || ( !g_input.hovered( area.x + 9, area.y + 35, size, 10 + ( items.size( ) * 15 ) ) && open ))
					open = !open;
			}

			area += vec2_t( 0, 42 );
		}

		void finish( vec2_t area ) {
			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y + 35, size, 10 + ( items.size( ) * 15 ), palette::med.alpha( m_alpha( 255.f * anim ) ) );
			render::rect( area.x + 9, area.y + 35, size, 10 + ( items.size( ) * 15 ), palette::light.alpha( m_alpha( 255.f * anim ) ) );

			int i{ 0 };
			for (auto e : items) {
				bool active = config[e.var].get<bool>( );
				if (active)
					render::gradient1337( area.x + 13, area.y + 41 + ( i * 15 ), size - 2, 13, m_accent.alpha( m_alpha( 55 ) ), colors::black.alpha( m_alpha( 0 ) ) );

				render::esp.string( area.x + 12 + ( active ? 5 * anim : 0 ), area.y + 40 + ( i * 15 ), ( active ? m_accent : colors::white ).alpha( m_alpha( 255.f * anim ) ), e.name );

				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + 9, area.y + 40 + ( i * 15 ), size, 15 )) {
					config[e.var].set<bool>( !config[e.var].get<bool>( ) );
				}

				i++;
			}
		}
	};

	class colorpicker_t {
	private:
		std::string title, var;
		value_t def;
		bool in_line;
		float anim{ 0.f };
		float hanim{ 0.f };
		vec2_t offset;
	public:
		bool primary{ false };
		bool secondary{ false };
	public:
		colorpicker_t( std::string _t, std::string _v, value_t _d = value_t( colors::white ), bool _i = false, vec2_t _o = vec2_t( 0, 0 ) ) {
			title = _t; var = _v; def = _d; in_line = _i; offset = _o;

			bool found = config.find( var ) != config.end( );
			if (!found) {
				config.insert( { var, def } );
			}
		}

		void render( vec2_t& area, bool use ) {
			animate( primary || secondary, 4.f, anim );

			auto size = tab_area( false ).x - 20 - 2;
			auto draw = area - ( in_line ? vec2_t( 0, 20 ) : vec2_t( 0, 0 ) ) + offset;

			if (!in_line)
				render::esp.string( draw.x, draw.y, colors::white.alpha( m_alpha( ) ), title );

			auto preview{ config[var].get_color( 255 ) };
			render::round_rect( draw.x + size - 15, draw.y + 5, 15, 8, 2, palette::light.alpha( m_alpha( ) ) );
			render::round_rect( draw.x + size - 14, draw.y + 6, 13, 6, 2, preview.alpha( m_alpha( ) ) );

			if (g_input.hovered( draw.x + size - 15, draw.y + 5, 15, 8 ) && !primary && !secondary && in_line) {
				auto sz = render::esp.size( title ).m_width + 10;
				hanim += 4.f * g_csgo.m_globals->m_frametime;
				hanim = std::clamp( hanim, 0.f, 1.f );

				render::round_rect( draw.x + size - 15 - ( float( sz ) / 2.f ), draw.y - 5 - ( 5 * hanim ), sz, 15, 2, palette::light.alpha( m_alpha( 255.f * hanim ) ) );
				render::round_rect( draw.x + size - 14 - ( float( sz ) / 2.f ), draw.y - 4 - ( 5 * hanim ), sz - 2, 13, 2, palette::med.alpha( m_alpha( 255.f * hanim ) ) );
				render::esp.string( draw.x + size - 10 - ( float( sz ) / 2.f ), draw.y - 3 - ( 5 * hanim ), colors::white.alpha( m_alpha( 255.f * hanim ) ), title );
			}
			else
				hanim -= 4.f * g_csgo.m_globals->m_frametime;
			hanim = std::clamp( hanim, 0.f, 1.f );

			if (g_input.GetKeyPress( 0x01 )) {
				if (( g_input.hovered( draw.x + size - 15, draw.y + 5, 15, 8 ) && ( use || ( primary && !secondary ) ) ) || ( !g_input.hovered( draw.x + size + 5 - 200, draw.y, 200, 200 ) ) && ( primary && !secondary ))
					primary = !primary;
			}

			/*if (g_input.GetKeyPress( 0x02 )) {
				if (( g_input.hovered( draw.x + size - 15, draw.y + 5, 15, 8 ) && ( use || ( !primary && secondary ) ) ) || ( !g_input.hovered( draw.x + size - 100, draw.y + 15, 100, 55 ) ) && ( !primary && secondary ))
					secondary = !secondary;
			}*/

			if (!in_line)
				area += vec2_t( 0, 20 );
		}

		void primary_r( vec2_t area ) {
			auto tab = tab_area( false ).x - 20 - 2;
			auto size = vec2_t( 200, 200 );
			auto draw = area - ( in_line ? vec2_t( 0, 20 ) : vec2_t( 0, 0 ) ) + offset + vec2_t( tab, 0.f ) - vec2_t( size.x, 0.f );

			auto rgb{ config[var].get_color( ) };
			auto hsv{ Color::rgb_to_hsv( rgb.r( ), rgb.g( ), rgb.b( ) ) };

			static auto n_hsv{ hsv };
			static auto n_alpha{ rgb.a( ) };

			render::round_rect( draw.x + 5, draw.y, size.x, size.y, 2, palette::light.alpha( m_alpha( ) ) );
			render::round_rect( draw.x + 5 + 1, draw.y + 1, size.x - 2, size.y - 2, 2, palette::med.alpha( m_alpha( ) ) );

			render::round_rect( draw.x + size.x - 15 + ( 5 * anim ), draw.y + 5, 15 - ( 5 * anim ), 8 + ( 2 * anim ), 2, palette::light.alpha( m_alpha( ) ) );
			render::round_rect( draw.x + size.x - 14 + ( 5 * anim ), draw.y + 6, 13 - ( 5 * anim ), 6 + ( 2 * anim ), 2, rgb.alpha( m_alpha( ) ) );

			static bool dragging_alpha{ false };
			static bool dragging_hue{ false };
			static bool dragging_main{ false };

			/* alpha */ {
				float dx = std::clamp( ( g_input.m_mouse.y - ( draw.y + 20 ) ) / ( size.y - 30 ), 0.f, 1.f );
				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( draw.x + size.x - 10, draw.y + 20, 10, size.y - 30 ) && !dragging_main && !dragging_hue)
					dragging_alpha = true;

				if (dragging_alpha) {
					n_alpha = 255.f * dx;
					config[var].set_color( Color::hsv_to_rgb( n_hsv.h, n_hsv.s, n_hsv.v ).alpha( 255.f * dx ) );
					dragging_alpha = g_input.GetKeyState( 0x01 );
				}
				else { dragging_alpha = false; }

				render::gradient( draw.x + size.x - 10, draw.y + 20, 10, size.y - 30, colors::black.alpha( m_alpha( ) ), rgb.alpha( m_alpha( ) ) );
				render::rect( draw.x + size.x - 10, draw.y + 20, 10, size.y - 30, palette::light.alpha( m_alpha( ) ) );
				render::rect( draw.x + size.x - 10, draw.y + 18 + ( ( size.y - 30 ) * ( n_alpha / 255.f ) ), 10, 3, palette::light.alpha( m_alpha( ) ) );
			}

			/* hue */ {
				float dx = std::clamp( ( g_input.m_mouse.x - ( draw.x + 10 ) ) / ( size.x - 25 ), 0.f, 1.f );
				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( draw.x + 10, draw.y + size.y - 20, size.x - 25, 10 ) && !dragging_main && !dragging_alpha)
					dragging_hue = true;

				if (dragging_hue) {
					n_hsv.h = 360.f * dx;
					config[var].set_color( Color::hsv_to_rgb( 360.f * dx, n_hsv.s, n_hsv.v ).alpha( n_alpha ) );
					dragging_hue = g_input.GetKeyState( 0x01 );
				}
				else { dragging_hue = false; }

				render::gradient1337( draw.x + 10, draw.y + size.y - 15, ( size.x - 25 ) / 4, 10, Color::hsv_to_rgb( 0.f, 1.f, 1.f ).alpha( m_alpha( ) ), Color::hsv_to_rgb( 90.f, 1.f, 1.f ).alpha( m_alpha( ) ) );
				render::gradient1337( draw.x + 10 - 1 + ( ( ( size.x - 25 ) / 4 ) ), draw.y + size.y - 15, ( size.x - 25 ) / 4, 10, Color::hsv_to_rgb( 90.f, 1.f, 1.f ).alpha( m_alpha( ) ), Color::hsv_to_rgb( 180.f, 1.f, 1.f ).alpha( m_alpha( ) ) );
				render::gradient1337( draw.x + 10 - 2 + ( ( ( size.x - 25 ) / 4 ) ) * 2, draw.y + size.y - 15, ( size.x - 25 ) / 4, 10, Color::hsv_to_rgb( 180.f, 1.f, 1.f ).alpha( m_alpha( ) ), Color::hsv_to_rgb( 270.f, 1.f, 1.f ).alpha( m_alpha( ) ) );
				render::gradient1337( draw.x + 10 - 3 + ( ( ( size.x - 25 ) / 4 ) ) * 3, draw.y + size.y - 15, ( size.x - 25 ) / 4, 10, Color::hsv_to_rgb( 270.f, 1.f, 1.f ).alpha( m_alpha( ) ), Color::hsv_to_rgb( 360.f, 1.f, 1.f ).alpha( m_alpha( ) ) );

				render::rect( draw.x + 10, draw.y + size.y - 15, size.x - 25, 10, palette::light.alpha( m_alpha( ) ) );
				render::rect( draw.x + 10 + ( ( size.x - 25 ) * ( n_hsv.h / 360.f ) ), draw.y + size.y - 15, 3, 10, palette::light.alpha( m_alpha( ) ) );
			}

			/* main */ {
				float _off = 25;
				float dx = std::clamp( ( g_input.m_mouse.x - ( draw.x + 10 ) ) / ( size.x - _off ), 0.f, 1.f );
				float dy = std::clamp( ( g_input.m_mouse.y - ( draw.y + 5 ) ) / ( size.y - _off ), 0.f, 1.f );

				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( draw.x + 10, draw.y + 10, size.x - _off, size.y - _off ) && !dragging_hue && !dragging_alpha)
					dragging_main = true;

				if (dragging_main) {
					n_hsv.s = dx; n_hsv.v = dy;
					config[var].set_color( Color::hsv_to_rgb( n_hsv.h, dx, dy ).alpha( n_alpha ) );
					dragging_main = g_input.GetKeyState( 0x01 );
				}
				else { dragging_main = false; }

				for (int i = 0; i < ( ( size.y - _off ) / 5 ); i++) {
					render::gradient1337( draw.x + 10, draw.y + 5 + ( i * 5 ), size.x - _off, 5, Color::hsv_to_rgb( n_hsv.h, 0.f, float( i ) / ( ( size.y - _off ) / 5 ) ).alpha( m_alpha( ) ), Color::hsv_to_rgb( n_hsv.h, 1.f, float( i ) / ( ( size.y - _off ) / 5 ) ).alpha( m_alpha( ) ) );
				}
				render::rect( draw.x + 10, draw.y + 5, size.x - _off, size.y - _off, palette::light.alpha( m_alpha( ) ) );

				render::rect( draw.x + 10 + ( ( size.x - _off ) * n_hsv.s ), draw.y + 5 + ( ( size.x - _off ) * n_hsv.v ), 3, 3, palette::light.alpha( m_alpha( ) ) );
			}
		}

		void secondary_r( vec2_t area ) {
			auto tab = tab_area( false ).x - 20 - 2;
			auto size = vec2_t( 100, 55 ); // 5 padding.
			auto draw = area - ( in_line ? vec2_t( 0, 20 ) : vec2_t( 0, 0 ) ) + offset + tab - size;
		}

		void finish( vec2_t area ) {
			if (primary)
				return primary_r( area );

			if (secondary)
				return secondary_r( area );
		}
	};

	class button_t {
	private:
		std::string title;
		std::function<void( )> fn;
		float anim{ 0.f };
	public:
		button_t( std::string _t, std::function<void( )> _fn ) {
			title = _t; fn = _fn;
		}

		void render( vec2_t& area, bool use ) {
			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y, size, 20, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x + 9, area.y, size, 20, palette::light.alpha( m_alpha( ) ) );

			render::gradient1337( area.x + 9 + 2, area.y + 2, size - 4, 16, m_accent.alpha( m_alpha( 55 * anim ) ), colors::black.alpha( m_alpha( 0 ) ) );
			render::esp.string( area.x + ( ( size - 4 ) / 2.f ), area.y + 3, colors::white.blend( m_accent, anim ).alpha( m_alpha( ) ), title, render::ALIGN_CENTER );

			anim -= 4.f * g_csgo.m_globals->m_frametime;
			anim = std::clamp( anim, 0.f, 1.f );

			if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + 9, area.y, size, 20 ) && use) {
				fn( );
				anim = 1.f;
			}

			area += vec2_t( 0, 25 );
		}
	};

	class hotkey_t {
	private:
		std::string title, var, mode;
		int key;
		float anim{ 0.f };
	public:
		bool open{ false };
		bool binding{ false };
	public:
		hotkey_t( std::string _t, std::string _v ) {
			title = _t; var = _v; mode = tfm::format( "%s_mode", _v );

			bool found = config.find( var ) != config.end( );
			if (!found) {
				config.insert( { var, value_t( 0 ) } );
			}

			bool found2 = config.find( mode ) != config.end( );
			if (!found2) {
				config.insert( { mode, value_t( 0 ) } );
			}
		}

		void render( vec2_t& area, bool use ) {
			auto tab = tab_area( false ).x - 20 - 2;
			key = config[var].get<int>( );

			animate( binding, 4.f, anim );

			render::esp.string( area.x, area.y, colors::white.alpha( m_alpha( ) ), title );

			auto size = render::esp.size( key_name( key ) ).m_width;
			render::esp.string( area.x + tab - size, area.y, colors::white.blend( m_accent, anim ).alpha( m_alpha( ) ), key_name( key ) );

			if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + tab - size, area.y, size, 15 ) && use) {
				binding = !binding;
			}

			if (binding) {
				for (int _key = 2; _key < 256; _key++) {
					if (g_input.GetKeyPress( _key )) {
						if (_key == VK_ESCAPE) {
							binding = false;
						}
						else if (_key == VK_BACK) {
							config[var].set<int>( 0 );
							binding = false;
						}
						else {
							config[var].set<int>( _key );
							binding = false;
						}
					}
				}
			}


			if (g_input.GetKeyPress( 0x02 )) {
				if (( g_input.hovered( area.x + tab - size, area.y, size, 15 ) && ( use || open ) ) || ( !g_input.hovered( area.x + tab - 50, area.y, 50, 40 ) && open ))
					open = !open;
			}

			area += vec2_t( 0, 20 );
		}

		void finish( vec2_t area ) {
			auto tab = tab_area( false ).x - 20 - 2;
			auto size{ vec2_t( 50, 40 ) };

			render::rect_filled( area.x + tab - size.x, area.y, size.x, size.y, palette::med.alpha( m_alpha( ) ) );
			render::rect( area.x + tab - size.x, area.y, size.x, size.y, palette::light.alpha( m_alpha( ) ) );

			std::vector<std::string> modes{ "hold", "toggle" };
			int i{ 0 };
			for (auto m : modes) {
				bool s = config[mode].get<int>( ) == i;

				if (s)
					render::gradient1337( area.x + tab - size.x + 2, area.y + 7 + ( i * 15 ), size.x - 4, 11, m_accent.alpha( m_alpha( 55 ) ), colors::black.alpha( m_alpha( 0 ) ) );

				render::esp.string( area.x + tab - size.x + 9 + ( 5 * int( s ) ), area.y + 5 + ( i * 15 ), ( s ? m_accent : colors::white ).alpha( m_alpha( ) ), m );

				if (g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + tab - size.x, area.y + 5 + ( i * 15 ), size.x, 15 )) {
					config[mode].set<int>( i );
					open = !open;
				}

				i++;
			}
		}
	};

	struct control_t {
		std::optional<checkbox_t> checkbox{ };
		void add_checkbox( checkbox_t c ) {
			checkbox = c;
		}

		std::optional<slider_t> slider{ };
		void add_slider( slider_t s ) {
			slider = s;
		}

		std::optional<dropdown_t> dropdown{ };
		void add_dropdown( dropdown_t d ) {
			dropdown = d;
		}

		std::optional<multidropdown_t> multidropdown{ };
		void add_multidropdown( multidropdown_t m ) {
			multidropdown = m;
		}

		std::optional<colorpicker_t> colorpicker{ };
		void add_colorpicker( colorpicker_t c ) {
			colorpicker = c;
		}

		std::optional<button_t> button{ };
		void add_button( button_t b ) {
			button = b;
		}

		std::optional<hotkey_t> hotkey{ };
		void add_hotkey( hotkey_t h ) {
			hotkey = h;
		}

		bool render( vec2_t& area = vec2_t( 0, 0 ), bool use = true ) {
			if (checkbox.has_value( )) {
				checkbox->render( area, use );
				return true;
			}

			if (slider.has_value( )) {
				slider->render( area, use );
				return true;
			}

			if (dropdown.has_value( )) {
				dropdown->render( area, use );
				return true;
			}

			if (multidropdown.has_value( )) {
				multidropdown->render( area, use );
				return true;
			}

			if (colorpicker.has_value( )) {
				colorpicker->render( area, use );
				return true;
			}

			if (button.has_value( )) {
				button->render( area, use );
				return true;
			}

			if (hotkey.has_value( )) {
				hotkey->render( area, use );
				return true;
			}

			return false;
		}

		void finish( vec2_t area ) {
			if (dropdown.has_value( ) && dropdown->open)
				return dropdown->finish( area );

			if (multidropdown.has_value( ) && multidropdown->open)
				return multidropdown->finish( area );

			if (colorpicker.has_value( ) && ( colorpicker->primary || colorpicker->secondary ))
				return colorpicker->finish( area );

			if (hotkey.has_value( ) && hotkey->open)
				return hotkey->finish( area );
		}

		bool contains( ) {
			return ( dropdown.has_value( ) && dropdown->open ) || ( multidropdown.has_value( ) && multidropdown->open ) || ( colorpicker.has_value( ) && ( colorpicker->primary || colorpicker->secondary ) ) || ( hotkey.has_value( ) && ( hotkey->open || hotkey->binding ) );
		}
	};

	inline int last_scroll{ 0 };
	class child_t {
	private:
		vec2_t size, offset, draw;
		int max_scroll{ 0 }, scroll{ 0 };
	public:
		std::vector<control_t> controls;
	public:
		auto& area( ) { return draw; }

		bool add_checkbox( checkbox_t* c ) {
			auto ctx = control_t( );
			ctx.add_checkbox( *c );
			controls.push_back( ctx );
			return true;
		}

		bool add_slider( slider_t* s ) {
			auto ctx = control_t( );
			ctx.add_slider( *s );
			controls.push_back( ctx );
			return true;
		}

		bool add_dropdown( dropdown_t* d ) {
			auto ctx = control_t( );
			ctx.add_dropdown( *d );
			controls.push_back( ctx );
			return true;
		}

		bool add_multidropdown( multidropdown_t* m ) {
			auto ctx = control_t( );
			ctx.add_multidropdown( *m );
			controls.push_back( ctx );
			return true;
		}

		bool add_colorpicker( colorpicker_t* c ) {
			auto ctx = control_t( );
			ctx.add_colorpicker( *c );
			controls.push_back( ctx );
			return true;
		}

		bool add_button( button_t* b ) {
			auto ctx = control_t( );
			ctx.add_button( *b );
			controls.push_back( ctx );
			return true;
		}

		bool add_hotkey( hotkey_t* h ) {
			auto ctx = control_t( );
			ctx.add_hotkey( *h );
			controls.push_back( ctx );
			return true;
		}

		void begin( std::string title, vec2_t _offset, vec2_t _size ) {
			offset = vec2_t( 107.5f, 10.f ) + _offset; size = _size;
			auto pos = m_pos + offset;

			render::rect_filled( pos.x, pos.y, size.x, size.y, palette::grey.alpha( m_alpha( ) ) );
			render::rect( pos.x, pos.y, size.x, size.y, palette::med.alpha( m_alpha( ) ) );

			render::rect_filled( pos.x, pos.y, size.x, 20, palette::med.alpha( m_alpha( ) ) );
			render::esp.string( pos.x + 5, pos.y + 3, colors::white.alpha( m_alpha( ) ), title );

			draw = pos + vec2_t( 8, 28 ) - vec2_t( 0, abs( scroll ) );
		}

		void finish( ) {
			auto pos = m_pos + offset;
			auto og_draw = pos + vec2_t( 8, 28 ) - vec2_t( 0, abs( scroll ) );

			control_t topmost{ };
			vec2_t _pos;
			for (auto& c : controls) {
				if (c.contains( )) {
					topmost = c;
					_pos = area( );
				}

				c.render( area( ), !topmost.contains( ) );
			}
			if (topmost.contains( ))
				topmost.finish( _pos );

			auto diff = ( draw.y - og_draw.y ) + ( -5 + abs( scroll ) );
			max_scroll = diff;
			float s_diff = diff - size.y;

			if (last_scroll != g_csgo.m_input_system->get_analog_value( analog_code_t::MOUSE_WHEEL ) && g_input.hovered( pos.x, pos.y, size.x, size.y ) && s_diff >= 0) {
				scroll += g_csgo.m_input_system->get_analog_delta( analog_code_t::MOUSE_WHEEL ) * 8;
				last_scroll = g_csgo.m_input_system->get_analog_value( analog_code_t::MOUSE_WHEEL );
				scroll = std::clamp( scroll, int( -( s_diff + 10 ) ), 0 );
			}
		}
	};
}