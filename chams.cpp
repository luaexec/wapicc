#include "includes.h"

Chams g_chams{ };;

Chams::model_type_t Chams::GetModelType( const ModelRenderInfo_t& info ) {

	std::string mdl{ info.m_model->m_name };

	if ( mdl.find( XOR( "player" ) ) != std::string::npos && info.m_index >= 1 && info.m_index <= 64 )
		return model_type_t::player;

	return model_type_t::invalid;
}

bool Chams::IsInViewPlane( const vec3_t& world ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];

	return w > 0.001f;
}

void Chams::SetColor( Color col, IMaterial* mat ) {

	if ( mat ) {
		mat->ColorModulate( col );

		auto pVar = mat->find_var( "$envmaptint" );
		if ( pVar )
			( *( void( __thiscall** )( int, float, float, float ) )( *(DWORD*)pVar + 44 ) )( (uintptr_t)pVar, col.r( ) / 255.f, col.g( ) / 255.f, col.b( ) / 255.f );
	}
	else
		g_csgo.m_render_view->SetColorModulation( col );
}

void Chams::SetAlpha( float alpha, IMaterial* mat ) {
	if ( mat )
		mat->AlphaModulate( alpha );

	else
		g_csgo.m_render_view->SetBlend( alpha );
}

void Chams::SetupMaterial( IMaterial* mat, Color col, bool z_flag ) {
	SetColor( col, mat );

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	mat->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
	mat->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
	mat->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

	g_csgo.m_studio_render->ForcedMaterialOverride( mat );
}

void Chams::init( ) {

	std::ofstream( "csgo/materials/wapicc_glowoverlay.vmt" ) << R"#("VertexLitGeneric" 
		{
		"$additive" "1"
		"$envmap" "models/effects/cube_white"
		"$envmaptint" "[1 1 1]"
		"$envmapfresnel" "1"
		"$envmapfresnelminmaxexp" "[0 1 2]"
		"$alpha" "0.8"
		})#";

	std::ofstream( "csgo\\materials\\wapicc_metallic.vmt" ) << R"#("VertexLitGeneric" 
	{
		"$basetexture" "vgui/white_additive"
		"$ignorez"      "1"
		"$envmap"       "env_cubemap"
		"$normalmapalphaenvmapmask"  "1"
		"$envmapcontrast"             "1"
		"$nofog"        "1"
		"$model"        "1"
		"$nocull"       "0"
		"$selfillum"    "1"
		"$halflambert"  "1"
		"$znearer"      "0"
		"$flat"         "1"
		})#";

	// find stupid materials.
	m_materials.push_back( g_csgo.m_material_system->FindMaterial( XOR( "debug/debugambientcube" ), XOR( "Model textures" ) ) );
	m_materials.push_back( g_csgo.m_material_system->FindMaterial( XOR( "debug/debugdrawflat" ), XOR( "Model textures" ) ) );
	m_materials.push_back( g_csgo.m_material_system->FindMaterial( XOR( "models/inventory_items/trophy_majors/gloss" ), XOR( "Model textures" ) ) );
	m_materials.push_back( g_csgo.m_material_system->FindMaterial( XOR( "wapicc_glowoverlay" ), XOR( "Model textures" ) ) );

	for ( int i = 0; i < m_materials.size( ); i++ ) {
		m_materials[i]->IncrementReferenceCount( );
	}
}

bool Chams::OverridePlayer( int index ) {
	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player )
		return false;

	// always skip the local player in DrawModelExecute.
	// this is because if we want to make the local player have less alpha
	// the static props are drawn after the players and it looks like aids.
	// therefore always process the local player in scene end.
	if ( player->m_bIsLocalPlayer( ) )
		return true;

	// see if this player is an enemy to us.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	// we have chams on enemies.
	if ( enemy && config["chams_enemy"].get<bool>( ) )
		return true;

	return false;
}

