#include "includes.h"
Resolver g_resolver{};;

LagRecord* Resolver::FindIdealRecord( AimPlayer* data ) {
	LagRecord* first_valid, * current;

	if ( data->m_records.empty( ) )
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for ( const auto& it : data->m_records ) {
		if ( it->dormant( ) || it->immune( ) || !it->valid( ) )
			continue;

		// get current record.
		current = it.get( );

		// first record that was valid, store it for later.
		if ( !first_valid )
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if ( it->m_shot || it->m_mode == Modes::R_UPDATE || it->m_mode == Modes::R_MOVE || it->m_mode == Modes::R_NONE )
			return current;
	}

	// none found above, return the first valid record if possible.
	return first_valid;
}

LagRecord* Resolver::FindLastRecord( AimPlayer* data ) {
	LagRecord* current;

	if ( data->m_records.empty( ) )
		return nullptr;

	// iterate records in reverse.
	for ( auto it = data->m_records.crbegin( ); it != data->m_records.crend( ); ++it ) {
		current = it->get( );

		// if this record is valid.
		// we are done since we iterated in reverse.
		if ( current->valid( ) && !current->immune( ) && !current->dormant( ) )
			return current;
	}

	return nullptr;
}

void Resolver::OnBodyUpdate( Player* player, float value ) {
	AimPlayer* data = &g_aimbot.m_players[player->index( ) - 1];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
}

float Resolver::GetAwayAngle( LagRecord* record ) {
	float  delta{ std::numeric_limits< float >::max( ) };
	vec3_t pos;
	ang_t  away;

	// other cheats predict you by their own latency.
	// they do this because, then they can put their away angle to exactly
	// where you are on the server at that moment in time.

	// the idea is that you would need to know where they 'saw' you when they created their user-command.
	// lets say you move on your client right now, this would take half of our latency to arrive at the server.
	// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

	// we have no historical origins.
	// no choice but to use the most recent one.
	//if( g_cl.m_net_pos.empty( ) ) {
	math::VectorAngles( g_cl.m_local->m_vecOrigin( ) - record->m_pred_origin, away );
	return away.y;
	//}

	// half of our rtt.
	// also known as the one-way delay.
	//float owd = ( g_cl.m_latency / 2.f );

	// since our origins are computed here on the client
	// we have to compensate for the delay between our client and the server
	// therefore the OWD should be subtracted from the target time.
	//float target = record->m_pred_time; //- owd;

	// iterate all.
	//for( const auto &net : g_cl.m_net_pos ) {
		// get the delta between this records time context
		// and the target time.
	//	float dt = std::abs( target - net.m_time );

		// the best origin.
	//	if( dt < delta ) {
	//		delta = dt;
	//		pos   = net.m_pos;
	//	}
	//}

	//math::VectorAngles( pos - record->m_pred_origin, away );
	//return away.y;
}

