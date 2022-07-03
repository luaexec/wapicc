#include "includes.h"
#include <math.h>

HVH g_hvh{ };;

void HVH::SendFakeFlick( ) {

	if ( !( cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) ) || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( g_csgo.m_globals->m_tick_count % 10 == 0 ) {
		g_cl.m_cmd->m_view_angles.y -= 113.f;
	}
	else if ( g_csgo.m_globals->m_tick_count % 10 == 9 ) {
		g_cl.m_cmd->m_view_angles.y += 113.f;
	}

}

void HVH::fake_flick( )
{
	if ( !( cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) ) || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( g_csgo.m_globals->m_tick_count % 10 == 0 ) {
		g_cl.m_cmd->m_view_angles.y += 113.f;
		g_cl.m_cmd->m_side_move = -450.f;
	}
	else if ( g_csgo.m_globals->m_tick_count % 10 == 9 ) {
		g_cl.m_cmd->m_view_angles.y -= 113.f;
		g_cl.m_cmd->m_side_move = 450.f;
	}
}

void HVH::AntiAimPitch( ) {
	bool safe = config["menu_ut"].get<bool>( );

	switch ( config["aa_pitch"].get<int>( ) ) {
		case 1:
			// down.
			g_cl.m_cmd->m_view_angles.x = safe ? 89.f : 720.f;
			break;

		case 2:
			// up.
			g_cl.m_cmd->m_view_angles.x = safe ? -89.f : -720.f;
			break;

		default:
			break;
	}
}

void HVH::AutoDirection( ) {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 35.f };

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// iterate players.
	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate player.
		if ( !g_aimbot.IsValidTarget( player ) )
			continue;

		// skip dormant players.
		if ( player->dormant( ) )
			continue;

		// get best target based on fov.
		float fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, player->WorldSpaceCenter( ) );

		if ( fov < target.fov ) {
			target.fov = fov;
			target.player = player;
		}
	}

	if ( !target.player ) {
		// we have a timeout.
		if ( m_auto_last > 0.f && m_auto_time > 0.f && g_csgo.m_globals->m_curtime < ( m_auto_last + m_auto_time ) )
			return;

		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( m_view - 180.f );
	angles.emplace_back( m_view + 90.f );
	angles.emplace_back( m_view - 90.f );

	// start the trace at the enemy shoot pos.
	vec3_t start = target.player->GetShootPosition( );

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ g_cl.m_shoot_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			g_cl.m_shoot_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			g_cl.m_shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize( );

		// should never happen.
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		for ( float i{ 0.f }; i < len; i += STEP ) {
			// get the current step position.
			vec3_t point = start + ( dir * i );

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents( point, MASK_SHOT_HULL );

			// contains nothing that can stop a bullet.
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if ( i > ( len * 0.5f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if ( i > ( len * 0.75f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if ( i > ( len * 0.9f ) )
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += ( STEP * mult );

			// mark that we found anything.
			valid = true;
		}
	}

	if ( !valid ) {
		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
			   []( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
				   return a.m_dist > b.m_dist;
			   } );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );

	// check if we are not doing a useless change.
	if ( best->m_dist != m_auto_dist ) {
		// set yaw to the best result.
		m_auto = math::NormalizedAngle( best->m_yaw );
		m_auto_dist = best->m_dist;
		m_auto_last = g_csgo.m_globals->m_curtime;
	}
}

void HVH::GetAntiAimDirection( ) {
	float  best_fov{ std::numeric_limits< float >::max( ) };
	float  best_dist{ std::numeric_limits< float >::max( ) };
	float  fov, dist;
	Player* target, * best_target{ nullptr };

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		target = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !target )
			continue;

		if ( !target->alive( ) )
			continue;

		fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, target->WorldSpaceCenter( ) );
		if ( fov < best_fov ) {
			best_fov = fov;
			best_target = target;
		}
	}

	if ( config["aa_at"].get<bool>( ) ) {
		if ( best_target ) {
			// todo - dex; calculate only the yaw needed for this (if we're not going to use the x component that is).
			ang_t angle;
			math::VectorAngles( best_target->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ), angle );
			m_view = angle.y;
		}
	}

	m_direction = m_view + 180.f;

	if ( config["aa_yaw"].get<int>( ) == 1 && m_mode == AntiAimMode::AIR )
		m_direction = m_view + 150.f + ( sin( g_csgo.m_globals->m_curtime * 3.f ) * 60.f );

	if ( config["aa_fs"].get<bool>( ) && best_target && m_mode != AntiAimMode::AIR ) {

		AutoDirection( );
		m_direction = m_auto;

	}

	m_direction += config["aa_offset"].get<int>( );

	// normalize the direction.
	math::NormalizeAngle( m_direction );
}

