#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld( ) {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for ( uint16_t h{ g_csgo.m_material_system->FirstMaterial( ) }; h != g_csgo.m_material_system->InvalidMaterial( ); h = g_csgo.m_material_system->NextMaterial( h ) ) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
		if ( !mat )
			continue;

		// store world materials.
		if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "World textures" ) )
			world.push_back( mat );

		// store props.
		else if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "StaticProp textures" ) )
			props.push_back( mat );
	}

	// night
	const float darkness = g_menu.main.visuals.night_darkness.get( ) / 100.f;

	if ( g_menu.main.visuals.world.get( ) == 1 ) {
		for ( const auto& w : world )
			w->ColorModulate( darkness, darkness, darkness );

		// IsUsingStaticPropDebugModes my nigga
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		for ( const auto& p : props )
			p->ColorModulate( 0.5f, 0.5f, 0.5f );

		//game::SetSkybox(XOR("sky_csgo_night02"));
	}

	// disable night.
	else {
		for ( const auto& w : world )
			w->ColorModulate( 1.f, 1.f, 1.f );

		// restore r_DrawSpecificStaticProp.
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for ( const auto& p : props )
			p->ColorModulate( 1.f, 1.f, 1.f );
	}

	// transparent props.
	if ( g_menu.main.visuals.transparent_props.get( ) ) {

		// IsUsingStaticPropDebugModes my nigga
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		float alpha = g_menu.main.visuals.transparent_props_amount.get( ) / 100;
		for ( const auto& p : props )
			p->AlphaModulate( alpha );
	}

	// disable transparent props.
	else {

		// restore r_DrawSpecificStaticProp.
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for ( const auto& p : props )
			p->AlphaModulate( 1.0f );
	}
}

void Visuals::ThirdpersonThink( ) {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;
	static float anim{ 0.f };

	// for whatever reason overrideview also gets called from the main menu.
	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive( );

	// camera should be in thirdperson.
	if ( m_thirdperson ) {

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

		anim += 5.0f * g_csgo.m_globals->m_frametime;

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		// cam_idealdist convar.
		offset.z = g_menu.main.visuals.thirdperson_distance.get( );

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
		offset.z *= tr.m_fraction * anim;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
	else
		anim -= 5.0f * g_csgo.m_globals->m_frametime;

	anim = std::clamp( anim, 0.f, 1.f );
}

// meme...
void Visuals::IndicateAngles( )
{
	if ( !g_csgo.m_engine->IsInGame( ) && !g_csgo.m_engine->IsConnected( ) )
		return;

	if ( !g_menu.main.antiaim.draw_angles.get( ) )
		return;

	if ( !g_csgo.m_input->CAM_IsThirdPerson( ) )
		return;

	if ( !g_cl.m_local || g_cl.m_local->m_iHealth( ) < 1 )
		return;

	const auto& pos = g_cl.m_local->GetRenderOrigin( );
	vec2_t tmp;

	if ( render::WorldToScreen( pos, tmp ) )
	{
		vec2_t draw_tmp;
		const vec3_t real_pos( 50.f * std::cos( math::deg_to_rad( g_cl.m_radar.y ) ) + pos.x, 50.f * sin( math::deg_to_rad( g_cl.m_radar.y ) ) + pos.y, pos.z );

		if ( render::WorldToScreen( real_pos, draw_tmp ) )
		{
			render::line( tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 0, 255, 0, 255 } );
			render::esp_small.string( draw_tmp.x, draw_tmp.y, { 0, 255, 0, 255 }, "FAKE", render::ALIGN_LEFT );
		}

		if ( g_menu.main.antiaim.fake_yaw.get( ) )
		{
			const vec3_t fake_pos( 50.f * cos( math::deg_to_rad( g_cl.m_angle.y ) ) + pos.x, 50.f * sin( math::deg_to_rad( g_cl.m_angle.y ) ) + pos.y, pos.z );

			if ( render::WorldToScreen( fake_pos, draw_tmp ) )
			{
				render::line( tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 } );
				render::esp_small.string( draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 }, "REAL", render::ALIGN_LEFT );
			}
		}

		if ( g_menu.main.antiaim.body_fake_stand.get( ) == 1 || g_menu.main.antiaim.body_fake_stand.get( ) == 2 || g_menu.main.antiaim.body_fake_stand.get( ) == 3 || g_menu.main.antiaim.body_fake_stand.get( ) == 4 || g_menu.main.antiaim.body_fake_stand.get( ) == 5 || g_menu.main.antiaim.body_fake_stand.get( ) == 6 )
		{
			float lby = g_cl.m_local->m_flLowerBodyYawTarget( );
			const vec3_t lby_pos( 50.f * cos( math::deg_to_rad( lby ) ) + pos.x,
				50.f * sin( math::deg_to_rad( lby ) ) + pos.y, pos.z );

			if ( render::WorldToScreen( lby_pos, draw_tmp ) )
			{
				render::line( tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 } );
				render::esp_small.string( draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 }, "LBY", render::ALIGN_LEFT );
			}
		}
	}
}