void Resolver::MatchShot( AimPlayer* data, LagRecord* record ) {
	// do not attempt to do this in nospread mode.
	if ( g_menu.main.config.mode.get( ) == 1 )
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon( );
	if ( weapon ) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime( ) + g_csgo.m_globals->m_interval;
	}

	// this record has a shot on it.
	if ( game::TIME_TO_TICKS( shoot_time ) == game::TIME_TO_TICKS( record->m_sim_time ) ) {
		if ( record->m_lag <= 2 )
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if ( data->m_records.size( ) >= 2 ) {
			LagRecord* previous = data->m_records[1].get( );

			if ( previous && !previous->dormant( ) )
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

void Resolver::on_player( LagRecord* record ) {

	// @doc: entity run-time.
	// @goal: envoke moving resolver past initial .22 'update'.
	float speed = record->m_anim_velocity.length( );
	if ( speed <= 0.2f )
		m_runtime[record->m_player->index( )] = g_csgo.m_globals->m_tick_count;

	auto flicked{ game::TICKS_TO_TIME( g_csgo.m_globals->m_tick_count - m_runtime[record->m_player->index( )] ) > 0.22f };

	// @doc: collect aimplayer data.
	AimPlayer* data{ &g_aimbot.m_players[record->m_player->index( ) - 1] };

	// @doc: envoke chosen resolver type.
	if ( !( record->m_player->m_fFlags( ) & FL_ONGROUND ) )
		air( data, record );
	else if ( ( speed > .1f || record->m_fake_walk ) && flicked )
		move( data, record );
	else
		standard( data, record );

	// @doc: standarise output.
	math::NormalizeAngle( record->m_eye_angles.y );

}

bool Resolver::do_update( AimPlayer* data, LagRecord* record ) {

	// @doc: player isn't idle.
	if ( g_csgo.m_globals->m_tick_count - m_runtime[record->m_player->index( )] > 0.f ) {
		data->dbg_info.push_back( tfm::format( "update-cond: runtime validity (%s)", g_csgo.m_globals->m_tick_count - m_runtime[record->m_player->index( )] ) );
		return false;
	}

	if ( data->m_body_update > record->m_anim_time )
		return false;

	// @doc: resolve player.
	record->m_eye_angles.y = record->m_body;

	// @doc: increment body-update time.
	data->m_body_update = record->m_anim_time + 1.1f;

	// @doc: output.
	record->m_dbg = m_dbg[record->m_player->index( )] = XOR( "UPD" );
	record->m_mode = Modes::R_UPDATE;

	// @doc: update flick record storage.
	data->m_update_record = record;

	return true;
}

bool Resolver::edge( AimPlayer* data, LagRecord* record, float extension, float& angle ) {

	if ( !config["acc_correct_edge"].get<bool>( ) )
		return false;

	auto angles{
		ang_t( 0.f, record->m_eye_angles.y, record->m_eye_angles.z )
	};

	vec3_t shoot_pos;
	math::AngleVectors( angles, &shoot_pos );

	vec3_t ext{ record->m_velocity };
	auto e_pos{ math::extrapolate_pos( shoot_pos, ext, 20 * extension ) };

	auto left_pos{ math::extend_vector( e_pos, 60 * extension, angles.y - 30.f ) },
		right_pos{ math::extend_vector( e_pos, 60 * extension, angles.y + 30.f ) };

	auto left_start{ math::extend_vector( e_pos, 30 * extension, angles.y - 90.f ) },
		right_start{ math::extend_vector( e_pos, 30 * extension, angles.y + 90.f ) };

	CTraceFilterSimple filter;
	filter.m_pass_ent1 = g_cl.m_local;

	CGameTrace left;
	g_csgo.m_engine_trace->TraceRay( Ray( left_pos, right_pos ), MASK_ALL, &filter, &left );

	CGameTrace right;
	g_csgo.m_engine_trace->TraceRay( Ray( right_pos, left_pos ), MASK_ALL, &filter, &right );

	CGameTrace lpiw;
	g_csgo.m_engine_trace->TraceRay( Ray( e_pos, left_start ), MASK_ALL, &filter, &lpiw );
	auto left_point_in_wall{ lpiw.m_fraction != 1.f };

	CGameTrace rpiw;
	g_csgo.m_engine_trace->TraceRay( Ray( e_pos, right_start ), MASK_ALL, &filter, &rpiw );
	auto right_point_in_wall{ rpiw.m_fraction != 1.f };

	CGameTrace liw;
	g_csgo.m_engine_trace->TraceRay( Ray( left_start, left_pos ), MASK_ALL, &filter, &liw );
	auto left_in_wall{ liw.m_fraction != 1.f ? true : left_point_in_wall };

	CGameTrace riw;
	g_csgo.m_engine_trace->TraceRay( Ray( right_start, right_pos ), MASK_ALL, &filter, &riw );
	auto right_in_wall{ riw.m_fraction != 1.f ? true : right_point_in_wall };

	if ( right_in_wall || left_in_wall ) {
		auto left_fr{ left_in_wall ? 0.f : left.m_fraction };
		auto right_fr{ right_in_wall ? 0.f : right.m_fraction };
		auto final_ang{ 180.f - ( right_fr - left_fr ) * 180.f };

		angle = final_ang;

		return true;
	}

	data->dbg_info.push_back( "edge: invalid" );
	return false;
}

void Resolver::freestand( AimPlayer* data, LagRecord* record, float extension ) {

	enum e_freestand : int {
		E_BACK = 180,
		E_LEFT = 90,
		E_RIGHT = -90,
		E_FRONT = 0,
	};

	auto fs_angle{ e_freestand::E_BACK };

	auto left_pos{ math::extrapolate_pos( record->m_origin, vec3_t( 300, 0, 0 ), 20 * extension ) },
		right_pos{ math::extrapolate_pos( record->m_origin, vec3_t( -300, 0, 0 ), 20 * extension ) },
		back_pos{ math::extrapolate_pos( record->m_origin, vec3_t( 0, 0, 0 ), 20 * extension ) };

	CTraceFilterSimple filter;
	filter.m_pass_ent1 = g_cl.m_local;

	CGameTrace back;
	g_csgo.m_engine_trace->TraceRay( Ray( g_cl.m_shoot_pos, back_pos ), MASK_ALL, &filter, &back );

	CGameTrace left;
	g_csgo.m_engine_trace->TraceRay( Ray( g_cl.m_shoot_pos, left_pos ), MASK_ALL, &filter, &left );

	CGameTrace right;
	g_csgo.m_engine_trace->TraceRay( Ray( g_cl.m_shoot_pos, right_pos ), MASK_ALL, &filter, &right );

	// @doc: logically solve.
	if ( back.m_fraction > right.m_fraction && back.m_fraction > left.m_fraction )
	{ // @doc: tagged a player, exit.
		fs_angle = e_freestand::E_BACK;
	}
	else if (
		left.hit( ) /* @note: hit entity. */
		|| left.m_fraction > right.m_fraction /* @note: greater fraction. */
		) {
		// @doc: right.
		fs_angle = e_freestand::E_RIGHT;
	}
	else if (
		right.hit( ) /* @note: hit entity. */
		|| right.m_fraction > left.m_fraction /* @note: greater fraction. */
		) {
		// @doc: left.
		fs_angle = e_freestand::E_LEFT;
	}

	// @doc: correct yaw.
	record->m_eye_angles.y = GetAwayAngle( record ) + fs_angle;

	// @doc: update.
	record->m_mode = Modes::R_FREESTAND;
	record->m_dbg = m_dbg[data->m_player->index( )] = tfm::format( "FS[%s]", fs_angle );

}

void Resolver::snap( AimPlayer* data, LagRecord* record, float angle, std::string prefix ) {

	// @doc: prerequisites.
	auto base{ GetAwayAngle( record ) }, reversed{ angle };

	// @doc: base angle offsets.
	enum e_snapyaw : int {
		E_BACK = 180,
		E_LEFT = 90,
		E_RIGHT = -90,
		E_FRONT = 0,
	};

	// @doc: available angles.
	auto angles{ std::vector<float>{
			base + e_snapyaw::E_BACK,
			base + e_snapyaw::E_LEFT,
			base + e_snapyaw::E_RIGHT,
			base + e_snapyaw::E_FRONT,
	} };

	// @doc: best angle methods.
	struct best_angle {
		float yaw;
		float diff;

		bool update( float ang ) {
			return abs( ang ) < abs( diff );
		}

		void set( float a, float d ) {
			yaw = a; diff = d;
		}

		best_angle( float y = 180.f, float d = 180.f ) {
			yaw = y; diff = d;
		}
	};

	// @doc: iterate and locate closest angle.
	best_angle best{ angles.front( ) };
	for ( auto& a : angles ) {
		auto difference{ reversed - a };

		if ( best.update( difference ) )
			best.set( a, difference );
	}

	// @doc: correct yaw.
	record->m_eye_angles.y = best.yaw;

	// @doc: update resolver information.
	record->m_dbg = m_dbg[data->m_player->index( )] = tfm::format( "%s%s[%s/%s]", "SNAP-", prefix, (int)best.yaw, (int)best.diff );
	record->m_mode = Modes::R_SNAP;

}

bool Resolver::flick( AimPlayer* data, LagRecord* record ) {

	if ( !config["acc_correct_rot"].get<bool>( ) )
		return false;

	if ( !data->m_update_record ) {
		data->dbg_info.push_back( "flick-yaw: empty upd" );
		return false;
	}

	if ( abs( record->m_sim_time - data->m_update_record->m_sim_time ) >= .22f ) { // @note: previously .22f.
		data->dbg_info.push_back( tfm::format( "flick-yaw: decay(%s)", abs( record->m_sim_time - data->m_update_record->m_sim_time ) ) );
		return false;
	}

	// @doc: lby delta.
	auto delta{ record->m_body - data->m_body };

	// @doc: rotated.
	auto yaw{ data->m_update_record->m_body - delta };

	// @doc: adjust.
	auto adj{ data->m_player->m_AnimOverlay( )[3].m_weight * ( delta * 2.f ) };
	yaw += adj;

	// @doc: correct yaw.
	record->m_eye_angles.y = yaw;

	// @doc: output.
	record->m_dbg = m_dbg[record->m_player->index( )] = tfm::format( "ROT[%s|%s]", (int)delta, (int)adj );
	record->m_mode = Modes::R_ROT;

	return true;

}

void Resolver::best_angle( LagRecord* record ) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPosition( );
	float away = GetAwayAngle( record );
	float best_dist = 0.f;

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( away - 180.f );
	angles.emplace_back( away + 90.f );
	angles.emplace_back( away - 90.f );

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPosition( );

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemypos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemypos.z };

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
		return;
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
			   []( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
				   return a.m_dist > b.m_dist;
			   } );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );

	// ...
	if ( best_dist != best->m_dist ) {
		best_dist = best->m_dist;
		record->m_eye_angles.y = best->m_yaw;
	}
}

