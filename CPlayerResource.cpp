#include "includes.h"


int CSPlayerResource::GetPlayerPing(int idx) {
	static Address m_iPing = g_netvars.get(HASH("DT_PlayerResource"), HASH("m_iPing"));


	return *(int*)((uintptr_t)this + m_iPing + idx * 4);
}

int CSPlayerResource::GetPlayerAssists(int idx) {
	static Address m_iAssists = g_netvars.get(HASH("DT_PlayerResource"), HASH("m_iAssists"));
	return *(int*)((uintptr_t)this + m_iAssists + idx * 4);
}

int CSPlayerResource::GetPlayerKills(int idx) {
	static Address m_iKills = g_netvars.get(HASH("DT_PlayerResource"), HASH("m_iKills"));
	return *(int*)((uintptr_t)this + m_iKills + idx * 4);
}

int CSPlayerResource::GetPlayerDeaths(int idx) {
	static Address m_iDeaths = g_netvars.get(HASH("DT_PlayerResource"), HASH("m_iDeaths"));
	return *(int*)((uintptr_t)this + m_iDeaths + idx * 4);
}