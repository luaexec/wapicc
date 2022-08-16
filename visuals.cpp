#include "includes.h"
#include "wapim.h"
#include "gh.h"
#include "tickbase.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld( ) {
	std::vector< IMaterial* > world, props, sky;

	static auto p_p_clr{ colors::white }, p_w_clr{ colors::white }, p_s_clr{ colors::white }, p_a_clr{ colors::black };
	auto _w = config["vis_world"].get<bool>( ) ? config["vis_world_clr"].get_color( 255 ) : colors::white;
	auto _p = config["vis_prop"].get<bool>( ) ? config["vis_prop_clr"].get_color( ) : colors::white;
	auto _s = config["vis_sky"].get<bool>( ) ? config["vis_sky_clr"].get_color( 255 ) : colors::white;
	auto _a = config["vis_amb"].get<bool>( ) ? config["vis_amb_clr"].get_color( 255 ) : colors::black;

	if ( p_w_clr != _w || p_p_clr != _p || p_s_clr != _s ) {
		for ( uint16_t h{ g_csgo.m_material_system->FirstMaterial( ) }; h != g_csgo.m_material_system->InvalidMaterial( ); h = g_csgo.m_material_system->NextMaterial( h ) ) {
			IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
			if ( !mat )
				continue;

			if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "World textures" ) ) {
				mat->ColorModulate( _w.r( ) / 255.f, _w.g( ) / 255.f, _w.b( ) / 255.f );
			}

			else if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "StaticProp textures" ) ) {
				mat->ColorModulate( _p.r( ) / 255.f, _p.g( ) / 255.f, _p.b( ) / 255.f );
				mat->AlphaModulate( _p.a( ) / 255.f );
			}

			else if ( strstr( mat->GetTextureGroupName( ), "SkyBox" ) ) {
				mat->ColorModulate( _s.r( ) / 255.f, _s.g( ) / 255.f, _s.b( ) / 255.f );
			}
		}

		p_p_clr = _p; p_w_clr = _w; p_s_clr = _s;
	}

	if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
		g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
	}

	if ( _a != p_a_clr ) {
		g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_r" ) )->SetValue( _a.r( ) / 255.f );
		g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_g" ) )->SetValue( _a.g( ) / 255.f );
		g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_b" ) )->SetValue( _a.b( ) / 255.f );
		p_a_clr = _a;
	}

	if ( config["vis_ss"].get<bool>( ) ) {
		g_csgo.m_cvar->FindVar( HASH( "cl_csm_rot_override" ) )->SetValue( "1" );
		g_csgo.m_cvar->FindVar( HASH( "cl_csm_rot_x" ) )->SetValue( "0" );
	}
	else {
		g_csgo.m_cvar->FindVar( HASH( "cl_csm_rot_override" ) )->SetValue( "0" );
		g_csgo.m_cvar->FindVar( HASH( "cl_csm_rot_x" ) )->SetValue( "50" );
	}
}

void Visuals::ThirdpersonThink( ) {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive( );

	// camera should be in thirdperson.
	if ( cfg_t::get_hotkey( "vis_tp", "vis_tp_mode" ) ) {

		// if alive and not in thirdperson already switch to thirdperson.
		if ( alive && !g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_input->CAM_ToThirdPerson( );

		// if dead and spectating in firstperson switch to thirdperson.
		else if ( g_cl.m_local->m_iObserverMode( ) == 4 ) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
				g_csgo.m_input->CAM_ToFirstPerson( );
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode( ) = 5;
		}
	}

	// camera should be in firstperson.
	else if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles( offset );

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		// cam_idealdist convar.
		offset.z = config["vis_tp_dist"].get<float>( );

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity( g_cl.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::ImpactData( )
{
	if ( !g_cl.m_processing ) return;

	if ( !config["vis_imp"].get<bool>( ) ) return;

	//call this in fsn or whatever
	static auto last_count = 0;
	auto& client_impact_list = *( CUtlVector< client_hit_verify_t >* )( (uintptr_t)g_cl.m_local + 0xBA84 );

	Color color = config["vis_imp_client"].get_color( );

	for ( auto i = client_impact_list.Count( ); i > last_count; i-- ) {
		g_csgo.m_debug_overlay->AddBoxOverlay( client_impact_list[i - 1].pos, vec3_t( -2, -2, -2 ), vec3_t( 2, 2, 2 ), ang_t( 0, 0, 0 ), color.r( ), color.g( ), color.b( ), color.a( ), 4.f );
	}

	if ( client_impact_list.Count( ) != last_count )
		last_count = client_impact_list.Count( );
}

void Visuals::Hitmarker( ) {
	static auto cross = g_csgo.m_cvar->FindVar( HASH( "weapon_debug_spread_show" ) );
	cross->SetValue( config["vis_xhair"].get<bool>( ) && !g_cl.m_local->m_bIsScoped( ) ? 3 : 0 );

	if ( !config["vis_hm"].get<bool>( ) )
		return;

	constexpr int line{ 6 };

	auto hm = [&]( int x, int y, int damage, bool hs, float s, float e ) {
		auto t = std::min( s, e );
		auto end_set = ( 2 + ( 5 * t ) );

		render::line( x + 2, y + 2, x + end_set, y + end_set, hs ? config["vis_hm_crit"].get_color( int( 255.f * t ) ) : config["vis_hm_def"].get_color( int( 255.f * t ) ) );
		render::line( x - 2, y + 2, x - end_set, y + end_set, hs ? config["vis_hm_crit"].get_color( int( 255.f * t ) ) : config["vis_hm_def"].get_color( int( 255.f * t ) ) );
		render::line( x + 2, y - 2, x + end_set, y - end_set, hs ? config["vis_hm_crit"].get_color( int( 255.f * t ) ) : config["vis_hm_def"].get_color( int( 255.f * t ) ) );
		render::line( x - 2, y - 2, x - end_set, y - end_set, hs ? config["vis_hm_crit"].get_color( int( 255.f * t ) ) : config["vis_hm_def"].get_color( int( 255.f * t ) ) );

		//render::esp.string( x + 1, int( y - 10 - ( 8 * t ) ), hs ? config["vis_hm_crit"].get_color( int( 255.f * t ) ) : config["vis_hm_def"].get_color( int( 255.f * t ) ), std::to_string( damage ), render::ALIGN_CENTER );
	};

	for ( auto i : g_shots.m_hits ) {
		float rem = ( i.m_time + 5.f ) - g_csgo.m_globals->m_curtime;
		if ( rem < 0.f || rem > 5.f )
			continue;

		vec2_t pos;
		if ( !render::WorldToScreen( i.m_pos, pos ) )
			continue;

		auto s = std::min( 5.f - rem, 0.5f ) * 2.f;
		auto e = std::min( rem, 0.5f ) * 2.f;

		hm( pos.x, pos.y, i.m_damage, i.m_group == 1, s, e );
	}
}

void Visuals::NoSmoke( ) {
	if ( !smoke1 )
		smoke1 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_fire" ), XOR( "Other textures" ) );

	if ( !smoke2 )
		smoke2 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), XOR( "Other textures" ) );

	if ( !smoke3 )
		smoke3 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods" ), XOR( "Other textures" ) );

	if ( !smoke4 )
		smoke4 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), XOR( "Other textures" ) );

	if ( config["remove_smoke"].get<bool>( ) ) {
		if ( !smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, true );
	}

	else {
		if ( smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, false );
	}
}