Resolver::Directions Resolver::get_direction( AimPlayer* data ) {
	CGameTrace tr;
	CTraceFilterSimple filter{ };

	if ( !g_cl.m_processing )
		return Directions::YAW_NONE;

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// get best target based on fov.
	auto origin = data->m_player->m_vecOrigin( );
	ang_t view;
	float fov = math::GetFOV( g_cl.m_cmd->m_view_angles, g_cl.m_local->GetShootPosition( ), data->m_player->WorldSpaceCenter( ) );

	// set best fov.
	if ( fov < target.fov ) {
		target.fov = fov;
		target.player = data->m_player;
	}

	// get best player.
	const auto player = target.player;
	if ( !player )
		return Directions::YAW_NONE;

	auto& bestOrigin = player->m_vecOrigin( );

	// skip this player in our traces.
	filter.SetPassEntity( g_cl.m_local );

	// calculate angle direction from thier best origin to our origin
	ang_t angDirectionAngle;
	math::VectorAngles( g_cl.m_local->m_vecOrigin( ) - bestOrigin, angDirectionAngle );

	vec3_t forward, right, up;
	math::AngleVectors( angDirectionAngle, &forward, &right, &up );

	auto vecStart = g_cl.m_local->GetShootPosition( );
	auto vecEnd = vecStart + forward * 100.0f;

	Ray rightRay( vecStart + right * 35.0f, vecEnd + right * 35.0f ), leftRay( vecStart - right * 35.0f, vecEnd - right * 35.0f );

	g_csgo.m_engine_trace->TraceRay( rightRay, MASK_SOLID, &filter, &tr );
	float rightLength = ( tr.m_endpos - tr.m_startpos ).length( );

	g_csgo.m_engine_trace->TraceRay( leftRay, MASK_SOLID, &filter, &tr );
	float leftLength = ( tr.m_endpos - tr.m_startpos ).length( );

	static auto leftTicks = 0;
	static auto rightTicks = 0;
	static auto backTicks = 0;

	if ( rightLength - leftLength > 20.0f )
		leftTicks++;
	else
		leftTicks = 0;

	if ( leftLength - rightLength > 20.0f )
		rightTicks++;
	else
		rightTicks = 0;

	if ( fabs( rightLength - leftLength ) <= 20.0f )
		backTicks++;
	else
		backTicks = 0;

	Directions direction = Directions::YAW_NONE;

	if ( rightTicks > 10 ) {
		direction = Directions::YAW_RIGHT;
	}
	else {
		if ( leftTicks > 10 ) {
			direction = Directions::YAW_LEFT;
		}
		else {
			if ( backTicks > 10 )
				direction = Directions::YAW_BACK;
		}
	}

	return direction;
}

