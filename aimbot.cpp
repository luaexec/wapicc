#include "includes.h"
#include "tickbase.h"

Aimbot g_aimbot{ };;

bool Aimbot::CanDT( ) {
	int idx = g_cl.m_weapon->m_iItemDefinitionIndex( );
	return g_cl.m_local->alive( )
		&& m_double_tap;
}

void Aimbot::DoubleTap( )
{
	static bool did_shift_before = false;
	static bool reset = true;
	static int clock = 0;
	auto ticks_to_shift = g_menu.main.aimbot.shift_amt.get( );
	g_cl.can_dt_shoot = false;

	if ( CanDT( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) )
	{
		if ( m_double_tap )
		{
			if ( reset )
			{
				if ( g_cl.m_cmd->m_buttons & IN_ATTACK )
					clock = 0;

				clock++;

				if ( clock >= 40 ) // recharge delay
				{
					g_cl.m_tick_to_recharge = ticks_to_shift;
					g_cl.can_recharge = true;
					g_cl.m_tick_to_shift = 0;
					clock = 0;
					reset = false;
				}
			}
			else
			{
				if ( g_cl.m_charged )
				{
					g_cl.can_dt_shoot = true;

					if ( g_cl.m_cmd->m_buttons & IN_ATTACK )
					{
						g_cl.m_visual_shift_ticks = 0;
						*g_cl.m_packet = true;
						g_cl.m_tick_to_shift = ticks_to_shift;
						reset = true;
						g_cl.m_charged = false;
						if ( g_menu.main.aimbot.dt_mode.get( ) == 0 ) {
							g_cl.m_cmd->m_side_move = 0; // this is to slow movement to increase accuracy
							g_cl.m_cmd->m_forward_move = 0;
						}
					}
				}
			}
		}
	}
}

