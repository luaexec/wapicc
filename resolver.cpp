#include "includes.h"

Resolver g_resolver{};;

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (it->m_shot || it->m_mode == Modes::RESOLVE_BODY || it->m_mode == Modes::RESOLVE_WALK || it->m_mode == Modes::RESOLVE_NONE)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr; // ( first_valid ) ? first_valid : nullptr
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant())
			return current;
	}

	return nullptr;
}

void Resolver::ResolveBodyUpdates(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// from csgo src sdk.
	const float CSGO_ANIM_LOWER_REALIGN_DELAY = 1.1f;

	// first we are gonna start of with some checks.
	if (!g_cl.m_processing) {
		data->m_moved = false;
		return;
	}

	// dont predict on doramnt players.
	if (record->dormant()) {
		return;
	}

	// if we have missed 2 shots lets predict until he moves again.
	if (data->m_body_index >= 2) {
		return;
	}


	bool lby_change = false;

	if (data->m_records.size() >= 2) {
		LagRecord* previous = data->m_records[1].get();

		if (previous && !previous->dormant() && previous->valid()) {
			lby_change = previous->m_body != record->m_body;
		}
	}


	// on ground and not moving.
	if ((record->m_flags & FL_ONGROUND) && record->m_velocity.length() <= 0.1f && (data->m_body_update != FLT_MAX || lby_change)) {
		// lets time our updates.
		if (record->m_sim_time >= data->m_body_update || lby_change) {
			// inform cheat of resolver method.
			record->m_mode = Modes::RESOLVE_BODY;

			// set angles to current LBY.
			record->m_eye_angles.y = record->m_body;

			// set next predicted time, till update.
			data->m_body_update = record->m_sim_time + CSGO_ANIM_LOWER_REALIGN_DELAY;
		}

	}
	else if (record->m_velocity.length() > 0.1f) {
		// set delayed predicted time, after they stop moving.
		data->m_body_update = record->m_sim_time + (CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f);
	}
}

void Resolver::OnBodyUpdate(Player* player, float value) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	float  delta{ std::numeric_limits< float >::max() };
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
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_pred_origin, away);
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

void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.
	if (g_menu.main.config.mode.get() == 1)
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval;
	}

	// this record has a shot on it.
	if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
		if (record->m_lag <= 2)
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if (data->m_records.size() >= 2) {
			LagRecord* previous = data->m_records[1].get();

			if (previous && !previous->dormant())
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

void Resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_velocity.length();

	// if on ground, moving, and not fakewalking.
	if ((record->m_flags & FL_ONGROUND) && speed > 0.1f && !(record->m_fake_walk ))
		record->m_mode = Modes::RESOLVE_WALK;

	// if on ground, not moving or fakewalking.
	if ((record->m_flags & FL_ONGROUND) && (speed <= 0.1f || record->m_fake_walk))
		record->m_mode = Modes::RESOLVE_STAND;

	// if not on ground.
	else if (!(record->m_flags & FL_ONGROUND))
		record->m_mode = Modes::RESOLVE_AIR;
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// mark this record if it contains a shot.
	MatchShot(data, record);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);

	// if we are in nospread mode, force all players pitches to down.
	// TODO; we should check thei actual pitch and up too, since those are the other 2 possible angles.
	// this should be somehow combined into some iteration that matches with the air angle iteration.
	if (g_menu.main.config.mode.get() == 1)
		record->m_eye_angles.x = 90.f;

	// we arrived here we can do the acutal resolve.
	if (record->m_mode == Modes::RESOLVE_WALK)
		ResolveWalk(data, record);

	else if (record->m_mode == Modes::RESOLVE_STAND)
		ResolveStand(data, record);

	else if (record->m_mode == Modes::RESOLVE_AIR)
		ResolveAir(data, record);

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record) {
	// apply lby to eyeangles.
	record->m_eye_angles.y = record->m_player->m_flLowerBodyYawTarget();

	// havent checked this yet in ResolveStand( ... )
	data->m_moved = false;

	data->m_body_index = 0;

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	std::memcpy(&data->m_walk_record, record, sizeof(LagRecord));
}

void Resolver::FindBestAngle(LagRecord* record) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPosition();
	float away = GetAwayAngle(record);
	float best_dist = 0.f;

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away - 180.f);
	angles.emplace_back(away + 90.f);
	angles.emplace_back(away - 90.f);

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPosition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.

		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	// ...
	if (best_dist != best->m_dist) {
		best_dist = best->m_dist;
		record->m_eye_angles.y = best->m_yaw;
	}
}