void Visuals::Hitmarker( ) {

	static auto cross = g_csgo.m_cvar->FindVar( HASH( "weapon_debug_spread_show" ) );
	cross->SetValue( g_menu.main.visuals.force_xhair.get( ) && !g_cl.m_local->m_bIsScoped( ) ? 3 : 0 );
	if ( !g_menu.main.misc.hitmarker.get( ) )
		return;

	if ( g_csgo.m_globals->m_curtime > m_hit_end )
		return;

	if ( m_hit_duration <= 0.f )
		return;

	float complete = ( g_csgo.m_globals->m_curtime - m_hit_start ) / m_hit_duration;
	int x = g_cl.m_width,
		y = g_cl.m_height,
		alpha = ( 1.f - complete ) * 240;

	constexpr int line{ 6 };


	render::rect_filled( x / 2 + 6, y / 2 + 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 7, y / 2 + 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 8, y / 2 + 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 9, y / 2 + 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 10, y / 2 + 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 - 6, y / 2 - 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 7, y / 2 - 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 8, y / 2 - 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 9, y / 2 - 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 10, y / 2 - 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 - 6, y / 2 + 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 7, y / 2 + 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 8, y / 2 + 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 9, y / 2 + 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 10, y / 2 + 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 + 6, y / 2 - 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 7, y / 2 - 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 8, y / 2 - 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 9, y / 2 - 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 10, y / 2 - 10, 1, 1, { 200, 200, 200, alpha } );

	// damage indicator above head
	std::string out = tfm::format( XOR( "%i\n" ), g_shots.iHitDmg );

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		if ( render::WorldToScreen( g_shots.iPlayermins, g_shots.iPlayerbottom ) && render::WorldToScreen( g_shots.iPlayermaxs, g_shots.iPlayertop ) )
		{
			// get the esp box info >_<
			Rect box;
			box.h = g_shots.iPlayerbottom.y - g_shots.iPlayertop.y;
			box.w = box.h / 2.f;
			box.x = g_shots.iPlayerbottom.x - ( box.w / 2.f );
			box.y = g_shots.iPlayerbottom.y - box.h;

			// text damage
			if ( !g_shots.iHeadshot )
				render::damage.string( box.x + box.w / 2, box.y - render::esp.m_size.m_height - 10, { 255, 0, 0, alpha }, out, render::ALIGN_CENTER );

			if ( g_shots.iHeadshot == true )
				render::damage.string( box.x + box.w / 2, box.y - render::esp.m_size.m_height - 10, { 255, 0, 0, alpha }, out, render::ALIGN_CENTER );
			render::damage.string( box.x + box.w / 2, box.y - render::esp.m_size.m_height - 10, { 255, 0, 0, alpha }, out, render::ALIGN_CENTER );
		}
	}
}

void Visuals::NoSmoke( ) {
	if ( !g_menu.main.visuals.nosmoke.get( ) )
		return;
	//colado https://www.unknowncheats.me/forum/counterstrike-global-offensive/262635-epic-wireframe-smoke.html
	std::vector<const char*> vistasmoke_mats =
	{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	for ( auto mat_s : vistasmoke_mats )
	{
		IMaterial* mat = g_csgo.m_material_system->FindMaterial( mat_s, XOR( "Other textures" ) );
		mat->SetFlag( MATERIAL_VAR_WIREFRAME, true );
	}
}

void Visuals::think( ) {
	// don't run anything if our local player isn't valid.
	if ( !g_cl.m_local )
		return;

	if ( g_menu.main.visuals.noscope.get( )
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped( ) ) {

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
		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}

	// draw esp on ents.
	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		draw( ent );
	}

	// draw everything else.
	SpreadCrosshair( );
	StatusIndicators( );
	Spectators( );
	PenetrationCrosshair( );
	ManualAntiAim( );
	Hitmarker( );
	DrawPlantedC4( );
}

