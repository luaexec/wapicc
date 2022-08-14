#include "tickbase.h"
c_tickbase g_tickbase{ };

void c_tickbase::on_paint( ) {

	// @doc: safety.
	if ( !g_cl.m_processing )
		return;

	// @doc: shift status.
	if ( cfg_t::get_hotkey( "acc_dt", "acc_dt_mode" ) && m_shift == e_shift::E_IDLE )
		m_shift = e_shift::E_CHARGE;
	else if ( !cfg_t::get_hotkey( "acc_dt", "acc_dt_mode" ) )
		m_shift = e_shift::E_IDLE;

}

void c_tickbase::on_cmove( ) {

	// @doc: safety.
	if ( !g_cl.m_processing )
		return;

	// @doc: dont choke.
	if ( g_tickbase.m_shift == e_shift::E_CHARGED )
		*g_cl.m_packet = true;

	// @doc: uncharge once fired.
	if ( ( g_cl.m_weapon_fire && g_cl.m_cmd->m_buttons & IN_ATTACK ) && g_tickbase.m_shift == e_shift::E_CHARGED && g_cl.m_weapon_info->m_weapon_type != WEAPONTYPE_KNIFE )
		g_tickbase.m_shift = e_shift::E_UNCHARGE;

}

void hk_tickbase::clmove( float accumulated_extra_samples, bool final_tick ) {

	// @doc: local player.
	if ( !g_cl.m_processing )
		return o_clmove( accumulated_extra_samples, final_tick );

	// @doc: safety check.
	if ( ( g_tickbase.m_shift == e_shift::E_CHARGE || g_tickbase.m_shift == e_shift::E_RECHARGE ) && !g_aimbot.m_target ) {

		if ( g_tickbase.m_shift == e_shift::E_RECHARGE )
			g_tickbase.m_charge_time--;
		else
			g_tickbase.m_charge_time = 0;

	}
	else
		g_tickbase.m_charge_time = game::TIME_TO_TICKS( 1.f );

	bool safety{ g_tickbase.m_charge_time <= 0 };

	// @doc: increment tickbase with fake commands.
	if ( ( g_tickbase.m_shift == e_shift::E_CHARGE || g_tickbase.m_shift == e_shift::E_RECHARGE ) && g_tickbase.m_ticks < g_tickbase.m_goal_ticks && safety ) {

		g_tickbase.m_ticks++;
		*g_cl.m_packet = false;
		if ( g_tickbase.m_ticks == g_tickbase.m_goal_ticks )
			g_tickbase.m_shift = e_shift::E_CHARGED;

		return;

	}

	// @doc: call original.
	o_clmove( accumulated_extra_samples, final_tick );

	// @doc: decrement tickbase.
	if ( g_tickbase.m_shift == e_shift::E_UNCHARGE || ( g_tickbase.m_shift == e_shift::E_IDLE && g_tickbase.m_ticks > 0 ) ) {

		while ( !( g_tickbase.m_ticks == 0 ) ) {

			o_clmove( accumulated_extra_samples, final_tick );
			g_tickbase.m_ticks--;

		}

		if ( g_tickbase.m_shift != e_shift::E_IDLE )
			g_tickbase.m_shift = e_shift::E_RECHARGE;

	}

}