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
		if ( condition ) { anim += speed * g_csgo.m_globals->m_frametime; }
		else { anim -= speed * g_csgo.m_globals->m_frametime; }
		anim = std::clamp( anim, 0.f, 1.f );
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

			if ( g_input.GetKeyPress( VK_INSERT ) )
				m_open = !m_open;

			static vec2_t c_pos{}, c_mpos{};
			static auto c_press = false;
			if ( g_input.hovered( m_pos, m_size.x / 2, 15 ) && g_input.GetKeyPress( 0x01 ) )
				c_press = true;

			if ( c_press ) {
				m_pos.x = c_mpos.x + ( ( g_input.m_mouse.x ) - c_pos.x );
				m_pos.y = c_mpos.y + ( ( g_input.m_mouse.y ) - c_pos.y );
				c_press = g_input.GetKeyState( 0x01 );
			}
			else { c_pos = vec2_t( g_input.m_mouse.x, g_input.m_mouse.y ); c_mpos = m_pos; c_press = false; }

			static vec2_t c_size{}, c_m_size{};
			static auto c_spress = false;
			if ( g_input.hovered( m_pos + m_size - vec2_t( 8, 8 ), 8, 8 ) && g_input.GetKeyPress( 0x01 ) )
				c_spress = true;

			if ( c_spress ) {
				m_size.x = c_m_size.x + ( ( g_input.m_mouse.x ) - c_size.x );
				m_size.y = c_m_size.y + ( ( g_input.m_mouse.y ) - c_size.y );
				c_spress = g_input.GetKeyState( 0x01 );
				m_size.x = std::clamp( m_size.x, 350.f, 1500.f ); m_size.y = std::clamp( m_size.y, 350.f, 1000.f );
			}
			else { c_size = vec2_t( g_input.m_mouse.x, g_input.m_mouse.y ); c_m_size = m_size; c_spress = false; }

			animate( ( c_spress || c_press ), 4.f, motion_anim );

			render::rect_filled( m_pos.x, m_pos.y, m_size.x, m_size.y, palette::dark.alpha( m_alpha( ) ) );
			render::rect( m_pos.x, m_pos.y, m_size.x, m_size.y, ( palette::grey.blend( m_accent, motion_anim ) ).alpha( m_alpha( ) ) );

			render::rect_filled( m_pos.x + 1, m_pos.y + 1, 100, m_size.y - 2, palette::grey.alpha( m_alpha( ) ) );

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
			render::rect_filled( area.x + 2, area.y + 2, 4, 4, m_accent.alpha( m_alpha( 255.f * anim ) ) );

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
				render::rect_filled( area.x + 11, area.y + 17, ( ( size - 4.f ) * float( config[var].get<float>( ) / float( max ) ) ), 4, m_accent.alpha( m_alpha( ) ) );
			}
			else {
				int bar = int( ( ( size / 2 ) - 2.f ) * float( float( abs( config[var].get( ) ) ) / float( max ) ) ), offset = ( config[var].get( ) >= 0 ? 0 : bar );
				render::rect_filled( area.x + 11 + ( ( size / 2 ) - 2.f ) - offset, area.y + 17, bar, 4, m_accent.alpha( m_alpha( ) ) );
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
		dropdown_t( std::string _t, std::string _v, value_t def, std::vector<std::string> _e) {
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

			if ( g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + 9, area.y + 15, size, 20 ) && ( use || open ) )
				open = !open;

			area += vec2_t( 0, 42 );
		}

		void finish( vec2_t area ) {
			float size = tab_area( false ).x - 30.f;
			render::rect_filled( area.x + 9, area.y + 35, size, 10 + (elements.size( ) * 15), palette::med.alpha( m_alpha( 255.f * anim ) ) );
			render::rect( area.x + 9, area.y + 35, size, 10 + ( elements.size( ) * 15 ), palette::light.alpha( m_alpha( 255.f * anim ) ) );

			int i{ 0 }, selection{ config[var].get<int>( ) };
			for (auto e : elements) {
				render::esp.string( area.x + 12, area.y + 40 + ( i * 15 ), ( selection == i ? m_accent : colors::white ).alpha( m_alpha( 255.f * anim ) ), e );

				if ( g_input.GetKeyPress( 0x01 ) && g_input.hovered( area.x + 9, area.y + 40 + ( i * 15 ), size, 15 ) ) {
					config[var].set<int>( i );
					open = false;
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

			return false;
		}

		void finish( vec2_t area ) {
			if (dropdown.has_value( ) && dropdown->open)
				return dropdown->finish( area );
		}

		bool contains( ) {
			return ( dropdown.has_value( ) && dropdown->open );
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