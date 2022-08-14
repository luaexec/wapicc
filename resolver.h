#pragma once

class ShotRecord;

class Resolver {
public:
	enum Modes : size_t {
		R_NONE = 0,
		R_SNAP,
		R_FREESTAND,
		R_EDGE,
		R_UPDATE,
		R_MOVE,
		R_AIR,
		R_ROT,
	};

	enum Directions : int {
		YAW_RIGHT = -1,
		YAW_BACK,
		YAW_LEFT,
		YAW_NONE,
	};

public:
	LagRecord* FindIdealRecord( AimPlayer* data );
	LagRecord* FindLastRecord( AimPlayer* data );

	void OnBodyUpdate( Player* player, float value );
	float GetAwayAngle( LagRecord* record );

	void MatchShot( AimPlayer* data, LagRecord* record );

	void on_player( LagRecord* record );

	bool do_update( AimPlayer* data, LagRecord* record );
	bool edge( AimPlayer* data, LagRecord* record, float extension, float& angle );
	void freestand( AimPlayer* data, LagRecord* record, float extension );
	void snap( AimPlayer* data, LagRecord* record, float angle, std::string prefix );
	bool flick( AimPlayer* data, LagRecord* record );
	void best_angle( LagRecord* record );
	Directions get_direction( AimPlayer* data );

	void standard( AimPlayer* data, LagRecord* record );
	void move( AimPlayer* data, LagRecord* record );
	void air( AimPlayer* data, LagRecord* record );

public:
	std::array< vec3_t, 64 > m_impacts;
	float m_runtime[64];
	std::string m_dbg[64];
};

extern Resolver g_resolver;