void Visuals::hotkeys( )
{
	if ( !g_cl.m_processing )
		return;

	if ( !config["hud_ind"].get<bool>( ) )
		return;

	struct Indicator_t {
		Color color;
		std::string text;
		int mode{ -1 };
		std::string get_mode( ) {
			return std::vector<std::string>{ "hold", "toggle" } [mode] ;
		}
	};
	std::vector< Indicator_t > indicators{ };

	if ( ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) ) {
		Indicator_t ind{ };
		ind.color = g_cl.m_lagcomp ? Color( 100, 255, 100 ) : Color( 255, 100, 100 );
		ind.text = XOR( "lc" );

		indicators.push_back( ind );
	}

	if ( 1 ) {
		float change = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_real_angle.y ) );

		Indicator_t ind{ };
		ind.color = Color( 255, 100, 100 ).blend( Color( 100, 255, 100 ), ( g_cl.m_body_pred - g_csgo.m_globals->m_curtime ) / 1.1f );
		ind.text = XOR( "lby" );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "acc_dt", "acc_dt_mode" ) ) {
		Indicator_t ind{ };
		ind.color = colors::white.blend( gui::m_accent, (float)g_tickbase.m_ticks / (float)g_tickbase.m_goal_ticks );
		ind.text = XOR( "dt" );
		ind.mode = config["acc_dt_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( g_aimbot.m_damage_ovr ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "dmg" );
		ind.mode = config["rage_ovrkey_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "acc_forcebody", "acc_forcebody_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "baim" );
		ind.mode = config["acc_forcebody_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "fakeflick" );
		ind.mode = config["aa_fflick_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "aa_lwalk", "aa_lwalk_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "lagwalk" );
		ind.mode = config["aa_lwalk_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "misc_as", "misc_as_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "airstuck" );
		ind.mode = config["misc_as_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "misc_peek", "misc_peek_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "quickpeek" );
		ind.mode = config["misc_peek_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( cfg_t::get_hotkey( "misc_walk", "misc_walk_mode" ) ) {
		Indicator_t ind{ };
		ind.color = gui::m_accent;
		ind.text = XOR( "fakewalk" );
		ind.mode = config["misc_walk_mode"].get<int>( );
		indicators.push_back( ind );
	}

	if ( indicators.empty( ) )
		return;

	size_t i{ 0 }, h{ ( g_cl.m_height / 2 ) - ( ( indicators.size( ) * 18 ) / 2 ) };
	for ( auto& ind : indicators ) {
		if ( ind.mode == -1 ) {
			int size{ render::esp.size( ind.text ).m_width + 20 };

			render::round_rect( 10, h + ( i * 18 ), size, 15, 2, gui::palette::dark.alpha( 255 ) );
			render::gradient1337( 11, h + 1 + ( i * 18 ), size - 2, 13, ind.color.alpha( 55 ), colors::black.alpha( 0 ) );
			render::esp.string( 10 + ( size / 2 ), h + 1 + ( i * 18 ), ind.color, ind.text, render::ALIGN_CENTER );
		}
		else {
			int size{ render::esp.size( ind.text ).m_width + render::esp.size( ind.get_mode( ) ).m_width + 20 };

			render::round_rect( 10, h + ( i * 18 ), size, 15, 2, gui::palette::dark.alpha( 255 ) );
			render::gradient1337( 11, h + 1 + ( i * 18 ), ( render::esp.size( ind.text ).m_width + 10 ) - 2, 13, ind.color.alpha( 55 ), colors::black.alpha( 0 ) );
			render::esp.string( 15, h + 1 + ( i * 18 ), ind.color, ind.text );
			render::esp.string( 15 + render::esp.size( ind.text ).m_width + 5, h + 1 + ( i * 18 ), colors::white, ind.get_mode( ) );
		}

		i++;
	}
}

void Visuals::think( ) {
	// don't run anything if our local player isn't valid.
	if ( !g_cl.m_local )
		return;

	static float anim{ 0.f };
	if ( g_cl.m_local->m_bIsScoped( ) ) {
		anim += 4.5f * g_csgo.m_globals->m_frametime;
	}
	else { anim -= 4.5f * g_csgo.m_globals->m_frametime; }
	anim = std::clamp( anim, 0.f, 1.f );

	if ( config["remove_scope"].get<bool>( )
		 && g_cl.m_local->alive( )
		 && g_cl.m_local->GetActiveWeapon( )
		 && g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		 ) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( );

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if ( size > 1 ) {
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		render::rect_filled( 0, y, w * anim, size, colors::black );
		render::rect_filled( x, 0, size, h * anim, colors::black );
	}

	auto& predicted_nades = g_grenades_pred.get_list( );

	static auto last_server_tick = g_csgo.m_cl->m_server_tick;
	if ( g_csgo.m_cl->m_server_tick != last_server_tick ) {
		predicted_nades.clear( );

		last_server_tick = g_csgo.m_cl->m_server_tick;
	}

	// draw esp on ents.
	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		if ( ent->dormant( ) )
			continue;

		if ( !ent->is( HASH( "CMolotovProjectile" ) )
			 && !ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) )
			continue;

		if ( ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) ) {
			const auto studio_model = ent->GetModel( );
			if ( !studio_model
				 || std::string_view( studio_model->m_name ).find( "fraggrenade" ) == std::string::npos )
				continue;
		}

		const auto handle = reinterpret_cast<Player*>( ent )->GetRefEHandle( );

		if ( ent->m_fEffects( ) & EF_NODRAW ) {
			predicted_nades.erase( handle );

			continue;
		}

		if ( predicted_nades.find( handle ) == predicted_nades.end( ) ) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple( handle ),
				std::forward_as_tuple(
				reinterpret_cast<Player*>( g_csgo.m_entlist->GetClientEntityFromHandle( ent->m_hThrower( ) ) ),
				ent->is( HASH( "CMolotovProjectile" ) ) ? MOLOTOV : HEGRENADE,
				ent->m_vecOrigin( ), ent->m_vecVelocity( ), ent->m_flSpawnTime_Grenade( ),
				game::TIME_TO_TICKS( reinterpret_cast<Player*>( ent )->m_flSimulationTime( ) - ent->m_flSpawnTime_Grenade( ) )
			)
			);
		}

		if ( predicted_nades.at( handle ).draw( ) )
			continue;

		predicted_nades.erase( handle );
	}

	g_grenades_pred.get_local_data( ).draw( );

	// draw esp on ents.
	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		draw( ent );
	}

	ModulateWorld( );
	hotkeys( );
	SpreadCrosshair( );
	Spectators( );
	PenetrationCrosshair( );
	Hitmarker( );
	DrawPlantedC4( );
}