void Visuals::Spectators( ) {
	if ( !g_menu.main.visuals.spectators.get( ) )
		return;

	std::vector< std::string > spectators{ XOR( "" ) };
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

		render::menu_shade.string( g_cl.m_width - 15, ( i * 14 ) + 15, { 255, 255, 255, 179 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::StatusIndicators( ) {
	// dont do if dead.
	if ( !g_cl.m_processing )
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// LC
	if ( g_menu.main.visuals.indicators.get( 1 ) && ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) ) {
		Indicator_t ind{ };
		ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LC" );

		indicators.push_back( ind );
	}

	// LBY
	if ( g_menu.main.visuals.indicators.get( 0 ) ) {
		// get the absolute change between current lby and animated angle.
		float change = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

		Indicator_t ind{ };
		ind.color = change > 35.f ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}


	// PING
	if ( g_menu.main.visuals.indicators.get( 2 ) ) {
		Indicator_t ind{ };
		ind.color = g_aimbot.m_fake_latency ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "PING" );

		indicators.push_back( ind );
	}

	if ( indicators.empty( ) )
		return;

	// iterate and draw indicators.
	for ( size_t i{ }; i < indicators.size( ); ++i ) {
		auto& indicator = indicators[i];

		render::indicator.string( 12, g_cl.m_height - 64 - ( 30 * i ), indicator.color, indicator.text );
	}

	auto local_player = g_cl.m_local;
	int screen_width, screen_height;
	g_csgo.m_engine->GetScreenSize( screen_width, screen_height );

	static float next_lby_update[65];
	//static float last_lby[65];

	const float curtime = g_csgo.m_globals->m_curtime;

	//if (local_player->GetVelocity().Length2D() > 0.1 && !global::is_fakewalking)
	//    return;

	if ( local_player->m_vecVelocity( ).length_2d( ) > 0.1f && !g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) ) )
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;
	static float last_lby[65];
	if ( last_lby[local_player->index( )] != local_player->m_flLowerBodyYawTarget( ) )
	{
		last_lby[local_player->index( )] = local_player->m_flLowerBodyYawTarget( );
		next_lby_update[local_player->index( )] = curtime + 1.125f + g_csgo.m_globals->m_interval;
	}

	if ( next_lby_update[local_player->index( )] < curtime )
	{
		next_lby_update[local_player->index( )] = curtime + 1.125f;
	}

	float time_remain_to_update = next_lby_update[local_player->index( )] - local_player->m_flSimulationTime( );
	float time_update = next_lby_update[local_player->index( )];


	float fill = 0;
	fill = ( ( ( time_remain_to_update ) ) );
	static float add = 0.000f;
	add = 1.125f - fill;

	float change1337 = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

	Color color1337 = { 255,0,0,255 };

	if ( change1337 > 35.f ) {
		color1337 = { 124,195,13,255 };
	}


	if ( !( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) && g_menu.main.visuals.indicators.get( 0 ) ) {
		render::draw_arc( 12 + 60, g_cl.m_height - 64 + 23 - 9, 8, 0, 360, 5, { 0,0,0,125 } );
		render::draw_arc( 12 + 60, g_cl.m_height - 64 + 23 - 9, 7, 0, 340 * add, 3, color1337 );
	}
	// std::string add1 = tfm::format(XOR("%i"), add);
	 //render::esp_small.string(500, 500, color1337, add1, render::ALIGN_CENTER);
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
	int type = weapon->m_iItemDefinitionIndex( );
	if ( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	int w, h;
	g_csgo.m_engine->GetScreenSize( w, h );

	float spreadDist = ( ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * 320.f ) / std::tan( math::deg_to_rad( g_cl.m_local->GetFOV( ) ) * 0.5f );
	float spreadRadius = ( spreadDist * ( h / 480.f ) ) * 50 / 250.f;


	for ( float i = 0; i <= spreadRadius; i++ )
	{
		Color col = g_menu.main.visuals.spread_xhair_col.get( );
		col.a( ) = ( static_cast<int>( i * ( 255.f / spreadRadius ) ) * g_menu.main.visuals.spread_xhair_blend.get( ) / 100.f );
		g_csgo.m_surface->DrawSetColor( col );
		g_csgo.m_surface->DrawOutlinedCircle( w / 2, h / 2, static_cast<int>( i ), 240 );
	}
}

