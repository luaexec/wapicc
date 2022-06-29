#include "includes.h"
#include "wapim.h"
Client g_cl{ };

ulong_t __stdcall Client::init( void* arg ) {
	g_cl.m_user = XOR( "wapicc" );

	if ( !g_csgo.init( ) )
		return 0;

	return 1;
}

void Client::DrawHUD( ) {
	//if (!g_menu.main.misc.watermark.get( ))
		//return;

	int ms = !g_csgo.m_engine->IsInGame( ) ? 0 : std::max( 0, (int)std::round( g_cl.m_latency * 1000.f ) );

	int rate = (int)std::round( 1.f / g_csgo.m_globals->m_interval );
	Color color = g_gui.m_color;

	colors::accent = g_gui.m_color;

	auto w{ 0 }, h{ 0 };
	g_csgo.m_engine->GetScreenSize( w, h );

	auto full = tfm::format( "wapicc %sms", ms );
	auto full_size = render::esp.size( full ).m_width + 15;
	auto start = "wapicc";
	auto start_size = render::esp.size( start ).m_width + 10;
	auto end = tfm::format( "%sms", ms );
	auto end_size = render::esp.size( end ).m_width + 5;

	render::round_rect( w - 15 - full_size, 15, full_size, 19, 2, gui::palette::dark.alpha( 255 ) );
	render::gradient1337( w - 15 - full_size + 2, 17, start_size, 15, gui::m_accent.alpha( 55 ), colors::black.alpha( 0 ) );
	render::esp.string( w - 15 - full_size + 5, 17, gui::m_accent.alpha( 255 ), start );
	render::esp.string( w - 15 - full_size + start_size, 17, colors::white.alpha( 255 ), end );
}

void Client::KillFeed( ) {
	if ( !g_menu.main.misc.killfeed.get( ) )
		return;

	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
	if ( !feed )
		return;

	int size = feed->notices.Count( );
	if ( !size )
		return;

	for ( int i{ }; i < size; ++i ) {
		NoticeText_t* notice = &feed->notices[i];

		if ( notice->fade == 1.5f )
			notice->fade = FLT_MAX;
	}
}

void Client::ThirdPersonFSN( ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	if ( g_csgo.m_input->CAM_IsThirdPerson( ) )
		*reinterpret_cast<ang_t*>( uintptr_t( g_cl.m_local ) + 0x31C4 + 0x4 ) = m_real_angle; // cancer.
}

void Client::AspectRatio( )
{
	static const auto r_aspectratio = g_csgo.m_cvar->FindVar( HASH( "r_aspectratio" ) );
	if ( g_menu.main.misc.aspectratio.get( ) ) {
		r_aspectratio->SetValue( g_menu.main.misc.aspectraio_val.get( ) / 100 );
	}
	else {
		r_aspectratio->SetValue( 0 );
	}
}

void Client::OnPaint( ) {
	g_csgo.m_engine->GetScreenSize( m_width, m_height );

	AspectRatio( );
	if ( g_csgo.m_engine->IsConnected( ) )
	{
		static const auto sv_cheats = g_csgo.m_cvar->FindVar( HASH( "sv_cheats" ) );
		sv_cheats->SetValue( 1 );
	}

	g_visuals.think( );
	g_grenades.paint( );
	g_notify.think( g_menu.main.config.menu_color.get( ) );

	DrawHUD( );
	KillFeed( );

	g_gui.think( );
	wapim::render( );
}

void Client::OnMapload( ) {
	g_netvars.SetupClassData( );

	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	Visuals::ModulateWorld( );

	g_skins.load( );

	m_sequences.clear( );

	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo( );

	if ( g_csgo.m_net ) {
		g_hooks.m_net_channel.reset( );
		g_hooks.m_net_channel.init( g_csgo.m_net );
		g_hooks.m_net_channel.add( INetChannel::PROCESSPACKET, util::force_cast( &Hooks::ProcessPacket ) );
		g_hooks.m_net_channel.add( INetChannel::SENDDATAGRAM, util::force_cast( &Hooks::SendDatagram ) );
	}
}

void Client::StartMove( CUserCmd* cmd ) {
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;

	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );
	if ( !m_local )
		return;

	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo( );

	if ( !g_csgo.m_net )
		return;

	if ( m_processing && m_tick_to_recharge > 0 && !m_charged && can_recharge ) {
		m_tick_to_recharge--;
		m_visual_shift_ticks++;
		cmd->m_tick = INT_MAX;
		*m_packet = true;
	}

	if ( can_recharge && m_tick_to_recharge == 0 )
	{
		m_charged = true;
		can_recharge = false;
	}

	m_max_lag = 14;
	m_lag = g_csgo.m_cl->m_choked_commands;
	m_lerp = game::GetClientInterpAmount( );
	m_latency = g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );
	math::clamp( m_latency, 0.f, 1.f );
	m_latency_ticks = game::TIME_TO_TICKS( m_latency );
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + m_latency_ticks;

	m_processing = m_local && m_local->alive( );
	if ( !m_processing )
		return;

	MouseFix( cmd );

	g_inputpred.update( );

	m_flags = m_local->m_fFlags( );

	m_shot = false;
}

