#include "includes.h"

void Checkbox::draw() {
	Rect area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y - 2 };

	// get gui color.
	Color color = g_gui.m_color;
	color.a() = m_parent->m_alpha;

	// render black outline on checkbox.
	render::rect(p.x, p.y, CHECKBOX_SIZE, CHECKBOX_SIZE, g_gui.dark_border);

	// render checkbox title.
	if (m_use_label)
		render::menu.string(p.x + LABEL_OFFSET, p.y, { 205, 205, 205, m_parent->m_alpha }, m_label);

	// render border.
	render::rect(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, g_gui.light_border);

	// render checked.
	if (m_checked) {
		render::rect_filled(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, color);
		render::rect_filled_fade(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 50, 50, 35, m_parent->m_alpha }, 0, 150);
		/*for (int i = 2; i < CHECKBOX_SIZE - 2; i++)
		{
			render::rect(p.x + i, p.y + i, 1, 1, color);
			render::rect(p.x + i, p.y + CHECKBOX_SIZE - i - 1, 1, 1, color);
		}*/

	}

	
	// draw inside
//	render::gradient ( p.x + BUTTON_X_OFFSET + 1, p.y + 1, m_w - BUTTON_X_OFFSET - 2, BUTTON_BOX_HEIGHT - 2, { 30, 30, 30, m_parent->m_alpha }, { 45, 45, 45, m_parent->m_alpha } );

	// draw border.
//	render::rect ( p.x + BUTTON_X_OFFSET, p.y, m_w - BUTTON_X_OFFSET, BUTTON_BOX_HEIGHT, { 42, 42, 42, m_parent->m_alpha } );

	// draw outer border.
	//render::rect ( p.x + BUTTON_X_OFFSET - 1, p.y, m_w - BUTTON_X_OFFSET + 2, BUTTON_BOX_HEIGHT, { 11, 11, 11, m_parent->m_alpha } );

	//render::rect( el.x + m_pos.x, el.y + m_pos.y, m_w, m_pos.h, { 255, 0, 0 } );
}

void Checkbox::think() {
	// set the click area to the length of the string, so we can also press the string to toggle.
	if (m_use_label)
		m_w = LABEL_OFFSET + render::menu.size(m_label).m_width;
}

void Checkbox::click() {
	// toggle.
	m_checked = !m_checked;

	if (m_callback)
		m_callback();
}