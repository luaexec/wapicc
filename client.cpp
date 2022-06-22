#include "includes.h"

Client g_cl{ };

char username[33] = "\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x90";

ulong_t __stdcall Client::init(void* arg) {
	g_cl.m_user = XOR("wapicc");

	if (!g_csgo.init())
		return 0;

	return 1;
}

void Client::DrawHUD() {
	if (!g_menu.main.misc.watermark.get())
		return;

	if (!g_csgo.m_engine->IsInGame())
		return;

	int ms = std::max(0, (int)std::round(g_cl.m_latency * 1000.f));

	int rate = (int)std::round(1.f / g_csgo.m_globals->m_interval);
	Color color = g_gui.m_color;

	colors::accent = g_gui.m_color;
}

void Client::UnlockHiddenConvars()
{
	if (!g_csgo.m_cvar)
		return;

	auto p = **reinterpret_cast<ConVar***>(g_csgo.m_cvar + 0x34);
	for (auto c = p->m_next; c != nullptr; c = c->m_next) {
		c->m_flags &= ~FCVAR_DEVELOPMENTONLY;
		c->m_flags &= ~FCVAR_HIDDEN;
	}
}

void Client::ClanTag()
{
}

void Client::Skybox()
{
	static auto sv_skyname = g_csgo.m_cvar->FindVar(HASH("sv_skyname"));
	if (g_menu.main.misc.skyboxchange.get()) {
		switch (g_menu.main.misc.skybox.get()) {
		case 0: //Tibet
			//sv_skyname->SetValue("cs_tibet");
			sv_skyname->SetValue(XOR("cs_tibet"));
			break;
		case 1: //Embassy
			//sv_skyname->SetValue("embassy");
			sv_skyname->SetValue(XOR("embassy"));
			break;
		case 2: //Italy
			//sv_skyname->SetValue("italy");
			sv_skyname->SetValue(XOR("italy"));
			break;
		case 3: //Daylight 1
			//sv_skyname->SetValue("sky_cs15_daylight01_hdr");
			sv_skyname->SetValue(XOR("sky_cs15_daylight01_hdr"));
			break;
		case 4: //Cloudy
			//sv_skyname->SetValue("sky_csgo_cloudy01");
			sv_skyname->SetValue(XOR("sky_csgo_cloudy01"));
			break;
		case 5: //Night 1
			sv_skyname->SetValue(XOR("sky_csgo_night02"));
			break;
		case 6: //Night 2
			//sv_skyname->SetValue("sky_csgo_night02b");
			sv_skyname->SetValue(XOR("sky_csgo_night02b"));
			break;
		case 7: //Night Flat
			//sv_skyname->SetValue("sky_csgo_night_flat");
			sv_skyname->SetValue(XOR("sky_csgo_night_flat"));
			break;
		case 8: //Day HD
			//sv_skyname->SetValue("sky_day02_05_hdr");
			sv_skyname->SetValue(XOR("sky_day02_05_hdr"));
			break;
		case 9: //Day
			//sv_skyname->SetValue("sky_day02_05");
			sv_skyname->SetValue(XOR("sky_day02_05"));
			break;
		case 10: //Rural
			//sv_skyname->SetValue("sky_l4d_rural02_ldr");
			sv_skyname->SetValue(XOR("sky_l4d_rural02_ldr"));
			break;
		case 11: //Vertigo HD
			//sv_skyname->SetValue("vertigo_hdr");
			sv_skyname->SetValue(XOR("vertigo_hdr"));
			break;
		case 12: //Vertigo Blue HD
			//sv_skyname->SetValue("vertigoblue_hdr");
			sv_skyname->SetValue(XOR("vertigoblue_hdr"));
			break;
		case 13: //Vertigo
			//sv_skyname->SetValue("vertigo");
			sv_skyname->SetValue(XOR("vertigo"));
			break;
		case 14: //Vietnam
			//sv_skyname->SetValue("vietnam");
			sv_skyname->SetValue(XOR("vietnam"));
			break;
		case 15: //Dusty Sky
			//sv_skyname->SetValue("sky_dust");
			sv_skyname->SetValue(XOR("sky_dust"));
			break;
		case 16: //Jungle
			sv_skyname->SetValue(XOR("jungle"));
			break;
		case 17: //Nuke
			sv_skyname->SetValue(XOR("nukeblank"));
			break;
		case 18: //Office
			sv_skyname->SetValue(XOR("office"));
			//game::SetSkybox(XOR("office"));
			break;
		default:
			break;
		}
	}

	float destiny = g_menu.main.visuals.Fogdensity.get() / 100.f;

	static const auto fog_enable = g_csgo.m_cvar->FindVar(HASH("fog_enable"));
	fog_enable->SetValue(1); //Включает туман на карте если он выключен по дефолту
	static const auto fog_override = g_csgo.m_cvar->FindVar(HASH("fog_override"));
	fog_override->SetValue(g_menu.main.visuals.FogOverride.get()); // Разрешает кастомизацию тумана
	static const auto fog_color = g_csgo.m_cvar->FindVar(HASH("fog_color"));
	fog_color->SetValue(std::string(std::to_string(g_menu.main.visuals.FogColor.get().r()) + " " + std::to_string(g_menu.main.visuals.FogColor.get().g()) + " " + std::to_string(g_menu.main.visuals.FogColor.get().b())).c_str()); //Цвет тумана rgb
	static const auto fog_start = g_csgo.m_cvar->FindVar(HASH("fog_start"));
	fog_start->SetValue(g_menu.main.visuals.FogStart.get()); // Дистанция с которой туман появляется
	static const auto fog_end = g_csgo.m_cvar->FindVar(HASH("fog_end"));
	fog_end->SetValue(g_menu.main.visuals.FogEnd.get()); // Дистанция с которой туман пропадает
	static const auto fog_destiny = g_csgo.m_cvar->FindVar(HASH("fog_maxdensity"));
	fog_destiny->SetValue(destiny); //Максимальная насыщенность тумана(0-1)
}