void AimPlayer::UpdateAnimations( LagRecord* record ) {
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState( );
	if ( !state )
		return;

	// player respawned.
	if ( m_player->m_flSpawnTime( ) != m_spawn ) {
		// reset animation state.
		game::ResetAnimationState( state );

		// note new spawn time.
		m_spawn = m_player->m_flSpawnTime( );
	}

	// backup curtime.
	float curtime = g_csgo.m_globals->m_curtime;
	float frametime = g_csgo.m_globals->m_frametime;

	// set curtime to animtime.
	// set frametime to ipt just like on the server during simulation.
	g_csgo.m_globals->m_curtime = record->m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// backup stuff that we do not want to fuck with.
	AnimationBackup_t backup;

	backup.m_origin = m_player->m_vecOrigin( );
	backup.m_abs_origin = m_player->GetAbsOrigin( );
	backup.m_velocity = m_player->m_vecVelocity( );
	backup.m_abs_velocity = m_player->m_vecAbsVelocity( );
	backup.m_flags = m_player->m_fFlags( );
	backup.m_eflags = m_player->m_iEFlags( );
	backup.m_duck = m_player->m_flDuckAmount( );
	backup.m_body = m_player->m_flLowerBodyYawTarget( );
	m_player->GetAnimLayers( backup.m_layers );

	// is player a bot?
	bool bot = game::IsFakePlayer( m_player->index( ) );

	// reset fakewalk & fakeflick state.
	record->m_fake_walk = false;
	record->m_fake_flick = false;
	record->m_mode = Resolver::Modes::R_NONE;

	// fix velocity.
	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
	if ( record->m_lag > 0 && m_records.size( ) >= 2 ) {
		// get pointer to previous record.
		LagRecord* previous = m_records[1].get( );

		if ( previous && !previous->dormant( ) ) {
			// get the delta in time.
			record->m_choke_time = record->m_sim_time - previous->m_sim_time;

			// how many ticks did he choke?
			const int lag_ticks = game::TIME_TO_TICKS( record->m_choke_time );

			// override this nigga.
			record->m_lag = lag_ticks;

			// is this nigga out of lag compensation.
			if ( ( lag_ticks - 1 ) > 19 || previous->m_sim_time == 0.f ) {
				record->m_choke_time = g_csgo.m_globals->m_interval;
				record->m_lag = 1;
			}

			// this nigga isnt choking.
			if ( record->m_lag < 1 ) {
				record->m_choke_time = g_csgo.m_globals->m_interval;
				record->m_lag = 1;
			}

			// setup default velocity.
			record->m_velocity = ( record->m_origin - previous->m_origin ) * ( 1.f / record->m_choke_time );
		}
	}

	// set this fucker, it will get overriden.
	record->m_anim_velocity = record->m_velocity;

	// fix various issues with the game eW91dHViZS5jb20vZHlsYW5ob29r
	// these issues can only occur when a player is choking data.
	if ( record->m_lag > 1 && !bot ) {
		// detect fakewalk.
		float speed = record->m_velocity.length( );

		if ( record->m_velocity.length( ) > 0.1f
			 && record->m_layers[6].m_weight == 0.0f
			 && record->m_layers[12].m_weight == 0.0f
			 && record->m_layers[6].m_playback_rate < 0.0001f
			 && ( record->m_flags & FL_ONGROUND ) )
			record->m_fake_walk = true;

		if ( record->m_fake_walk )
			record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };

		// detect fakeflicking players
		if ( record->m_velocity.length( ) < 18.f
			 && record->m_layers[6].m_weight != 1.0f
			 && record->m_layers[6].m_weight != 0.0f
			 && record->m_layers[6].m_weight != m_records.front( ).get( )->m_layers[6].m_weight
			 && ( record->m_flags & FL_ONGROUND ) )
			record->m_fake_flick = true;

		// we need atleast 2 updates/records
		// to fix these issues.
		if ( m_records.size( ) >= 2 ) {
			// get pointer to previous record.
			LagRecord* previous = m_records[1].get( );

			if ( previous && !previous->dormant( ) ) {

				if ( record->m_flags & FL_ONGROUND )
					++air_tick;

				// set previous flags.
				m_player->m_fFlags( ) = previous->m_flags;

				// strip the on ground flag.
				m_player->m_fFlags( ) &= ~FL_ONGROUND;

				// been onground for 2 consecutive ticks? fuck yeah.
				if ( record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND )
					m_player->m_fFlags( ) |= FL_ONGROUND;

				// fix jump_fall.
				if ( record->m_layers[4].m_weight != 1.f && previous->m_layers[4].m_weight == 1.f && record->m_layers[5].m_weight != 0.f )
					m_player->m_fFlags( ) |= FL_ONGROUND;

				if ( record->m_flags & FL_ONGROUND && !( previous->m_flags & FL_ONGROUND ) )
					m_player->m_fFlags( ) &= ~FL_ONGROUND;

				// fix crouching players.
				// the duck amount we receive when people choke is of the last simulation.
				// if a player chokes packets the issue here is that we will always receive the last duckamount.
				// but we need the one that was animated.
				// therefore we need to compute what the duckamount was at animtime.

				// delta in duckamt and delta in time..
				float duck = record->m_duck - previous->m_duck;
				float time = record->m_sim_time - previous->m_sim_time;

				// get the duckamt change per tick.
				float change = ( duck / time ) * g_csgo.m_globals->m_interval;

				// fix crouching players.
				m_player->m_flDuckAmount( ) = previous->m_duck + change;

				if ( !record->m_fake_walk ) {
					// fix the velocity till the moment of animation.
					vec3_t velo = record->m_velocity - previous->m_velocity;

					// accel per tick.
					vec3_t accel = ( velo / time ) * g_csgo.m_globals->m_interval;

					// set the anim velocity to the previous velocity.
					// and predict one tick ahead.
					record->m_anim_velocity = previous->m_velocity + accel;
				}

				// fix these niggas.
				if ( ( record->m_origin - previous->m_origin ).is_zero( ) )
					record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };
			}
		}
	}

	// get invalidated bone cache.
	static auto& invalidatebonecache = pattern::find( g_csgo.m_client_dll, XOR( "C6 05 ? ? ? ? ? 89 47 70" ) ).add( 0x2 );

	// set player.
	for ( int i = 0; i < 13; i++ )
		m_player->m_AnimOverlay( )[i].m_owner = m_player;

	bool fake = !bot && config["acc_correct"].get<bool>( );

	// if using fake angles, correct angles.
	if ( fake ) {
		g_resolver.on_player( record );

		// set stuff before animating.
		m_player->m_vecOrigin( ) = record->m_origin;
		m_player->m_vecVelocity( ) = m_player->m_vecAbsVelocity( ) = record->m_anim_velocity;
		m_player->m_flLowerBodyYawTarget( ) = record->m_body;

		// EFL_DIRTY_ABSVELOCITY
		// skip call to C_BaseEntity::CalcAbsoluteVelocity
		m_player->m_iEFlags( ) &= ~0x1000;

		// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
		m_player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

		// write potentially resolved angles.
		m_player->m_angEyeAngles( ) = record->m_eye_angles;

		// fix animating in same frame.
		if ( state->m_frame >= g_csgo.m_globals->m_frame )
			state->m_frame = g_csgo.m_globals->m_frame - 1;

		// make sure we keep track of the original invalidation state
		const auto oldbonecache = invalidatebonecache;

		// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
		m_player->m_bClientSideAnimation( ) = true;
		m_player->UpdateClientSideAnimation( );
		m_player->m_bClientSideAnimation( ) = false;

		// we don't want to enable cache invalidation by accident
		invalidatebonecache = oldbonecache;

		// store updated/animated poses and rotation in lagrecord.
		m_player->GetPoseParameters( record->m_poses );
		record->m_abs_ang = m_player->GetAbsAngles( );

		// restore backup data.
		m_player->m_vecOrigin( ) = backup.m_origin;
		m_player->m_vecVelocity( ) = backup.m_velocity;
		m_player->m_vecAbsVelocity( ) = backup.m_abs_velocity;
		m_player->m_fFlags( ) = backup.m_flags;
		m_player->m_iEFlags( ) = backup.m_eflags;
		m_player->m_flDuckAmount( ) = backup.m_duck;
		m_player->m_flLowerBodyYawTarget( ) = backup.m_body;
		m_player->SetAbsOrigin( backup.m_abs_origin );
		m_player->SetAnimLayers( backup.m_layers );

		// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
		// also dont restore the render angles which indicate the model rotation.

		// restore globals.
		g_csgo.m_globals->m_curtime = curtime;
		g_csgo.m_globals->m_frametime = frametime;

		// players animations have updated.
		return m_player->InvalidatePhysicsRecursive( InvalidatePhysicsBits_t::ANIMATION_CHANGED );
	}
}

