#include "includes.h"

Movement g_movement{ };;

void Movement::JumpRelated() {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP)
		return;

	if ((g_cl.m_cmd->m_buttons & IN_JUMP) && !(g_cl.m_flags & FL_ONGROUND)) {
		if (config["misc_bhop"].get<bool>( ))
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;
	}
}

void Movement::Strafe() {
	vec3_t velocity;
	float  delta;

	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	if ((g_cl.m_buttons & IN_SPEED) || !(g_cl.m_buttons & IN_JUMP) || (g_cl.m_flags & FL_ONGROUND))
		return;

	velocity = g_cl.m_local->m_vecVelocity();

	m_speed = velocity.length_2d();

	float tmpsmooth = config["misc_strafe_smooth"].get<float>( ) / 100.f;
	m_ideal = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( ( 15.f * tmpsmooth ) / m_speed ) ) : 90.f;
	m_ideal2 = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( ( 30.f * tmpsmooth ) / m_speed ) ) : 90.f;

	math::clamp(m_ideal, 0.f, 90.f);
	math::clamp(m_ideal2, 0.f, 90.f);

	m_mins = g_cl.m_local->m_vecMins();
	m_maxs = g_cl.m_local->m_vecMaxs();

	m_origin = g_cl.m_local->m_vecOrigin();

	m_switch_value *= -1.f;

	++m_strafe_index;

	if ( !config["misc_strafe"].get<bool>( ) )
		return;

	m_circle_yaw = m_old_yaw = g_cl.m_strafe_angles.y;

	static float yaw_add = 0.f;
	static const auto cl_sidespeed = g_csgo.m_cvar->FindVar(HASH("cl_sidespeed"));

	bool back = g_cl.m_cmd->m_buttons & IN_BACK;
	bool forward = g_cl.m_cmd->m_buttons & IN_FORWARD;
	bool right = g_cl.m_cmd->m_buttons & IN_MOVELEFT;
	bool left = g_cl.m_cmd->m_buttons & IN_MOVERIGHT;

	if (back) {
		yaw_add = -180.f;
		if (right)
			yaw_add -= 45.f;
		else if (left)
			yaw_add += 45.f;
	}
	else if (right) {
		yaw_add = 90.f;
		if (back)
			yaw_add += 45.f;
		else if (forward)
			yaw_add -= 45.f;
	}
	else if (left) {
		yaw_add = -90.f;
		if (back)
			yaw_add -= 45.f;
		else if (forward)
			yaw_add += 45.f;
	}
	else {
		yaw_add = 0.f;
	}

	g_cl.m_strafe_angles.y += yaw_add;
	g_cl.m_cmd->m_forward_move = 0.f;
	g_cl.m_cmd->m_side_move = 0.f;

	delta = math::NormalizedAngle(g_cl.m_strafe_angles.y - math::rad_to_deg(atan2(g_cl.m_local->m_vecVelocity().y, g_cl.m_local->m_vecVelocity().x)));

	g_cl.m_cmd->m_side_move = delta > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();

	g_cl.m_strafe_angles.y = math::NormalizedAngle(g_cl.m_strafe_angles.y - delta);
}