Resolver::Directions Resolver::HandleDirections(AimPlayer* data) {
	CGameTrace tr;
	CTraceFilterSimple filter{ };

	if (!g_cl.m_processing)
		return Directions::YAW_NONE;

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// get best target based on fov.
	auto origin = data->m_player->m_vecOrigin();
	ang_t view;
	float fov = math::GetFOV(g_cl.m_cmd->m_view_angles, g_cl.m_local->GetShootPosition(), data->m_player->WorldSpaceCenter());

	// set best fov.
	if (fov < target.fov) {
		target.fov = fov;
		target.player = data->m_player;
	}

	// get best player.
	const auto player = target.player;
	if (!player)
		return Directions::YAW_NONE;

	auto& bestOrigin = player->m_vecOrigin();

	// skip this player in our traces.
	filter.SetPassEntity(g_cl.m_local);

	// calculate angle direction from thier best origin to our origin
	ang_t angDirectionAngle;
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - bestOrigin, angDirectionAngle);

	vec3_t forward, right, up;
	math::AngleVectors(angDirectionAngle, &forward, &right, &up);

	auto vecStart = g_cl.m_local->GetShootPosition();
	auto vecEnd = vecStart + forward * 100.0f;

	Ray rightRay(vecStart + right * 35.0f, vecEnd + right * 35.0f), leftRay(vecStart - right * 35.0f, vecEnd - right * 35.0f);

	g_csgo.m_engine_trace->TraceRay(rightRay, MASK_SOLID, &filter, &tr);
	float rightLength = (tr.m_endpos - tr.m_startpos).length();

	g_csgo.m_engine_trace->TraceRay(leftRay, MASK_SOLID, &filter, &tr);
	float leftLength = (tr.m_endpos - tr.m_startpos).length();

	static auto leftTicks = 0;
	static auto rightTicks = 0;
	static auto backTicks = 0;

	if (rightLength - leftLength > 20.0f)
		leftTicks++;
	else
		leftTicks = 0;

	if (leftLength - rightLength > 20.0f)
		rightTicks++;
	else
		rightTicks = 0;

	if (fabs(rightLength - leftLength) <= 20.0f)
		backTicks++;
	else
		backTicks = 0;

	Directions direction = Directions::YAW_NONE;

	if (rightTicks > 10) {
		direction = Directions::YAW_RIGHT;
	}
	else {
		if (leftTicks > 10) {
			direction = Directions::YAW_LEFT;
		}
		else {
			if (backTicks > 10)
				direction = Directions::YAW_BACK;
		}
	}

	return direction;
}

void Resolver::collect_wall_detect(const Stage_t stage)
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_cl.m_local)
		return;

	auto g_pLocalPlayer = g_cl.m_local;

	last_eye_positions.insert(last_eye_positions.begin(), g_pLocalPlayer->m_vecOrigin() + g_pLocalPlayer->m_vecViewOffset());

	if (last_eye_positions.size() > 64)
		last_eye_positions.pop_back();

	auto nci = g_csgo.m_engine->GetNetChannelInfo();
	if (!nci)
		return;

	const int latency_ticks = game::TIME_TO_TICKS(nci->GetLatency(nci->FLOW_OUTGOING));
	auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

	for (auto i = 1; i < g_csgo.m_globals->m_max_clients; i++)
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!player || player == g_pLocalPlayer)
		{
			continue;
		}

		if (!player->enemy(g_pLocalPlayer))
		{
			continue;
		}

		if (!player->alive())
		{
			continue;
		}

		if (player->dormant())
		{
			continue;
		}

		if (player->m_vecVelocity().length_2d() > 0.1f)
		{
			continue;
		}

		if (using_anti_freestand)
		{
			const auto at_target_angle = math::CalcAngle(player->m_vecOrigin(), last_eye);

			const auto eye_pos = player->get_eye_pos();

			const float height = 64;

			vec3_t direction_1, direction_2, direction_3;
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

			const auto left_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_1 * 16.f);
			const auto right_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_2 * 16.f);
			const auto back_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_3 * 16.f);

			anti_freestanding_record.left_damage = penetration::scale(player, left_damage[i], 1.f, HITGROUP_CHEST);
			anti_freestanding_record.right_damage = penetration::scale(player, right_damage[i], 1.f, HITGROUP_CHEST);
			anti_freestanding_record.back_damage = penetration::scale(player, back_damage[i], 1.f, HITGROUP_CHEST);

			Ray ray;
			CGameTrace trace;
			CTraceFilterWorldOnly filter;

			Ray first_ray(left_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.left_fraction = trace.m_fraction;

			Ray second_ray(right_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.right_fraction = trace.m_fraction;

			Ray third_ray(back_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.back_fraction = trace.m_fraction;
		}
	}
}