void AimPlayer::OnNetUpdate( Player* player ) {
	bool reset = ( !config["rage_enable"].get<bool>( ) || player->m_lifeState( ) == LIFE_DEAD || !player->enemy( g_cl.m_local ) );
	bool disable = ( !reset && !g_cl.m_processing );

	// if this happens, delete all the lagrecords.
	if ( reset ) {
		player->m_bClientSideAnimation( ) = true;
		m_body_update = FLT_MAX;
		m_records.clear( );
		return;
	}

	// just disable anim if this is the case.
	if ( disable ) {
		player->m_bClientSideAnimation( ) = true;
		m_body_update = FLT_MAX;
		return;
	}

	// update player ptr if required.
	// reset player if changed.
	if ( m_player != player ) {
		m_records.clear( );
		m_body_index = 0;
		m_freestand_index = 0;
		m_lm_index = 0;
		m_body_update = FLT_MAX;
	}

	// update player ptr.
	m_player = player;

	// indicate that this player has been out of pvs.
	// insert dummy record to separate records
	// to fix stuff like animation and prediction.
	if ( player->dormant( ) ) {
		bool insert = true;


		// dont get data when doramnt.
		m_moved = false;
		m_body_update = FLT_MAX;

		// we have any records already?
		if ( !m_records.empty( ) ) {

			LagRecord* front = m_records.front( ).get( );

			// we already have a dormancy separator.
			if ( front->dormant( ) )
				insert = false;
		}

		if ( insert ) {
			// add new record.
			m_records.emplace_front( std::make_shared< LagRecord >( player ) );

			// get reference to newly added record.
			LagRecord* current = m_records.front( ).get( );

			// mark as dormant.
			current->m_dormant = true;

		}

		return;
	}

	bool update = ( m_records.empty( ) || player->m_flSimulationTime( ) > m_records.front( ).get( )->m_sim_time );


	// this is the first data update we are receving
	// OR we received data with a newer simulation context.
	if ( update ) {
		// add new record.
		m_records.emplace_front( std::make_shared< LagRecord >( player ) );

		// get reference to newly added record.
		LagRecord* current = m_records.front( ).get( );

		// mark as non dormant.
		current->m_dormant = false;

		// update animations on current record.
		// call resolver.
		UpdateAnimations( current );

		// create bone matrix for this record.
		g_bones.setup( m_player, nullptr, current );
	}

	// no need to store insane amt of data.
	while ( m_records.size( ) > 64 )
		m_records.pop_back( );
}

