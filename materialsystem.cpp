#include "includes.h"

bool Hooks::OverrideConfig( MaterialSystem_Config_t* _cfg, bool update ) {
	if ( config["vis_fb"].get<bool>( ) )
		_cfg->m_nFullbright = true;

	return g_hooks.m_material_system.GetOldMethod< OverrideConfig_t >( IMaterialSystem::OVERRIDECONFIG )( this, _cfg, update );
}