void Client::KillFeed() {
	if (!g_menu.main.misc.killfeed.get())
		return;

	if (!g_csgo.m_engine->IsInGame())
		return;

	KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement(HASH("SFHudDeathNoticeAndBotStatus"));
	if (!feed)
		return;

	int size = feed->notices.Count();
	if (!size)
		return;

	for (int i{ }; i < size; ++i) {
		NoticeText_t* notice = &feed->notices[i];

		if (notice->fade == 1.5f)
			notice->fade = FLT_MAX;
	}
}

void Client::OnPaint() {
	g_csgo.m_engine->GetScreenSize(m_width, m_height);

	// render stuff.
	g_visuals.think();
	g_grenades.paint();
	g_notify.think();

	DrawHUD();

	g_visuals.IndicateAngles();

	KillFeed();

	g_gui.think();
}

void Client::OnMapload() {
	g_netvars.SetupClassData();

	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	g_visuals.ModulateWorld();

	g_skins.load();

	m_sequences.clear();

	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo();

	if (g_csgo.m_net) {
		g_hooks.m_net_channel.reset();
		g_hooks.m_net_channel.init(g_csgo.m_net);
		g_hooks.m_net_channel.add(INetChannel::PROCESSPACKET, util::force_cast(&Hooks::ProcessPacket));
		g_hooks.m_net_channel.add(INetChannel::SENDDATAGRAM, util::force_cast(&Hooks::SendDatagram));
	}
}

void Client::StartMove(CUserCmd* cmd) {
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;

	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());
	if (!m_local)
		return;

	m_max_lag = (m_local->m_fFlags() & FL_ONGROUND) ? 16 : 15;
	m_lag = g_csgo.m_cl->m_choked_commands;
	m_lerp = game::GetClientInterpAmount();
	m_latency = g_csgo.m_net->GetLatency(INetChannel::FLOW_OUTGOING);
	math::clamp(m_latency, 0.f, 1.f);
	m_latency_ticks = game::TIME_TO_TICKS(m_latency);
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + m_latency_ticks;

	m_processing = m_local && m_local->alive();
	if (!m_processing)
		return;

	g_inputpred.update();

	m_flags = m_local->m_fFlags();

	m_shot = false;
}