void Client::BackupPlayers( bool restore ) {
	if ( restore ) {
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[i - 1].restore( player );
		}
	}

	else {
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[i - 1].store( player );
		}
	}
}

void Client::DoMove( ) {
	penetration::PenetrationOutput_t tmp_pen_data{ };

	m_strafe_angles = m_cmd->m_view_angles;

	g_movement.JumpRelated( );
	g_movement.Strafe( );
	g_movement.FakeWalk( );
	g_movement.AutoPeek( );

	g_inputpred.run( );

	m_cmd->m_view_angles = m_view_angles;

	math::AngleVectors( m_view_angles, &m_forward_dir );

	m_shoot_pos = m_local->GetShootPosition( );

	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;

	m_weapon = m_local->GetActiveWeapon( );

	if ( m_weapon ) {
		m_weapon_info = m_weapon->GetWpnData( );
		m_weapon_id = m_weapon->m_iItemDefinitionIndex( );
		m_weapon_type = m_weapon_info->m_weapon_type;

		if ( m_weapon_type != WEAPONTYPE_GRENADE )
			m_weapon->UpdateAccuracyPenalty( );

		if ( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE ) {
			penetration::PenetrationInput_t in;
			in.m_from = m_local;
			in.m_target = nullptr;
			in.m_pos = m_shoot_pos + ( m_forward_dir * m_weapon_info->m_range );
			in.m_damage = 1.f;
			in.m_damage_pen = 1.f;
			in.m_can_pen = true;

			penetration::run( &in, &tmp_pen_data );
		}

		m_pen_data = tmp_pen_data;

		m_player_fire = g_csgo.m_globals->m_curtime >= m_local->m_flNextAttack( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN );

		UpdateRevolverCock( );
		m_weapon_fire = CanFireWeapon( );
	}

	if ( g_input.GetKeyState( g_menu.main.misc.last_tick_defuse.get( ) ) && g_visuals.m_c4_planted ) {
		float defuse = ( m_local->m_bHasDefuser( ) ) ? 5.f : 10.f;
		float remaining = g_visuals.m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
		float dt = remaining - defuse - ( g_cl.m_latency / 2.f );

		m_cmd->m_buttons &= ~IN_USE;
		if ( dt <= game::TICKS_TO_TIME( 2 ) )
			m_cmd->m_buttons |= IN_USE;
	}

	g_grenades.think( );

	g_hvh.SendPacket( );

	g_aimbot.think( );

	g_hvh.AntiAim( );
}

void Client::EndMove( CUserCmd* cmd ) {
	UpdateInformation( );

	if ( !g_csgo.m_cl->m_choked_commands ) {
		m_real_angle = m_cmd->m_view_angles;
	}

	if ( g_menu.main.config.mode.get( ) == 0 )
		m_cmd->m_view_angles.SanitizeAngle( );

	g_movement.FixMove( cmd, m_strafe_angles );

	if ( *m_packet ) {
		g_hvh.m_step_switch = (bool)g_csgo.RandomInt( 0, 1 );

		m_old_lag = m_lag;

		m_radar = cmd->m_view_angles;
		m_radar.normalize( );

		vec3_t cur = m_local->m_vecOrigin( );
		vec3_t prev = m_net_pos.empty( ) ? cur : m_net_pos.front( ).m_pos;

		m_lagcomp = ( cur - prev ).length_sqr( ) > 4096.f;

		m_net_pos.emplace_front( g_csgo.m_globals->m_curtime, cur );
	}

	m_old_packet = *m_packet;
	m_old_shot = m_shot;
}

void Client::OnTick( CUserCmd* cmd ) {
	if ( g_menu.main.misc.ranks.get( ) && cmd->m_buttons & IN_SCORE ) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll( &msg );
	}

	if ( isShifting ) {
		*m_packet = ticksToShift == 1;
		cmd->m_buttons &= ~( IN_ATTACK | IN_ATTACK2 );
		if ( ignoreallcmds ) {
			cmd->m_tick = INT_MAX;
		}
		else
			g_cl.doubletapCharge--;
		return;
	}

	StartMove( cmd );

	if ( !m_processing )
		return;

	BackupPlayers( false );

	DoMove( );

	EndMove( cmd );

	BackupPlayers( true );

	g_inputpred.restore( );

	g_aimbot.DoubleTap( );
}

