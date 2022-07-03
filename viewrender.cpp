#include "includes.h"

void Hooks::OnRenderStart( ) {
	// call og.
	g_hooks.m_view_render.GetOldMethod< OnRenderStart_t >( CViewRender::ONRENDERSTART )( this );

	/* field of view */ {
		auto fov = config["misc_fov"].get<float>( );
		static float anim[2]{ 0.f };

		if ( !g_cl.m_local || !g_cl.m_processing )
			return;

		if ( !g_cl.m_local->alive( ) )
			return;

		if ( !g_cl.m_weapon || !g_cl.m_weapon_info )
			return;

		g_csgo.m_view_render->m_view.m_fov = fov;

		bool correct = g_cl.m_weapon_info->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE;

		if ( g_cl.m_weapon->m_zoomLevel( ) >= 1 && correct ) {
			anim[0] += 4.f * g_csgo.m_globals->m_frametime;
		}
		else { anim[0] -= 4.f * g_csgo.m_globals->m_frametime; }
		anim[0] = std::clamp( anim[0], 0.f, 1.f );

		if ( g_cl.m_weapon->m_zoomLevel( ) >= 2 && correct ) {
			anim[1] += 4.f * g_csgo.m_globals->m_frametime;
		}
		else { anim[1] -= 4.f * g_csgo.m_globals->m_frametime; }
		anim[1] = std::clamp( anim[1], 0.f, 1.f );

		g_csgo.m_view_render->m_view.m_fov -= config["misc_inc"].get<float>( ) * anim[0];
		g_csgo.m_view_render->m_view.m_fov -= config["misc_inc"].get<float>( ) * anim[1];
	}

	g_csgo.m_view_render->m_view.m_viewmodel_fov = config["misc_vfov"].get<float>( );
}

void Hooks::RenderView( const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw ) {
	// ...

	g_hooks.m_view_render.GetOldMethod< RenderView_t >( CViewRender::RENDERVIEW )( this, view, hud_view, clear_flags, what_to_draw );
}

void Hooks::Render2DEffectsPostHUD( const CViewSetup& setup ) {
	if ( !config["remove_flash"].get<bool>( ) )
		g_hooks.m_view_render.GetOldMethod< Render2DEffectsPostHUD_t >( CViewRender::RENDER2DEFFECTSPOSTHUD )( this, setup );
}

void Hooks::RenderSmokeOverlay( bool unk ) {
	// do not render smoke overlay.
	if ( !config["remove_smoke"].get<bool>( ) )
		g_hooks.m_view_render.GetOldMethod< RenderSmokeOverlay_t >( CViewRender::RENDERSMOKEOVERLAY )( this, unk );
}
