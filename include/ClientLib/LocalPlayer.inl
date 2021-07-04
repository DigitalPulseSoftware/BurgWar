// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalPlayer.hpp>

namespace bw
{
	inline LocalPlayer::LocalPlayer(Nz::UInt16 playerIndex, std::string name, std::optional<Nz::UInt8> localPlayerIndex) :
	m_localPlayerIndex(std::move(localPlayerIndex)),
	m_name(std::move(name)),
	m_ping(InvalidPing),
	m_playerIndex(playerIndex),
	m_controlledEntityId(InvalidEntityId)
	{
	}
	
	inline EntityId LocalPlayer::GetControlledEntityId() const
	{
		return m_controlledEntityId;
	}

	inline Nz::UInt8 LocalPlayer::GetLocalPlayerIndex() const
	{
		assert(m_localPlayerIndex);
		return *m_localPlayerIndex;
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

	inline bool LocalPlayer::IsLocalPlayer() const
	{
		return m_localPlayerIndex.has_value();
	}

	inline const std::string& LocalPlayer::ToString() const
	{
		return m_name;
	}

	inline void LocalPlayer::UpdateControlledEntityId(EntityId controlledEntityId)
	{
		m_controlledEntityId = controlledEntityId;
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