void Client::KeepCommunication( bool* send_packet, int command_number ) {
	if ( !g_cl.m_processing || !g_csgo.m_engine->IsInGame( ) )
		return;

	auto chan = g_csgo.m_engine->GetNetChannelInfo( );
	if ( !chan || !g_csgo.m_net ) {
		g_cl.m_saved_command.clear( );
		return;
	}

	if ( *send_packet )
		g_cl.m_saved_command.push_back( command_number );

	else {
		const auto current_choke = g_csgo.m_net->m_choked_packets;
		g_csgo.m_net->m_choked_packets = 0;
		g_csgo.m_net->SendDatagram( );
		--g_csgo.m_net->m_out_seq;
		g_csgo.m_net->m_choked_packets = current_choke;
	}
}

void Client::SetAngles( ) {
	if ( !g_cl.m_local || !g_cl.m_processing )
		return;

	g_cl.m_local->m_fEffects( ) |= EF_NOINTERP;

	g_cl.m_local->SetAbsAngles( m_rotation );
	g_cl.m_local->m_angRotation( ) = m_rotation;
	g_cl.m_local->m_angNetworkAngles( ) = m_rotation;

	if ( g_csgo.m_input->CAM_IsThirdPerson( ) )
		g_csgo.m_prediction->SetLocalViewAngles( m_radar );
}

void Client::UpdateAnimations( ) {
	if ( !g_cl.m_local || !g_cl.m_processing )
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	g_cl.m_local->SetAbsAngles( ang_t( 0.f, m_abs_yaw, 0.f ) );
}

void Client::UpdateLocal( )
{
	//if ( m_lag > 0 )
		//return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	float backup_frametime = g_csgo.m_globals->m_frametime;
	float backup_curtime = g_csgo.m_globals->m_curtime;

	const float v3 = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	const float v4 = ( v3 / g_csgo.m_globals->m_interval ) + .5f;

	static float backup_poses[24];
	static C_AnimationLayer backup_animlayer[13];

	g_csgo.m_globals->m_curtime = g_cl.m_local->m_nTickBase( ) * g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	math::clamp( m_real_angle.x, -90.f, 90.f );
	m_real_angle.normalize( );

	if ( state->m_frame >= v4 )
		state->m_frame -= 1;

	if ( g_csgo.m_globals->m_curtime != state->m_time )
	{
		g_cl.m_update_anims = true;
		g_cl.m_local->UpdateAnimationState( state, vec3_t( m_real_angle.x, m_real_angle.y, m_real_angle.z ) );
		g_cl.m_local->UpdateClientSideAnimation( );
		m_abs_yaw = state->m_goal_feet_yaw;

		g_cl.m_update_anims = false;

		g_cl.m_local->GetAnimLayers( backup_animlayer );
		g_cl.m_local->GetPoseParameters( backup_poses );
	}

	auto ApplyLocalPlayerModifications = [&]( ) -> void {
		if ( !backup_animlayer || !backup_poses )
			return;

		if ( backup_animlayer )
			backup_animlayer[12].m_weight = 0.f;
	};
	ApplyLocalPlayerModifications( );


	if ( backup_animlayer )
		g_cl.m_local->SetAnimLayers( backup_animlayer );
	if ( backup_poses )
		g_cl.m_local->SetPoseParameters( backup_poses );

	g_csgo.m_globals->m_curtime = backup_curtime;
	g_csgo.m_globals->m_frametime = backup_frametime;
}

void Client::UpdateInformation( ) {
	if ( g_cl.m_lag > 0 )
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if ( !state )
		return;

	m_anim_frame = g_csgo.m_globals->m_curtime - m_anim_time;
	m_anim_time = g_csgo.m_globals->m_curtime;

	m_angle = g_cl.m_cmd->m_view_angles;

	math::clamp( m_angle.x, -90.f, 90.f );
	m_angle.normalize( );

	g_cl.m_local->m_flLowerBodyYawTarget( ) = m_body;

	if ( state->m_ground ) {
		const float CSGO_ANIM_LOWER_REALIGN_DELAY = 1.1f;

		if ( state->m_speed > 0.1f || fabsf( state->m_fall_velocity ) > 100.f ) {
			g_cl.m_body_pred = g_cl.m_anim_time + ( CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f );
			g_cl.m_body = m_angle.y;
		}

		else {
			if ( g_cl.m_anim_time > g_cl.m_body_pred ) {
				g_cl.m_body_pred = g_cl.m_anim_time + CSGO_ANIM_LOWER_REALIGN_DELAY;
				g_cl.m_body = m_angle.y;
			}
		}
	}

	m_rotation = g_cl.m_local->m_angAbsRotation( );
	m_speed = state->m_speed;
	m_ground = state->m_ground;
}

