#include "includes.h"

Shots g_shots{ };

void Shots::OnShotFire(Player* target, float damage, int bullets, LagRecord* record, int hitbox) {

	// iterate all bullets in this shot.
	for (int i{ }; i < bullets; ++i) {
		// setup new shot data.
		ShotRecord shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
		shot.m_lat = g_cl.m_latency;
		shot.m_damage = damage;
		shot.m_hitbox = hitbox;
		shot.m_pos = g_cl.m_local->GetShootPosition();

		// we are not shooting manually.
		// and this is the first bullet, only do this once.
		if (target && i == 0) {
			// increment total shots on this player.
			AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
			if (data)
				++data->m_shots;
		}

		// add to tracks.
		m_shots.push_front(shot);
	}

	// no need to keep an insane amount of shots.
	while (m_shots.size() > 128)
		m_shots.pop_back();
}

void Shots::OnImpact(IGameEvent* evt) {
	int        attacker;
	vec3_t     pos, dir, start, end;
	float      time;
	CGameTrace trace;

	if (!evt || !g_cl.m_local)
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	pos = {
		evt->m_keys->FindKey(HASH("x"))->GetFloat(),
		evt->m_keys->FindKey(HASH("y"))->GetFloat(),
		evt->m_keys->FindKey(HASH("z"))->GetFloat()
	};

	time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	if (g_menu.main.visuals.impact_beams.get())
		m_vis_impacts.push_back({ pos, g_cl.m_local->GetShootPosition(), g_cl.m_local->m_nTickBase() });

	if (m_shots.empty())
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	for (auto& s : m_shots) {

		if (s.m_matched)
			continue;

		float predicted = s.m_time + s.m_lat;

		float delta = std::abs(time - predicted);

		// fuck this.
		if (delta > 1.f)
			continue;

		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}
	}

	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	shot->m_matched = true;

	ImpactRecord impact;
	impact.m_shot = shot;
	impact.m_tick = g_cl.m_local->m_nTickBase();
	impact.m_pos = pos;

	m_impacts.push_front(impact);

	while (m_impacts.size() > 128)
		m_impacts.pop_back();

	if (g_menu.main.config.mode.get() == 1)
		return;

	Player* target = shot->m_target;
	if (!target)
		return;

	if (!target->alive())
		return;

	AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
	if (!data)
		return;

	if (!shot->m_record->m_bones)
		return;

	BackupRecord backup;
	backup.store(target);

	shot->m_record->cache();

	start = shot->m_pos;

	dir = (pos - start).normalized();

	end = start + (dir * 8192.f);

	g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT, target, &trace);

	std::string info = tfm::format(XOR("pdmg: %s - phb: %s - choke: %s - delta: %s"), shot->m_damage, shot->m_hitbox, shot->m_record->m_lag, match.delta);
	if (shot->m_record->m_broke_lc) {
		g_notify.add(tfm::format(XOR("missed shot (r: broke lc - %s)\n"), info));
	}
	else if (!trace.m_entity || !trace.m_entity->IsPlayer()) {
		g_notify.add(tfm::format(XOR("missed shot (r: spread - %s)\n"), info));
	}
	else if (trace.m_entity == target) {
		g_notify.add(tfm::format(XOR("missed shot (r: unknown - %s)\n"), info));

		size_t mode = shot->m_record->m_mode;

		++data->m_missed_shots;
	}

	bool ut = g_menu.main.config.mode.get() == 1 && g_menu.main.aimbot.nospread.get();

	if (g_aimbot.CanHit(start, end, shot->m_record, shot->m_hitbox, false, nullptr))
		canhit = true;

	if (!g_aimbot.CanHit(start, end, shot->m_record, shot->m_hitbox, false, nullptr))
	{
		if (!ut)
		{
			//g_notify.add(XOR("shot missed due to spread\n"));
		}
		// bind
		iHit = false;
	}
	else
		iHit = true, ++data->m_missed_shots;

	// restore player to his original state.
	backup.restore(target);
}

