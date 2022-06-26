#pragma once

struct AntiFreestandingRecord
{
	int right_damage = 0, left_damage = 0, back_damage = 0;
	float right_fraction = 0.f, left_fraction = 0.f, back_fraction = 0.f;
};

class Resolver {
public:
	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_WALK,
		RESOLVE_STAND,
		RESOLVE_AIR,
		RESOLVE_BODY,
		RESOLVE_LM,
		RESOLVE_FREESTAND,
	};

	enum class Directions : int {
		YAW_RIGHT = -1,
		YAW_BACK,
		YAW_LEFT,
		YAW_NONE,
	};

public:
	LagRecord* FindIdealRecord(AimPlayer* data);
	LagRecord* FindLastRecord(AimPlayer* data);

	bool ResolveBodyUpdates(Player* player, LagRecord* record);
	void OnBodyUpdate(Player* player, float value);
	float GetAwayAngle(LagRecord* record);

	bool MatchShot(AimPlayer* data, LagRecord* record);
	void SetMode(LagRecord* record);

	void collect_wall_detect(const Stage_t stage);
	bool AntiFreestanding(Player* entity, AimPlayer* data, float& yaw);

	void ResolveAngles(Player* player, LagRecord* record);
	void ResolveWalk(AimPlayer* data, LagRecord* record);
	void FindBestAngle(LagRecord* record);
	Directions HandleDirections(AimPlayer* data);
	void ResolveStand(AimPlayer* data, LagRecord* record);
	void ResolveAir(AimPlayer* data, LagRecord* record);
	void ResolvePoses(Player* player, LagRecord* record);

public:
	std::array< vec3_t, 64 > m_impacts;
	int m_runtime[64];

	// check if the players yaw is sideways.
	__forceinline bool IsLastMoveValid(LagRecord* record, float m_yaw) {
		const auto away = GetAwayAngle(record);
		const float delta = fabs(math::NormalizedAngle(away - m_yaw));
		return delta > 20.f && delta < 160.f;
	}

	AntiFreestandingRecord anti_freestanding_record;

	class PlayerResolveRecord
	{
	public:
		struct AntiFreestandingRecord
		{
			int right_damage = 0, left_damage = 0;
			float right_fraction = 0.f, left_fraction = 0.f;
		};

	public:
		AntiFreestandingRecord m_sAntiEdge;
	};

	vec3_t last_eye;

	bool using_anti_freestand;

	float left_damage[64];
	float right_damage[64];
	float back_damage[64];

	std::vector<vec3_t> last_eye_positions;

};

extern Resolver g_resolver;