#pragma once
#include "includes.h"

enum e_scan : int {
	S_DEFAULT = 0,
	S_PREFER,
	S_IGNORE,
};

struct hitscan_t {
	int hitbox;
	e_scan scan;

	hitscan_t( int hb, e_scan s = e_scan::S_DEFAULT ) {
		hitbox = hb; scan = s;
	}

	void reset( ) {
		hitbox = -1;
		scan = e_scan::S_DEFAULT;
	}

	bool valid( ) {
		return hitbox != -1;
	}
};

class aimdata_t {
public: // @doc: targetting.
	Player* p; int pid;
	LagRecord* r{ std::make_unique<LagRecord>( ).get( ) }; int rid;
public: // @doc: selection.
	std::vector<hitscan_t> hitboxs;
	std::vector<vec3_t> points;
public: // @doc: information.
	int missed_shots;
	float update;
public: // @doc: best.
	vec3_t aimpoint;
	float fov, damage, hitchance;
public: // @doc: methods.
	void reset( ) {
		r = std::make_unique<LagRecord>( ).get( );
		hitboxs.clear( ); points.clear( );
		aimpoint = vec3_t{ };

		fov = 181.f; damage = hitchance = 0.f;
	}

	void clone( aimdata_t* o ) {
		p = o->p; pid = o->pid;
		r = o->r; rid = o->rid;
		hitboxs = o->hitboxs; points = o->points;
		aimpoint = o->aimpoint;
		fov = o->fov; damage = o->damage; hitchance = o->hitchance;
	}
};

class c_ragebot {
public: // @doc: targetting.
	std::array<aimdata_t*, 64U> players;
	std::array<aimdata_t*, 3U> targets;
	aimdata_t* target;
public: // @doc: core.
	void reset( );
	void identify( );
	bool allocate( );
	void fire( );
public: // @doc: callback.
	void on_cmove( ) {

		reset( );
		identify( );

		if ( allocate( ) )
			fire( );

	}
};

extern c_ragebot g_ragebot;