void Visuals::ManualAntiAim( ) {
	int   x, y;

	// dont do if dead.
	if ( !g_cl.m_processing )
		return;

	if ( !g_menu.main.antiaim.manul_antiaim.get( ) )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	Color color = g_menu.main.antiaim.color_manul_antiaim.get( );


	if ( g_hvh.m_left ) {
		render::rect_filled( x - 50, y - 10, 1, 21, color );
		render::rect_filled( x - 51, y - 10 + 1, 1, 19, color );
		render::rect_filled( x - 52, y - 10 + 2, 1, 17, color );
		render::rect_filled( x - 53, y - 10 + 3, 1, 15, color );
		render::rect_filled( x - 54, y - 10 + 4, 1, 13, color );
		render::rect_filled( x - 55, y - 10 + 5, 1, 11, color );
		render::rect_filled( x - 56, y - 10 + 6, 1, 9, color );
		render::rect_filled( x - 57, y - 10 + 7, 1, 7, color );
		render::rect_filled( x - 58, y - 10 + 8, 1, 5, color );
		render::rect_filled( x - 59, y - 10 + 9, 1, 3, color );
		render::rect_filled( x - 60, y - 10 + 10, 1, 1, color );
	}
	else {
		render::rect_filled( x - 50, y - 10, 1, 21, { 0, 0, 0, 125 } );
		render::rect_filled( x - 51, y - 10 + 1, 1, 19, { 0, 0, 0, 125 } );
		render::rect_filled( x - 52, y - 10 + 2, 1, 17, { 0, 0, 0, 125 } );
		render::rect_filled( x - 53, y - 10 + 3, 1, 15, { 0, 0, 0, 125 } );
		render::rect_filled( x - 54, y - 10 + 4, 1, 13, { 0, 0, 0, 125 } );
		render::rect_filled( x - 55, y - 10 + 5, 1, 11, { 0, 0, 0, 125 } );
		render::rect_filled( x - 56, y - 10 + 6, 1, 9, { 0, 0, 0, 125 } );
		render::rect_filled( x - 57, y - 10 + 7, 1, 7, { 0, 0, 0, 125 } );
		render::rect_filled( x - 58, y - 10 + 8, 1, 5, { 0, 0, 0, 125 } );
		render::rect_filled( x - 59, y - 10 + 9, 1, 3, { 0, 0, 0, 125 } );
		render::rect_filled( x - 60, y - 10 + 10, 1, 1, { 0, 0, 0, 125 } );
	}

	if ( g_hvh.m_right ) {
		render::rect_filled( x + 50, y - 10, 1, 21, color );
		render::rect_filled( x + 51, y - 10 + 1, 1, 19, color );
		render::rect_filled( x + 52, y - 10 + 2, 1, 17, color );
		render::rect_filled( x + 53, y - 10 + 3, 1, 15, color );
		render::rect_filled( x + 54, y - 10 + 4, 1, 13, color );
		render::rect_filled( x + 55, y - 10 + 5, 1, 11, color );
		render::rect_filled( x + 56, y - 10 + 6, 1, 9, color );
		render::rect_filled( x + 57, y - 10 + 7, 1, 7, color );
		render::rect_filled( x + 58, y - 10 + 8, 1, 5, color );
		render::rect_filled( x + 59, y - 10 + 9, 1, 3, color );
		render::rect_filled( x + 60, y - 10 + 10, 1, 1, color );
	}
	else {
		render::rect_filled( x + 50, y - 10, 1, 21, { 0, 0, 0, 125 } );
		render::rect_filled( x + 51, y - 10 + 1, 1, 19, { 0, 0, 0, 125 } );
		render::rect_filled( x + 52, y - 10 + 2, 1, 17, { 0, 0, 0, 125 } );
		render::rect_filled( x + 53, y - 10 + 3, 1, 15, { 0, 0, 0, 125 } );
		render::rect_filled( x + 54, y - 10 + 4, 1, 13, { 0, 0, 0, 125 } );
		render::rect_filled( x + 55, y - 10 + 5, 1, 11, { 0, 0, 0, 125 } );
		render::rect_filled( x + 56, y - 10 + 6, 1, 9, { 0, 0, 0, 125 } );
		render::rect_filled( x + 57, y - 10 + 7, 1, 7, { 0, 0, 0, 125 } );
		render::rect_filled( x + 58, y - 10 + 8, 1, 5, { 0, 0, 0, 125 } );
		render::rect_filled( x + 59, y - 10 + 9, 1, 3, { 0, 0, 0, 125 } );
		render::rect_filled( x + 60, y - 10 + 10, 1, 1, { 0, 0, 0, 125 } );
	}
	if ( g_hvh.m_back ) {
		render::rect_filled( x - 10, y + 50, 21, 1, color );
		render::rect_filled( x - 10 + 1, y + 51, 19, 1, color );
		render::rect_filled( x - 10 + 2, y + 52, 17, 1, color );
		render::rect_filled( x - 10 + 3, y + 53, 15, 1, color );
		render::rect_filled( x - 10 + 4, y + 54, 13, 1, color );
		render::rect_filled( x - 10 + 5, y + 55, 11, 1, color );
		render::rect_filled( x - 10 + 6, y + 56, 9, 1, color );
		render::rect_filled( x - 10 + 7, y + 57, 7, 1, color );
		render::rect_filled( x - 10 + 8, y + 58, 5, 1, color );
		render::rect_filled( x - 10 + 9, y + 59, 3, 1, color );
		render::rect_filled( x - 10 + 10, y + 60, 1, 1, color );
	}
	else {
		render::rect_filled( x - 10, y + 50, 21, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 1, y + 51, 19, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 2, y + 52, 17, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 3, y + 53, 15, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 4, y + 54, 13, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 5, y + 55, 11, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 6, y + 56, 9, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 7, y + 57, 7, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 8, y + 58, 5, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 9, y + 59, 3, 1, { 0, 0, 0, 125 } );
		render::rect_filled( x - 10 + 10, y + 60, 1, 1, { 0, 0, 0, 125 } );
	}
}

