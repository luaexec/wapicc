#pragma once

// modelled after the original valve 'developer 1' debug console
// https://github.com/LestaD/SourceEngine2007/blob/master/se2007/engine/console.cpp

class NotifyText {
public:
	std::string m_text;
	Color		m_color;
	float		m_time;

public:
	__forceinline NotifyText(const std::string& text, Color color, float time) : m_text{ text }, m_color{ color }, m_time{ time } {}
};

class Notify {
private:
	std::vector< std::shared_ptr< NotifyText > > m_notify_text;

public:
	__forceinline Notify() : m_notify_text{} {}

	__forceinline void add(const std::string& text, Color color = colors::accent, float time = 5.f, bool console = true) {
		// modelled after 'CConPanel::AddToNotify'
		m_notify_text.push_back(std::make_shared< NotifyText >(text, color, time));

		if (console)
			g_cl.print(text);
	}

	// modelled after 'CConPanel::DrawNotify' and 'CConPanel::ShouldDraw'
	void think() {
		int		x{ 8 }, y{ 5 }, size{ render::esp.m_size.m_height + 1 };
		Color	color;

		// update lifetimes.
		for (size_t i{}; i < m_notify_text.size(); ++i) {
			auto notify = m_notify_text[i];

			notify->m_time -= g_csgo.m_globals->m_frametime;

			if (notify->m_time <= 0.f) {
				m_notify_text.erase(m_notify_text.begin() + i);
				continue;
			}
		}

		while (m_notify_text.size( ) > 10)
			m_notify_text.pop_back( );

		// we have nothing to draw.
		if (m_notify_text.empty())
			return;

		// iterate entries.
		for (size_t i{}; i < m_notify_text.size(); ++i) {
			auto notify = m_notify_text[i];

			auto s = std::min( 5.f - notify->m_time, 0.5f ) * 2.f;
			auto e = std::min( notify->m_time, 0.5f ) * 2.f;
			auto t = std::min( s, e );
			color = notify->m_color;

			render::esp.string( x - ( render::esp.size( "__stdcall" ).m_width + 5 ) + ( (render::esp.size( "__stdcall" ).m_width + 5) * t ), y, color.alpha( 255.f * t ), "__stdcall" );
			render::esp.string( x + ( render::esp.size( "__stdcall" ).m_width + 5 ) - render::esp.size( notify->m_text ).m_width + ( render::esp.size( notify->m_text ).m_width * t ), y, colors::white.alpha( 255.f * t ), notify->m_text );
			y += size;
		}
	}
};

extern Notify g_notify;