bool Chams::GenerateLerpedMatrix( int index, BoneArray* out ) {
	LagRecord* current_record;
	AimPlayer* data;

	Player* ent = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !ent )
		return false;

	if ( !g_aimbot.IsValidTarget( ent ) )
		return false;

	data = &g_aimbot.m_players[index - 1];
	if ( !data || data->m_records.empty( ) )
		return false;

	if ( data->m_records.size( ) < 2 )
		return false;

	auto* channel_info = g_csgo.m_engine->GetNetChannelInfo( );
	if ( !channel_info )
		return false;

	static float max_unlag = 0.2f;
	static auto sv_maxunlag = g_csgo.m_cvar->FindVar( HASH( "sv_maxunlag" ) );
	if ( sv_maxunlag ) {
		max_unlag = sv_maxunlag->GetFloat( );
	}

	// CHANGE TO AROUND 500-1k IF ISSUES COME UP

	for ( auto it = data->m_records.rbegin( ); it != data->m_records.rend( ); it++ ) {
		current_record = it->get( );

		bool end = it + 1 == data->m_records.rend( );

		if ( current_record && current_record->valid( ) && ( !end && ( ( it + 1 )->get( ) ) ) ) {
			if ( current_record->m_origin.dist_to( ent->GetAbsOrigin( ) ) < 1.f ) {
				return false;
			}

			vec3_t next = end ? ent->GetAbsOrigin( ) : ( it + 1 )->get( )->m_origin;
			float  time_next = end ? ent->m_flSimulationTime( ) : ( it + 1 )->get( )->m_sim_time;

			float total_latency = channel_info->GetAvgLatency( 0 ) + channel_info->GetAvgLatency( 1 );
			std::clamp( total_latency, 0.f, max_unlag );

			float correct = total_latency + g_cl.m_lerp;
			float time_delta = time_next - current_record->m_sim_time;
			float add = end ? 1.f : time_delta;
			float deadtime = current_record->m_sim_time + correct + add;

			float curtime = g_csgo.m_globals->m_curtime;
			float delta = deadtime - curtime;

			float mul = 1.f / add;
			vec3_t lerp = math::Interpolate( next, current_record->m_origin, std::clamp( delta * mul, 0.f, 1.f ) );

			matrix3x4_t ret[128];

			std::memcpy( ret,
						 current_record->m_bones,
						 sizeof( ret ) );

			for ( size_t i{ }; i < 128; ++i ) {
				vec3_t matrix_delta = current_record->m_bones[i].GetOrigin( ) - current_record->m_origin;
				ret[i].SetOrigin( matrix_delta + lerp );
			}

			std::memcpy( out,
						 ret,
						 sizeof( ret ) );

			return true;
		}
	}

	return false;
}

void Chams::RenderHistoryChams( int index ) {
	AimPlayer* data;
	LagRecord* record;

	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if ( !player )
		return;

	if ( !g_aimbot.IsValidTarget( player ) )
		return;

	auto do_render = [&]( std::string var, int mat, bool z ) {
		Color clr{ config[tfm::format( "chams_%s", var )].get_color( ) };
		SetAlpha( clr.a( ) / 255.f );
		SetupMaterial( m_materials[mat], clr, z );
		player->DrawModel( );
	};

	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );
	if ( enemy ) {
		data = &g_aimbot.m_players[index - 1];
		if ( !data )
			return;

		record = g_resolver.FindLastRecord( data );
		if ( !record )
			return;

		// was the matrix properly setup?
		BoneArray arr[128];
		if ( Chams::GenerateLerpedMatrix( index, arr ) ) {
			// backup the bone cache before we fuck with it.
			auto backup_bones = player->m_BoneCache( ).m_pCachedBones;

			// replace their bone cache with our custom one.
			player->m_BoneCache( ).m_pCachedBones = arr;

			switch ( config["chams_bt_mat"].get<int>( ) ) {
				case 0:
					{

						do_render( "bt_clr", 0, true );

						break;
					}
				case 1:
					{

						do_render( "bt_clr", 1, true );

						break;
					}
				case 2:
					{

						do_render( "bt_clr", 0, true );
						do_render( "bt_acc", 2, true );

						break;
					}
				case 3:
					{

						do_render( "bt_clr", 0, true );
						do_render( "bt_acc", 3, true );

						break;
					}
				default:
					break;
			}

			// reset their bone cache to the previous one.
			player->m_BoneCache( ).m_pCachedBones = backup_bones;
		}
	}
}