void Visuals::PenetrationCrosshair( ) {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if ( !g_menu.main.visuals.pen_crosshair.get( ) || !g_cl.m_processing )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;


	valid_player_hit = ( g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy( g_cl.m_local ) );
	if ( valid_player_hit )
		final_color = colors::light_blue;

	else if ( g_cl.m_pen_data.m_pen )
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//             draw damage string?

	// draw small square in center of screen.
	int damage1337 = g_cl.m_pen_data.m_damage;

	if ( g_menu.main.visuals.pen_damage.get( ) && ( g_cl.m_pen_data.m_pen || valid_player_hit ) )
		render::esp_small.string( x + 3, y + 2, { final_color }, std::to_string( damage1337 ).c_str( ), render::ALIGN_LEFT );
	if ( g_cl.m_pen_data.m_damage > 1 ) {
		render::rect_filled( x - 1, y, 1, 1, { final_color } );
		render::rect_filled( x, y, 1, 1, { final_color } );
		render::rect_filled( x + 1, y, 1, 1, { final_color } );
		render::rect_filled( x, y + 1, 1, 1, { final_color } );
		render::rect_filled( x, y - 1, 1, 1, { final_color } );
		//shadow
		render::rect_filled( x - 2, y, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 2, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 2, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y - 1, 1, 1, { 0,0, 0, 125 } );
	}
}

void Visuals::draw( Entity* ent ) {
	if ( ent->IsPlayer( ) ) {
		Player* player = ent->as< Player* >( );

		// dont draw dead players.
		//if (!player->alive())
			//return;

		if ( player->m_bIsLocalPlayer( ) )
			return;

		// draw player esp.
		DrawPlayer( player );
	}

	else if ( g_menu.main.visuals.items.get( ) && ent->IsBaseCombatWeapon( ) && !ent->dormant( ) )
		DrawItem( ent->as< Weapon* >( ) );

	else if ( g_menu.main.visuals.proj.get( ) )
		DrawProjectile( ent->as< Weapon* >( ) );
}

