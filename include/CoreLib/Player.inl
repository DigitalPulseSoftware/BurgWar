// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Player.hpp>

namespace bw
{
	inline const Ndk::EntityHandle& Player::GetControlledEntity() const
	{
		return m_playerEntity;
	}

	inline LayerIndex Player::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline Match* Player::GetMatch() const
	{
		return m_match;
	}

	inline const std::string& Player::GetName() const
	{
		return m_name;
	}

	inline Nz::UInt8 Player::GetPlayerIndex() const
	{
		return m_playerIndex;
	}

	inline MatchClientSession& Player::GetSession()
	{
		return m_session;
	}

	inline const MatchClientSession& Player::GetSession() const
	{
		return m_session;
	}

	inline std::size_t Player::GetWeaponCount() const
	{
		return m_weapons.size();
	}

	inline bool Player::HasWeapon(const std::string& weaponClass) const
	{
		return m_weaponByName.find(weaponClass) != m_weaponByName.end();
	}

	inline bool Player::IsAdmin() const
	{
		return m_isAdmin;
	}

	inline bool Player::IsInMatch() const
	{
		return m_match != nullptr;
	}

	inline bool Player::IsReady() const
	{
		return m_isReady;
	}

	template<typename T>
	void Player::SendPacket(const T& packet)
	{
		m_session.SendPacket(packet);
	}
}