void Visuals::Spectators( ) {
	if ( !config["hud_spec"].get<bool>( ) )
		return;

	std::vector< std::string > spectators{ XOR( "spectators" ) };
	int h = render::menu_shade.m_size.m_height;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !player )
			continue;

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		if ( player->dormant( ) )
			continue;

		if ( player->m_lifeState( ) == LIFE_ALIVE )
			continue;

		if ( player->GetObserverTarget( ) != g_cl.m_local )
			continue;

		player_info_t info;
		if ( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
	}

	size_t total_size = spectators.size( ) * ( h - 1 );

	for ( size_t i{ }; i < spectators.size( ); ++i ) {
		const std::string& name = spectators[i];

		render::menu_shade.string( g_cl.m_width - 20, ( g_cl.m_height / 2 ) - ( total_size / 2 ) + ( i * ( h - 1 ) ),
								   { 255, 255, 255, 179 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::SpreadCrosshair( ) {
	// dont do if dead.
	if ( !g_cl.m_processing )
		return;

	if ( !g_menu.main.visuals.spread_xhair.get( ) )
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon( );
	if ( !weapon )
		return;

	WeaponInfo* data = weapon->GetWpnData( );
	if ( !data )
		return;

	// do not do this on: bomb, knife and nades.
	CSWeaponType type = data->m_weapon_type;
	if ( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	// calc radius.
	float radius = ( ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * 320.f ) / ( std::tan( math::deg_to_rad( g_cl.m_local->GetFOV( ) ) * 0.5f ) + FLT_EPSILON );

	// scale by screen size.
	radius *= g_cl.m_height * ( 1.f / 480.f );

	// get color.
	Color col = g_menu.main.visuals.spread_xhair_col.get( );

	// modify alpha channel.
	col.a( ) = 200 * ( g_menu.main.visuals.spread_xhair_blend.get( ) / 100.f );

	int segements = std::max( 16, (int)std::round( radius * 0.75f ) );
	render::circle( g_cl.m_width / 2, g_cl.m_height / 2, radius, segements, col );
}

void Visuals::PenetrationCrosshair( ) {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if ( !config["vis_pen"].get<bool>( ) || !g_cl.m_processing )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;


	/*valid_player_hit = (g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy(g_cl.m_local));
	if (valid_player_hit)
		final_color = colors::transparent_yellow;*/

	if ( g_cl.m_pen_data.m_pen )
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//             draw damage string?

	// draw small square in center of screen.
	//if (g_cl.m_pen_data.m_pen) {
	render::rect_filled( x - 1, y, 1, 1, { final_color } );
	render::rect_filled( x, y, 1, 1, { final_color } );
	render::rect_filled( x + 1, y, 1, 1, { final_color } );
	render::rect_filled( x, y + 1, 1, 1, { final_color } );
	render::rect_filled( x, y - 1, 1, 1, { final_color } );
	//}
}

void Visuals::draw( Entity* ent ) {
	if ( ent->IsPlayer( ) ) {
		Player* player = ent->as< Player* >( );

		if ( player->m_bIsLocalPlayer( ) )
			return;

		DrawPlayer( player );
	}
}

void Visuals::OffScreen( Player* player, int alpha ) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[3], verts_outline[3];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = []( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation ) {
		ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
		vec3_t fwd, right, up( 0.f, 0.f, 1.f );
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors( view_angles, &fwd );

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize( );

		// calculate front / side positions.
		right = up.cross( fwd );
		front = delta.dot( fwd );
		side = delta.dot( right );

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad( -out_rotation );
		sa = std::sin( yaw_rad );
		ca = std::cos( yaw_rad );

		// rotate offscreen position around.
		out_offscreen_pos.x = (int)( ( g_cl.m_width / 2.f ) + ( radius * sa ) );
		out_offscreen_pos.y = (int)( ( g_cl.m_height / 2.f ) - ( radius * ca ) );
	};

	if ( !g_cl.m_processing || !g_cl.m_local->enemy( player ) )
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter( );
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if ( !is_on_screen
		 || screen_pos.x < -leeway_x
		 || screen_pos.x >( g_cl.m_width + leeway_x )
		 || screen_pos.y < -leeway_y
		 || screen_pos.y >( g_cl.m_height + leeway_y ) ) {

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized( );

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = 200.f * ( g_cl.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x, offscreen_pos.y };        // 0,  0
		verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f }; // -1, 1
		verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
		verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
		verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[0] = render::RotateVertex( offscreen_pos, verts[0], offscreen_rotation );
		verts[1] = render::RotateVertex( offscreen_pos, verts[1], offscreen_rotation );
		verts[2] = render::RotateVertex( offscreen_pos, verts[2], offscreen_rotation );
		// verts_outline[ 0 ] = render::RotateVertex( offscreen_pos, verts_outline[ 0 ], offscreen_rotation );
		// verts_outline[ 1 ] = render::RotateVertex( offscreen_pos, verts_outline[ 1 ], offscreen_rotation );
		// verts_outline[ 2 ] = render::RotateVertex( offscreen_pos, verts_outline[ 2 ], offscreen_rotation );

		// todo - dex; finish this, i want it.
		// auto &damage_data = m_offscreen_damage[ player->index( ) ];
		// 
		// // the local player was damaged by another player recently.
		// if( damage_data.m_time > 0.f ) {
		//     // // only a small amount of time left, start fading into white again.
		//     // if( damage_data.m_time < 1.f ) {
		//     //     // calculate step needed to reach 255 in 1 second.
		//     //     // float step = UINT8_MAX / ( 1000.f * g_csgo.m_globals->m_frametime );
		//     //     float step = ( 1.f / g_csgo.m_globals->m_frametime ) / UINT8_MAX;
		//     //     
		//     //     // increment the new value for the color.
		//     //     // if( damage_data.m_color_step < 255.f )
		//     //         damage_data.m_color_step += step;
		//     // 
		//     //     math::clamp( damage_data.m_color_step, 0.f, 255.f );
		//     // 
		//     //     damage_data.m_color.g( ) = (uint8_t)damage_data.m_color_step;
		//     //     damage_data.m_color.b( ) = (uint8_t)damage_data.m_color_step;
		//     // }
		//     // 
		//     // g_cl.print( "%f %f %u %u %u\n", damage_data.m_time, damage_data.m_color_step, damage_data.m_color.r( ), damage_data.m_color.g( ), damage_data.m_color.b( ) );
		//     
		//     // decrement timer.
		//     damage_data.m_time -= g_csgo.m_globals->m_frametime;
		// }
		// 
		// else
		//     damage_data.m_color = colors::white;

		// render!
		color = config["esp_oof_clr"].get_color( ); // damage_data.m_color;
		color.a( ) *= alpha / 255.f;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );

		// g_csgo.m_surface->DrawSetColor( colors::black );
		// g_csgo.m_surface->DrawTexturedPolyLine( 3, verts_outline );
	}
}

void Visuals::DrawPlayer( Player* player ) {
	if ( !player->enemy( g_cl.m_local ) )
		return;

	int i = player->index( );
	if ( ( !player->alive( ) && m_alpha[i] <= 0.f ) || player->m_iTeamNum( ) == g_cl.m_local->m_iTeamNum( ) ) {
		if ( player->alive( ) ) {
			m_alpha[i] = 155.f;
		}
		return;
	}

	Rect bbox;
	if ( !GetPlayerBoxRect( player, bbox ) && player->alive( ) ) {
		if ( config["esp_off"].get<bool>( ) )
			OffScreen( player, m_alpha[i] );
		return;
	}

	m_bbox[i] = bbox;

	if ( !player->alive( ) ) {
		m_alpha[i] -= 480.f * g_csgo.m_globals->m_frametime;
	}
	else if ( player->dormant( ) ) {
		m_alpha[i] -= 20.f * g_csgo.m_globals->m_frametime;
	}
	else {
		m_alpha[i] += 240.f * g_csgo.m_globals->m_frametime;
	}
	m_alpha[i] = std::clamp( m_alpha[i], 0.f, 255.f );

	player_info_t info;
	if ( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
		return;

	/*  name  */
	if ( config["esp_name"].get<bool>( ) ) {
		std::string name{ std::string( info.m_name ).substr( 0, 24 ) };

		Color clr = config["esp_name_clr"].get_color( );
		clr.a( ) *= m_alpha[i] / 255.f;

		render::esp.string( bbox.x + bbox.w / 2, bbox.y - render::esp.m_size.m_height, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ), name, render::ALIGN_CENTER );
	}

	/*  box  */
	if ( config["esp_box"].get<bool>( ) ) {
		Color clr = config["esp_box_clr"].get_color( );
		clr.a( ) *= m_alpha[i] / 255.f;

		render::rect( bbox.x + 1, bbox.y + 1, bbox.w, bbox.h, Color( 10, 10, 10, int( m_alpha[i] / 2.f ) ) );
		render::rect( bbox.x, bbox.y, bbox.w, bbox.h, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );
	}

	/*  health  */
	if ( config["esp_hp"].get<bool>( ) ) {
		int y = bbox.y + 1;
		int h = bbox.h - 2;

		int hp = std::min( 100, player->m_iHealth( ) );

		if ( m_hp[player->index( )] > hp )
			m_hp[player->index( )] -= 200.f * g_csgo.m_globals->m_frametime;
		else
			m_hp[player->index( )] = hp;

		hp = m_hp[player->index( )];

		auto hp_clr = config["esp_hp_empty"].get_color( ).blend( config["esp_hp_full"].get_color( ), float( hp ) / 100.f );
		hp_clr.a( ) *= m_alpha[i] / 255.f;

		int fill = (int)std::round( hp * h / 100.f );

		render::rect_filled( bbox.x - 6, y - 1, 3, h + 2, { 10, 10, 10, int( m_alpha[i] ) } );

		render::rect( bbox.x - 5, y + h - fill, 1, fill, !player->dormant( ) ? hp_clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );

		if ( hp < 93 )
			render::pixel.string( bbox.x - 5, y + ( h - fill ) - 5, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), std::to_string( hp ), render::ALIGN_CENTER );
	}

	/*  bottom bars  */
	if ( player->alive( ) ) {
		int  offset{ 0 };

		if ( config["esp_lby"].get<bool>( ) ) {
			AimPlayer* data = &g_aimbot.m_players[player->index( ) - 1];

			if ( data && data->m_records.size( ) >= 2 ) {
				LagRecord* current = data->m_records.front( ).get( );

				if ( current ) {
					float cycle = std::clamp<float>( data->m_body_update - current->m_sim_time, 0.f, 1.0f );
					float width = ( bbox.w * cycle ) / 1.1f;

					if ( width > 0.f ) {
						render::rect_filled( bbox.x, bbox.y + bbox.h + 2, bbox.w, 3, { 10, 10, 10, int( m_alpha[i] ) } );

						Color clr = config["esp_lby_clr"].get_color( );
						clr.a( ) *= m_alpha[i] / 255.f;
						render::rect( bbox.x + 1, bbox.y + bbox.h + 3, width, 1, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );

						offset += 4;
					}
				}
			}
		}

		if ( 1 ) {
			Weapon* weapon = player->GetActiveWeapon( );
			if ( weapon ) {
				WeaponInfo* data = weapon->GetWpnData( );
				if ( data ) {
					int bar;
					float scale;

					int max = data->m_max_clip1;
					int current = weapon->m_iClip1( );

					C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[1];

					bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

					if ( max != -1 && config["esp_ammo"].get<bool>( ) ) {
						if ( reload )
							scale = layer1->m_cycle;

						else
							scale = (float)current / max;

						bar = (int)std::round( ( bbox.w - 2 ) * scale );

						render::rect_filled( bbox.x, bbox.y + bbox.h + 2 + offset, bbox.w, 3, { 10, 10, 10, int( m_alpha[i] ) } );

						Color clr = config["esp_ammo_clr"].get_color( );
						clr.a( ) *= m_alpha[i] / 255.f;
						render::rect( bbox.x + 1, bbox.y + bbox.h + 3 + offset, bar, 1, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );

						if ( current <= (int)std::round( max / 5 ) && !reload )
							render::esp_small.string( bbox.x + bar, bbox.y + bbox.h + offset, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), std::to_string( current ), render::ALIGN_CENTER );

						offset += 6;
					}

					if ( config["esp_wpnt"].get<bool>( ) ) {
						std::string name{ weapon->GetLocalizedName( ) };

						Color clr = config["esp_wpnt_clr"].get_color( );
						clr.a( ) *= m_alpha[i] / 255.f;

						std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );

						render::esp_small.string( bbox.x + bbox.w / 2, bbox.y + bbox.h + offset, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ), name, render::ALIGN_CENTER );

						offset += 13;
					}

					if ( config["esp_wpni"].get<bool>( ) ) {
						Color clr = config["esp_wpni_clr"].get_color( );
						clr.a( ) *= m_alpha[i] / 255.f;

						std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[weapon->m_iItemDefinitionIndex( )] );
						render::cs.string( bbox.x + bbox.w / 2, bbox.y + bbox.h + offset, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ), icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}

	/*  flags  */
	if ( player->alive( ) && !player->dormant( ) ) {
		std::vector< std::pair< std::string, Color > > flags;

		if ( !config["esp_flags"].get<bool>( ) )
			return;

		AimPlayer* data{ &g_aimbot.m_players[i - 1] };
		if ( data ) {
			if ( !data->m_records.empty( ) ) {
				LagRecord* front{ data->m_records.front( ).get( ) };

				flags.push_back( { front->m_dbg, { 255, 255, 255, int( m_alpha[i] ) } } );
			}
		}

		{
			if ( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
				flags.push_back( { XOR( "hk" ), { 103, 160, 224, int( m_alpha[i] ) } } );

			else if ( player->m_bHasHelmet( ) )
				flags.push_back( { XOR( "h" ), { 103, 160, 224, int( m_alpha[i] ) } } );

			else if ( player->m_ArmorValue( ) > 0 )
				flags.push_back( { XOR( "k" ), { 103, 160, 224, int( m_alpha[i] ) } } );
		}

		if ( player->m_bIsScoped( ) )
			flags.push_back( { XOR( "zoom" ), { 224, 160, 103, int( m_alpha[i] ) } } );

		if ( player->m_flFlashBangTime( ) > 0.f )
			flags.push_back( { XOR( "blind" ), { 224, 103, 206, int( m_alpha[i] ) } } );

		C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[1];
		if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
			flags.push_back( { XOR( "r" ), { 60, 180, 225, int( m_alpha[i] ) } } );

		if ( player->HasC4( ) )
			flags.push_back( { XOR( "b" ), { 224, 103, 106, int( m_alpha[i] ) } } );

		// iterate flags.
		for ( size_t i{ }; i < flags.size( ); ++i ) {
			// get flag job (pair).
			const auto& f = flags[i];

			int offset = i * ( render::esp_small.m_size.m_height - 1 );

			// draw flag.
			render::esp_small.string( bbox.x + bbox.w + 2, bbox.y + offset, f.second, f.first );
		}
	}

	/* debug */
	if ( player->alive( ) && !player->dormant( ) ) {

		for ( int i{ 0 }; i < 12; i++ ) {

			C_AnimationLayer layer{ player->m_AnimOverlay( )[i] };
			int y{ 0 };

			auto render_layer = [&]( std::string prefix, double value ) {
				render::esp.string( bbox.x + bbox.w + 2 + 20 + ( i * 50 ), bbox.y + ( y * 15 ), colors::white, tfm::format( "%s: %s", prefix, std::round( value ) ) );
				y++;
			};

			render_layer( "act", layer.m_activty );
			render_layer( "time", layer.m_anim_time );
			render_layer( "bits", layer.m_bits );
			render_layer( "cycle", layer.m_cycle );
			render_layer( "pcycle", layer.m_prev_cycle );
			render_layer( "fade", layer.m_fade_out_time );
			render_layer( "flags", layer.m_flags );
			render_layer( "order", layer.m_order );
			render_layer( "rate", layer.m_playback_rate );
			render_layer( "pr", layer.m_priority );
			render_layer( "seq", layer.m_sequence );
			render_layer( "weight", layer.m_weight );
			render_layer( "wrate", layer.m_weight_delta_rate );

		}

	}

	if ( player->alive( ) && !player->dormant( ) ) {
		if ( config["esp_skeleton"].get<bool>( ) )
			DrawSkeleton( player, m_alpha[i] );
	}
}

void Visuals::DrawPlantedC4( ) {
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = []( float damage, int armor_value ) {
		float new_damage, armor;

		if ( armor_value > 0 ) {
			new_damage = damage * 0.5f;
			armor = ( damage - new_damage ) * 0.5f;

			if ( armor > (float)armor_value ) {
				armor = (float)armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max( 0, (int)std::floor( damage ) );
	};

	// store menu vars for later.
	mode_2d = g_menu.main.visuals.planted_c4.get( 0 );
	mode_3d = g_menu.main.visuals.planted_c4.get( 1 );
	if ( !mode_2d && !mode_3d )
		return;

	// bomb not currently active, do nothing.
	if ( !m_c4_planted )
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter( );
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length( );

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp( ( dist * dist ) / ( ( m_planted_c4_radius_scaled * -2.f ) * m_planted_c4_radius_scaled ) );

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage( range_damage, g_cl.m_local->m_ArmorValue( ) );
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	damage_str = tfm::format( XOR( "%i" ), final_damage );

	// get damage color.
	damage_color = ( final_damage < g_cl.m_local->m_iHealth( ) ) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen( m_planted_c4_explosion_origin, screen_pos );

	// 'on screen (2D)'.
	if ( mode_2d ) {
		// g_cl.m_height - 80 - ( 30 * i )
		// 80 - ( 30 * 2 ) = 20

		// render::menu_shade.string( 60, g_cl.m_height - 20 - ( render::hud_size.m_height / 2 ), 0xff0000ff, "", render::hud );

		// todo - dex; move this next to indicators?

		if ( explode_time_diff > 0.f )
			render::esp.string( 2, 65, colors::white, time_str, render::ALIGN_LEFT );

		if ( g_cl.m_local->alive( ) )
			render::esp.string( 2, 65 + render::esp.m_size.m_height, damage_color, damage_str, render::ALIGN_LEFT );
	}

	// 'on bomb (3D)'.
	if ( mode_3d && is_visible ) {
		if ( explode_time_diff > 0.f )
			render::esp_small.string( screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER );

		// only render damage string if we're alive.
		if ( g_cl.m_local->alive( ) )
			render::esp_small.string( screen_pos.x, (int)screen_pos.y + render::esp_small.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER );
	}
}

bool Visuals::GetPlayerBoxRect( Player* player, Rect& box ) {
	vec3_t min, max, out_vec;
	float left, bottom, right, top;

	auto& tran_frame = player->coord_frame( );
	min = player->m_vecMins( );
	max = player->m_vecMaxs( );

	vec2_t screen_boxes[8];
	vec3_t points[] =
	{
		{ min.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z }
	};

	for ( int i = 0; i <= 7; i++ ) {
		math::VectorTransform( points[i], tran_frame, out_vec );
		if ( !render::WorldToScreen( out_vec, screen_boxes[i] ) )
			return false;
	}
	vec2_t box_array[] = {
		screen_boxes[3],
		screen_boxes[5],
		screen_boxes[0],
		screen_boxes[4],
		screen_boxes[2],
		screen_boxes[1],
		screen_boxes[6],
		screen_boxes[7]
	};
	left = screen_boxes[3].x,
		bottom = screen_boxes[3].y,
		right = screen_boxes[3].x,
		top = screen_boxes[3].y;
	for ( int i = 0; i <= 7; i++ ) {
		if ( left > box_array[i].x )
			left = box_array[i].x;
		if ( bottom < box_array[i].y )
			bottom = box_array[i].y;
		if ( right < box_array[i].x )
			right = box_array[i].x;
		if ( top > box_array[i].y )
			top = box_array[i].y;
	}
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = ( bottom - top );

	return true;
}

void Visuals::DrawHistorySkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if ( !g_menu.main.misc.fake_latency.get( ) )
		return;

	// get player's model.
	model = player->GetModel( );
	if ( !model )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	data = &g_aimbot.m_players[player->index( ) - 1];
	if ( !data )
		return;

	record = g_resolver.FindLastRecord( data );
	if ( !record )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i ) {
		// get bone.
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		record->m_bones->get_bone( bone_pos, i );
		record->m_bones->get_bone( parent_pos, parent );

		Color clr = player->enemy( g_cl.m_local ) ? g_menu.main.players.skeleton_enemy.get( ) : g_menu.main.players.skeleton_friendly.get( );
		clr.a( ) = opacity;

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::DrawSkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel( );
	if ( !model )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	// get bone matrix.
	if ( !player->SetupBones( matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime ) )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i ) {
		// get bone.
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone( bone_pos, i );
		matrix->get_bone( parent_pos, parent );

		Color clr = config["esp_skeleton_clr"].get_color( );
		clr.a( ) *= opacity / 255.f;

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::RenderGlow( ) {
	Color   color;
	Player* player;

	if ( !g_cl.m_local )
		return;

	if ( !g_csgo.m_glow->m_object_definitions.Count( ) )
		return;

	for ( int i{ }; i < g_csgo.m_glow->m_object_definitions.Count( ); ++i ) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];

		// skip non-players.
		if ( !obj->m_entity || !obj->m_entity->IsPlayer( ) )
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >( );

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[player->index( ) - 1];

		bool enemy = player->enemy( g_cl.m_local );

		if ( !enemy || !config["esp_glow"].get<bool>( ) )
			continue;

		color = config["esp_glow_clr"].get_color( );
		color.a( ) *= m_alpha[player->index( )] / 255.f;

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { (float)color.r( ) / 255.f, (float)color.g( ) / 255.f, (float)color.b( ) / 255.f };
		obj->m_alpha = (float)color.a( ) / 255.f;
	}
}

void Visuals::AddMatrix( Player* player, matrix3x4_t* bones ) {
	auto& hit = m_hit_matrix.emplace_back( );

	std::memcpy( hit.pBoneToWorld, bones, player->bone_cache( ).count( ) * sizeof( matrix3x4_t ) );

	hit.time = g_csgo.m_globals->m_realtime;//g_csgo.m_globals->m_realtime + g_menu.main.players.chams_shot_fadetime.get();

	static int m_nSkin = 0xA1C;
	static int m_nBody = 0xA20;

	hit.info.m_origin = player->GetAbsOrigin( );
	hit.info.m_angles = player->GetAbsAngles( );

	auto renderable = player->renderable( );
	if ( !renderable )
		return;

	auto model = player->GetModel( );
	if ( !model )
		return;

	auto hdr = *(studiohdr_t**)( player->GetModelPtr( ) );
	if ( !hdr )
		return;

	hit.state.m_pStudioHdr = hdr;
	hit.state.m_pStudioHWData = g_csgo.m_model_cache->GetHardwareData( model->m_studio );
	hit.state.m_pRenderable = renderable;
	hit.state.m_drawFlags = 0;

	hit.info.m_renderable = renderable;
	hit.info.m_model = model;
	hit.info.m_lighting_offset = nullptr;
	hit.info.m_lighting_origin = nullptr;
	hit.info.m_hitboxset = player->m_nHitboxSet( );
	hit.info.m_skin = (int)( uintptr_t( player ) + m_nSkin );
	hit.info.m_body = (int)( uintptr_t( player ) + m_nBody );
	hit.info.m_index = player->index( );
	hit.info.m_instance = util::get_method<ModelInstanceHandle_t( __thiscall* )( void* ) >( renderable, 30u )( renderable );
	hit.info.m_flags = 0x1;

	hit.info.m_model_to_world = &hit.model_to_world;
	hit.state.m_pModelToWorld = &hit.model_to_world;

	math::angle_matrix( hit.info.m_angles, hit.info.m_origin, hit.model_to_world );
}

void Visuals::override_material( bool ignoreZ, bool use_env, Color& color, IMaterial* material ) {
	material->SetFlag( MATERIAL_VAR_IGNOREZ, ignoreZ );
	material->IncrementReferenceCount( );

	bool found;
	auto var = material->FindVar( "$envmaptint", &found );

	if ( found )
		var->set_vec_value( color.r( ), color.g( ), color.b( ) );

	g_csgo.m_studio_render->ForcedMaterialOverride( material );
}

void Visuals::on_post_screen_effects( ) {
	if ( !g_cl.m_processing )
		return;

	const auto local = g_cl.m_local;
	if ( !local || !config["chams_shot"].get<bool>( ) || !g_csgo.m_engine->IsInGame( ) )
		m_hit_matrix.clear( );

	if ( m_hit_matrix.empty( ) || !g_csgo.m_model_render )
		return;

	auto ctx = g_csgo.m_material_system->get_render_context( );
	if ( !ctx )
		return;

	auto it = m_hit_matrix.begin( );

	while ( it != m_hit_matrix.end( ) ) {
		if ( !it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			 !it->info.m_renderable || !it->info.m_model_to_world || !it->info.m_model ) {
			++it;
			continue;
		}

		auto alpha = 1.0f;
		auto delta = g_csgo.m_globals->m_realtime - it->time;

		if ( delta > 0.0f ) {
			alpha -= delta;

			if ( delta > 1.0f ) {
				it = m_hit_matrix.erase( it );
				continue;
			}
		}

		auto do_render = [&]( std::string var, int mat, bool z ) {
			Color clr{ config[tfm::format( "chams_%s", var )].get_color( ) };
			g_chams.SetAlpha( ( clr.a( ) / 255.f ) * alpha );
			g_chams.SetupMaterial( g_chams.m_materials[mat], clr, z );
		};

		switch ( config["chams_shot_mat"].get<int>( ) ) {
			case 0:
				{

					do_render( "shot_clr", 0, true );

					break;
				}
			case 1:
				{

					do_render( "shot_clr", 1, true );

					break;
				}
			case 2:
				{

					do_render( "shot_clr", 0, true );
					do_render( "shot_acc", 2, true );

					break;
				}
			case 3:
				{

					do_render( "shot_clr", 0, true );
					do_render( "shot_acc", 3, true );

					break;
				}
			default:
				break;
		}

		g_csgo.m_model_render->DrawModelExecute( ctx, it->state, it->info, it->pBoneToWorld );
		g_csgo.m_model_render->ForceMat( nullptr );

		++it;
	}
}

void Visuals::DrawHitboxMatrix( LagRecord* record, Color col, float time ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel( );
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	set = hdr->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if ( !set )
		return;

	for ( int i{ }; i < set->m_hitboxes; ++i ) {
		bbox = set->GetHitbox( i );
		if ( !bbox )
			continue;

		// bbox.
		if ( bbox->m_radius <= 0.f ) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms( record->m_bones[bbox->m_bone], rot_matrix, matrix );

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin( );

			// draw box.
			//g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix( bbox->m_angle, matrix );

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms( record->m_bones[bbox->m_bone], matrix, matrix );

			// get world positions from new matrix.
			math::VectorTransform( bbox->m_mins, matrix, mins );
			math::VectorTransform( bbox->m_maxs, matrix, maxs );

			//g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, 0 );
		}
	}
}

