#pragma once
#include "framework.h"

namespace wapim {
	__forceinline void rage( ) {

		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

		}
		general->finish( );

		static auto accuracy = std::make_unique<gui::child_t>( );
		accuracy->begin( "accuracy", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

		}
		accuracy->finish( );

	}

	__forceinline void antiaim( ) {
		static bool r{ false };
		
		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{
			 
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
			
			static auto fakeflick = std::make_unique<gui::hotkey_t>(
				"fake flick", "aa_fflick"
			);
			if (!r)
				other->add_hotkey( fakeflick.get( ) );

		}
		other->finish( );

		if (!r)
			r = true;
	}

	__forceinline void player( ) {
		
		static auto overlay = std::make_unique<gui::child_t>( );
		overlay->begin( "overlay", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{
			static bool r{ false };

			static auto name = std::make_unique<gui::checkbox_t>(
				"player name", "esp_name", value_t( false )
			);
			static auto name_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_name_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( name.get( ) );
				overlay->add_colorpicker( name_clr.get( ) );
			}

			static auto box = std::make_unique<gui::checkbox_t>(
				"bounding box", "esp_box", value_t( false )
			);
			static auto box_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_box_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( box.get( ) );
				overlay->add_colorpicker( box_clr.get( ) );
			}

			static auto hp = std::make_unique<gui::checkbox_t>(
				"health bar", "esp_hp", value_t( false )
			);
			static auto hp_full = std::make_unique<gui::colorpicker_t>(
				"full", "esp_hp_full", value_t( colors::green ), true, vec2_t( -20, 0 )
			);
			static auto hp_empty = std::make_unique<gui::colorpicker_t>(
				"empty", "esp_hp_empty", value_t( colors::red ), true
			);
			if (!r) {
				overlay->add_checkbox( hp.get( ) );
				overlay->add_colorpicker( hp_full.get( ) );
				overlay->add_colorpicker( hp_empty.get( ) );
			}

			static auto ammo = std::make_unique<gui::checkbox_t>(
				"ammo bar", "esp_ammo", value_t( false )
			);
			static auto ammo_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_ammo_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( ammo.get( ) );
				overlay->add_colorpicker( ammo_clr.get( ) );
			}

			static auto lby = std::make_unique<gui::checkbox_t>(
				"lby bar", "esp_lby", value_t( false )
			);
			static auto lby_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_lby_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( lby.get( ) );
				overlay->add_colorpicker( lby_clr.get( ) );
			}

			static auto wpn_txt = std::make_unique<gui::checkbox_t>(
				"weapon text", "esp_wpnt", value_t( false )
			);
			static auto wpn_txt_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_wpnt_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( wpn_txt.get( ) );
				overlay->add_colorpicker( wpn_txt_clr.get( ) );
			}

			static auto wpn_ico = std::make_unique<gui::checkbox_t>(
				"weapon icon", "esp_wpni", value_t( false )
			);
			static auto wpn_ico_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_wpni_clr", value_t( colors::white ), true
			);
			if (!r) {
				overlay->add_checkbox( wpn_ico.get( ) );
				overlay->add_colorpicker( wpn_ico_clr.get( ) );
			}

			static auto flags = std::make_unique<gui::checkbox_t>(
				"flags", "esp_flags", value_t( false )
			);
			if (!r)
				overlay->add_checkbox( flags.get( ) );

			if (!r)
				r = true;

		}
		overlay->finish( );

	}

	__forceinline void world( ) {

		static auto world = std::make_unique<gui::child_t>( );
		world->begin( "world", vec2_t( 0, 0 ), gui::tab_area( true ) );
		{

		}
		world->finish( );

		static auto draw = std::make_unique<gui::child_t>( );
		draw->begin( "draw", vec2_t( 0.f, gui::tab_area( true ).y + 7.5f ), gui::tab_area( true ) );
		{

		}
		draw->finish( );

		static auto other = std::make_unique<gui::child_t>( );
		other->begin( "other", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

		}
		other->finish( );
	}

	__forceinline void misc( ) {
		static bool r{ false };

		static auto movement = std::make_unique<gui::child_t>( );
		movement->begin( "movement", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			static auto bhop = std::make_unique<gui::checkbox_t>(
				"enable", "misc_bhop", value_t( false )
			);
			if (!r)
				movement->add_checkbox( bhop.get( ) );

			static auto strafe = std::make_unique<gui::checkbox_t>(
				"autostrafer", "misc_strafe", value_t( false )
			);
			if (!r)
				movement->add_checkbox( strafe.get( ) );

			static auto smooth = std::make_unique<gui::slider_t>(
				"smoothing", "misc_strafe_smooth", value_t( 50 ), 0, 100, "%"
			);
			if (!r)
				movement->add_slider( smooth.get( ) );

			static auto slide = std::make_unique<gui::checkbox_t>(
				"force slide", "misc_slide", value_t( false )
			);
			if (!r)
				movement->add_checkbox( slide.get( ) );

			static auto autopeek = std::make_unique<gui::hotkey_t>(
				"quick peek assist", "misc_peek"
			);
			if (!r)
				movement->add_hotkey( autopeek.get( ) );

			static auto fakewalk = std::make_unique<gui::hotkey_t>(
				"fakewalk", "misc_walk"
			);
			if (!r)
				movement->add_hotkey( fakewalk.get( ) );

		}
		movement->finish( );

		static auto settings = std::make_unique<gui::child_t>( );
		settings->begin( "settings", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

			static auto accent = std::make_unique<gui::colorpicker_t>(
				"menu accent", "menu_accent", value_t( Color( 242, 97, 97 ) )
			);
			if (!r)
				settings->add_colorpicker( accent.get( ) );

			static auto cfg = std::make_unique<gui::dropdown_t>(
				"config list", "cfg_list", value_t( 0 ), std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }
			);
			if (!r)
				settings->add_dropdown( cfg.get( ) );

			static std::function<void( )> save_fn = [&]( ) {
				std::string cur_cfg = std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }[config["cfg_list"].get<int>( )];
				cur_cfg.append( ".ini" );
				cfg_t::save( cur_cfg );
			};
			static auto save = std::make_unique<gui::button_t>(
				"save", save_fn
			);
			if (!r)
				settings->add_button( save.get( ) );

			static std::function<void( )> load_fn = [&]( ) {
				std::string cur_cfg = std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }[config["cfg_list"].get<int>( )];
				cur_cfg.append( ".ini" );
				cfg_t::load( cur_cfg );
			};
			static auto load = std::make_unique<gui::button_t>(
				"load", load_fn
				);
			if (!r)
				settings->add_button( load.get( ) );

		}
		settings->finish( );

		if (!r)
			r = true;
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