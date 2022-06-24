#pragma once
#include "framework.h"

namespace wapim {
	__forceinline void render( ) {
		static auto window = std::make_unique<gui::window_t>( );
		if ( window->begin( XOR( "__stdcall" ) ) ) {

			// RENDER_CONT.

		} window->finish( );
	}
}