bool HVH::DoEdgeAntiAim( Player* player, ang_t& out ) {
	CGameTrace trace;
	static CTraceFilterSimple_game filter{ };

	if ( player->m_MoveType( ) == MOVETYPE_LADDER )
		return false;

	// skip this player in our traces.
	filter.SetPassEntity( player );

	// get player bounds.
	vec3_t mins = player->m_vecMins( );
	vec3_t maxs = player->m_vecMaxs( );

	// make player bounds bigger.
	mins.x -= 20.f;
	mins.y -= 20.f;
	maxs.x += 20.f;
	maxs.y += 20.f;

	// get player origin.
	vec3_t start = player->GetAbsOrigin( );

	// offset the view.
	start.z += 56.f;

	g_csgo.m_engine_trace->TraceRay( Ray( start, start, mins, maxs ), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace );
	if ( !trace.m_startsolid )
		return false;

	float  smallest = 1.f;
	vec3_t plane;

	// trace around us in a circle, in 20 steps (anti-degree conversion).
	// find the closest object.
	for ( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;

		// set end point based on range and step.
		end.x += std::cos( step ) * 32.f;
		end.y += std::sin( step ) * 32.f;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f } ), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace );

		// we found an object closer, then the previouly found object.
		if ( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// no valid object was found.
	if ( smallest == 1.f || plane.z >= 0.1f )
		return false;

	// invert the normal of this object
	// this will give us the direction/angle to this object.
	vec3_t inv = -plane;
	vec3_t dir = inv;
	dir.normalize( );

	// extend point into object by 24 units.
	vec3_t point = start;
	point.x += ( dir.x * 24.f );
	point.y += ( dir.y * 24.f );

	// check if we can stick our head into the wall.
	if ( g_csgo.m_engine_trace->GetPointContents( point, CONTENTS_SOLID ) & CONTENTS_SOLID ) {
		// trace from 72 units till 56 units to see if we are standing behind something.
		g_csgo.m_engine_trace->TraceRay( Ray( point + vec3_t{ 0.f, 0.f, 16.f }, point ), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace );

		// we didnt start in a solid, so we started in air.
		// and we are not in the ground.
		if ( trace.m_fraction < 1.f && !trace.m_startsolid && trace.m_plane.m_normal.z > 0.7f ) {
			// mean we are standing behind a solid object.
			// set our angle to the inversed normal of this object.
			out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );
			return true;
		}
	}

	// if we arrived here that mean we could not stick our head into the wall.
	// we can still see if we can stick our head behind/asides the wall.

	// adjust bounds for traces.
	mins = { ( dir.x * -3.f ) - 1.f, ( dir.y * -3.f ) - 1.f, -1.f };
	maxs = { ( dir.x * 3.f ) + 1.f, ( dir.y * 3.f ) + 1.f, 1.f };

	// move this point 48 units to the left 
	// relative to our wall/base point.
	vec3_t left = start;
	left.x = point.x - ( inv.y * 48.f );
	left.y = point.y - ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( left, point, mins, maxs ), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace );
	float l = trace.m_startsolid ? 0.f : trace.m_fraction;

	// move this point 48 units to the right 
	// relative to our wall/base point.
	vec3_t right = start;
	right.x = point.x + ( inv.y * 48.f );
	right.y = point.y + ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( right, point, mins, maxs ), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace );
	float r = trace.m_startsolid ? 0.f : trace.m_fraction;

	// both are solid, no edge.
	if ( l == 0.f && r == 0.f )
		return false;

	// set out to inversed normal.
	out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );

	// left started solid.
	// set angle to the left.
	if ( l == 0.f ) {
		out.y += 90.f;
		return true;
	}

	// right started solid.
	// set angle to the right.
	if ( r == 0.f ) {
		out.y -= 90.f;
		return true;
	}

	return false;
}

void HVH::DoExploitWalk( )
{
	if ( !cfg_t::get_hotkey( "aa_lwalk", "aa_lwalk_mode" ) )
		return;

	static int old_cmds = 0;
	if ( old_cmds != g_cl.m_cmd->m_command_number )
		old_cmds = g_cl.m_cmd->m_command_number;

	g_cl.m_cmd->m_command_number = old_cmds;
	g_cl.m_cmd->m_tick = INT_MAX / 2;
}

