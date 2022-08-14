#include "cfg.h"

std::map< std::string, value_t > config{
};

bool cfg_t::get_hotkey( std::string str, std::string mode ) {
	int vk = config[str].get<int>( );
	int activation_type = config[mode].get<int>( );

	switch (activation_type) {
	case 0:
		return g_input.GetKeyState( vk );
		break;
	case 1: {
		if (g_input.GetKeyPress( vk )) {
			m_hotkey_states[str] = !m_hotkey_states[str];
		}
		return m_hotkey_states[str];
		break;
	}
	}

	return false;
}

void cfg_t::init( ) {
	m_init = false;
	m_path.resize( MAX_PATH + 1 );

	// https://stackoverflow.com/questions/2414828/get-path-to-my-documents/12607759
	HRESULT result = SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, m_path.data( ) );
	if (result != S_OK)
		return;

	m_path = tfm::format( "%s\\%s", m_path.data( ), "wapicc" );
	CreateDirectory( m_path.c_str( ), NULL );

	m_init = true;

	save( );
}

void cfg_t::save( std::string name, bool to_clipboard ) {
	if (!m_init)
		return;

	std::string file;
	file = tfm::format( "%s\\%s", m_path.data( ), name.data( ) );

	for (auto& e : config) {
		WritePrivateProfileStringA( "wapicc", e.first.c_str( ), std::to_string( e.second.get<double>( ) ).c_str( ), file.c_str( ) );
	}
}

void cfg_t::load( std::string name, bool from_clipboard ) {
	if (!m_init)
		return;

	std::string file;
	file = tfm::format( "%s\\%s", m_path.data( ), name.data( ) );

	if (!std::filesystem::exists( file ))
		return load( );

	for (auto& e : config) {
		char value[64] = { '\0' }, sort = (char)( e.first.c_str( ) );

		bool found = config.find( e.first.c_str( ) ) != config.end( );
		if (!found) {
			config.insert( { e.first.c_str( ), atof( value ) } );
		}

		GetPrivateProfileStringA( "wapicc", e.first.c_str( ), "", value, 64, file.c_str( ) );

		e.second.set<double>( atof( value ) );
	}
}