void Client::print( const std::string text, ... ) {
	va_list     list;
	int         size;
	std::string buf;

	if ( text.empty( ) )
		return;

	va_start( list, text );

	size = std::vsnprintf( 0, 0, text.c_str( ), list );

	buf.resize( size );

	std::vsnprintf( buf.data( ), size + 1, text.c_str( ), list );

	va_end( list );

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf( g_menu.main.config.menu_color.get( ), XOR( "__stdcall " ) );
	g_csgo.m_cvar->ConsoleColorPrintf( colors::white, buf.c_str( ) );
}

bool Client::CanFireWeapon( ) {
	if ( !m_player_fire )
		return false;

	if ( m_weapon_type == WEAPONTYPE_GRENADE )
		return false;

	if ( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1( ) < 1 )
		return false;

	if ( ( m_weapon_id == GLOCK || m_weapon_id == FAMAS ) && m_weapon->m_iBurstShotsRemaining( ) > 0 ) {
		if ( g_csgo.m_globals->m_curtime >= m_weapon->m_fNextBurstShot( ) )
			return true;
	}

	if ( m_weapon_id == REVOLVER ) {
		int act = m_weapon->m_Activity( );

		if ( !m_revolver_fire ) {
			if ( ( act == 185 || act == 193 ) && m_revolver_cock == 0 )
				return g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack( );

			return false;
		}
	}

	if ( g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack( ) )
		return true;

	return false;
}

void Client::UpdateRevolverCock( ) {
	m_revolver_fire = false;

	if ( m_revolver_cock == -1 )
		m_revolver_cock = 0;

	if ( m_weapon_id != REVOLVER || m_weapon->m_iClip1( ) < 1 || !m_player_fire || g_csgo.m_globals->m_curtime < m_weapon->m_flNextPrimaryAttack( ) ) {
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	int shoot = (int)( 0.25f / ( std::round( g_csgo.m_globals->m_interval * 1000000.f ) / 1000000.f ) );

	m_revolver_query = shoot - 1;

	if ( m_revolver_query == m_revolver_cock ) {
		m_revolver_cock = -1;

		m_revolver_fire = true;
	}

	else {
		if ( g_menu.main.config.mode.get( ) == 0 && m_revolver_query > m_revolver_cock )
			m_cmd->m_buttons |= IN_ATTACK;

		if ( m_cmd->m_buttons & IN_ATTACK )
			m_revolver_cock++;

		else m_revolver_cock = 0;
	}

	if ( m_revolver_cock > 0 )
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences( ) {
	if ( !g_csgo.m_net )
		return;

	if ( m_sequences.empty( ) || g_csgo.m_net->m_in_seq > m_sequences.front( ).m_seq ) {
		m_sequences.emplace_front( g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq );
	}

	while ( m_sequences.size( ) > 128 )
		m_sequences.pop_back( );
}

void Client::MouseFix( CUserCmd* cmd ) {
	static ang_t delta_viewangles{ };
	ang_t delta = cmd->m_view_angles - delta_viewangles;

	static ConVar* sensitivity = g_csgo.m_cvar->FindVar( HASH( "sensitivity" ) );

	if ( delta.x != 0.f ) {
		static ConVar* m_pitch = g_csgo.m_cvar->FindVar( HASH( "m_pitch" ) );

		int final_dy = static_cast<int>( ( delta.x / m_pitch->GetFloat( ) ) / sensitivity->GetFloat( ) );
		if ( final_dy <= 32767 ) {
			if ( final_dy >= -32768 ) {
				if ( final_dy >= 1 || final_dy < 0 ) {
					if ( final_dy <= -1 || final_dy > 0 )
						final_dy = final_dy;
					else
						final_dy = -1;
				}
				else {
					final_dy = 1;
				}
			}
			else {
				final_dy = 32768;
			}
		}
		else {
			final_dy = 32767;
		}

		cmd->m_mousedy = static_cast<short>( final_dy );
	}

	if ( delta.y != 0.f ) {
		static ConVar* m_yaw = g_csgo.m_cvar->FindVar( HASH( "m_yaw" ) );

		int final_dx = static_cast<int>( ( delta.y / m_yaw->GetFloat( ) ) / sensitivity->GetFloat( ) );
		if ( final_dx <= 32767 ) {
			if ( final_dx >= -32768 ) {
				if ( final_dx >= 1 || final_dx < 0 ) {
					if ( final_dx <= -1 || final_dx > 0 )
						final_dx = final_dx;
					else
						final_dx = -1;
				}
				else {
					final_dx = 1;
				}
			}
			else {
				final_dx = 32768;
			}
		}
		else {
			final_dx = 32767;
		}

		cmd->m_mousedx = static_cast<short>( final_dx );
	}

	delta_viewangles = cmd->m_view_angles;
}

int Client::GetNextUpdate( ) const {
	auto tick = game::TIME_TO_TICKS( m_body_pred - game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) * g_csgo.m_globals->m_interval ) );
	return tick;
}