void HVH::DoRealAntiAim( ) {
	DoExploitWalk( );

	if ( g_input.GetKeyPress( config["aa_left"].get<int>( ) ) && ( m_manual == AntiAimSide::M_LEFT || m_manual != AntiAimSide::M_LEFT ) ) {
		m_manual = m_manual == AntiAimSide::M_LEFT ? AntiAimSide::M_NONE : AntiAimSide::M_LEFT;
	}

	if ( g_input.GetKeyPress( config["aa_right"].get<int>( ) ) && ( m_manual == AntiAimSide::M_RIGHT || m_manual != AntiAimSide::M_RIGHT ) ) {
		m_manual = m_manual == AntiAimSide::M_RIGHT ? AntiAimSide::M_NONE : AntiAimSide::M_RIGHT;
	}

	if ( g_input.GetKeyPress( config["aa_back"].get<int>( ) ) && ( m_manual == AntiAimSide::M_BACK || m_manual != AntiAimSide::M_BACK ) ) {
		m_manual = m_manual == AntiAimSide::M_BACK ? AntiAimSide::M_NONE : AntiAimSide::M_BACK;
	}

	// if we have a yaw antaim.
	if ( 1 ) {

		switch ( m_manual ) {
			case AntiAimSide::M_BACK:
				m_direction = m_view + 180.f;
				break;
			case AntiAimSide::M_LEFT:
				m_direction = m_view + 90.f;
				break;
			case AntiAimSide::M_RIGHT:
				m_direction = m_view - 90.f;
				break;
			default:
				break;
		}

		// if we have a yaw active, which is true if we arrived here.
		// set the yaw to the direction before applying any other operations.
		g_cl.m_cmd->m_view_angles.y = m_direction;

		bool stand = m_mode == AntiAimMode::STAND;
		bool air = m_mode == AntiAimMode::AIR;

		// todo: figure out later wat eso does with this shit.
		float goal_feet_yaw = g_cl.m_abs_yaw;
		float eye_delta = math::NormalizedAngle( goal_feet_yaw - g_cl.m_cmd->m_view_angles.y );

		// check if we will have a lby fake this tick.
		if ( !g_cl.m_lag && g_csgo.m_globals->m_curtime >= g_cl.m_body_pred && stand ) {
			// there will be an lbyt update on this tick.
			*g_cl.m_packet = true;
			if ( config["aa_fb"].get<bool>( ) && !cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) ) {

				g_cl.m_cmd->m_view_angles.y += eye_delta + config["aa_fb_ang"].get<int>( );

			}
		}

		fake_flick( );
	}

	if ( config["aa_dshift"].get<bool>( ) && m_mode == AntiAimMode::STAND && !cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) && m_manual == AntiAimSide::M_NONE ) {
		auto force_turn = config["aa_dturn"].get<bool>( );
		float distortion_delta;

		distortion_delta = sin( g_csgo.m_globals->m_curtime * 3.f ) * config["aa_dangle"].get<float>( );

		if ( force_turn )
			distortion_delta += ( 360.f * ( ( g_cl.m_body_pred - g_csgo.m_globals->m_curtime ) / 1.1f ) ) * ( abs( distortion_delta ) / distortion_delta );

		math::NormalizeAngle( distortion_delta );

		g_cl.m_cmd->m_view_angles.y += distortion_delta;
	}

	// normalize angle.
	math::NormalizeAngle( g_cl.m_cmd->m_view_angles.y );
}

void HVH::DoFakeAntiAim( ) {
	// do fake yaw operations.

	// enforce this otherwise low fps dies.
	// cuz the engine chokes or w/e
	// the fake became the real, think this fixed it.
	*g_cl.m_packet = true;

	if ( config["aa_fake"].get<bool>( ) )
		g_cl.m_cmd->m_view_angles.y = m_direction + config["aa_fake_offset"].get<float>( );

	SendFakeFlick( );

	// normalize fake angle.
	math::NormalizeAngle( g_cl.m_cmd->m_view_angles.y );
}

void HVH::AntiAim( ) {
	bool attack, attack2;

	attack = g_cl.m_cmd->m_buttons & IN_ATTACK;
	attack2 = g_cl.m_cmd->m_buttons & IN_ATTACK2;

	if ( g_cl.m_weapon && g_cl.m_weapon_fire ) {
		bool knife = g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS;
		bool revolver = g_cl.m_weapon_id == REVOLVER;

		// if we are in attack and can fire, do not anti-aim.
		if ( attack || ( attack2 && ( knife || revolver ) ) )
			return;
	}

	// disable conditions.
	if ( g_csgo.m_gamerules->m_bFreezePeriod( ) || ( g_cl.m_flags & FL_FROZEN ) || ( g_cl.m_cmd->m_buttons & IN_USE ) )
		return;

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if ( g_cl.m_weapon_type == WEAPONTYPE_GRENADE
		 && ( !g_cl.m_weapon->m_bPinPulled( ) || attack || attack2 )
		 && g_cl.m_weapon->m_fThrowTime( ) > 0.f && g_cl.m_weapon->m_fThrowTime( ) < g_csgo.m_globals->m_curtime )
		return;

	m_mode = AntiAimMode::STAND;

	if ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) )
		m_mode = AntiAimMode::AIR;

	else if ( g_cl.m_speed > 0.1f || cfg_t::get_hotkey( "aa_fflick", "aa_fflick_mode" ) )
		m_mode = AntiAimMode::WALK;

	// set pitch.
	AntiAimPitch( );
	GetAntiAimDirection( );

	if ( config["aa_fake"].get<bool>( ) ) {
		// do not allow 2 consecutive sendpacket true if faking angles.
		if ( *g_cl.m_packet && g_cl.m_old_packet )
			*g_cl.m_packet = false;

		// run the real on sendpacket false.
		if ( !*g_cl.m_packet || !*g_cl.m_final_packet )
			DoRealAntiAim( );

		// run the fake on sendpacket true.
		else DoFakeAntiAim( );
	}

	// no fake, just run real.
	else DoRealAntiAim( );
}