void AimPlayer::OnRoundStart( Player* player ) {
	m_player = player;
	m_update_record = std::make_unique<LagRecord>( ).get( );
	m_shots = 0;
	m_missed_shots = 0;

	// reset stand and body index.
	m_lm_index = 0;
	m_freestand_index = 0;
	m_body_index = 0;

	m_records.clear( );
	m_hitboxes.clear( );

	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void AimPlayer::SetupHitboxes( LagRecord* record, bool history ) {
	// reset hitboxes.
	m_hitboxes.clear( );

	if ( !g_cl.m_weapon_info )
		return;

	bool prefer_head = record->m_velocity.length_2d( ) > 71.f;

	auto head_always = config["rage_head_always"].get<bool>( );
	auto head_move = config["rage_head_move"].get<bool>( ) && prefer_head;
	auto head_resolve = config["rage_head_resolve"].get<bool>( ) && !( record->m_mode != Resolver::Modes::R_NONE && record->m_mode != Resolver::Modes::R_MOVE && record->m_mode != Resolver::Modes::R_UPDATE );

	if ( head_always || head_move || head_resolve )
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::PREFER } );

	auto body_always = config["rage_body_always"].get<bool>( );
	auto body_fake = config["rage_body_fake"].get<bool>( ) && record->m_mode != Resolver::Modes::R_NONE && record->m_mode != Resolver::Modes::R_MOVE && record->m_mode != Resolver::Modes::R_UPDATE;
	bool body_air = config["rage_body_air"].get<bool>( ) && !( record->m_pred_flags & FL_ONGROUND );

	if ( body_always || body_fake || body_air )
		m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::PREFER } );

	if ( config["rage_body_lethal"].get<bool>( ) )
		m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::LETHAL } );

	bool only{ false };
	if ( cfg_t::get_hotkey( "acc_forcebody", "acc_forcebody_mode" ) || g_cl.m_weapon_id == ZEUS ) {
		only = m_force_body = true;
		m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::PREFER } );
	}

	bool ignore_limbs = record->m_velocity.length_2d( ) > 71.f && config["acc_limbs"].get<bool>( );
	if ( config["rage_hb_head"].get<bool>( ) && !only ) {
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_ubody"].get<bool>( ) && !only ) {
		m_hitboxes.push_back( { HITBOX_UPPER_CHEST, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_body"].get<bool>( ) ) {
		m_hitboxes.push_back( { HITBOX_THORAX, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_CHEST, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_pelvis"].get<bool>( ) ) {
		m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_arm"].get<bool>( ) && !ignore_limbs && !only ) {
		m_hitboxes.push_back( { HITBOX_L_UPPER_ARM, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_UPPER_ARM, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_leg"].get<bool>( ) && !ignore_limbs && !only ) {
		m_hitboxes.push_back( { HITBOX_L_THIGH, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_THIGH, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_L_CALF, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_CALF, HitscanMode::NORMAL } );
	}

	if ( config["rage_hb_foot"].get<bool>( ) && !ignore_limbs && !only ) {
		m_hitboxes.push_back( { HITBOX_L_FOOT, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_FOOT, HitscanMode::NORMAL } );
	}
}

void Aimbot::init( ) {
	// clear old targets.
	m_targets.clear( );

	m_target = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop = false;

	m_best_dist = std::numeric_limits< float >::max( );
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max( );
	m_best_height = std::numeric_limits< float >::max( );
}

void Aimbot::StripAttack( ) {
	if ( g_cl.m_weapon_id == REVOLVER )
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;

	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::think( ) {
	// do all startup routines.
	init( );

	// we have no aimbot enabled.
	if ( !config["rage_enable"].get<bool>( ) || !g_cl.m_processing )
		return;

	// sanity.
	if ( !g_cl.m_weapon )
		return;

	// no grenades or bomb.
	if ( g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4 )
		return;

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.
	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	// one tick before being able to shoot.
	if ( revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query ) {
		*g_cl.m_packet = false;
		return;
	}

	// no point in aimbotting if we cannot fire this tick.
	if ( !g_cl.m_weapon_fire && g_tickbase.m_shift != e_shift::E_UNCHARGE )
		return;

	// setup bones for all valid targets.
	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		if ( !IsValidTarget( player ) )
			continue;

		AimPlayer* data = &m_players[i - 1];
		if ( !data )
			continue;

		if ( data->m_records.empty( ) )
			continue;

		// store player as potential target this tick.
		m_targets.emplace_back( data );
	}

	// run knifebot.
	if ( g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS ) {

		knife( );

		return;
	}

	// scan available targets... if we even have any.
	find( );

	// finally set data when shooting.
	apply( );
}

bool Aimbot::CheckHitchance( Player* player, const ang_t& angle ) {
	constexpr float HITCHANCE_MAX = 100.f;
	constexpr int   SEED_MAX = 255;

	vec3_t     start{ g_cl.m_shoot_pos }, end, fwd, right, up, dir, wep_spread;
	float      inaccuracy, spread;
	CGameTrace tr;
	auto hc{ g_tickbase.m_shift == e_shift::E_UNCHARGE ? config["rage_dthc"].get<float>( ) : config["rage_hitchance"].get<float>( ) };
	size_t     total_hits{ }, needed_hits{ (size_t)std::ceil( ( hc * SEED_MAX ) / HITCHANCE_MAX ) };

	if ( hc == 0.f )
		return true;

	// get needed directional vectors.
	math::AngleVectors( angle, &fwd, &right, &up );

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	inaccuracy = g_cl.m_weapon->GetInaccuracy( );
	spread = g_cl.m_weapon->GetSpread( );

	// iterate all possible seeds.
	for ( int i{ }; i <= SEED_MAX; ++i ) {
		// get spread.
		wep_spread = g_cl.m_weapon->CalculateSpread( i, inaccuracy, spread );

		// get spread direction.
		dir = ( fwd + ( right * wep_spread.x ) + ( up * wep_spread.y ) ).normalized( );

		// get end of trace.
		end = start + ( dir * g_cl.m_weapon_info->m_range );

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, player, &tr );

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if ( tr.m_entity == player && game::IsValidHitgroup( tr.m_hitgroup ) )
			++total_hits;

		// we made it.
		if ( total_hits >= needed_hits )
			return true;

		// we cant make it anymore.
		if ( ( SEED_MAX - i + total_hits ) < needed_hits )
			return false;
	}

	return false;
}

void Aimbot::find( ) {
	struct BestTarget_t { Player* player; vec3_t pos; float damage; LagRecord* record; int hitbox; };

	vec3_t       tmp_pos;
	float        tmp_damage;
	int          tmp_hitbox;
	BestTarget_t best;
	best.player = nullptr;
	best.damage = -1.f;
	best.pos = vec3_t{ };
	best.record = nullptr;
	best.hitbox = -1;

	if ( m_targets.empty( ) )
		return;

	// iterate all targets.
	for ( const auto& t : m_targets ) {
		if ( t->m_records.empty( ) )
			continue;

		// this player broke lagcomp.
		// his bones have been resetup by our lagcomp.
		// therfore now only the front record is valid.
		if ( g_lagcomp.StartPrediction( t ) ) {
			LagRecord* front = t->m_records.front( ).get( );

			t->SetupHitboxes( front, false );
			if ( t->m_hitboxes.empty( ) )
				continue;

			// rip something went wrong..
			if ( t->GetBestAimPosition( tmp_pos, tmp_damage, tmp_hitbox, front ) && SelectTarget( front, tmp_pos, tmp_damage ) ) {

				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = front;
				best.hitbox = tmp_hitbox;
				break;
			}
		}

		// player did not break lagcomp.
		// history aim is possible at this point.
		else {
			LagRecord* ideal = g_resolver.FindIdealRecord( t );
			if ( !ideal )
				continue;

			t->SetupHitboxes( ideal, false );
			if ( t->m_hitboxes.empty( ) )
				continue;

			// try to select best record as target.
			if ( t->GetBestAimPosition( tmp_pos, tmp_damage, tmp_hitbox, ideal ) && SelectTarget( ideal, tmp_pos, tmp_damage ) ) {
				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = ideal;
				best.hitbox = tmp_hitbox;
			}


			if ( best.damage && best.player && !best.record ) {
				LagRecord* last = g_resolver.FindLastRecord( t );
				if ( last ) {

					t->SetupHitboxes( last, true );

					if ( !t->m_hitboxes.empty( ) ) {

						// rip something went wrong..
						if ( t->GetBestAimPosition( tmp_pos, tmp_damage, tmp_hitbox, last ) && SelectTarget( last, tmp_pos, tmp_damage ) ) {
							// if we made it so far, set shit.

							//  && (tmp_hitbox >= 2 || last->m_mode == Resolver::Modes::RESOLVE_WALK || last->m_mode == Resolver::Modes::RESOLVE_BODY)
							if ( best.damage <= tmp_damage ) {
								best.player = t->m_player;
								best.pos = tmp_pos;
								best.damage = tmp_damage;
								best.record = last;
								best.hitbox = tmp_hitbox;

							}
						}
					}
				}
			}
		}

		if ( best.player && best.record && best.damage )
			break;
	}

	// verify our target and set needed data.
	if ( best.player && best.record && best.damage ) {
		// calculate aim angle.
		math::VectorAngles( best.pos - g_cl.m_shoot_pos, m_angle );

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;
		m_hitbox = best.hitbox;

		// we are on land.
		const bool on_land = !( g_cl.m_flags & FL_ONGROUND ) && g_cl.m_local->m_fFlags( ) & FL_ONGROUND;

		// write data, needed for traces / etc.
		m_record->cache( );

		bool on = config["menu_ut"].get<bool>( );
		bool hit = on && CheckHitchance( best.player, m_angle ); //CanHitchance( m_angle, m_aim, m_target, config["rage_hitchance"].get<float>( ), m_hitbox, m_damage );

		//m_stop = !( g_cl.m_buttons & IN_JUMP );

		//m_stop = ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && on && ( ( !g_cl.m_weapon_fire && config["rage_stop"].get<int>( ) == 0 || ( config["rage_stop"].get<int>( ) == 1 ) ) );

		// failed to hit them?
		if ( !hit ) {
			// set autostop shit.
			if ( ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && !on_land ) {
				m_stop = !( g_cl.m_buttons & IN_JUMP );
			}
		}

		// if we can scope.
		bool can_scope = !g_cl.m_local->m_bIsScoped( ) && ( g_cl.m_weapon_id == AUG || g_cl.m_weapon_id == SG553 || g_cl.m_weapon_type == WEAPONTYPE_SNIPER_RIFLE );

		if ( can_scope ) {
			if ( config["acc_scope"].get<bool>( ) && on && !hit && ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && !on_land ) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}
		}

		if ( hit || !on || g_tickbase.m_shift == e_shift::E_UNCHARGE ) {
			// right click attack.
			if ( !config["menu_ut"].get<bool>( ) && g_cl.m_weapon_id == REVOLVER )
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;

			// left click attack.
			else
				g_cl.m_cmd->m_buttons |= IN_ATTACK;
		}

	}
}

bool Aimbot::CanHit( const vec3_t start, const vec3_t end, LagRecord* animation, int box, bool in_shot, BoneArray* bones )
{
	if ( !animation || !animation->m_player )
		return false;

	// always try to use our aimbot matrix first.
	BoneArray* matrix = nullptr;

	// this is basically for using a custom matrix.
	if ( in_shot )
		matrix = bones;

	if ( !matrix )
		return false;

	const model_t* model = animation->m_player->GetModel( );
	if ( !model )
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( animation->m_player->m_nHitboxSet( ) );
	if ( !set )
		return false;

	mstudiobbox_t* bbox = set->GetHitbox( box );
	if ( !bbox )
		return false;


	vec3_t min, max;

	const auto is_capsule = bbox->m_radius != -1.f;

	if ( is_capsule )
	{
		math::VectorTransform( bbox->m_mins, animation->m_bones[bbox->m_bone], min );
		math::VectorTransform( bbox->m_maxs, animation->m_bones[bbox->m_bone], max );
		const auto dist = math::SegmentToSegment( start, end, min, max );

		if ( dist < bbox->m_radius )
			return true;
	}

	if ( !is_capsule )
	{
		math::VectorTransform( math::vector_rotate( bbox->m_mins, bbox->m_angle ), animation->m_bones[bbox->m_bone], min );
		math::VectorTransform( math::vector_rotate( bbox->m_maxs, bbox->m_angle ), animation->m_bones[bbox->m_bone], max );

		math::VectorITransform( start, animation->m_bones[bbox->m_bone], min );
		math::vector_i_rotate( end, animation->m_bones[bbox->m_bone], max );

		if ( math::IntersectLineWithBB( min, max, bbox->m_mins, bbox->m_maxs ) )
			return true;
	}

	return false;
}

bool Aimbot::CanHitchance( ang_t angle, vec3_t point, Player* player, float chance, int hitbox, float damage ) // elite hitchance
{
	if ( !g_cl.m_weapon )
		return false;

	if ( !g_cl.m_weapon_info )
		return false;

	if ( chance <= 0.f )
		return true;

	vec3_t forward, right, up;
	auto eye_position = g_cl.m_shoot_pos;
	math::AngleVectors( angle, &forward, &right, &up );

	int TraceHits = 0;
	int TraceHitsRightHitbox = 0;
	int needed_hits = int( floor( std::ceil( ( chance * 128 ) / 100 ) ) );
	int cNeededHits = static_cast<int>( 128.f * ( chance / 100.f ) );

	//weap->UpdateAccuracyPenalty();
	float weap_sir = g_cl.m_weapon->GetSpread( );
	float weap_inac = g_cl.m_weapon->GetInaccuracy( );
	auto recoil_index = g_cl.m_weapon->m_flRecoilIndex( );

	if ( weap_sir <= 0.f )
		return true;

	for ( int i = 0; i <= 128; i++ ) {
		float a = g_csgo.RandomFloat( 0.f, 1.f );
		float b = g_csgo.RandomFloat( 0.f, 6.2831855f );
		float c = g_csgo.RandomFloat( 0.f, 1.f );
		float d = g_csgo.RandomFloat( 0.f, 6.2831855f );

		float inac = a * weap_inac;
		float sir = c * weap_sir;

		if ( g_cl.m_weapon_id == 64 ) {
			a = 1.f - a * a;
			a = 1.f - c * c;
		}
		else if ( g_cl.m_weapon_id == 28 && recoil_index < 3.0f ) {
			for ( int i = 3; i > recoil_index; i-- ) {
				a *= a;
				c *= c;
			}

			a = 1.0f - a;
			c = 1.0f - c;
		}

		// credits: haircuz
		else if ( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) && g_cl.m_weapon_id ) {
			if ( g_cl.m_weapon->GetInaccuracy( ) < 0.009f ) {
				return true;
			}
		}

		vec3_t sirVec( ( cos( b ) * inac ) + ( cos( d ) * sir ), ( sin( b ) * inac ) + ( sin( d ) * sir ), 0 ), direction;

		direction.x = forward.x + ( sirVec.x * right.x ) + ( sirVec.y * up.x );
		direction.y = forward.y + ( sirVec.x * right.y ) + ( sirVec.y * up.y );
		direction.z = forward.z + ( sirVec.x * right.z ) + ( sirVec.y * up.z );
		direction.normalize( );

		ang_t viewAnglesSpread;
		math::VectorAngles( direction, viewAnglesSpread, &up );
		viewAnglesSpread.normalize( );

		vec3_t viewForward;
		math::AngleVectors( viewAnglesSpread, &viewForward );
		viewForward.normalized( );

		viewForward = g_cl.m_shoot_pos + ( viewForward * g_cl.m_weapon_info->m_range );
		CGameTrace tr;

		// glass fix xD
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( g_cl.m_shoot_pos, viewForward ), MASK_SHOT | CONTENTS_GRATE | CONTENTS_WINDOW, player, &tr );

		// additional checks if we are actually hitting that specific hitbox.
		if ( tr.m_entity == player ) {
			if ( game::IsValidHitgroup( tr.m_hitgroup ) )
				++TraceHits;

			if ( config["acc_boost"].get<int>( ) > 0 ) {
				if ( tr.m_hitbox == m_hitbox )
					++TraceHitsRightHitbox;
			}
		}

		if ( TraceHits >= needed_hits ) {

			if ( ( ( static_cast<float>( TraceHitsRightHitbox ) / static_cast<float>( 128.f ) ) >= ( config["acc_boost"].get<float>( ) / 100.f ) ) || config["acc_boost"].get<float>( ) <= 0.f )
				return true;
		}

		if ( ( 128 - i + TraceHits ) < cNeededHits )
			return false;
	}
	return false;
}

bool AimPlayer::SetupHitboxPoints( LagRecord* record, BoneArray* bones, int index, std::vector< vec3_t >& points ) {

	points.clear( );

	const model_t* model = record->m_player->GetModel( );
	if ( !model )
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if ( !set )
		return false;

	mstudiobbox_t* bbox = set->GetHitbox( index );
	if ( !bbox )
		return false;

	// get hitbox scales.
	float scale = config["rage_scale"].get<float>( ) * 0.01f;
	float bscale = config["rage_scale"].get<float>( ) * 0.01f;

	// these indexes represent boxes.
	if ( bbox->m_radius <= 0.f ) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		math::ConcatTransforms( bones[bbox->m_bone], rot_matrix, matrix );

		// extract origin from matrix.
		vec3_t origin = matrix.GetOrigin( );

		// compute raw center point.
		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if ( index == HITBOX_R_FOOT || index == HITBOX_L_FOOT ) {
			float d1 = ( bbox->m_mins.z - center.z ) * 0.875f;

			// invert.
			if ( index == HITBOX_L_FOOT )
				d1 *= -1.f;

			// side is more optimal then center.
			points.push_back( { center.x, center.y, center.z + d1 } );

			if ( config["rage_hb_foot"].get<bool>( ) ) {
				// get point offset relative to center point
				// and factor in hitbox scale.
				float d2 = ( bbox->m_mins.x - center.x ) * scale;
				float d3 = ( bbox->m_maxs.x - center.x ) * scale;

				// heel.
				points.push_back( { center.x + d2, center.y, center.z } );

				// toe.
				points.push_back( { center.x + d3, center.y, center.z } );
			}
		}

		// nothing to do here we are done.
		if ( points.empty( ) )
			return false;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for ( auto& p : points ) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.dot( matrix[0] ), p.dot( matrix[1] ), p.dot( matrix[2] ) };

			// transform point to world space.
			p += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// factor in the pointscale.
		float r = bbox->m_radius * scale;
		float br = bbox->m_radius * bscale;

		// compute raw center point.
		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;

		// head has 5 points.
		if ( index == HITBOX_HEAD ) {
			// add center.
			points.push_back( center );

			if ( config["rage_hb_head"].get<bool>( ) ) {
				// rotation matrix 45 degrees.
				// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
				// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.push_back( { bbox->m_maxs.x + ( rotation * r ), bbox->m_maxs.y + ( -rotation * r ), bbox->m_maxs.z } );

				// right.
				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + r } );

				// left.
				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - r } );

				// back.
				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y - r, bbox->m_maxs.z } );

				// get animstate ptr.
				CCSGOPlayerAnimState* state = record->m_player->m_PlayerAnimState( );

				// add this point only under really specific circumstances.
				// if we are standing still and have the lowest possible pitch pose.
				if ( state && record->m_anim_velocity.length( ) <= 0.1f && record->m_eye_angles.x <= state->m_min_pitch ) {

					// bottom point.
					points.push_back( { bbox->m_maxs.x - r, bbox->m_maxs.y, bbox->m_maxs.z } );
				}
			}
		}

		// body has 5 points.
		else if ( index == HITBOX_BODY ) {
			// center.
			points.push_back( center );

			// back.
			if ( config["rage_hb_body"].get<bool>( ) )
				points.push_back( { center.x, bbox->m_maxs.y - br, center.z } );
		}

		else if ( ( index == HITBOX_PELVIS && config["rage_hb_pelvis"].get<bool>( ) ) || ( index == HITBOX_UPPER_CHEST && config["rage_hb_ubody"].get<bool>( ) ) ) {
			// back.
			points.push_back( { center.x, bbox->m_maxs.y - r, center.z } );
		}

		// other stomach/chest hitboxes have 2 points.
		else if ( index == HITBOX_THORAX || index == HITBOX_CHEST ) {
			// add center.
			points.push_back( center );

			// add extra point on back.
			if ( config["rage_hb_body"].get<bool>( ) )
				points.push_back( { center.x, bbox->m_maxs.y - r, center.z } );
		}

		else if ( index == HITBOX_R_CALF || index == HITBOX_L_CALF ) {
			// add center.
			points.push_back( center );

			// half bottom.
			if ( config["rage_hb_leg"].get<bool>( ) )
				points.push_back( { bbox->m_maxs.x - ( bbox->m_radius / 2.f ), bbox->m_maxs.y, bbox->m_maxs.z } );
		}

		else if ( ( index == HITBOX_R_THIGH || index == HITBOX_L_THIGH ) && config["rage_hb_leg"].get<bool>( ) ) {
			// add center.
			points.push_back( center );
		}

		// arms get only one point.
		else if ( ( index == HITBOX_R_UPPER_ARM || index == HITBOX_L_UPPER_ARM ) && config["rage_hb_arm"].get<bool>( ) ) {
			// elbow.
			points.push_back( { bbox->m_maxs.x + bbox->m_radius, center.y, center.z } );
		}

		// nothing left to do here.
		if ( points.empty( ) )
			return false;

		// transform capsule points.
		for ( auto& p : points )
			math::VectorTransform( p, bones[bbox->m_bone], p );
	}

	return true;

}