void Client::BackupPlayers(bool restore) {
	if (restore) {
		for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

			if (!g_aimbot.IsValidTarget(player))
				continue;

			g_aimbot.m_backup[i - 1].restore(player);
		}
	}

	else {
		for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

			if (!g_aimbot.IsValidTarget(player))
				continue;

			g_aimbot.m_backup[i - 1].store(player);
		}
	}
}

void Client::DoMove() {
	penetration::PenetrationOutput_t tmp_pen_data{ };

	m_strafe_angles = m_cmd->m_view_angles;

	g_movement.JumpRelated();
	g_movement.Strafe();
	g_movement.FakeWalk();
	g_movement.AutoPeek();

	g_inputpred.run();

	m_cmd->m_view_angles = m_view_angles;

	math::AngleVectors(m_view_angles, &m_forward_dir);

	m_shoot_pos = m_local->GetShootPosition();

	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;

	m_weapon = m_local->GetActiveWeapon();

	if (m_weapon) {
		m_weapon_info = m_weapon->GetWpnData();
		m_weapon_id = m_weapon->m_iItemDefinitionIndex();
		m_weapon_type = m_weapon_info->m_weapon_type;

		if (m_weapon_type != WEAPONTYPE_GRENADE)
			m_weapon->UpdateAccuracyPenalty();

		if (m_weapon_type != WEAPONTYPE_KNIFE && m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE) {
			penetration::PenetrationInput_t in;
			in.m_from = m_local;
			in.m_target = nullptr;
			in.m_pos = m_shoot_pos + (m_forward_dir * m_weapon_info->m_range);
			in.m_damage = 1.f;
			in.m_damage_pen = 1.f;
			in.m_can_pen = true;

			penetration::run(&in, &tmp_pen_data);
		}

		m_pen_data = tmp_pen_data;

		m_player_fire = g_csgo.m_globals->m_curtime >= m_local->m_flNextAttack() && !g_csgo.m_gamerules->m_bFreezePeriod() && !(g_cl.m_flags & FL_FROZEN);

		UpdateRevolverCock();
		m_weapon_fire = CanFireWeapon();
	}

	if (g_input.GetKeyState(g_menu.main.misc.last_tick_defuse.get()) && g_visuals.m_c4_planted) {
		float defuse = (m_local->m_bHasDefuser()) ? 5.f : 10.f;
		float remaining = g_visuals.m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
		float dt = remaining - defuse - (g_cl.m_latency / 2.f);

		m_cmd->m_buttons &= ~IN_USE;
		if (dt <= game::TICKS_TO_TIME(2))
			m_cmd->m_buttons |= IN_USE;
	}

	g_grenades.think();

	g_hvh.SendPacket();

	g_aimbot.think();

	g_hvh.AntiAim();
}

void Client::EndMove(CUserCmd* cmd) {
	UpdateInformation();

	if (g_menu.main.config.mode.get() == 0)
		m_cmd->m_view_angles.SanitizeAngle();

	g_movement.FixMove(cmd, m_strafe_angles);
	g_movement.MoonWalk(cmd);

	if (*m_packet) {
		g_hvh.m_step_switch = (bool)g_csgo.RandomInt(0, 1);

		m_old_lag = m_lag;

		m_radar = cmd->m_view_angles;
		m_radar.normalize();

		vec3_t cur = m_local->m_vecOrigin();

		vec3_t prev = m_net_pos.empty() ? cur : m_net_pos.front().m_pos;

		m_lagcomp = (cur - prev).length_sqr() > 4096.f;

		m_net_pos.emplace_front(g_csgo.m_globals->m_curtime, cur);
	}

	m_old_packet = *m_packet;
	m_old_shot = m_shot;
}

