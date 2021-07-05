// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayerEntity.hpp>

namespace bw
{
	inline Nz::UInt32 ClientLayerEntity::GetServerId() const
	{
		return m_serverEntityId;
	}

	inline const ClientLayerEntityHandle& ClientLayerEntity::GetWeaponEntity() const
	{
		return m_weaponEntity;
	}

	inline bool ClientLayerEntity::IsClientside() const
	{
		return m_serverEntityId == ClientsideId;
	}

	inline bool ClientLayerEntity::HasHealth() const
	{
		return m_health.has_value();
	}
}