void Visuals::DrawBeams( ) {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if ( !g_cl.m_local )
		return;

	if ( !config["vis_beam"].get<bool>( ) )
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if ( !g_cl.m_processing ) {
		if ( !vis_impacts->empty( ) )
			vis_impacts->clear( );
	}

	else {
		impact_count = vis_impacts->size( );
		if ( !impact_count )
			return;

		curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

		for ( size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ]( i );
			if ( !impact )
				continue;

			// impact is too old, erase it.
			if ( std::abs( curtime - game::TICKS_TO_TIME( impact->m_tickbase ) ) > g_menu.main.visuals.impact_beams_time.get( ) ) {
				vis_impacts->erase( vis_impacts->begin( ) + i );

				continue;
			}

			// already rendering this impact, skip over it.
			if ( impact->m_ignore )
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if ( i == ( impact_count - 1 ) )
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ( ( i + 1 ) < impact_count && impact->m_tickbase != vis_impacts->operator[ ]( i + 1 ).m_tickbase )
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if ( is_final_impact ) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = ( impact->m_impact_pos - start ).normalized( );
				dist = ( impact->m_impact_pos - start ).length( );

				end = start + ( dir * dist );

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( XOR( "sprites/white.vmt" ) );
				beam_info.m_pszModelName = XOR( "sprites/white.vmt" );
				beam_info.m_flHaloScale = 0.5f;
				beam_info.m_flLife = 3.f;
				beam_info.m_flWidth = 1.f;
				beam_info.m_flEndWidth = 1.f;
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 0.25f;   // beam 'jitter'.
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 0.5f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0;

				beam_info.m_flRed = (float)config["vis_beam_clr"].get_color( ).r( ) / 255.f;
				beam_info.m_flGreen = (float)config["vis_beam_clr"].get_color( ).g( ) / 255.f;
				beam_info.m_flBlue = (float)config["vis_beam_clr"].get_color( ).b( ) / 255.f;

				// attempt to render the beam.
				beam = game::CreateGenericBeam( beam_info );
				if ( beam ) {
					g_csgo.m_beams->DrawBeam( beam );

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DebugAimbotPoints( Player* player ) {
	std::vector< vec3_t > p2{ };

	AimPlayer* data = &g_aimbot.m_players.at( player->index( ) - 1 );
	if ( !data || data->m_records.empty( ) )
		return;

	LagRecord* front = data->m_records.front( ).get( );
	if ( !front || front->dormant( ) )
		return;

	// get bone matrix.
	BoneArray matrix[128];
	if ( !g_bones.setup( player, matrix, front ) )
		return;

	data->SetupHitboxes( front, false );
	if ( data->m_hitboxes.empty( ) )
		return;

	for ( const auto& it : data->m_hitboxes ) {
		std::vector< vec3_t > p1{ };

		if ( !data->SetupHitboxPoints( front, matrix, it.m_index, p1 ) )
			continue;

		for ( auto& p : p1 )
			p2.push_back( p );
	}

	if ( p2.empty( ) )
		return;

	for ( auto& p : p2 ) {
		vec2_t screen;

		if ( render::WorldToScreen( p, screen ) )
			render::rect_filled( screen.x, screen.y, 2, 2, { 0, 255, 255, 255 } );
	}
}