void Client::OnTick(CUserCmd* cmd) {
	if (g_menu.main.misc.ranks.get() && cmd->m_buttons & IN_SCORE) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll(&msg);
	}

	StartMove(cmd);

	if (!m_processing)
		return;

	BackupPlayers(false);

	DoMove();

	EndMove(cmd);

	BackupPlayers(true);

	g_inputpred.restore();
}

void Client::SetAngles() {
	if (!g_cl.m_local || !g_cl.m_processing)
		return;

	//g_cl.m_local->m_fEffects() |= EF_NOINTERP;

	// apply the rotation.
	g_cl.m_local->SetAbsAngles(m_rotation);
	g_cl.m_local->m_angRotation() = m_rotation;
	g_cl.m_local->m_angNetworkAngles() = m_rotation;

	// set radar angles.
	if (g_csgo.m_input->CAM_IsThirdPerson())
		g_csgo.m_prediction->SetLocalViewAngles(m_radar);
}

void Client::UpdateAnimations() {
	if (!g_cl.m_local || !g_cl.m_processing)
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	// prevent model sway on player.
	g_cl.m_local->m_AnimOverlay()[12].m_weight = 0.f;

	m_poses[pose_params::jump_fall] = 0.f;

	// update animations with last networked data.
	g_cl.m_local->SetPoseParameters(g_cl.m_poses);

	// update abs yaw with last networked abs yaw.
	g_cl.m_local->SetAbsAngles(ang_t(0.f, g_cl.m_abs_yaw, 0.f));
}

void Client::UpdateInformation() {
	if (g_cl.m_lag > 0)
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	// update time.
	m_anim_frame = g_csgo.m_globals->m_curtime - m_anim_time;
	m_anim_time = g_csgo.m_globals->m_curtime;

	// current angle will be animated.
	m_angle = g_cl.m_cmd->m_view_angles;

	math::clamp(m_angle.x, -90.f, 90.f);
	m_angle.normalize();

	// write angles to model.
	g_csgo.m_prediction->SetLocalViewAngles(m_angle);

	// set lby to predicted value.
	g_cl.m_local->m_flLowerBodyYawTarget() = m_body;

	// CCSGOPlayerAnimState::Update, bypass already animated checks.
	if (state->m_frame == g_csgo.m_globals->m_frame)
		state->m_frame -= 1;

	// call original, bypass hook.
	if (g_hooks.m_UpdateClientSideAnimation) //not real fix but why not
		g_hooks.m_UpdateClientSideAnimation(g_cl.m_local);

	// get last networked poses.
	g_cl.m_local->GetPoseParameters(g_cl.m_poses);

	// store updated abs yaw.
	g_cl.m_abs_yaw = state->m_goal_feet_yaw;

	// we landed.
	if (!m_ground && state->m_ground) {
		m_body = m_angle.y;
		m_body_pred = m_anim_time;
	}

	// walking, delay lby update by .22.
	else if (state->m_speed > 0.1f) {
		if (state->m_ground)
			m_body = m_angle.y;

		m_body_pred = m_anim_time + 0.22f;
	}

	// standing update every 1.1s
	else if (m_anim_time > m_body_pred) {
		m_body = m_angle.y;
		m_body_pred = m_anim_time + 1.1f;
	}

	// save updated data.
	m_rotation = g_cl.m_local->m_angAbsRotation();
	m_speed = state->m_speed;
	m_ground = state->m_ground;
}

