#pragma once
#include "framework.h"

namespace wapim {
	__forceinline void rage( ) {

		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			static auto checkbox = std::make_unique<gui::checkbox_t>(
				"checkbox", "checkbox", value_t( false )
			);
			static auto _checkbox = general->add_checkbox( checkbox.get( ) );

			static auto slider = std::make_unique<gui::slider_t>(
				"slider", "slider", value_t( 50 ), 0, 100, "%"
				);
			static auto _slider = general->add_slider( slider.get( ) );

			static auto slider2 = std::make_unique<gui::slider_t>(
				"slider2", "slider2", value_t( 50 ), -100, 100, " XD"
				);
			static auto _slider2 = general->add_slider( slider2.get( ) );

			static auto dropdown = std::make_unique<gui::dropdown_t>(
				"dropdown", "dropdown", value_t( 0 ), std::vector<std::string>{ "first", "second", "third" }
			);
			static auto _dropdown = general->add_dropdown( dropdown.get( ) );

		}
		general->finish( );

		static auto accuracy = std::make_unique<gui::child_t>( );
		accuracy->begin( "accuracy", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{
			
		}
		accuracy->finish( );

	}

	__forceinline void antiaim( ) {
		
		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{
			// GENERAL.
		}
		general->finish( );

		static auto fakelag = std::make_unique<gui::child_t>( );
		fakelag->begin( "fakelag", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( true ) );
		{
			
			static auto enable = std::make_unique<gui::checkbox_t>(
				"enable", "fl_enable", value_t( false )
			);
			static auto _enable = fakelag->add_checkbox( enable.get( ) );

			static auto mode = std::make_unique<gui::dropdown_t>(
				"mode", "fl_mode", value_t( 0 ), std::vector<std::string>{ "maximum", "dynamic", "fluctuate" }
			);
			static auto _mode = fakelag->add_dropdown( mode.get( ) );

			static auto limit = std::make_unique<gui::slider_t>(
				"limit", "fl_limit", value_t( 2 ), 0, 14, "T"
			);
			static auto _limit = fakelag->add_slider( limit.get( ) );

			static auto variance = std::make_unique<gui::slider_t>(
				"variance", "fl_var", value_t( 0 ), 0, 100, "%"
				);
			static auto _variance = fakelag->add_slider( variance.get( ) );

			static auto lagcomp = std::make_unique<gui::checkbox_t>(
				"break lagcomp", "fl_lc", value_t( false )
			);
			static auto _lagcomp = fakelag->add_checkbox( lagcomp.get( ) );

		}
		fakelag->finish( );

		static auto other = std::make_unique<gui::child_t>( );
		other->begin( "other", vec2_t( gui::tab_area( false ).x + 7.5f, gui::tab_area( true ).y + 7.5f ), gui::tab_area( true ) );
		{
			// GENERAL.
		}
		other->finish( );

	}

	__forceinline void player( ) {
		//
	}

	__forceinline void world( ) {
		//
	}

	__forceinline void misc( ) {
		//
	}

	__forceinline void render( ) {
		static auto window = std::make_unique<gui::window_t>( std::vector<std::string>{
			"rage", "antiaim", "player", "world", "misc"
		} );

		if (window->begin( "wapicc" )) {

			switch (window->cur_tab) {
			case 0:
				rage( );
				break;
			case 1:
				antiaim( );
				break;
			case 2:
				player( );
				break;
			case 3:
				world( );
				break;
			case 4:
				misc( );
				break;
			default:
				break;
			}

		}
	}
}