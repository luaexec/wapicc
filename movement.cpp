#include "includes.h"

Movement g_movement{ };;

void Movement::JumpRelated( ) {
	static const auto sv_autobunnyhopping = g_csgo.m_cvar->FindVar( HASH( "sv_autobunnyhopping" ) );
	static const auto sv_enablebunnyhopping = g_csgo.sv_enablebunnyhopping;

	if ( !sv_autobunnyhopping || !sv_enablebunnyhopping )
		return;

	if ( sv_autobunnyhopping->GetInt( ) == 1 && sv_enablebunnyhopping->GetInt( ) == 1 )
		return;

	if ( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP )
		return;

	if ( ( g_cl.m_cmd->m_buttons & IN_JUMP ) && !( g_cl.m_flags & FL_ONGROUND ) ) {
		if ( config["misc_bhop"].get<bool>( ) )
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;
	}
}

void Movement::Strafe( ) {
	if ( !config["misc_strafe"].get<bool>( ) )
		return;

	if ( ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) || ( g_cl.m_cmd->m_buttons & IN_SPEED ) && !( g_cl.m_cmd->m_buttons & IN_JUMP ) )
		return;

	if ( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	auto velocity = g_cl.m_local->m_vecVelocity( );
	velocity.z = 0.0f;

	m_speed = velocity.length_2d( );

	float tmpsmooth = config["misc_strafe_smooth"].get<float>( ) / 100.f;
	m_ideal = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( ( 15.f * tmpsmooth ) / m_speed ) ) : 90.f;

	math::clamp( m_ideal, 0.0f, 90.f );

	m_switch_value *= -1.f;

	++m_strafe_index;

	if ( ( g_cl.m_cmd->m_forward_move != 0.0f || g_cl.m_cmd->m_side_move != 0.0f ) ) {
		float m_wish{ };
		bool back = g_cl.m_cmd->m_buttons & IN_BACK;
		bool forward = g_cl.m_cmd->m_buttons & IN_FORWARD;
		bool left = g_cl.m_cmd->m_buttons & IN_MOVELEFT;
		bool right = g_cl.m_cmd->m_buttons & IN_MOVERIGHT;

		if ( forward ) {
			if ( left ) {
				m_wish += ( m_directions::left / 2 );
			}
			else if ( right ) {
				m_wish += ( m_directions::right / 2 );
			}
			else {
				m_wish += m_directions::forwards;
			}
		}
		else if ( back ) {
			if ( left ) {
				m_wish += m_directions::back_left;
			}
			else if ( right ) {
				m_wish += m_directions::back_right;
			}
			else {
				m_wish += m_directions::backwards;
			}

			g_cl.m_cmd->m_forward_move = 0.0f;
		}
		else if ( left ) {
			m_wish += m_directions::left;
		}
		else if ( right ) {
			m_wish += m_directions::right;
		}

		g_cl.m_strafe_angles.y += std::remainderf( m_wish, 360.f );
		g_cl.m_cmd->m_side_move = 0.0f;
	}

	g_cl.m_cmd->m_forward_move = 0.0f;

	auto yaw_delta = std::remainderf( g_cl.m_strafe_angles.y - m_old_yaw, 360.0f );
	auto abs_angle_delta = abs( yaw_delta );
	m_old_yaw = g_cl.m_strafe_angles.y;

	if ( abs_angle_delta <= m_ideal || abs_angle_delta >= 30.0f ) {
		auto velocity_direction = math::rad_to_deg( std::atan2( velocity.y, velocity.x ) );
		auto velocity_delta = std::remainderf( g_cl.m_strafe_angles.y - velocity_direction, 360.0f );

		if ( velocity_delta <= m_ideal || m_speed <= 15.0f ) {
			if ( -( m_ideal ) <= velocity_delta || m_speed <= 15.0f ) {
				g_cl.m_strafe_angles.y += m_switch_value * m_ideal;
				g_cl.m_cmd->m_side_move = 450 * m_switch_value;
			}
			else {
				g_cl.m_strafe_angles.y = velocity_direction - m_ideal;
				g_cl.m_cmd->m_side_move = 450;
			}
		}
		else {
			g_cl.m_strafe_angles.y = velocity_direction + m_ideal;
			g_cl.m_cmd->m_side_move = -450;
		}
	}
	else if ( yaw_delta > 0.0f ) {
		g_cl.m_cmd->m_side_move = -450;
	}
	else if ( yaw_delta < 0.0f ) {
		g_cl.m_cmd->m_side_move = 450;
	}

	g_cl.m_strafe_angles.normalize( );
}