bool AimPlayer::GetBestAimPosition( vec3_t& aim, float& damage, int& hitbox, LagRecord* record ) {
	bool                  done, pen;
	float                 dmg, pendmg;
	HitscanData_t         scan;
	std::vector< vec3_t > points;

	// get player hp.
	int hp = std::min( 100, m_player->m_iHealth( ) );

	if ( g_cl.m_weapon_id == ZEUS ) {
		dmg = hp;
		pen = false;
	}

	else {
		auto mindmg = g_aimbot.m_damage_ovr ? config["rage"].get<int>( ) : config["rage_dmg"].get<int>( );
		dmg = mindmg;
		if ( mindmg >= hp )
			dmg = hp;

		pendmg = dmg;

		pen = true;
	}

	// write all data of this record l0l.
	record->cache( );

	// iterate hitboxes.
	for ( const auto& it : m_hitboxes ) {
		done = false;

		// setup points on hitbox.
		if ( !SetupHitboxPoints( record, record->m_bones, it.m_index, points ) )
			continue;

		// iterate points on hitbox.
		for ( const auto& point : points ) {

			if ( g_cl.m_shoot_pos.dist_to( point ) > g_cl.m_weapon_info->m_range )
				continue;

			penetration::PenetrationInput_t in;

			in.m_damage = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen = pen;
			in.m_target = m_player;
			in.m_from = g_cl.m_local;
			in.m_pos = point;

			penetration::PenetrationOutput_t out;

			// we can hit p!
			if ( penetration::run( &in, &out ) ) {

				// nope we did not hit head..
				if ( it.m_index == HITBOX_HEAD && out.m_hitgroup != HITGROUP_HEAD )
					continue;

				if ( m_allow_lethal && out.m_damage >= m_player->m_iHealth( ) && it.m_index >= 2 )
					done = true;

				// 2 shots will be sufficient to kill.
				else if ( m_prefer_body && ( out.m_damage * 2.f ) >= m_player->m_iHealth( ) && it.m_index >= 2 )
					done = true;

				// this hitbox has normal selection, it needs to have more damage.
				else if ( it.m_mode == HitscanMode::NORMAL ) {
					// we did more damage.
					if ( out.m_damage >= scan.m_damage ) {
						// save new best data.
						scan.m_damage = out.m_damage;
						scan.m_pos = point;
						scan.m_hitbox = it.m_index;

						// if the first point is lethal
						// screw the other ones.
						if ( scan.m_damage >= m_player->m_iHealth( ) && scan.m_hitbox >= 2 )
							break;
					}
				}

				// we found a preferred / lethal hitbox.
				if ( done ) {
					// save new best data.
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					scan.m_hitbox = it.m_index;
					break;
				}
			}
		}

		// ghetto break out of outer loop.
		if ( done )
			break;
	}

	// we found something that we can damage.
	// set out vars.
	if ( scan.m_damage > 0.f ) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		hitbox = scan.m_hitbox;
		return true;
	}

	return false;
}