void Visuals::DrawProjectile( Weapon* ent ) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin( );
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	Color col = g_menu.main.visuals.proj_color.get( );
	col.a( ) = 0xb4;

	if ( ent->is( HASH( "CHostage" ) ) ) {
		std::string distance;
		int dist = ( ( ( ent->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ) ).length_sqr( ) ) * 0.0625 ) * 0.001;
		//if (dist > 0)
		//distance = tfm::format(XOR("%i FT"), dist);
		if ( dist > 0 ) {
			if ( dist > 5 ) {
				while ( !( dist % 5 == 0 ) ) {
					dist = dist - 1;
				}

				if ( dist % 5 == 0 )
					distance = tfm::format( XOR( "%i FT" ), dist );
			}
			else
				distance = tfm::format( XOR( "%i FT" ), dist );
		}
		if ( dist < 150 ) {
			render::esp_small.string( screen.x, screen.y, colors::light_blue, XOR( "HOSTAGE" ), render::ALIGN_CENTER );
			render::esp_small.string( screen.x, screen.y - 7, colors::light_blue, distance, render::ALIGN_CENTER );
		}
	}

	// draw decoy.
	if ( ent->is( HASH( "CDecoyProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "DECOY" ), render::ALIGN_CENTER );

	// draw molotov.
	else if ( ent->is( HASH( "CMolotovProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "MOLOTOV" ), render::ALIGN_CENTER );

	else if ( ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) ) {
		const model_t* model = ent->GetModel( );

		if ( model ) {
			// grab modelname.
			std::string name{ ent->GetModel( )->m_name };

			if ( name.find( XOR( "flashbang" ) ) != std::string::npos )
				render::esp_small.string( screen.x, screen.y, col, XOR( "FLASH" ), render::ALIGN_CENTER );

			else if ( name.find( XOR( "fraggrenade" ) ) != std::string::npos ) {

				render::esp_small.string( screen.x, screen.y, col, XOR( "GRENADE" ), render::ALIGN_CENTER );
			}
		}
	}

	// find classes.
	else if ( ent->is( HASH( "CInferno" ) ) ) {
		render::esp_small.string( screen.x, screen.y, col, XOR( "FIRE" ), render::ALIGN_CENTER );
	}

	else if ( ent->is( HASH( "CSmokeGrenadeProjectile" ) ) ) {
		render::esp_small.string( screen.x, screen.y, col, XOR( "SMOKE" ), render::ALIGN_CENTER );
	}
}

void Visuals::DrawItem( Weapon* item ) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle( item->m_hOwnerEntity( ) );
	if ( owner )
		return;

	std::string distance;
	int dist = ( ( ( item->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ) ).length_sqr( ) ) * 0.0625 ) * 0.001;
	//if (dist > 0)
	//distance = tfm::format(XOR("%i FT"), dist);
	if ( dist > 0 ) {
		if ( dist > 5 ) {
			while ( !( dist % 5 == 0 ) ) {
				dist = dist - 1;
			}

			if ( dist % 5 == 0 )
				distance = tfm::format( XOR( "%i FT" ), dist );
		}
		else
			distance = tfm::format( XOR( "%i FT" ), dist );
	}

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin( );
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	WeaponInfo* data = item->GetWpnData( );
	if ( !data )
		return;

	Color col = g_menu.main.visuals.item_color.get( );
	col.a( ) = 0xb4;

	Color col1337 = g_menu.main.visuals.dropammo_color.get( );
	col1337.a( ) = 0xb4;

	// render bomb in green.
	if ( item->is( HASH( "CC4" ) ) )

		render::esp_small.string( screen.x, screen.y, { 150, 200, 60, 0xb4 }, XOR( "C4" ), render::ALIGN_CENTER );

	// if not bomb
	// normal item, get its name.
	else {
		std::string name{ item->GetLocalizedName( ) };

		// smallfonts needs uppercase.
		std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

		if ( g_menu.main.visuals.distance.get( ) )
			render::esp_small.string( screen.x, screen.y - 8, col, distance, render::ALIGN_CENTER );
		render::esp_small.string( screen.x, screen.y, col, name, render::ALIGN_CENTER );
	}

	if ( !g_menu.main.visuals.ammo.get( ) )
		return;

	// nades do not have ammo.
	if ( data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE )
		return;

	if ( item->m_iItemDefinitionIndex( ) == 0 || item->m_iItemDefinitionIndex( ) == C4 )
		return;

	std::string ammo = tfm::format( XOR( "(%i/%i)" ), item->m_iClip1( ), item->m_iPrimaryReserveAmmoCount( ) );
	//render::esp_small.string( screen.x, screen.y - render::esp_small.m_size.m_height - 1, col, ammo, render::ALIGN_CENTER );

	int current = item->m_iClip1( );
	int max = data->m_max_clip1;
	float scale = (float)current / max;
	int bar = (int)std::round( ( 51 - 2 ) * scale );
	render::rect_filled( screen.x - 25, screen.y + 9, 51, 4, { 0,0,0,180 } );
	render::rect_filled( screen.x - 25 + 1, screen.y + 9 + 1, bar, 2, col1337 );


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

	if ( !g_menu.main.players.offscreen.get( ) )
		return;

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

		float size = g_menu.main.config.offscreen_mode.get( ) / 100.f;
		float pos = g_menu.main.config.offscreen_mode1.get( );

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized( );

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * ( g_cl.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x + ( 1 * size ) , offscreen_pos.y + ( 1 * size ) };        // 0,  0
		verts[1] = { offscreen_pos.x - ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // -1, 1
		verts[2] = { offscreen_pos.x + ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
		verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
		verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[0] = render::RotateVertex( offscreen_pos, verts[0], offscreen_rotation );
		verts[1] = render::RotateVertex( offscreen_pos, verts[1], offscreen_rotation );
		verts[2] = render::RotateVertex( offscreen_pos, verts[2], offscreen_rotation );

		// render!
		int alpha1337 = sin( abs( fmod( -math::pi + ( g_csgo.m_globals->m_curtime * ( 2 / .75 ) ), ( math::pi * 2 ) ) ) ) * 255;

		if ( alpha1337 < 0 )
			alpha1337 = alpha1337 * ( -1 );

		color = g_menu.main.players.offscreen_color.get( ); // damage_data.m_color;
		color.a( ) = ( alpha == 255 ) ? alpha1337 : alpha / 2;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );

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
	if ( g_menu.main.players.name.get( 0 ) ) {
		std::string name{ std::string( info.m_name ).substr( 0, 24 ) };

		Color clr = g_menu.main.players.name_color.get( );
		clr.a( ) *= m_alpha[i] / 255.f;

		render::esp.string( bbox.x + bbox.w / 2, bbox.y - render::esp.m_size.m_height, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ), name, render::ALIGN_CENTER );
	}

	/*  box  */
	if ( g_menu.main.players.box.get( 0 ) ) {
		Color clr = g_menu.main.players.box_enemy.get( );
		clr.a( ) *= m_alpha[i] / 255.f;

		render::rect_outlined( bbox.x, bbox.y, bbox.w, bbox.h, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ), { 10, 10, 10, int( m_alpha[i] ) } );
	}

	/*  health  */
	if ( g_menu.main.players.health.get( 0 ) ) {
		int y = bbox.y + 1;
		int h = bbox.h - 2;

		int hp = std::min( 100, player->m_iHealth( ) );

		if ( m_hp[player->index( )] > hp )
			m_hp[player->index( )] -= 200.f * g_csgo.m_globals->m_frametime;
		else
			m_hp[player->index( )] = hp;

		hp = m_hp[player->index( )];

		int r = std::min( ( 510 * ( 100 - hp ) ) / 100, 255 );
		int g = std::min( ( 510 * hp ) / 100, 255 );

		int fill = (int)std::round( hp * h / 100.f );

		render::rect_filled( bbox.x - 6, y - 1, 4, h + 2, { 10, 10, 10, int( m_alpha[i] ) } );

		render::rect( bbox.x - 5, y + h - fill, 2, fill, !player->dormant( ) ? Color( r, g, 0, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ) );

		if ( hp < 93 )
			render::pixel.string( bbox.x - 5, y + ( h - fill ) - 5, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), std::to_string( hp ), render::ALIGN_CENTER );
	}

	/*  bottom bars  */
	if ( player->alive( ) ) {
		int  offset{ 0 };

		if ( g_menu.main.players.lby_update.get( ) ) {
			AimPlayer* data = &g_aimbot.m_players[player->index( ) - 1];

			if ( data && data->m_moved && data->m_records.size( ) ) {
				LagRecord* current = data->m_records.front( ).get( );

				if ( current ) {
					if ( !( current->m_velocity.length_2d( ) > 0.1 && !current->m_fake_walk ) ) {
						float cycle = std::clamp<float>( data->m_body_update - current->m_anim_time, 0.f, 1.0f );
						float width = ( bbox.w * cycle ) / 1.1f;

						if ( width > 0.f ) {
							render::rect_filled( bbox.x, bbox.y + bbox.h + 2, bbox.w, 4, { 10, 10, 10, int( m_alpha[i] ) } );

							Color clr = g_menu.main.players.lby_update_color.get( );
							clr.a( ) = m_alpha[i];
							render::rect( bbox.x + 1, bbox.y + bbox.h + 3, width, 2, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );

							offset += 5;
						}
					}
				}
			}
		}

		if ( g_menu.main.players.weapon.get( 0 ) ) {
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

					if ( max != -1 && g_menu.main.players.ammo.get( ) ) {
						if ( reload )
							scale = layer1->m_cycle;

						else
							scale = (float)current / max;

						bar = (int)std::round( ( bbox.w - 2 ) * scale );

						render::rect_filled( bbox.x, bbox.y + bbox.h + 2 + offset, bbox.w, 4, { 10, 10, 10, int( m_alpha[i] ) } );

						Color clr = g_menu.main.players.ammo_color.get( );
						clr.a( ) = int( m_alpha[i] );
						render::rect( bbox.x + 1, bbox.y + bbox.h + 3 + offset, bar, 2, !player->dormant( ) ? clr : Color( 141, 141, 141, int( m_alpha[i] ) ) );

						if ( current <= (int)std::round( max / 5 ) && !reload )
							render::esp_small.string( bbox.x + bar, bbox.y + bbox.h + offset, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), std::to_string( current ), render::ALIGN_CENTER );

						offset += 6;
					}

					if ( g_menu.main.players.weapon_mode.get( 0 ) ) {
						std::string name{ weapon->GetLocalizedName( ) };

						// smallfonts needs upper case.
						std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );

						render::esp_small.string( bbox.x + bbox.w / 2, bbox.y + bbox.h + offset, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), name, render::ALIGN_CENTER );
					}

					if ( g_menu.main.players.weapon_mode.get( 1 ) ) {
						offset -= 5;

						std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[weapon->m_iItemDefinitionIndex( )] );
						render::cs.string( bbox.x + bbox.w / 2, bbox.y + bbox.h + offset, !player->dormant( ) ? Color( 255, 255, 255, int( m_alpha[i] ) ) : Color( 141, 141, 141, int( m_alpha[i] ) ), icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}

	/*  flags  */
	if ( player->alive( ) && !player->dormant( ) ) {
		std::vector< std::pair< std::string, Color > > flags;

		auto items = g_menu.main.players.flags_enemy.GetActiveIndices( );

		for ( auto it = items.begin( ); it != items.end( ); ++it ) {

			//AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
			//if (data != nullptr && data->m_records.front().get() != nullptr) {
			//	flags.push_back({ data->m_records.front().get()->m_resolver, { 205, 205, 205, int(m_alpha[i]) } });
			//}

			// money.
			if ( *it == 0 )
				flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 150, 200, 60, int( m_alpha[i] ) } } );

			// armor.
			if ( *it == 1 ) {
				// helmet and kevlar.
				if ( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "hk" ), { 103, 160, 224, int( m_alpha[i] ) } } );

				// only helmet.
				else if ( player->m_bHasHelmet( ) )
					flags.push_back( { XOR( "h" ), { 103, 160, 224, int( m_alpha[i] ) } } );

				// only kevlar.
				else if ( player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "k" ), { 103, 160, 224, int( m_alpha[i] ) } } );
			}

			// scoped.
			if ( *it == 2 && player->m_bIsScoped( ) )
				flags.push_back( { XOR( "zoom" ), { 224, 160, 103, int( m_alpha[i] ) } } );

			// flashed.
			if ( *it == 3 && player->m_flFlashBangTime( ) > 0.f )
				flags.push_back( { XOR( "blind" ), { 224, 103, 206, int( m_alpha[i] ) } } );

			// reload.
			if ( *it == 4 ) {
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[1];

				// check if reload animation is going on.
				if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
					flags.push_back( { XOR( "r" ), { 60, 180, 225, int( m_alpha[i] ) } } );
			}

			// bomb.
			if ( *it == 5 && player->HasC4( ) )
				flags.push_back( { XOR( "b" ), { 224, 103, 106, int( m_alpha[i] ) } } );
		}

		// iterate flags.
		for ( size_t i{ }; i < flags.size( ); ++i ) {
			// get flag job (pair).
			const auto& f = flags[i];

			int offset = i * ( render::esp_small.m_size.m_height - 1 );

			// draw flag.
			render::esp_small.string( bbox.x + bbox.w + 2, bbox.y + offset, f.second, f.first );
		}
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

	std::string bomb = m_last_bombsite.c_str( );

	// 'on screen (2D)'.
	if ( mode_2d ) {
		std::string timer1337 = tfm::format( XOR( "%s - %.1fs" ), bomb.substr( 0, 1 ), explode_time_diff );
		std::string damage1337 = tfm::format( XOR( "%i" ), final_damage );

		Color colortimer = { 135, 172, 10, 255 };
		if ( explode_time_diff < 10 ) colortimer = { 200, 200, 110, 255 };
		if ( explode_time_diff < 5 ) colortimer = { 192, 32, 17, 255 };

		if ( m_c4_planted && !bombexploded && !bombedefused ) {
			if ( explode_time_diff > 0.f ) {
				render::indicator.string( 6, 11, { 0,0, 0, 125 }, timer1337 );
				render::indicator.string( 5, 10, colortimer, timer1337 );
			}
			//Render.StringCustom(5, 0, 0, getSite(c4) + timer + "s", colortimer, font);
			if ( g_cl.m_local->m_iHealth( ) <= final_damage ) {
				render::indicator.string( 6, 31, { 0,0, 0, 125 }, tfm::format( XOR( "FATAL" ) ) );
				render::indicator.string( 5, 30, { 192, 32, 17, 255 }, tfm::format( XOR( "FATAL" ) ) );
			}
			else if ( final_damage > 1 ) {
				render::indicator.string( 5, 31, { 0,0, 0, 125 }, tfm::format( XOR( "- %iHP" ), damage1337 ) );
				render::indicator.string( 6, 30, { 255, 255, 152, 255 }, tfm::format( XOR( "- %iHP" ), damage1337 ) );
			}
		}
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

	if ( player->dormant( ) )
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

		Color clr = player->enemy( g_cl.m_local ) ? g_menu.main.players.skeleton_enemy.get( ) : g_menu.main.players.skeleton_friendly.get( );
		clr.a( ) = opacity;

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

	float blend = g_menu.main.players.glow_blend.get( ) / 100.f;

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

		if ( enemy && !g_menu.main.players.glow.get( 0 ) )
			continue;

		if ( !enemy && !g_menu.main.players.glow.get( 1 ) )
			continue;

		// enemy color
		if ( enemy )
			color = g_menu.main.players.glow_enemy.get( );

		// friendly color
		else
			color = g_menu.main.players.glow_friendly.get( );

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { (float)color.r( ) / 255.f, (float)color.g( ) / 255.f, (float)color.b( ) / 255.f };
		obj->m_alpha = opacity * blend;
	}
}

void Visuals::DrawHitboxMatrix( LagRecord* record, Color col, float time ) {
	if ( !g_menu.main.aimbot.debugaim.get( ) )
		return;
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
			g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
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

			g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, 0 );
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


	if ( !g_menu.main.visuals.impact_beams.get( ) )
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
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( XOR( "sprites/purplelaser1.vmt" ) );
				beam_info.m_pszModelName = XOR( "sprites/purplelaser1.vmt" );
				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = g_menu.main.visuals.impact_beams_time.get( );
				beam_info.m_flWidth = 2.f;
				beam_info.m_flEndWidth = 2.f;
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 0.f;   // beam 'jitter'.
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 0.5f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0;

				if ( !impact->m_hit_player ) {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get( ).b( );
				}

				else {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_hurt_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_hurt_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_hurt_color.get( ).b( );
				}

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