void HVH::SendPacket( ) {
	// if not the last packet this shit wont get sent anyway.
	// fix rest of hack by forcing to false.
	if ( !*g_cl.m_final_packet )
		*g_cl.m_packet = false;

	if ( cfg_t::get_hotkey( "misc_as", "misc_as_mode" ) ) {
		g_cl.m_cmd->m_command_number = g_cl.m_cmd->m_tick = INT_MAX;
		return;
	}

	// fake-lag enabled.
	if ( config["fl_enable"].get<bool>( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN ) ) {
		// limit of lag.
		int limit = std::min( config["fl_limit"].get<int>( ), 14 );

		// indicates wether to lag or not.
		bool active{ };

		// get current origin.
		vec3_t cur = g_cl.m_local->m_vecOrigin( );

		// get prevoius origin.
		vec3_t prev = g_cl.m_net_pos.empty( ) ? g_cl.m_local->m_vecOrigin( ) : g_cl.m_net_pos.front( ).m_pos;

		// delta between the current origin and the last sent origin.
		float delta = ( cur - prev ).length_sqr( );

		if ( g_aimbot.CanDT( ) )
			active = false;
		else {
			active = ( delta > 0.1f && g_cl.m_speed > 0.1f ) || ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) );
		}

		if ( active ) {

			if ( config["fl_mode"].get<int>( ) == 0 )
				*g_cl.m_packet = false;

			else if ( config["fl_mode"].get<int>( ) == 1 ) {
				auto speed = g_cl.m_local->m_vecVelocity( ).length( );
				if ( speed <= 200.f )
					limit = config["fl_limit"].get<float>( ) * ( speed / 200.f );

				*g_cl.m_packet = false;
			}

			else if ( config["fl_mode"].get<int>( ) == 2 ) {
				if ( g_csgo.m_globals->m_tick_count % 40 < 35 )
					*g_cl.m_packet = false;
			}

			// here comes smelly fart, probably the lowest ping wapicc user.
			if ( !( g_csgo.m_globals->m_tick_count % ( 101 - config["fl_var"].get<int>( ) ) ) && ( g_cl.m_flags & FL_ONGROUND ) )
				*g_cl.m_packet = true;

			if ( delta <= 4096.f && config["fl_lc"].get<bool>( ) && !( g_cl.m_flags & FL_ONGROUND ) )
				*g_cl.m_packet = false;

			if ( g_cl.m_lag >= limit )
				*g_cl.m_packet = true;
		}
	}

	if ( 1 ) {
		vec3_t                start = g_cl.m_local->m_vecOrigin( ), end = start, vel = g_cl.m_local->m_vecVelocity( );
		CTraceFilterWorldOnly filter;
		CGameTrace            trace;

		// gravity.
		vel.z -= ( g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval );

		// extrapolate.
		end += ( vel * g_csgo.m_globals->m_interval );

		// move down.
		end.z -= 2.f;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end ), MASK_SOLID, &filter, &trace );

		// check if landed.
		if ( trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f && !( g_cl.m_flags & FL_ONGROUND ) )
			*g_cl.m_packet = true;
	}

	// force fake-lag to 14 when fakelagging.
	if ( cfg_t::get_hotkey( "misc_walk", "misc_walk_mode" ) ) {
		*g_cl.m_packet = false;
	}

	if ( cfg_t::get_hotkey( "aa_lwalk", "aa_lwalk_mode" ) ) {
		*g_cl.m_packet = false;
	}

	// do not lag while shooting.
	if ( g_cl.m_old_shot )
		*g_cl.m_packet = true;

	// we somehow reached the maximum amount of lag.
	// we cannot lag anymore and we also cannot shoot anymore since we cant silent aim.
	if ( g_cl.m_lag >= g_cl.m_max_lag ) {
		// set bSendPacket to true.
		*g_cl.m_packet = true;

		// disable firing, since we cannot choke the last packet.
		g_cl.m_weapon_fire = false;
	}
}