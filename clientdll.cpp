#include "includes.h"

void Hooks::LevelInitPreEntity( const char* map ) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	if (g_csgo.m_globals->m_interval != 0) {
		g_csgo.cl_updaterate->SetValue( rate );
		g_csgo.cl_cmdrate->SetValue( rate );
	}

	g_aimbot.reset( );
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >( CHLClient::LEVELINITPREENTITY )( this, map );
}

void Hooks::LevelInitPostEntity( ) {
	g_cl.OnMapload( );

	// from L3D.
	g_cl.m_latency = 0.f;
	g_cl.m_lerp = 0.f;
	g_cl.m_saved_command.clear( );
	g_shots.m_shots.clear( );
	g_hooks.m_net_channel.reset( );
	g_hooks.m_client_state.reset( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >( CHLClient::LEVELINITPOSTENTITY )( this );
}

void Hooks::LevelShutdown( ) {
	// from L3D.
	g_cl.m_latency = 0.f;
	g_cl.m_lerp = 0.f;
	g_cl.m_saved_command.clear( );
	g_shots.m_shots.clear( );
	g_hooks.m_net_channel.reset( );
	g_hooks.m_client_state.reset( );

	g_aimbot.reset( );

	g_cl.m_local = nullptr;
	g_cl.m_weapon = nullptr;
	g_cl.m_processing = false;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end = false;

	g_cl.m_sequences.clear( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >( CHLClient::LEVELSHUTDOWN )( this );
}

void WriteUsercmd( bf_write* buf, CUserCmd* in, CUserCmd* out )
{
	static auto WriteUsercmdF = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D" ) );

	__asm
	{
		mov ecx, buf
		mov edx, in
		push out
		call WriteUsercmdF
		add esp, 4
	}
}


bool Hooks::WriteUsercmdDeltaToBuffer( int slot, bf_write* buf, int from, int to, bool isnewcommand )
{

	if (g_cl.m_processing && g_csgo.m_engine->IsConnected( ) && g_csgo.m_engine->IsInGame( )) {

		if (g_csgo.m_gamerules->m_bFreezePeriod( ))
			return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >( CHLClient::USRCMDTODELTABUFFER )( this, slot, buf, from, to, isnewcommand );

		if (g_cl.m_tick_to_shift <= 0 || g_csgo.m_cl->m_choked_commands > 3)
			return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >( CHLClient::USRCMDTODELTABUFFER )( this, slot, buf, from, to, isnewcommand );

		if (from != -1)
			return true;

		uintptr_t stackbase;
		__asm mov stackbase, ebp;
		CCLCMsg_Move_t* msg = reinterpret_cast<CCLCMsg_Move_t*>( stackbase + 0xFCC );
		auto net_channel = *reinterpret_cast <INetChannel**> ( reinterpret_cast <uintptr_t> ( g_csgo.m_cl ) + 0x9C );
		int32_t new_commands = msg->new_commands;

		int32_t next_cmdnr = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands + 1;

		int32_t total_new_commands = std::min( g_cl.m_tick_to_shift, 16 ); // depended on server restriction
		g_cl.m_tick_to_shift -= total_new_commands;

		from = -1;
		msg->new_commands = total_new_commands;
		msg->backup_commands = 0;

		for (to = next_cmdnr - new_commands + 1; to <= next_cmdnr; to++) {
			if (!g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >( CHLClient::USRCMDTODELTABUFFER )( this, slot, buf, from, to, isnewcommand ))
				return false;

			from = to;
		}

		CUserCmd* last_realCmd = g_csgo.m_input->GetUserCmd( slot, from );
		CUserCmd fromCmd;

		if (last_realCmd)
			fromCmd = *last_realCmd;

		CUserCmd toCmd = fromCmd;
		toCmd.m_command_number++;
		toCmd.m_tick++;

		for (int i = new_commands; i <= total_new_commands; i++) {
			WriteUsercmd( buf, &toCmd, &fromCmd );
			fromCmd = toCmd;
			toCmd.m_command_number++;
			toCmd.m_tick++;
		}
		return true;

	}
	else
		return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >( CHLClient::USRCMDTODELTABUFFER )( this, slot, buf, from, to, isnewcommand );
}

void Hooks::FrameStageNotify( Stage_t stage ) {
	// save stage.
	if (stage != FRAME_START)
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	if (stage == FRAME_RENDER_START) {
		g_cl.UpdateAnimations( );

		g_cl.ThirdPersonFSN( );

		g_visuals.ImpactData( );

		g_visuals.DrawBeams( );
	}

	g_resolver.collect_wall_detect( stage );

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >( CHLClient::FRAMESTAGENOTIFY )( this, stage );

	if (stage == FRAME_RENDER_START) {
		// ...
	}

	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		// restore non-compressed netvars.
		// g_netdata.apply( );

		g_skins.think( );
	}

	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
		g_visuals.NoSmoke( );
	}

	else if (stage == FRAME_NET_UPDATE_END) {
		// restore non-compressed netvars.
		g_netdata.apply( );

		//g_cl.UpdateLocal( );
		g_cl.animate( );

		// update all players.
		for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
			if (!player || player->m_bIsLocalPlayer( ))
				continue;

			AimPlayer* data = &g_aimbot.m_players[i - 1];
			data->OnNetUpdate( player );
		}
	}
}