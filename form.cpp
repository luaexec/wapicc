#include "includes.h"

void Form::draw( ) {
	// opacity should reach 1 in 500 milliseconds.
	constexpr float frequency = 1.f / 0.5f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// if open		-> increment
	// if closed	-> decrement
	m_open ? m_opacity += step : m_opacity -= step;

	// clamp the opacity.
	math::clamp( m_opacity, 0.f, 1.f );

	m_alpha = 255;
	if (!m_open)
		return;

	// background.
	render::rect_filled( m_x, m_y, m_width, m_height, g_gui.background );

	// élement "gr oupboxes"
	for (int i = 0; i < 2; ++i)
	{
		//bg
		render::rect_filled( m_x + 11 + ( i * ( ( m_width / 2 ) - 5 ) - 1 ), m_y + 10, ( m_width / 2 ) - 15, m_height - 22, g_gui.background );

		//render::gradient(m_x + 11 + (i * ((m_width / 2) - 5) - 1) + 1, m_y + 10 + 1, ((m_width / 2) - 15) / 2, 1, Color(0, 0, 0, 0), g_gui.m_color, true);
		//render::gradient(m_x + 11 + (i * ((m_width / 2) - 5) - 1) + 1 + ((m_width / 2) - 15) / 2, m_y + 10 + 1, ((m_width / 2) - 15) / 2, 1, g_gui.m_color, Color(0, 0, 0, 0), true);


		//outline
		render::rect( m_x + 11 + ( i * ( ( m_width / 2 ) - 5 ) - 1 ), m_y + 10, ( m_width / 2 ) - 15, m_height - 22, g_gui.dark_border );
		render::rect( m_x + 11 + ( i * ( ( m_width / 2 ) - 5 ) - 1 ) - 1, m_y + 10 - 1, ( m_width / 2 ) - 15 + 2, m_height - 22 + 2, g_gui.light_border );

	}


	if (!m_tabs.empty( )) {
		// tabs background and border.
		Rect tabs_area = GetTabsRect( );
		render::rect_filled( tabs_area.x, tabs_area.y, tabs_area.w, tabs_area.h, g_gui.background );
		//render::gradient(tabs_area.x + 1, tabs_area.y + 1, tabs_area.w / 2, 1, Color(0, 0, 0, 0), g_gui.m_color, true);
		//render::gradient(tabs_area.x + 1 + tabs_area.w / 2, tabs_area.y + 1, tabs_area.w / 2, 1, g_gui.m_color, Color(0, 0, 0, 0), true);
		render::rect( tabs_area.x, tabs_area.y, tabs_area.w, tabs_area.h, g_gui.light_border );
		for (short i = 1; i < 4; i++)
			render::rect( tabs_area.x - i, tabs_area.y - i, tabs_area.w + i * 2, tabs_area.h + i * 2, g_gui.wide_border );
		render::rect( tabs_area.x - 1, tabs_area.y - 1, tabs_area.w + 2, tabs_area.h + 2, g_gui.dark_border );
		size_t font_w = 8;

		render::rect( tabs_area.x - 1, tabs_area.y - 1, tabs_area.w + 2, tabs_area.h + 2, g_gui.dark_border );
		render::menu.string( tabs_area.x + tabs_area.w - render::menu.size( XOR( "wapicc" ) ).m_width - 5, tabs_area.y + 6, g_gui.font_color, XOR( "wapicc" ) );
		//render::gradient(m_x + 1, m_y + 1, m_width / 2, 1, Color(0, 0, 0, 0), g_gui.m_color, true);
		//render::gradient(m_x + 1 + m_width / 2, m_y + 1, m_width / 2, 1, g_gui.m_color, Color(0, 0, 0, 0), true);
		render::rect( m_x, m_y, m_width, m_height, g_gui.light_border );
		for (short i = 1; i < 4; i++)
			render::rect( m_x - i, m_y - i, m_width + i * 2, m_height + i * 2, g_gui.wide_border );
		render::rect( m_x - 1, m_y - 1, m_width + 2, m_height + 2, g_gui.dark_border );
		for (size_t i{}; i < m_tabs.size( ); ++i) {
			const auto& t = m_tabs[i];

			// text
			render::menu.string( tabs_area.x + font_w, tabs_area.y + 6,
				t == m_active_tab ? g_gui.m_color : g_gui.font_color, t->m_title );
			font_w += render::menu.size( t->m_title ).m_width + 5;

			// active tab indicator
	/*		render::rect_filled_fade(tabs_area.x + (i * (tabs_area.w / m_tabs.size())) + 10, tabs_area.y + 14,
				font_width + 11, 2,
				t == m_active_tab ? Color{ 0, 0, 0, 0 } : Color{ 0, 0, 0, m_alpha }, 0, 150);*/


		}

		// this tab has elements.
		if (!m_active_tab->m_elements.empty( )) {
			// elements background and border.
			Rect el = GetElementsRect( );

			// iterate elements to display.
			for (const auto& e : m_active_tab->m_elements) {

				// draw the active element last.
				if (!e || ( m_active_element && e == m_active_element ))
					continue;

				if (!e->m_show)
					continue;

				// this element we dont draw.
				if (!( e->m_flags & ElementFlags::DRAW ))
					continue;

				e->draw( );
			}

			// we still have to draw one last fucker.
			if (m_active_element && m_active_element->m_show)
				m_active_element->draw( );
		}
		//so other shit doesnt overlap it
		// border.



	}
}