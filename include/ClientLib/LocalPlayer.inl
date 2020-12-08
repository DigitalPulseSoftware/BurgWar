// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalPlayer.hpp>

namespace bw
{
	inline LocalPlayer::LocalPlayer(Nz::UInt16 playerIndex, std::string name) :
	m_name(std::move(name)),
	m_ping(InvalidPing),
	m_playerIndex(playerIndex)
	{
	}
	
	inline const std::string& LocalPlayer::GetName() const
	{
		return m_name;
	}

	inline Nz::UInt16 LocalPlayer::GetPing() const
	{
		return m_ping;
	}
	
	inline Nz::UInt16 LocalPlayer::GetPlayerIndex() const
	{
		return m_playerIndex;
	}

	inline const std::string& LocalPlayer::ToString() const
	{
		return m_name;
	}
	
	inline void LocalPlayer::UpdateName(std::string name)
	{
		m_name = std::move(name);
	}

	inline void LocalPlayer::UpdatePing(Nz::UInt16 ping)
	{
		m_ping = ping;
	}
}
