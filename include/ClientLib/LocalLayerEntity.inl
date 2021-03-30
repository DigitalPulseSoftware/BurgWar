// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerEntity.hpp>

namespace bw
{
	inline Nz::UInt32 LocalLayerEntity::GetServerId() const
	{
		return m_serverEntityId;
	}

	inline const LocalLayerEntityHandle& LocalLayerEntity::GetWeaponEntity() const
	{
		return m_weaponEntity;
	}

	inline bool LocalLayerEntity::IsClientside() const
	{
		return m_serverEntityId == ClientsideId;
	}

	inline bool LocalLayerEntity::HasHealth() const
	{
		return m_health.has_value();
	}
}
