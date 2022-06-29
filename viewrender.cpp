#include "includes.h"

void Hooks::OnRenderStart( ) {
	// call og.
	g_hooks.m_view_render.GetOldMethod< OnRenderStart_t >( CViewRender::ONRENDERSTART )( this );

	/* field of view */ {
		bool ovr = g_menu.main.visuals.fov.get( );
		auto fov = g_menu.main.visuals.fov_amt.get( );
		static float anim[2]{ 0.f };

		if (!ovr || !g_cl.m_local || !g_cl.m_processing)
			return;

		if (!g_cl.m_local->alive( ))
			return;

		g_csgo.m_view_render->m_view.m_fov = fov;

		bool correct = g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE;

		if (g_cl.m_local->GetActiveWeapon( )->m_zoomLevel( ) >= 1 && correct) {
			anim[0] += 4.f * g_csgo.m_globals->m_frametime;
		}
		else { anim[0] -= 4.f * g_csgo.m_globals->m_frametime; }
		anim[0] = std::clamp( anim[0], 0.f, 1.f );

		if (g_cl.m_local->GetActiveWeapon( )->m_zoomLevel( ) >= 2 && correct) {
			anim[1] += 4.f * g_csgo.m_globals->m_frametime;
		}
		else { anim[1] -= 4.f * g_csgo.m_globals->m_frametime; }
		anim[1] = std::clamp( anim[1], 0.f, 1.f );

		if (g_menu.main.visuals.fov_scoped.get( )) {
			g_csgo.m_view_render->m_view.m_fov -= 10 * anim[0];
			g_csgo.m_view_render->m_view.m_fov -= 10 * anim[1];
		}
	}

	if (g_menu.main.visuals.viewmodel_fov.get( ))
		g_csgo.m_view_render->m_view.m_viewmodel_fov = g_menu.main.visuals.viewmodel_fov_amt.get( );
}

void Hooks::RenderView( const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw ) {
	// ...

	g_hooks.m_view_render.GetOldMethod< RenderView_t >( CViewRender::RENDERVIEW )( this, view, hud_view, clear_flags, what_to_draw );
}

void Hooks::Render2DEffectsPostHUD( const CViewSetup& setup ) {
	if (!g_menu.main.visuals.noflash.get( ))
		g_hooks.m_view_render.GetOldMethod< Render2DEffectsPostHUD_t >( CViewRender::RENDER2DEFFECTSPOSTHUD )( this, setup );
}

void Hooks::RenderSmokeOverlay( bool unk ) {
	// do not render smoke overlay.
	if (!g_menu.main.visuals.nosmoke.get( ))
		g_hooks.m_view_render.GetOldMethod< RenderSmokeOverlay_t >( CViewRender::RENDERSMOKEOVERLAY )( this, unk );
}
