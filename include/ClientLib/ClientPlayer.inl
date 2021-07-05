// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientPlayer.hpp>

namespace bw
{
	inline ClientPlayer::ClientPlayer(Nz::UInt16 playerIndex, std::string name, std::optional<Nz::UInt8> localPlayerIndex) :
	m_localPlayerIndex(std::move(localPlayerIndex)),
	m_name(std::move(name)),
	m_ping(InvalidPing),
	m_playerIndex(playerIndex),
	m_controlledEntityId(InvalidEntityId)
	{
	}
	
	inline EntityId ClientPlayer::GetControlledEntityId() const
	{
		return m_controlledEntityId;
	}

	inline Nz::UInt8 ClientPlayer::GetLocalPlayerIndex() const
	{
		assert(m_localPlayerIndex);
		return *m_localPlayerIndex;
	}

	inline const std::string& ClientPlayer::GetName() const
	{
		return m_name;
	}

	inline Nz::UInt16 ClientPlayer::GetPing() const
	{
		return m_ping;
	}
	
	inline Nz::UInt16 ClientPlayer::GetPlayerIndex() const
	{
		return m_playerIndex;
	}

	inline bool ClientPlayer::IsLocalPlayer() const
	{
		return m_localPlayerIndex.has_value();
	}

	inline const std::string& ClientPlayer::ToString() const
	{
		return m_name;
	}

	inline void ClientPlayer::UpdateControlledEntityId(EntityId controlledEntityId)
	{
		m_controlledEntityId = controlledEntityId;
	}

	inline void ClientPlayer::UpdateName(std::string name)
	{
		m_name = std::move(name);
	}

	inline void ClientPlayer::UpdatePing(Nz::UInt16 ping)
	{
		m_ping = ping;
	}
}
