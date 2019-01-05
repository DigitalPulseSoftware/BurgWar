// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Match.hpp>
#include <cassert>

namespace bw
{
	inline ServerEntityStore& Match::GetEntityStore()
	{
		return *m_entityStore;
	}

	inline const ServerEntityStore& Match::GetEntityStore() const
	{
		return *m_entityStore;
	}

	inline sol::state& Match::GetLuaState()
	{
		return m_scriptingContext->GetLuaState();
	}

	inline const std::shared_ptr<Gamemode>& Match::GetGamemode()
	{
		return m_gamemode;
	}

	inline const NetworkStringStore& Match::GetNetworkStringStore() const
	{
		return m_networkStringStore;
	}

	inline MatchSessions& Match::GetSessions()
	{
		return m_sessions;
	}

	inline const MatchSessions& Match::GetSessions() const
	{
		return m_sessions;
	}

	inline Terrain& Match::GetTerrain()
	{
		assert(m_terrain);
		return *m_terrain;
	}

	inline const Terrain& Match::GetTerrain() const
	{
		assert(m_terrain);
		return *m_terrain;
	}

	inline ServerWeaponStore& Match::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	inline const ServerWeaponStore& Match::GetWeaponStore() const
	{
		return *m_weaponStore;
	}
}