void Client::UpdateLocal()
{

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	float backup_frametime = g_csgo.m_globals->m_frametime;
	float backup_curtime = g_csgo.m_globals->m_curtime;

	const float v3 = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
	const float v4 = (v3 / g_csgo.m_globals->m_interval) + .5f;

	static float backup_poses[24];
	static C_AnimationLayer backup_animlayer[13];

	g_csgo.m_globals->m_curtime = g_cl.m_local->m_nTickBase() * g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	math::clamp(m_real_angle.x, -90.f, 90.f);
	m_real_angle.normalize();

	// CCSGOPlayerAnimState::Update, bypass already animated checks.
	if (state->m_frame >= v4)
		state->m_frame = v4 - 1;

	if (g_csgo.m_globals->m_curtime != state->m_time)
	{
		g_cl.m_local->UpdateAnimationState(state, vec3_t(m_real_angle.x, m_real_angle.y, m_real_angle.z));
		g_cl.m_local->UpdateClientSideAnimation();
		m_abs_yaw = state->m_goal_feet_yaw;

		g_cl.m_local->GetAnimLayers(backup_animlayer);
		g_cl.m_local->GetPoseParameters(backup_poses);
	}

	auto ApplyLocalPlayerModifications = [&]() -> void {
		// havent got updated layers and poses.
		if (!backup_animlayer || !backup_poses)
			return;

		// prevent model sway on player.
		if (backup_animlayer)
			backup_animlayer[12].m_weight = 0.f;
	};
	ApplyLocalPlayerModifications();

	if (backup_animlayer)
		g_cl.m_local->SetAnimLayers(backup_animlayer);
	if (backup_poses)
		g_cl.m_local->SetPoseParameters(backup_poses);

	g_csgo.m_globals->m_curtime = backup_curtime;
	g_csgo.m_globals->m_frametime = backup_frametime;
}

void Client::print(const std::string text, ...) {
	va_list     list;
	int         size;
	std::string buf;

	if (text.empty())
		return;

	va_start(list, text);

	size = std::vsnprintf(0, 0, text.c_str(), list);

	buf.resize(size);

	std::vsnprintf(buf.data(), size + 1, text.c_str(), list);

	va_end(list);

	g_csgo.m_cvar->ConsoleColorPrintf(colors::accent, XOR("__thiscall "));
	g_csgo.m_cvar->ConsoleColorPrintf(colors::white, buf.c_str());
}

bool Client::CanFireWeapon() {
	// the player cant fire.
	if (!m_player_fire)
		return false;

	if (m_weapon_type == WEAPONTYPE_GRENADE)
		return false;

	// if we have no bullets, we cant shoot.
	if (m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1() < 1)
		return false;

	// do we have any burst shots to handle?
	if ((m_weapon_id == GLOCK || m_weapon_id == FAMAS) && m_weapon->m_iBurstShotsRemaining() > 0) {
		// new burst shot is coming out.
		if (g_csgo.m_globals->m_curtime >= m_weapon->m_fNextBurstShot())
			return true;
	}

	// r8 revolver.
	if (m_weapon_id == REVOLVER) {
		int act = m_weapon->m_Activity();

		// mouse1.
		if (!m_revolver_fire) {
			if ((act == 185 || act == 193) && m_revolver_cock == 0)
				return g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack();

			return false;
		}
	}

	// yeez we have a normal gun.
	if (g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack())
		return true;

	return false;
}

void Client::UpdateRevolverCock() {
	m_revolver_fire = false;

	if (m_revolver_cock == -1)
		m_revolver_cock = 0;

	if (m_weapon_id != REVOLVER || m_weapon->m_iClip1() < 1 || !m_player_fire || g_csgo.m_globals->m_curtime < m_weapon->m_flNextPrimaryAttack()) {
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	int shoot = (int)(0.25f / (std::round(g_csgo.m_globals->m_interval * 1000000.f) / 1000000.f));

	m_revolver_query = shoot - 1;

	if (m_revolver_query == m_revolver_cock) {
		m_revolver_cock = -1;

		m_revolver_fire = true;
	}

	else {
		if (g_menu.main.config.mode.get() == 0 && m_revolver_query > m_revolver_cock)
			m_cmd->m_buttons |= IN_ATTACK;

		if (m_cmd->m_buttons & IN_ATTACK)
			m_revolver_cock++;

		else m_revolver_cock = 0;
	}

	if (m_revolver_cock > 0)
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences() {
	if (!g_csgo.m_net)
		return;

	if (m_sequences.empty() || g_csgo.m_net->m_in_seq > m_sequences.front().m_seq) {
		m_sequences.emplace_front(g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq);
	}

	while (m_sequences.size() > 2048)
		m_sequences.pop_back();
}