void Resolver::standard( AimPlayer* data, LagRecord* record ) {

	// @doc: lby-update.
	if ( do_update( data, record ) )
		return;

	// @doc: flick yaw.
	if ( flick( data, record ) )
		return;

	float edge_yaw{ };
	if ( data->m_missed_shots <= 1 && edge( data, record, .5f, edge_yaw ) )
		return snap( data, record, edge_yaw, "EDGE" );

	// @doc: standard.
	switch ( data->m_missed_shots % 4 ) {
		case 0:
			{
				auto direction{ get_direction( data ) };
				if ( direction == Directions::YAW_LEFT )
					record->m_eye_angles.y = GetAwayAngle( record ) + 90.f;
				else if ( direction == Directions::YAW_RIGHT )
					record->m_eye_angles.y = GetAwayAngle( record ) - 90.f;
				else
					best_angle( record );

				record->m_mode = Modes::R_FREESTAND;
				record->m_dbg = tfm::format( "LG[%s]", direction );
			}
			break;
		case 1:
			{
				freestand( data, record, .75f );
			}
			break;
		case 2:
			{
				if ( (bool)record->m_layers[3].m_weight && ( record->m_layers[3].m_weight_delta_rate != 0.f ) ) {
					snap( data, record, record->m_body * -1.f, "SHF" );
					record->m_eye_angles.y += ( record->m_layers[3].m_weight_delta_rate ) * 180.f;
				}
				else
					snap( data, record, record->m_eye_angles.y * -1.f, "YAW" );
			}
			break;
		case 3:
			{
				record->m_eye_angles.y = GetAwayAngle( record ) + 180.f;
				record->m_mode = Modes::R_FREESTAND;
				record->m_dbg = XOR( "BACK" );
			}
			break;

		default:
			break;
	}

	// @doc: normalise output.
	math::NormalizeAngle( record->m_eye_angles.y );
}

void Resolver::move( AimPlayer* data, LagRecord* record ) {

	// @doc: correct yaw.
	record->m_eye_angles.y = record->m_body;

	// @doc: increment body-update.
	data->m_body_update = record->m_anim_time + 0.22f;

	// @doc: miscellaneous data assignment.
	data->m_body_index = 0;
	data->m_missed_shots = 0;

	// @doc: info.
	record->m_mode = Modes::R_MOVE;
	record->m_dbg = m_dbg[data->m_player->index( )] = XOR( "WALK" );

	if ( config["acc_correct_lm"].get<bool>( ) )
		data->m_update_record = record;

}

void Resolver::air( AimPlayer* data, LagRecord* record ) {

	// @doc: rotate yaw.
	record->m_eye_angles.y = GetAwayAngle( record ) + 180.f;

	// @doc: info.
	record->m_mode = Modes::R_AIR;
	record->m_dbg = m_dbg[data->m_player->index( )] = XOR( "FLY" );

}