bool hitPlayer[64];

bool Resolver::AntiFreestanding(Player* entity, AimPlayer* data, float& yaw)
{

	const auto freestanding_record = anti_freestanding_record;

	auto local_player = g_cl.m_local;
	if (!local_player)
		return false;

	const float at_target_yaw = math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;

	if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
		yaw = at_target_yaw;

	auto set = false;

	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
	{
		if (freestanding_record.right_fraction < freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw + 125.f;
		}
		else if (freestanding_record.right_fraction > freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw - 73.f;
		}
		else {
			yaw = at_target_yaw;
		}
	}
	else
	{
		if (freestanding_record.left_damage > freestanding_record.right_damage) {
			yaw = at_target_yaw + 130.f;
			set = true;
		}
		else
			yaw = at_target_yaw;
	}

	return true;
}

void Resolver::ResolveStand(AimPlayer* data, LagRecord* record) {

	// get predicted away angle for the player.
	float away = GetAwayAngle(record);

	// pointer for easy access.
	LagRecord* move = &data->m_walk_record;

	// we have a valid moving record.
	if (move->m_sim_time > 0.f) {
		// lets check if we can collect move data.
		if (!data->m_moved) {
			vec3_t delta = move->m_origin - record->m_origin;
			// check if moving record is close.
			if (delta.length() <= 128.f) {
				// indicate that we are using the moving lby.
				data->m_moved = true;
			}
		}
	}


	if (data->m_moved) {
		float diff = math::NormalizedAngle(record->m_body - move->m_body);
		const Directions direction = HandleDirections(data);

		// last move is valid.
		if (IsLastMoveValid(record, move->m_body) && !record->m_fake_walk && fabsf(move->m_body - record->m_body) >= 36.f) {
			record->m_mode = Modes::RESOLVE_LM;

			// if these are the case then it is 100%
			if (direction == Directions::YAW_LEFT)
				record->m_eye_angles.y = away + 90.f;
			else if (direction == Directions::YAW_RIGHT)
				record->m_eye_angles.y = away - 90.f;

			// if not just lastmove resolve
			else
				record->m_eye_angles.y = move->m_body;
		}
		else {

			if (data->m_missed_shots < 1) {
				// set our resolver mode
				record->m_mode = Modes::RESOLVE_FREESTAND;

				if (direction == Directions::YAW_LEFT)
					record->m_eye_angles.y = away + 90.f;
				else if (direction == Directions::YAW_RIGHT)
					record->m_eye_angles.y = away - 90.f;

				// seemed to work the best.
				else
					FindBestAngle(record);
			}
			else {
				record->m_eye_angles.y = away - 180.f;
			}
		}
	}
	else if (!data->m_moved)
	{
		const Directions direction = HandleDirections(data);

		record->m_mode = Modes::RESOLVE_FREESTAND;

		if (data->m_missed_shots < 1) {
			// set our resolver mode
			record->m_mode = Modes::RESOLVE_FREESTAND;

			if (direction == Directions::YAW_LEFT)
				record->m_eye_angles.y = away + 90.f;
			else if (direction == Directions::YAW_RIGHT)
				record->m_eye_angles.y = away - 90.f;

			// seemed to work the best.
			else
				FindBestAngle(record);
		}
		else {
			record->m_eye_angles.y = away - 180.f;
		}
	}
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record) {
	float velyaw = math::rad_to_deg(std::atan2(record->m_velocity.y, record->m_velocity.x));

	switch (data->m_shots % 3) {
	case 0:
		record->m_eye_angles.y = velyaw + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = velyaw - 135.f;
		break;

	case 2:
		record->m_eye_angles.y = velyaw + 135.f;
		break;
	}
}

void Resolver::ResolvePoses(Player* player, LagRecord* record) {

}