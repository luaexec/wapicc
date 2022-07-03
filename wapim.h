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
					gui::multitems_t( "fake flick", "acc_correct_fflick" ), gui::multitems_t( "lower-body update", "acc_correct_upd" ), gui::multitems_t( "onshot", "acc_correct_os" ), gui::multitems_t( "last-moving", "acc_correct_lm" )
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

				auto ind = std::make_unique<gui::checkbox_t>(
					"manual arrows", "aa_ind", value_t( false )
					);
				auto ind_clr = std::make_unique<gui::colorpicker_t>(
					"color", "aa_ind_clr", value_t( colors::white ), true
					);
				other->add_checkbox( ind.get( ) );
				other->add_colorpicker( ind_clr.get( ) );

				auto left = std::make_unique<gui::hotkey_t>(
					"manual left", "aa_left"
					);
				other->add_hotkey( left.get( ) );

				auto right = std::make_unique<gui::hotkey_t>(
					"manual right", "aa_right"
					);
				other->add_hotkey( right.get( ) );

				auto back = std::make_unique<gui::hotkey_t>(
					"manual back", "aa_back"
					);
				other->add_hotkey( back.get( ) );

			}

		}
		other->finish( );

		if ( !r )
			r = true;
	}

	__forceinline void player( ) {
		static bool r{ false };

		static auto overlay = std::make_unique<gui::child_t>( );
		overlay->begin( "overlay", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto name = std::make_unique<gui::checkbox_t>(
					"player name", "esp_name", value_t( false )
					);
				auto name_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_name_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( name.get( ) );
				overlay->add_colorpicker( name_clr.get( ) );

				auto box = std::make_unique<gui::checkbox_t>(
					"bounding box", "esp_box", value_t( false )
					);
				auto box_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_box_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( box.get( ) );
				overlay->add_colorpicker( box_clr.get( ) );

				auto hp = std::make_unique<gui::checkbox_t>(
					"health bar", "esp_hp", value_t( false )
					);
				auto hp_full = std::make_unique<gui::colorpicker_t>(
					"full", "esp_hp_full", value_t( colors::green ), true, vec2_t( -20, 0 )
					);
				auto hp_empty = std::make_unique<gui::colorpicker_t>(
					"empty", "esp_hp_empty", value_t( colors::red ), true
					);
				overlay->add_checkbox( hp.get( ) );
				overlay->add_colorpicker( hp_full.get( ) );
				overlay->add_colorpicker( hp_empty.get( ) );

				auto ammo = std::make_unique<gui::checkbox_t>(
					"ammo bar", "esp_ammo", value_t( false )
					);
				auto ammo_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_ammo_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( ammo.get( ) );
				overlay->add_colorpicker( ammo_clr.get( ) );

				auto lby = std::make_unique<gui::checkbox_t>(
					"lby bar", "esp_lby", value_t( false )
					);
				auto lby_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_lby_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( lby.get( ) );
				overlay->add_colorpicker( lby_clr.get( ) );

				auto wpn_txt = std::make_unique<gui::checkbox_t>(
					"weapon text", "esp_wpnt", value_t( false )
					);
				auto wpn_txt_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_wpnt_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( wpn_txt.get( ) );
				overlay->add_colorpicker( wpn_txt_clr.get( ) );

				auto wpn_ico = std::make_unique<gui::checkbox_t>(
					"weapon icon", "esp_wpni", value_t( false )
					);
				auto wpn_ico_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_wpni_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( wpn_ico.get( ) );
				overlay->add_colorpicker( wpn_ico_clr.get( ) );

				auto skeleton = std::make_unique<gui::checkbox_t>(
					"skeleton", "esp_skeleton", value_t( false )
					);
				auto skeleton_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_skeleton_clr", value_t( colors::purple ), true
					);
				overlay->add_checkbox( skeleton.get( ) );
				overlay->add_colorpicker( skeleton_clr.get( ) );

				auto oof = std::make_unique<gui::checkbox_t>(
					"offscreen arrows", "esp_oof", value_t( false )
					);
				auto oof_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_oof_clr", value_t( colors::white ), true
					);
				overlay->add_checkbox( oof.get( ) );
				overlay->add_colorpicker( oof_clr.get( ) );

				auto flags = std::make_unique<gui::checkbox_t>(
					"flags", "esp_flags", value_t( false )
					);
				overlay->add_checkbox( flags.get( ) );

				auto glow = std::make_unique<gui::checkbox_t>(
					"glow esp", "esp_glow", value_t( false )
					);
				auto glow_clr = std::make_unique<gui::colorpicker_t>(
					"color", "esp_glow_clr", value_t( colors::purple ), true
					);
				overlay->add_checkbox( glow.get( ) );
				overlay->add_colorpicker( glow_clr.get( ) );

			}

		}
		overlay->finish( );

		static auto models = std::make_unique<gui::child_t>( );
		models->begin( "models", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

			if ( !r ) {

				/* enemy */ {

					auto enemy = std::make_unique<gui::checkbox_t>(
						"enemy chams", "chams_enemy", value_t( false )
						);
					auto enemy_vis = std::make_unique<gui::colorpicker_t>(
						"visible", "chams_enemy_vis", value_t( colors::black ), true, vec2_t( -20, 0 )
						);
					auto enemy_occ = std::make_unique<gui::colorpicker_t>(
						"occluded", "chams_enemy_occ", value_t( Color( 255, 100, 100 ) ), true
						);
					auto enemy_mat = std::make_unique<gui::dropdown_t>(
						"enemy material", "chams_enemy_mat", value_t( 0 ), std::vector<std::string>{
						"textured", "flat", "metallic", "glow"
					} );
					auto enemy_acc = std::make_unique<gui::colorpicker_t>(
						"accent", "chams_enemy_acc", value_t( colors::white ), true, vec2_t( 0, -25 )
						);
					models->add_checkbox( enemy.get( ) );
					models->add_colorpicker( enemy_vis.get( ) );
					models->add_colorpicker( enemy_occ.get( ) );
					models->add_dropdown( enemy_mat.get( ) );
					models->add_colorpicker( enemy_acc.get( ) );

				}

				/* backtrack */ {

					auto bt = std::make_unique<gui::checkbox_t>(
						"backtrack chams", "chams_bt", value_t( false )
						);
					auto bt_clr = std::make_unique<gui::colorpicker_t>(
						"color", "chams_bt_clr", value_t( colors::white ), true
						);
					auto bt_mat = std::make_unique<gui::dropdown_t>(
						"backtrack material", "chams_bt_mat", value_t( 0 ), std::vector<std::string>{
						"textured", "flat", "metallic", "glow"
					} );
					auto bt_acc = std::make_unique<gui::colorpicker_t>(
						"accent", "chams_bt_acc", value_t( colors::white ), true, vec2_t( 0, -25 )
						);
					models->add_checkbox( bt.get( ) );
					models->add_colorpicker( bt_clr.get( ) );
					models->add_dropdown( bt_mat.get( ) );
					models->add_colorpicker( bt_acc.get( ) );

				}

				/* shot */ {

					auto shot = std::make_unique<gui::checkbox_t>(
						"shot chams", "chams_shot", value_t( false )
						);
					auto shot_clr = std::make_unique<gui::colorpicker_t>(
						"color", "chams_shot_clr", value_t( colors::white ), true
						);
					auto shot_mat = std::make_unique<gui::dropdown_t>(
						"shot material", "chams_shot_mat", value_t( 0 ), std::vector<std::string>{
						"textured", "flat", "metallic", "glow"
					} );
					auto shot_acc = std::make_unique<gui::colorpicker_t>(
						"accent", "chams_shot_acc", value_t( colors::white ), true, vec2_t( 0, -25 )
						);
					models->add_checkbox( shot.get( ) );
					models->add_colorpicker( shot_clr.get( ) );
					models->add_dropdown( shot_mat.get( ) );
					models->add_colorpicker( shot_acc.get( ) );

				}

				/* local */ {

					auto local = std::make_unique<gui::checkbox_t>(
						"local chams", "chams_local", value_t( false )
						);
					auto local_clr = std::make_unique<gui::colorpicker_t>(
						"color", "chams_local_clr", value_t( colors::white ), true
						);
					auto local_mat = std::make_unique<gui::dropdown_t>(
						"local material", "chams_local_mat", value_t( 0 ), std::vector<std::string>{
						"textured", "flat", "metallic", "glow"
					} );
					auto local_acc = std::make_unique<gui::colorpicker_t>(
						"accent", "chams_local_acc", value_t( colors::white ), true, vec2_t( 0, -25 )
						);
					models->add_checkbox( local.get( ) );
					models->add_colorpicker( local_clr.get( ) );
					models->add_dropdown( local_mat.get( ) );
					models->add_colorpicker( local_acc.get( ) );

				}

			}

		}
		models->finish( );

		if ( !r )
			r = true;

	}

	__forceinline void world( ) {
		static bool r{ false };

		static auto draw = std::make_unique<gui::child_t>( );
		draw->begin( "draw", vec2_t( 0.f, gui::tab_area( true ).y + 7.5f ), gui::tab_area( true ) );
		{

			if ( !r ) {

				auto imp = std::make_unique<gui::checkbox_t>(
					"bullet impacts", "vis_imp", value_t( false )
					);
				auto imp_cl = std::make_unique<gui::colorpicker_t>(
					"client", "vis_imp_client", value_t( Color( 255, 0, 0, 155 ) ), true, vec2_t( -20, 0 )
					);
				auto imp_sv = std::make_unique<gui::colorpicker_t>(
					"server", "vis_imp_server", value_t( Color( 0, 0, 255, 155 ) ), true
					);
				draw->add_checkbox( imp.get( ) );
				draw->add_colorpicker( imp_cl.get( ) );
				draw->add_colorpicker( imp_sv.get( ) );

				auto hm = std::make_unique<gui::checkbox_t>(
					"hitmarker", "vis_hm", value_t( false )
					);
				auto hm_crit = std::make_unique<gui::colorpicker_t>(
					"critical", "vis_hm_crit", value_t( Color( 255, 0, 0 ) ), true, vec2_t( -20, 0 )
					);
				auto hm_def = std::make_unique<gui::colorpicker_t>(
					"default", "vis_hm_def", value_t( Color( 255, 255, 255 ) ), true
					);
				draw->add_checkbox( hm.get( ) );
				draw->add_colorpicker( hm_crit.get( ) );
				draw->add_colorpicker( hm_def.get( ) );

				auto hud = std::make_unique<gui::multidropdown_t>(
					"panel", std::vector<gui::multitems_t>{
					gui::multitems_t( "spectators", "hud_spec" ), gui::multitems_t( "indictators", "hud_ind" )
				} );
				draw->add_multidropdown( hud.get( ) );

				auto gr = std::make_unique<gui::checkbox_t>(
					"grenade trajectory", "vis_gr", value_t( false )
					);
				auto gr_hit = std::make_unique<gui::colorpicker_t>(
					"hit", "vis_gr_hit", value_t( Color( 200, 255, 200 ) ), true, vec2_t( -20, 0 )
					);
				auto gr_def = std::make_unique<gui::colorpicker_t>(
					"default", "vis_gr_def", value_t( Color( 255, 200, 200 ) ), true
					);
				draw->add_checkbox( gr.get( ) );
				draw->add_colorpicker( gr_hit.get( ) );
				draw->add_colorpicker( gr_def.get( ) );

			}

		}
		draw->finish( );

		static auto world = std::make_unique<gui::child_t>( );
		world->begin( "world", vec2_t( 0, 0 ), gui::tab_area( true ) );
		{

			if ( !r ) {

				auto wrld = std::make_unique<gui::checkbox_t>(
					"world", "vis_world", value_t( false )
					);
				auto wrld_clr = std::make_unique<gui::colorpicker_t>(
					"color", "vis_world_clr", value_t( colors::white ), true
					);
				world->add_checkbox( wrld.get( ) );
				world->add_colorpicker( wrld_clr.get( ) );

				auto ambient = std::make_unique<gui::checkbox_t>(
					"ambient", "vis_amb", value_t( false )
					);
				auto ambient_clr = std::make_unique<gui::colorpicker_t>(
					"color", "vis_amb_clr", value_t( colors::black ), true
					);
				world->add_checkbox( ambient.get( ) );
				world->add_colorpicker( ambient_clr.get( ) );

				auto prop = std::make_unique<gui::checkbox_t>(
					"props", "vis_prop", value_t( false )
					);
				auto prop_clr = std::make_unique<gui::colorpicker_t>(
					"color", "vis_prop_clr", value_t( colors::white.alpha( 155 ) ), true
					);
				world->add_checkbox( prop.get( ) );
				world->add_colorpicker( prop_clr.get( ) );

				auto sky = std::make_unique<gui::checkbox_t>(
					"skybox", "vis_sky", value_t( false )
					);
				auto sky_clr = std::make_unique<gui::colorpicker_t>(
					"color", "vis_sky_clr", value_t( colors::white ), true
					);
				world->add_checkbox( sky.get( ) );
				world->add_colorpicker( sky_clr.get( ) );

				auto extra = std::make_unique<gui::multidropdown_t>(
					"more..", std::vector<gui::multitems_t>{
					gui::multitems_t( "fullbright", "vis_fb" ), gui::multitems_t( "sunset", "vis_ss" )
				} );
				world->add_multidropdown( extra.get( ) );

			}

		}
		world->finish( );

		static auto other = std::make_unique<gui::child_t>( );
		other->begin( "other", vec2_t( gui::tab_area( false ).x + 7.5f, 0.f ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto removals = std::make_unique<gui::multidropdown_t>(
					"removals", std::vector<gui::multitems_t>{
					gui::multitems_t( "smoke", "remove_smoke" ), gui::multitems_t( "flash", "remove_flash" ),
						gui::multitems_t( "scope", "remove_scope" ), gui::multitems_t( "fog", "remove_fog" ),
						gui::multitems_t( "recoil", "remove_recoil" ), gui::multitems_t( "teammates", "remove_team" ),
				} );
				other->add_multidropdown( removals.get( ) );

				auto thirdperson = std::make_unique<gui::hotkey_t>(
					"third-person", "vis_tp"
					);
				other->add_hotkey( thirdperson.get( ) );

				auto tp_dist = std::make_unique<gui::slider_t>(
					"third-person distance", "vis_tp_dist", value_t( 75 ), 0, 150
					);
				other->add_slider( tp_dist.get( ) );

				auto crosshair = std::make_unique<gui::checkbox_t>(
					"force crosshair", "vis_xhair", value_t( false )
					);
				other->add_checkbox( crosshair.get( ) );

				auto killfeed = std::make_unique<gui::checkbox_t>(
					"preserve killfeed", "vis_kf", value_t( false )
					);
				other->add_checkbox( killfeed.get( ) );

				auto beam = std::make_unique<gui::checkbox_t>(
					"impact beam", "vis_beam", value_t( false )
					);
				auto beam_clr = std::make_unique<gui::colorpicker_t>(
					"color", "vis_beam_clr", value_t( colors::red ), true
					);
				other->add_checkbox( beam.get( ) );
				other->add_colorpicker( beam_clr.get( ) );

				auto pen = std::make_unique<gui::checkbox_t>(
					"penetration crosshair", "vis_pen", value_t( false )
					);
				other->add_checkbox( pen.get( ) );

			}

		}
		other->finish( );

		if ( !r )
			r = true;
	}

	__forceinline void misc( ) {
		static bool r{ false };

		static auto movement = std::make_unique<gui::child_t>( );
		movement->begin( "miscellaneous", vec2_t( 0, 0 ), gui::tab_area( false ) );
		{

			if ( !r ) {

				auto fov = std::make_unique<gui::slider_t>(
					"field of view", "misc_fov", value_t( 90 ), 0, 150
					);
				movement->add_slider( fov.get( ) );

				auto vfov = std::make_unique<gui::slider_t>(
					"viewmodel field of view", "misc_vfov", value_t( 90 ), 0, 150
					);
				movement->add_slider( vfov.get( ) );

				auto zoom = std::make_unique<gui::slider_t>(
					"zoom increments", "misc_inc", value_t( 10 ), 0, 45
					);
				movement->add_slider( zoom.get( ) );

				auto bhop = std::make_unique<gui::checkbox_t>(
					"bunnyhop", "misc_bhop", value_t( false )
					);
				movement->add_checkbox( bhop.get( ) );

				auto strafe = std::make_unique<gui::checkbox_t>(
					"autostrafer", "misc_strafe", value_t( false )
					);
				movement->add_checkbox( strafe.get( ) );

				auto smooth = std::make_unique<gui::slider_t>(
					"smoothing", "misc_strafe_smooth", value_t( 50 ), 0, 100, "%"
					);
				movement->add_slider( smooth.get( ) );

				auto autopeek = std::make_unique<gui::hotkey_t>(
					"quick peek assist", "misc_peek"
					);
				movement->add_hotkey( autopeek.get( ) );

				auto fakewalk = std::make_unique<gui::hotkey_t>(
					"fakewalk", "misc_walk"
					);
				movement->add_hotkey( fakewalk.get( ) );

				auto airstuck = std::make_unique<gui::hotkey_t>(
					"airstuck", "misc_as"
					);
				movement->add_hotkey( airstuck.get( ) );

			}

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
		static bool r{ false };
		static auto window = std::make_unique<gui::window_t>( std::vector<std::string>{
			"rage", "antiaim", "player", "world", "misc"
		} );

		if ( !r ) {
			rage( ); antiaim( ); player( ); world( ); misc( );
			r = true;
		}

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