void Movement::DoPrespeed( ) {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	// min and max values are based on 128 ticks.
	mod = g_csgo.m_globals->m_interval * 128.f;

	// scale min and max based on tickrate.
	min = 2.25f * mod;
	max = 5.f * mod;

	// compute ideal strafe angle for moving in a circle.
	strafe = m_ideal * 2.f;

	// clamp ideal strafe circle value to min and max step.
	math::clamp( strafe, min, max );

	// calculate time.
	time = 320.f / m_speed;

	// clamp time.
	math::clamp( time, 0.35f, 1.f );

	// init step.
	step = strafe;

	while ( true ) {
		// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
		if ( !WillCollide( time, step ) || max <= step )
			break;

		// if we will collide with an object with the current strafe step then increment step to prevent a collision.
		step += 0.2f;
	}

	if ( step > max ) {
		// reset step.
		step = strafe;

		while ( true ) {
			// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
			if ( !WillCollide( time, step ) || step <= -min )
				break;

			// if we will collide with an object with the current strafe step decrement step to prevent a collision.
			step -= 0.2f;
		}

		if ( step < -min ) {
			if ( GetClosestPlane( plane ) ) {
				// grab the closest object normal
				// compute the angle of the normal
				// and push us away from the object.
				angle = math::rad_to_deg( std::atan2( plane.y, plane.x ) );
				step = -math::NormalizedAngle( m_circle_yaw - angle ) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	// add the computed step to the steps of the previous circle iterations.
	m_circle_yaw = math::NormalizedAngle( m_circle_yaw + step );

	// apply data to usercmd.
	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_side_move = ( step >= 0.f ) ? -450.f : 450.f;
}

bool Movement::GetClosestPlane( vec3_t& plane ) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	// trace around us in a circle
	for ( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;
		end.x += std::cos( step ) * dist;
		end.y += std::sin( step ) * dist;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, m_mins, m_maxs ), CONTENTS_SOLID, &filter, &trace );

		// we found an object closer, then the previouly found object.
		if ( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// did we find any valid object?
	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide( float time, float change ) {
	struct PredictionData_t {
		vec3_t start;
		vec3_t end;
		vec3_t velocity;
		float  direction;
		bool   ground;
		float  predicted;
	};

	PredictionData_t      data;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// set base data.
	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity( );
	data.direction = math::rad_to_deg( std::atan2( data.velocity.y, data.velocity.x ) );

	for ( data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval ) {
		// predict movement direction by adding the direction change.
		// make sure to normalize it, in case we go over the -180/180 turning point.
		data.direction = math::NormalizedAngle( data.direction + change );

		// pythagoras.
		float hyp = data.velocity.length_2d( );

		// adjust velocity for new direction.
		data.velocity.x = std::cos( math::deg_to_rad( data.direction ) ) * hyp;
		data.velocity.y = std::sin( math::deg_to_rad( data.direction ) ) * hyp;

		// assume we bhop, set upwards impulse.
		if ( data.ground )
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat( );

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;

		// we adjusted the velocity for our new direction.
		// see if we can move in this direction, predict our new origin if we were to travel at this velocity.
		data.end += ( data.velocity * g_csgo.m_globals->m_interval );

		// trace
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// check if we hit any objects.
		if ( trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f )
			return true;
		if ( trace.m_startsolid || trace.m_allsolid )
			return true;

		// adjust start and end point.
		data.start = data.end = trace.m_endpos;

		// move endpoint 2 units down, and re-trace.
		// do this to check if we are on th floor.
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// see if we moved the player into the ground for the next iteration.
		data.ground = trace.hit( ) && trace.m_plane.m_normal.z > 0.7f;
	}

	// the entire loop has ran
	// we did not hit shit.
	return false;
}

void Movement::FixMove( CUserCmd* cmd, ang_t wish_angles ) {
	vec3_t view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	math::AngleVectors( wish_angles, &view_fwd, &view_right, &view_up );
	math::AngleVectors( cmd->m_view_angles, &cmd_fwd, &cmd_right, &cmd_up );

	const auto v8 = sqrtf( ( view_fwd.x * view_fwd.x ) + ( view_fwd.y * view_fwd.y ) );
	const auto v10 = sqrtf( ( view_right.x * view_right.x ) + ( view_right.y * view_right.y ) );
	const auto v12 = sqrtf( view_up.z * view_up.z );

	const vec3_t norm_view_fwd( ( 1.f / v8 ) * view_fwd.x, ( 1.f / v8 ) * view_fwd.y, 0.f );
	const vec3_t norm_view_right( ( 1.f / v10 ) * view_right.x, ( 1.f / v10 ) * view_right.y, 0.f );
	const vec3_t norm_view_up( 0.f, 0.f, ( 1.f / v12 ) * view_up.z );

	const auto v14 = sqrtf( ( cmd_fwd.x * cmd_fwd.x ) + ( cmd_fwd.y * cmd_fwd.y ) );
	const auto v16 = sqrtf( ( cmd_right.x * cmd_right.x ) + ( cmd_right.y * cmd_right.y ) );
	const auto v18 = sqrtf( cmd_up.z * cmd_up.z );

	const vec3_t norm_cmd_fwd( ( 1.f / v14 ) * cmd_fwd.x, ( 1.f / v14 ) * cmd_fwd.y, 0.f );
	const vec3_t norm_cmd_right( ( 1.f / v16 ) * cmd_right.x, ( 1.f / v16 ) * cmd_right.y, 0.f );
	const vec3_t norm_cmd_up( 0.f, 0.f, ( 1.f / v18 ) * cmd_up.z );

	const auto v22 = norm_view_fwd.x * cmd->m_forward_move;
	const auto v26 = norm_view_fwd.y * cmd->m_forward_move;
	const auto v28 = norm_view_fwd.z * cmd->m_forward_move;
	const auto v24 = norm_view_right.x * cmd->m_side_move;
	const auto v23 = norm_view_right.y * cmd->m_side_move;
	const auto v25 = norm_view_right.z * cmd->m_side_move;
	const auto v30 = norm_view_up.x * cmd->m_up_move;
	const auto v27 = norm_view_up.z * cmd->m_up_move;
	const auto v29 = norm_view_up.y * cmd->m_up_move;

	cmd->m_forward_move = ( ( ( ( norm_cmd_fwd.x * v24 ) + ( norm_cmd_fwd.y * v23 ) ) + ( norm_cmd_fwd.z * v25 ) )
							+ ( ( ( norm_cmd_fwd.x * v22 ) + ( norm_cmd_fwd.y * v26 ) ) + ( norm_cmd_fwd.z * v28 ) ) )
		+ ( ( ( norm_cmd_fwd.y * v30 ) + ( norm_cmd_fwd.x * v29 ) ) + ( norm_cmd_fwd.z * v27 ) );
	cmd->m_side_move = ( ( ( ( norm_cmd_right.x * v24 ) + ( norm_cmd_right.y * v23 ) ) + ( norm_cmd_right.z * v25 ) )
						 + ( ( ( norm_cmd_right.x * v22 ) + ( norm_cmd_right.y * v26 ) ) + ( norm_cmd_right.z * v28 ) ) )
		+ ( ( ( norm_cmd_right.x * v29 ) + ( norm_cmd_right.y * v30 ) ) + ( norm_cmd_right.z * v27 ) );
	cmd->m_up_move = ( ( ( ( norm_cmd_up.x * v23 ) + ( norm_cmd_up.y * v24 ) ) + ( norm_cmd_up.z * v25 ) )
					   + ( ( ( norm_cmd_up.x * v26 ) + ( norm_cmd_up.y * v22 ) ) + ( norm_cmd_up.z * v28 ) ) )
		+ ( ( ( norm_cmd_up.x * v30 ) + ( norm_cmd_up.y * v29 ) ) + ( norm_cmd_up.z * v27 ) );

	wish_angles = cmd->m_view_angles;

	if ( g_cl.m_local->m_MoveType( ) != MOVETYPE_LADDER && g_cl.m_local->m_MoveType( ) != MOVETYPE_NOCLIP )
		cmd->m_buttons &= ~( IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT );
}

void Movement::AutoPeek( ) {
	if ( g_aimbot.m_stop ) {
		switch ( config["rage_stop"].get<int>( ) ) {
			case 0:
				{
					if ( g_cl.CanFireWeapon( ) )
						Movement::QuickStop( );
					break;
				}
			case 1:
				{
					Movement::QuickStop( );
					break;
				}
			case 2:
				{
					Movement::FakeWalk( true );
					break;
				}
			default:
				break;
		}
	}
}

void Movement::QuickStop( ) {

	auto weapon = g_cl.m_local->GetActiveWeapon( );
	float max_speed = weapon->m_zoomLevel( ) > 0 ? weapon->GetWpnData( )->m_max_player_speed_alt : weapon->GetWpnData( )->m_max_player_speed;
	float speed = g_cl.m_local->m_vecVelocity( ).length( );

	if ( speed <= max_speed / 3.f )
		return;

	ang_t dir;
	math::VectorAngles( g_cl.m_local->m_vecVelocity( ), dir );
	dir.y = g_cl.m_view_angles.y - dir.y;
	vec3_t forward;
	math::AngleVectors( dir, &forward );
	vec3_t new_dir = forward * -speed;
	g_cl.m_cmd->m_forward_move = new_dir.x;
	g_cl.m_cmd->m_side_move = new_dir.y;
	return;
}

void Movement::FakeWalk( bool force ) {
	vec3_t velocity{ g_cl.m_local->m_vecVelocity( ) };
	int    ticks{ };

	if ( !cfg_t::get_hotkey( "misc_walk", "misc_walk_mode" ) && !force )
		return;

	if ( !g_cl.m_local->GetGroundEntity( ) )
		return;

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat( ) * g_cl.m_local->m_surfaceFriction( );

	for ( ; ticks < g_cl.m_max_lag; ++ticks ) {
		// calculate speed.
		float speed = velocity.length( );

		// if too slow return.
		if ( speed <= 0.1f )
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max( speed, g_csgo.sv_stopspeed->GetFloat( ) );

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max( 0.f, speed - drop );

		if ( newspeed != speed ) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	const int next_update_tick = g_cl.GetNextUpdate( ) - 1;
	const int max = std::min<int>( 16, next_update_tick );
	const int choked = g_csgo.m_cl->m_choked_commands;
	int ticks_to_pred = max - choked;

	// zero forwardmove and sidemove.
	if ( ticks > ticks_to_pred || !g_csgo.m_cl->m_choked_commands ) {
		g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.f;
	}
}