void Shots::OnHurt(IGameEvent* evt) {
	int         attacker, victim, group, hp;
	float       damage;
	std::string name;

	if (!evt || !g_cl.m_local)
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
	victim = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if (attacker < 1 || attacker > 64 || victim < 1 || victim > 64)
		return;

	// we were not the attacker or we hurt ourselves.
	else if (attacker != g_csgo.m_engine->GetLocalPlayer() || victim == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey(HASH("hitgroup"))->GetInt();

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if (group == HITGROUP_GEAR)
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >(victim);
	if (!target)
		return;

	// get player info.
	player_info_t info;
	if (!g_csgo.m_engine->GetPlayerInfo(victim, &info))
		return;

	// get player name;
	name = std::string(info.m_name).substr(0, 24);

	// get damage reported by the server.
	damage = (float)evt->m_keys->FindKey(HASH("dmg_health"))->GetInt();

	// get remaining hp.
	hp = evt->m_keys->FindKey(HASH("health"))->GetInt();

	// setup headshot marker
	if (group == HITGROUP_HEAD)
		iHeadshot = true;
	else
		iHeadshot = false;

	// hitmarker.
	if (g_menu.main.misc.hitmarker.get()) {
		g_visuals.m_hit_duration = 2.f;
		g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
		g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;

		// bind to draw
		iHitDmg = damage;

		// get interpolated origin.
		iPlayerOrigin = target->GetAbsOrigin();

		// get hitbox bounds.
		// hehe boy
		target->ComputeHitboxSurroundingBox(&iPlayermins, &iPlayermaxs);

		// correct x and y coordinates.
		iPlayermins = { iPlayerOrigin.x, iPlayerOrigin.y, iPlayermins.z };
		iPlayermaxs = { iPlayerOrigin.x, iPlayerOrigin.y, iPlayermaxs.z + 8.f };

		g_csgo.m_sound->EmitAmbientSound(XOR("buttons/arena_switch_press_02.wav"), 1.f);
	}

	// print this shit.
	if (g_menu.main.misc.notifications.get(1)) {
		std::string out = tfm::format(XOR("Hit %s in the %s for %i damage (%i health remaining)\n"), name, m_groups[group], (int)damage, hp);
		g_notify.add(out);
	}

	// if we hit a player, mark vis impacts.
	if (!m_vis_impacts.empty()) {
		for (auto& i : m_vis_impacts) {
			if (i.m_tickbase == g_cl.m_local->m_nTickBase())
				i.m_hit_player = true;
		}
	}

	// no impacts to match.
	if (m_impacts.empty())
		return;

	ImpactRecord* impact{ nullptr };

	// iterate stored impacts.
	for (auto& i : m_impacts) {

		// this impact doesnt match with our current hit.
		if (i.m_tick != g_cl.m_local->m_nTickBase())
			continue;

		// wrong player.
		if (i.m_shot->m_target != target)
			continue;

		// shit fond.
		impact = &i;
		break;
	}

	// no impact matched.
	if (!impact)
		return;

	// setup new data for hit track and push to hit track.
	HitRecord hit;
	hit.m_impact = impact;
	hit.m_group = group;
	hit.m_damage = damage;

	auto hit_matrix = impact->m_shot->m_record->m_bones;
	if (hp <= 0)
		g_chams.AddMatrix(target, hit_matrix);

	if (group == HITGROUP_GENERIC)
		return;

	//g_cl.print( "hit %x time: %f lat: %f dmg: %f\n", impact->m_shot->m_record, impact->m_shot->m_time, impact->m_shot->m_lat, impact->m_shot->m_damage );

	m_hits.push_front(hit);

	while (m_hits.size() > 128)
		m_hits.pop_back();

	AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
	if (!data)
		return;

	// we hit, reset missed shots counter.
	data->m_missed_shots = 0;

	size_t mode = impact->m_shot->m_record->m_mode;

	// if we hit head
	// shoot at this 5 more times.
	if (group == HITGROUP_HEAD) {
		LagRecord* record = hit.m_impact->m_shot->m_record;

		//switch( record->m_mode ) {
		//case Resolver::Modes::RESOLVE_STAND:
		//	data->m_prefer_stand.clear( );
		//	data->m_prefer_stand.push_front( math::NormalizedAngle( record->m_eye_angles.y - record->m_lbyt ) );
		//	break;

		//case Resolver::Modes::RESOLVE_AIR:
		//	if( g_menu.main.config.mode.get( ) == 1 ) {
		//		data->m_prefer_air.clear( );
		//		data->m_prefer_air.push_front( math::NormalizedAngle( record->m_eye_angles.y - record->m_away ) );
		//
		//		g_notify.add( tfm::format( "air hit %f\n", data->m_prefer_air.front( ) ) );
		//	}
		//
		//	break;

		//default:
		//	break;
		//}
	}
}