void Movement::DoPrespeed() {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	mod = g_csgo.m_globals->m_interval * 128.f;

	min = 2.25f * mod;
	max = 5.f * mod;

	strafe = m_ideal * 2.f;

	math::clamp(strafe, min, max);

	time = 320.f / m_speed;

	math::clamp(time, 0.35f, 1.f);

	step = strafe;

	while (true) {
		if (!WillCollide(time, step) || max <= step)
			break;

		step += 0.2f;
	}

	if (step > max) {
		step = strafe;

		while (true) {
			if (!WillCollide(time, step) || step <= -min)
				break;

			step -= 0.2f;
		}

		if (step < -min) {
			if (GetClosestPlane(plane)) {
				angle = math::rad_to_deg(std::atan2(plane.y, plane.x));
				step = -math::NormalizedAngle(m_circle_yaw - angle) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	m_circle_yaw = math::NormalizedAngle(m_circle_yaw + step);

	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_side_move = (step >= 0.f) ? -450.f : 450.f;
}

bool Movement::GetClosestPlane(vec3_t& plane) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	for (float step{ }; step <= math::pi_2; step += (math::pi / 10.f)) {
		vec3_t end = start;
		end.x += std::cos(step) * dist;
		end.y += std::sin(step) * dist;

		g_csgo.m_engine_trace->TraceRay(Ray(start, end, m_mins, m_maxs), CONTENTS_SOLID, &filter, &trace);

		if (trace.m_fraction < smallest) {
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide(float time, float change) {
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

	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity();
	data.direction = math::rad_to_deg(std::atan2(data.velocity.y, data.velocity.x));

	for (data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval) {
		data.direction = math::NormalizedAngle(data.direction + change);

		float hyp = data.velocity.length_2d();

		data.velocity.x = std::cos(math::deg_to_rad(data.direction)) * hyp;
		data.velocity.y = std::sin(math::deg_to_rad(data.direction)) * hyp;

		if (data.ground)
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat();

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval;

		data.end += (data.velocity * g_csgo.m_globals->m_interval);

		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f)
			return true;
		if (trace.m_startsolid || trace.m_allsolid)
			return true;

		data.start = data.end = trace.m_endpos;

		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		data.ground = trace.hit() && trace.m_plane.m_normal.z > 0.7f;
	}

	return false;
}

void Movement::MoonWalk(CUserCmd* cmd) {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	g_cl.m_cmd->m_buttons |= IN_BULLRUSH;

	if ( config["misc_slide"].get<bool>( ) ) {
		if (cmd->m_side_move < 0.f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}

		if (cmd->m_side_move > 0.f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}

		if (cmd->m_forward_move > 0.f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}

		if (cmd->m_forward_move < 0.f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}
	}

}

void Movement::FixMove(CUserCmd* cmd, const ang_t& wish_angles) {

	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	if (!(g_cl.m_flags & FL_ONGROUND) && cmd->m_view_angles.z != 0.f)
		cmd->m_side_move = 0.f;

	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	len = move.normalize();
	if (!len)
		return;

	math::VectorAngles(move, move_angle);

	delta = (cmd->m_view_angles.y - wish_angles.y);

	move_angle.y += delta;

	math::AngleVectors(move_angle, &dir);

	dir *= len;

	g_cl.m_cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);

	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER) {
		if (cmd->m_view_angles.x >= 45.f && wish_angles.x < 45.f && std::abs(delta) <= 65.f)
			dir.x = -dir.x;

		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		if (cmd->m_forward_move > 200.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < -200.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 200.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < -200.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}

	else {
		if (cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f)
			dir.x = -dir.x;

		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		if (cmd->m_forward_move > 0.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < 0.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 0.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < 0.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}
}

void Movement::AutoPeek() {
	if ( cfg_t::get_hotkey( "misc_peek", "misc_peek_mode" ) ) {
		if (g_cl.m_old_shot)
			m_invert = true;

		vec3_t move{ g_cl.m_cmd->m_forward_move, g_cl.m_cmd->m_side_move, 0.f };

		if (m_invert) {
			move *= -1.f;
			g_cl.m_cmd->m_forward_move = move.x;
			g_cl.m_cmd->m_side_move = move.y;
		}
	}

	else m_invert = false;

	if ( ( cfg_t::get_hotkey( "misc_peek", "misc_peek_mode" ) || g_aimbot.m_stop ) && g_cl.m_local->m_fFlags() & FL_ONGROUND ) {
		Movement::QuickStop();
	}
}

void Movement::QuickStop() {
	ang_t angle;
	math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

	float speed = g_cl.m_local->m_vecVelocity().length();

	angle.y = g_cl.m_view_angles.y - angle.y;

	vec3_t direction;
	math::AngleVectors(angle, &direction);

	vec3_t stop = direction * -speed;

	if (g_cl.m_speed > 13.f) {
		g_cl.m_cmd->m_forward_move = stop.x;
		g_cl.m_cmd->m_side_move = stop.y;
	}
	else {
		g_cl.m_cmd->m_forward_move = 0.f;
		g_cl.m_cmd->m_side_move = 0.f;
	}
}

void Movement::FakeWalk( bool force ) {
	vec3_t velocity{ g_cl.m_local->m_vecVelocity() };
	int    ticks{ }, max{ 16 };

	if ( !( cfg_t::get_hotkey( "misc_walk", "misc_walk_mode" ) ) || force )
		return;

	if (!g_cl.m_local->GetGroundEntity())
		return;

	float friction = g_csgo.sv_friction->GetFloat() * g_cl.m_local->m_surfaceFriction();

	for (; ticks < g_cl.m_max_lag; ++ticks) {
		float speed = velocity.length();

		if (speed <= 0.1f)
			break;

		float control = std::max(speed, g_csgo.sv_stopspeed->GetFloat());

		float drop = control * friction * g_csgo.m_globals->m_interval;

		float newspeed = std::max(0.f, speed - drop);

		if (newspeed != speed) {
			newspeed /= speed;

			velocity *= newspeed;
		}
	}

	if (ticks > ((max - 1) - g_csgo.m_cl->m_choked_commands) || !g_csgo.m_cl->m_choked_commands) {
		g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.f;
	}
}