bool Aimbot::SelectTarget( LagRecord* record, const vec3_t& aim, float damage ) {
	float dist, fov, height;
	int   hp;

	if ( math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, aim ) > 180.f )
		return false;

	switch ( config["rage_ts"].get<int>( ) ) {
		// crosshair.
		case 0:
			fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, aim );

			if ( fov < m_best_fov ) {
				m_best_fov = fov;
				return true;
			}

			break;

			// damage.
		case 1:
			if ( damage > m_best_damage ) {
				m_best_damage = damage;
				return true;
			}

			break;

			// lowest hp.
		case 2:
			// fix for retarded servers?
			hp = std::min( 100, record->m_player->m_iHealth( ) );

			if ( hp < m_best_hp ) {
				m_best_hp = hp;
				return true;
			}

			break;

			// least lag.
		case 3:
			if ( record->m_lag < m_best_lag ) {
				m_best_lag = record->m_lag;
				return true;
			}

			break;

		default:
			return false;
	}

	return false;
}

void Aimbot::apply( ) {
	bool attack, attack2;

	// attack states.
	attack = ( g_cl.m_cmd->m_buttons & IN_ATTACK );
	attack2 = ( g_cl.m_weapon_id == REVOLVER && g_cl.m_cmd->m_buttons & IN_ATTACK2 );

	// ensure we're attacking.
	if ( attack || attack2 ) {
		// choke every shot.

		bool should_fl = g_cl.m_lag < 15 && !g_aimbot.m_double_tap;

		if ( g_cl.m_weapon_fire )
			*g_cl.m_packet = true;

		if ( m_target ) {
			// make sure to aim at un-interpolated data.
			// do this so BacktrackEntity selects the exact record.
			if ( m_record && !m_record->m_broke_lc )
				g_cl.m_cmd->m_tick = game::TIME_TO_TICKS( m_record->m_sim_time + g_cl.m_lerp );

			// set angles to target.
			g_cl.m_cmd->m_view_angles = m_angle;

			// if not silent aim, apply the viewangles.
			if ( !config["rage_silent"].get<bool>( ) )
				g_csgo.m_engine->SetViewAngles( m_angle );

			if ( m_record ) {
				// store fired shot.
				g_shots.OnShotFire( m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_hitbox : -1, m_target ? m_record : nullptr );

				// store matrix for shot record chams
				g_visuals.AddMatrix( m_target, m_record->m_bones );
			}

			//g_visuals.DrawHitboxMatrix( m_record, colors::white, 10.f );
		}

		// nospread.
		if ( !config["rage_ut"].get<bool>( ) )
			NoSpread( );

		g_cl.m_cmd->m_view_angles -= g_cl.m_local->m_aimPunchAngle( ) * g_csgo.weapon_recoil_scale->GetFloat( );

		// set that we fired.
		g_cl.m_shot = true;
	}
}

void Aimbot::NoSpread( ) {
	bool    attack2;
	vec3_t  spread, forward, right, up, dir;

	// revolver state.
	attack2 = ( g_cl.m_weapon_id == REVOLVER && ( g_cl.m_cmd->m_buttons & IN_ATTACK2 ) );

	// get spread.
	spread = g_cl.m_weapon->CalculateSpread( g_cl.m_cmd->m_random_seed, attack2 );

	// compensate.
	g_cl.m_cmd->m_view_angles -= { -math::rad_to_deg( std::atan( spread.length_2d( ) ) ), 0.f, math::rad_to_deg( std::atan2( spread.x, spread.y ) ) };
}