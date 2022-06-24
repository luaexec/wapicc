#pragma once
#include "includes.h"

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

	namespace palette {
		inline auto dark = Color( 26, 26, 31 );
		inline auto grey = Color( 43, 43, 51 );
		inline auto med = Color( 49, 49, 59 );
		inline auto light = Color( 58, 58, 69 );
	}

	class window_t {
	private:
		float motion_anim{ 0.f };
	public:
	public:
		bool begin( std::string title ) {
			animate( m_open, 4.5f, m_anim );

			if ( g_input.GetKeyPress( VK_INSERT ) )
				m_open = !m_open;

			static vec2_t c_pos{}, c_mpos{};
			static bool c_press = false;
			if ( g_input.hovered( m_pos, m_size.x - 25, 25 ) && g_input.GetKeyPress( 0x01 ) )
				c_press = true;

			if ( c_press ) {
				m_pos.x = c_mpos.x + ( ( g_input.m_mouse.x ) - c_pos.x );
				m_pos.y = c_mpos.y + ( ( g_input.m_mouse.y ) - c_pos.y );
				c_press = g_input.GetKeyState( 0x01 );
			}
			else { c_pos = vec2_t( g_input.m_mouse.x, g_input.m_mouse.y ); c_mpos = m_pos; c_press = false; }

			static vec2_t c_size{}, c_m_size{};
			static bool c_spress = false;
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

			render::rect_filled( m_pos.x + 1, m_pos.y + 1, m_size.x - 2, 20, palette::grey.alpha( m_alpha( ) ) );
			render::rect( m_pos.x + 2, m_pos.y + 19, m_size.x - 4, 12, m_accent.alpha( m_alpha( ) ) );

			return m_open && m_anim > 0.f;
		}

		void finish( ) {}
	};
}