#pragma once
#include "framework.h"

namespace wapim {
	__forceinline void rage( ) {
		static bool r{ false };

		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto enable = std::make_unique<gui::checkbox_t>(
					"enable", "rage_enable", value_t( false )
					);
				general->add_checkbox( enable.get( ) );

				auto selection = std::make_unique<gui::dropdown_t>(
					"target selection", "rage_ts", value_t( 0 ), std::vector<std::string>{
					"field of view", "highest damage", "lowest health", "least delay"
				} );
				general->add_dropdown( selection.get( ) );

				auto silent = std::make_unique<gui::checkbox_t>(
					"silent-aim", "rage_silent", value_t( true )
					);
				general->add_checkbox( silent.get( ) );

				auto hitbox = std::make_unique<gui::multidropdown_t>(
					"hitboxes", std::vector<gui::multitems_t>{
					gui::multitems_t( "head", "rage_hb_head" ), gui::multitems_t( "upper body", "rage_hb_ubody" ), gui::multitems_t( "body", "rage_hb_body" ), gui::multitems_t( "pelvis", "rage_hb_pelvis" ), gui::multitems_t( "arms", "rage_hb_arm" ), gui::multitems_t( "legs", "rage_hb_leg" ), gui::multitems_t( "feet", "rage_hb_foot" )
				} );
				general->add_multidropdown( hitbox.get( ) );

				auto mpscale = std::make_unique<gui::slider_t>(
					"multipoint-scale", "rage_scale", value_t( 0 ), 0.f, 100.f, "%"
					);
				general->add_slider( mpscale.get( ) );

				auto hitchance = std::make_unique<gui::slider_t>(
					"hitchance", "rage_hitchance", value_t( 0 ), 0.f, 100.f, "%"
					);
				general->add_slider( hitchance.get( ) );

				auto damage = std::make_unique<gui::slider_t>(
					"minimum damage", "rage_dmg", value_t( 0 ), 0.f, 100.f, "DMG"
					);
				general->add_slider( damage.get( ) );

			}

		}
		general->finish( );

		static auto accuracy = std::make_unique<gui::child_t>( );
		accuracy->begin( "accuracy", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto scope = std::make_unique<gui::checkbox_t>(
					"autoscope", "acc_scope", value_t( false )
					);
				accuracy->add_checkbox( scope.get( ) );

				auto autostop = std::make_unique<gui::dropdown_t>(
					"autostop", "rage_stop", value_t( 0 ), std::vector<std::string>{
					"standard", "force stop", "fake walk"
				} );
				accuracy->add_dropdown( autostop.get( ) );

				auto resolver = std::make_unique<gui::checkbox_t>(
					"anti-aim correction", "acc_correct", value_t( false )
					);
				accuracy->add_checkbox( resolver.get( ) );

				auto res_modes = std::make_unique<gui::multidropdown_t>(
					"correction additions", std::vector<gui::multitems_t>{
					gui::multitems_t( "fake flick", "acc_correct_fflick" ), gui::multitems_t( "lower-body update", "acc_correct_upd" ), gui::multitems_t( "onshot", "acc_correct_os" ), gui::multitems_t( "choking flick", "acc_correct_fw" ), gui::multitems_t( "last-moving", "acc_correct_lm" )
				} );
				accuracy->add_multidropdown( res_modes.get( ) );

				auto prefer_head = std::make_unique<gui::multidropdown_t>(
					"prefer head conditions", std::vector<gui::multitems_t>{
					gui::multitems_t( "always", "rage_head_always" ), gui::multitems_t( "moving", "rage_head_move" ), gui::multitems_t( "adaptive", "rage_head_resolve" )
				} );
				accuracy->add_multidropdown( prefer_head.get( ) );

				auto prefer_body = std::make_unique<gui::multidropdown_t>(
					"prefer body conditions", std::vector<gui::multitems_t>{
					gui::multitems_t( "always", "rage_body_always" ), gui::multitems_t( "lethal", "rage_body_lethal" ), gui::multitems_t( "fake", "rage_body_fake" ), gui::multitems_t( "air", "rage_body_air" )
				} );
				accuracy->add_multidropdown( prefer_body.get( ) );

				auto ilimbs = std::make_unique<gui::checkbox_t>(
					"ignore moving target limbs", "acc_limbs", value_t( false )
					);
				accuracy->add_checkbox( ilimbs.get( ) );

				auto body_key = std::make_unique<gui::hotkey_t>(
					"force body-aim", "acc_forcebody"
					);
				accuracy->add_hotkey( body_key.get( ) );

			}

		}
		accuracy->finish( );

		if ( !r )
			r = true;
	}

	__forceinline void antiaim( ) {
		static bool r{ false };

		static auto general = std::make_unique<gui::child_t>( );
		general->begin( "general", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto pitch = std::make_unique<gui::dropdown_t>(
					"pitch", "aa_pitch", value_t( 0 ), std::vector<std::string>{
					"default", "down", "up"
				} );
				general->add_dropdown( pitch.get( ) );

				auto at = std::make_unique<gui::checkbox_t>(
					"at targets", "aa_at", value_t( false )
					);
				general->add_checkbox( at.get( ) );

				auto yaw = std::make_unique<gui::dropdown_t>(
					"yaw", "aa_yaw", value_t( 0 ), std::vector<std::string>{
					"backwards", "180z"
				} );
				general->add_dropdown( yaw.get( ) );

				auto offset = std::make_unique<gui::slider_t>(
					"yaw offset", "aa_offset", value_t( 0 ), -180.f, 180.f
					);
				general->add_slider( offset.get( ) );

				auto fs = std::make_unique<gui::checkbox_t>(
					"freestand", "aa_fs", value_t( false )
					);
				general->add_checkbox( fs.get( ) );

				auto fb = std::make_unique<gui::checkbox_t>(
					"fake body", "aa_fb", value_t( false )
					);
				general->add_checkbox( fb.get( ) );

				auto fb_deg = std::make_unique<gui::slider_t>(
					"fake body angle", "aa_fb_ang", value_t( 0 ), -180.f, 180.f
					);
				general->add_slider( fb_deg.get( ) );

				auto distort = std::make_unique<gui::multidropdown_t>(
					"distortion", std::vector<gui::multitems_t>{
					gui::multitems_t( "shift", "aa_dshift" ), gui::multitems_t( "force-turn", "aa_dturn" )
				} );
				general->add_multidropdown( distort.get( ) );

				auto d_deg = std::make_unique<gui::slider_t>(
					"distortion angle", "aa_dangle", value_t( 0 ), -180.f, 180.f
					);
				general->add_slider( d_deg.get( ) );

				auto fake = std::make_unique<gui::checkbox_t>(
					"fake yaw", "aa_fake", value_t( false )
					);
				general->add_checkbox( fake.get( ) );

				auto fake_offset = std::make_unique<gui::slider_t>(
					"fake yaw offset", "aa_fake_offset", value_t( 0 ), -180.f, 180.f
					);
				general->add_slider( fake_offset.get( ) );

			}

		}
		general->finish( );

		static auto fakelag = std::make_unique<gui::child_t>( );
		fakelag->begin( "fakelag", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( true ) );
		{

			if ( !r ) {

				auto enable = std::make_unique<gui::checkbox_t>(
					"enable", "fl_enable", value_t( false )
					);
				fakelag->add_checkbox( enable.get( ) );

				auto mode = std::make_unique<gui::dropdown_t>(
					"mode", "fl_mode", value_t( 0 ), std::vector<std::string>{ "maximum", "dynamic", "fluctuate" }
				);
				fakelag->add_dropdown( mode.get( ) );

				auto limit = std::make_unique<gui::slider_t>(
					"limit", "fl_limit", value_t( 2 ), 0, 14, "T"
					);
				fakelag->add_slider( limit.get( ) );

				auto variance = std::make_unique<gui::slider_t>(
					"variance", "fl_var", value_t( 0 ), 0, 100, "%"
					);
				fakelag->add_slider( variance.get( ) );

				auto lagcomp = std::make_unique<gui::checkbox_t>(
					"break lagcomp", "fl_lc", value_t( false )
					);
				fakelag->add_checkbox( lagcomp.get( ) );

			}

		}
		fakelag->finish( );

		static auto other = std::make_unique<gui::child_t>( );
		other->begin( "other", vec2_t( gui::tab_area( false ).x + 7.5f, gui::tab_area( true ).y + 7.5f ), gui::tab_area( true ) );
		{

			if ( !r ) {

				auto fakeflick = std::make_unique<gui::hotkey_t>(
					"fake flick", "aa_fflick"
					);
				other->add_hotkey( fakeflick.get( ) );

				auto lagwalk = std::make_unique<gui::hotkey_t>(
					"lag walk", "aa_lwalk"
					);
				other->add_hotkey( lagwalk.get( ) );

			}

		}
		other->finish( );

		if ( !r )
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
			if ( !r ) {
				overlay->add_checkbox( name.get( ) );
				overlay->add_colorpicker( name_clr.get( ) );
			}

			static auto box = std::make_unique<gui::checkbox_t>(
				"bounding box", "esp_box", value_t( false )
				);
			static auto box_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_box_clr", value_t( colors::white ), true
				);
			if ( !r ) {
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
			if ( !r ) {
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
			if ( !r ) {
				overlay->add_checkbox( ammo.get( ) );
				overlay->add_colorpicker( ammo_clr.get( ) );
			}

			static auto lby = std::make_unique<gui::checkbox_t>(
				"lby bar", "esp_lby", value_t( false )
				);
			static auto lby_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_lby_clr", value_t( colors::white ), true
				);
			if ( !r ) {
				overlay->add_checkbox( lby.get( ) );
				overlay->add_colorpicker( lby_clr.get( ) );
			}

			static auto wpn_txt = std::make_unique<gui::checkbox_t>(
				"weapon text", "esp_wpnt", value_t( false )
				);
			static auto wpn_txt_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_wpnt_clr", value_t( colors::white ), true
				);
			if ( !r ) {
				overlay->add_checkbox( wpn_txt.get( ) );
				overlay->add_colorpicker( wpn_txt_clr.get( ) );
			}

			static auto wpn_ico = std::make_unique<gui::checkbox_t>(
				"weapon icon", "esp_wpni", value_t( false )
				);
			static auto wpn_ico_clr = std::make_unique<gui::colorpicker_t>(
				"color", "esp_wpni_clr", value_t( colors::white ), true
				);
			if ( !r ) {
				overlay->add_checkbox( wpn_ico.get( ) );
				overlay->add_colorpicker( wpn_ico_clr.get( ) );
			}

			static auto flags = std::make_unique<gui::checkbox_t>(
				"flags", "esp_flags", value_t( false )
				);
			if ( !r )
				overlay->add_checkbox( flags.get( ) );

			if ( !r )
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
				"bunnyhop", "misc_bhop", value_t( false )
				);
			if ( !r )
				movement->add_checkbox( bhop.get( ) );

			static auto strafe = std::make_unique<gui::checkbox_t>(
				"autostrafer", "misc_strafe", value_t( false )
				);
			if ( !r )
				movement->add_checkbox( strafe.get( ) );

			static auto smooth = std::make_unique<gui::slider_t>(
				"smoothing", "misc_strafe_smooth", value_t( 50 ), 0, 100, "%"
				);
			if ( !r )
				movement->add_slider( smooth.get( ) );

			static auto autopeek = std::make_unique<gui::hotkey_t>(
				"quick peek assist", "misc_peek"
				);
			if ( !r )
				movement->add_hotkey( autopeek.get( ) );

			static auto fakewalk = std::make_unique<gui::hotkey_t>(
				"fakewalk", "misc_walk"
				);
			if ( !r )
				movement->add_hotkey( fakewalk.get( ) );

		}
		movement->finish( );

		static auto settings = std::make_unique<gui::child_t>( );
		settings->begin( "settings", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

			static auto accent = std::make_unique<gui::colorpicker_t>(
				"menu accent", "menu_accent", value_t( Color( 242, 97, 97 ) )
				);
			if ( !r )
				settings->add_colorpicker( accent.get( ) );

			static auto untrusted = std::make_unique<gui::checkbox_t>(
				"anti-untrusted", "menu_ut", value_t( true )
				);
			if ( !r )
				settings->add_checkbox( untrusted.get( ) );

			static auto cfg = std::make_unique<gui::dropdown_t>(
				"config list", "cfg_list", value_t( 0 ), std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }
			);
			if ( !r )
				settings->add_dropdown( cfg.get( ) );

			static std::function<void( )> save_fn = [&]( ) {
				std::string cur_cfg = std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }[config["cfg_list"].get<int>( )];
				cur_cfg.append( ".ini" );
				cfg_t::save( cur_cfg );
			};
			static auto save = std::make_unique<gui::button_t>(
				"save", save_fn
				);
			if ( !r )
				settings->add_button( save.get( ) );

			static std::function<void( )> load_fn = [&]( ) {
				std::string cur_cfg = std::vector<std::string>{ "alpha", "bravo", "charlie", "delta" }[config["cfg_list"].get<int>( )];
				cur_cfg.append( ".ini" );
				cfg_t::load( cur_cfg );
			};
			static auto load = std::make_unique<gui::button_t>(
				"load", load_fn
				);
			if ( !r )
				settings->add_button( load.get( ) );

		}
		settings->finish( );

		if ( !r )
			r = true;
	}

	__forceinline void render( ) {
		static auto window = std::make_unique<gui::window_t>( std::vector<std::string>{
			"rage", "antiaim", "player", "world", "misc"
		} );

		if ( window->begin( "wapicc" ) ) {

			switch ( window->cur_tab ) {
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