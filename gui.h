#pragma once

class GUI {
private:
	std::vector< Form* > m_forms;

public:
	bool  m_open;
	Form* m_drag_form;
	Point m_drag_offset;
	Color m_color{ colors::white };
	Color wide_border{ Color( 32,32,32 ) };
	Color dark_border{ Color( 9,9,9 ) };
	Color light_border{ Color( 49,49,49 ) };
	Color font_color{ Color( 150,150,150 ) };
	Color background{ Color( 23,23,23 ) };

	bool open = false;
	bool dragging = false;
	bool setup = false;
	bool typing = false;

public:
	void think( );
	void draw( );

	// registers a new form.
	__forceinline void RegisterForm( Form* form, int key = -1 ) {
		// set key to toggle form.
		form->SetToggle( key );

		// add form to our container.
		m_forms.push_back( form );
	}
};

class Input {
public:
	struct key_t {
		bool down;
		bool pressed;
		int  tick;
		int  oldtick;
	};

	std::array< key_t, 256 > m_keys;
	Point					 m_mouse;
	std::string				 m_buffer;

public:
	__forceinline void update( ) {
		// iterate all keys.
		for (int i{}; i <= 254; ++i) {
			key_t* key = &m_keys[i];

			key->pressed = false;

			if (key->down && key->tick > key->oldtick) {
				key->oldtick = key->tick;
				key->pressed = true;
			}
		}
	}

	// mouse within coords.
	__forceinline bool IsCursorInBounds( int x, int y, int x2, int y2 ) const {
		return m_mouse.x > x && m_mouse.y > y && m_mouse.x < x2&& m_mouse.y < y2;
	}

	// mouse within rectangle.
	__forceinline bool IsCursorInRect( Rect area ) const {
		return IsCursorInBounds( area.x, area.y, area.x + area.w, area.y + area.h );
	}

	__forceinline bool hovered( vec2_t pos, vec2_t size ) {
		return ( m_mouse.x > pos.x && m_mouse.x < pos.x + size.x && m_mouse.y > pos.y && m_mouse.y < pos.y + size.y );
	}

	__forceinline bool hovered( int x, int y, vec2_t size ) {
		return ( m_mouse.x > x && m_mouse.x < x + size.x && m_mouse.y > y && m_mouse.y < y + size.y );
	}

	__forceinline bool hovered( vec2_t pos, int w, int h ) {
		return ( m_mouse.x > pos.x && m_mouse.x < pos.x + w && m_mouse.y > pos.y && m_mouse.y < pos.y + h );
	}

	__forceinline bool hovered( int x, int y, int w, int h ) {
		return ( m_mouse.x > x && m_mouse.x < x + w && m_mouse.y > y && m_mouse.y < y + h );
	}

	__forceinline void SetDown( int vk ) {
		key_t* key = &m_keys[vk];

		key->down = true;
		key->tick = g_winapi.GetTickCount( );
	}

	__forceinline void SetUp( int vk ) {
		key_t* key = &m_keys[vk];
		key->down = false;
	}

	// key is being held.
	__forceinline bool GetKeyState( int vk ) {
		if (vk == -1)
			return false;

		return m_keys[vk].down;
	}

	// key was pressed.
	__forceinline bool GetKeyPress( int vk ) {
		if (vk == -1)
			return false;

		key_t* key = &m_keys[vk];
		return key->pressed;
	}
};

extern GUI	 g_gui;
extern Input g_input;