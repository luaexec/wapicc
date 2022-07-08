#pragma once

// modelled after the original valve 'developer 1' debug console
// https://github.com/LestaD/SourceEngine2007/blob/master/se2007/engine/console.cpp

class NotifyText {
public:
	std::string m_text;
	Color		m_color;
	float		m_time;

public:
	__forceinline NotifyText( const std::string& text, Color color, float time ) : m_text{ text }, m_color{ color }, m_time{ time } {}
	__forceinline float remainder( ) { return g_csgo.m_globals->m_curtime - m_time; }
};

class Notify {
private:
	std::vector< std::shared_ptr< NotifyText > > m_notify_text;

public:
	__forceinline Notify( ) : m_notify_text{} {}

	__forceinline void add( const std::string& text, Color color = colors::white, float time = 5.f, bool console = true ) {
		m_notify_text.push_back( std::make_shared< NotifyText >( text, color, g_csgo.m_globals->m_curtime + time ) );

		if ( console )
			g_cl.print( text );
	}

	void think( Color nigga ) {
		int y{ 0 }, palpha{ -1 };

		auto out_quart = [&]( double t ) {
			t = ( --t ) * t;
			return 1 - t * t;
		};

		auto out_quint = [&]( double t ) {
			const double t2 = ( --t ) * t;
			return 1 + t * t2 * t2;
		};

		auto in_quint = [&]( double t ) {
			const double t2 = t * t;
			return t * t2 * t2;
		};

		auto inout_sine = [&]( double t ) {
			return 0.5 * ( 1 + sin( 3.1415926 * ( t - 0.5 ) ) );
		};

		if ( m_notify_text.empty( ) )
			return;

		for ( size_t idx{ }; idx < m_notify_text.size( ); ++idx ) {
			auto info = m_notify_text[idx].get( );

			auto size = render::esp.size( std::string( "wapicc" ).append( info->m_text ) ).m_width;

			float start = 5 - info->remainder( );
			start = std::clamp( start, 0.f, 1.f );

			float end = info->remainder( );
			end = std::clamp( end, 0.f, 1.f );
			float end_a = 1 - end;

			if ( end_a != 1.f )
				y -= int( 15.f * out_quart( end_a ) );

			float alpha_multiplier = end_a != 0 ? out_quint( end ) : start != 1 ? inout_sine( start ) : 1.f;

			auto accent_s = nigga.alpha( 255 );
			accent_s.a( ) *= alpha_multiplier;

			auto accent_e = info->m_color.alpha( 255 );
			accent_e.a( ) *= in_quint( alpha_multiplier );

			int max_logs = 10;

			bool over = ( m_notify_text.size( ) > max_logs ) && ( m_notify_text.size( ) - max_logs > idx );
			if ( info->remainder( ) <= 0 || over ) {
				m_notify_text.erase( m_notify_text.begin( ) + idx );
				continue;
			}

			render::esp.string( 7 * out_quart( start ), y, accent_s, "wapicc" );
			render::esp.string( 7 * out_quart( start ) + render::esp.size( "wapicc" ).m_width + 4, y, accent_e, info->m_text );

			y += 15;
			palpha = 255 * alpha_multiplier;
		}

	}
};

extern Notify g_notify;