bool Chams::DrawModel( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	// store and validate model type.
	model_type_t type = GetModelType( info );
	if ( type == model_type_t::invalid )
		return true;

	// is a valid player.
	if ( type == model_type_t::player ) {
		// do not cancel out our own calls from SceneEnd
		// also do not cancel out calls from the glow.
		if ( !m_running && !g_csgo.m_studio_render->m_pForcedMaterial && OverridePlayer( info.m_index ) )
			return false;
	}

	return true;
}

void Chams::SceneEnd( ) {
	// store and sort ents by distance.
	if ( SortPlayers( ) ) {
		// iterate each player and render them.
		for ( const auto& p : m_players )
			RenderPlayer( p );
	}

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );
}

void Chams::RenderPlayer( Player* player ) {
	// prevent recruisive model cancelation.
	m_running = true;

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );

	auto do_render = [&]( std::string var, int mat, bool z, float blend = 1.f ) {
		Color clr{ config[tfm::format( "chams_%s", var )].get_color( ) };
		SetAlpha( ( clr.a( ) / 255.f ) * blend );
		SetupMaterial( m_materials[mat], clr, z );
		player->DrawModel( );
	};

	// this is the local player.
	// we always draw the local player manually in drawmodel.
	if ( player->m_bIsLocalPlayer( ) && config["chams_local"].get<bool>( ) ) {

		float blend = g_cl.m_local->m_bIsScoped( ) || g_cl.m_weapon_info->m_weapon_type == CSWeaponType::WEAPONTYPE_GRENADE ? 0.5f : 1.f;

		switch ( config["chams_local_mat"].get<int>( ) ) {
			case 0:
				{

					do_render( "local_clr", 0, false, blend );

					break;
				}
			case 1:
				{

					do_render( "local_clr", 1, false, blend );

					break;
				}
			case 2:
				{

					do_render( "local_clr", 0, false, blend );
					do_render( "local_acc", 2, false );

					break;
				}
			case 3:
				{

					do_render( "local_clr", 0, false, blend );
					do_render( "local_acc", 3, false );

					break;
				}
			default:
				break;
		}

	}
	else
		player->DrawModel( );

	// check if is an enemy.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	if ( enemy && config["chams_bt"].get<bool>( ) ) {
		RenderHistoryChams( player->index( ) );
	}

	if ( enemy && config["chams_enemy"].get<bool>( ) ) {
		switch ( config["chams_enemy_mat"].get<int>( ) ) {
			case 0:
				{

					do_render( "enemy_occ", 0, true );
					do_render( "enemy_vis", 0, false );

					break;
				}
			case 1:
				{

					do_render( "enemy_occ", 1, true );
					do_render( "enemy_vis", 1, false );

					break;
				}
			case 2:
				{

					do_render( "enemy_occ", 0, true );
					do_render( "enemy_acc", 2, true );

					do_render( "enemy_vis", 0, false );
					do_render( "enemy_acc", 2, false );

					break;
				}
			case 3:
				{

					do_render( "enemy_occ", 0, true );
					do_render( "enemy_acc", 3, true );

					do_render( "enemy_vis", 0, false );
					do_render( "enemy_acc", 3, false );

					break;
				}
			default:
				break;
		}
	}
	else
		player->DrawModel( );

	m_running = false;
}

bool Chams::SortPlayers( ) {
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = []( Entity* a, Entity* b ) {
		vec3_t local = g_cl.m_local->GetAbsOrigin( );

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		float len1 = ( a->GetAbsOrigin( ) - local ).length_sqr( );
		float len2 = ( b->GetAbsOrigin( ) - local ).length_sqr( );

		return len1 < len2;
	};

	// reset player container.
	m_players.clear( );

	// find all players that should be rendered.
	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		// get player ptr by idx.
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate.
		if ( !player || !player->IsPlayer( ) || !player->alive( ) || player->dormant( ) )
			continue;

		// do not draw players occluded by view plane.
		if ( !IsInViewPlane( player->WorldSpaceCenter( ) ) )
			continue;

		// this player was not skipped to draw later.
		// so do not add it to our render list.
		if ( !OverridePlayer( i ) )
			continue;

		m_players.push_back( player );
	}

	// any players?
	if ( m_players.empty( ) )
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort( m_players.begin( ), m_players.end( ), distance_predicate );

	return true;
}