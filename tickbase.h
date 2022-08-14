#pragma once
#include "includes.h"

// @doc: clmove hook.
namespace hk_tickbase {
	// @note clsendmove is objectively better.
	typedef void( *clmove_t )( float, bool );
	inline clmove_t o_clmove;
	void clmove( float accumulated_extra_samples, bool final_tick );
}

enum e_shift : int {
	E_IDLE = 0,
	E_CHARGED,
	E_CHARGE,
	E_UNCHARGE,
	E_RECHARGE,
};

class c_tickbase {
public:
	// @doc: tickbase control data.
	e_shift m_shift{ };
	int m_ticks{ 0 }, m_goal_ticks{ 13 };
	int m_charge_time{ 0 };

public:
	void on_paint( );
	void on_cmove( );
};

extern c_tickbase g_tickbase;