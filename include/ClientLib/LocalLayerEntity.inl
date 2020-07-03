// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayerEntity.hpp>

namespace bw
{
	inline void LocalLayerEntity::Disable()
	{
		return Enable(false);
	}

	inline const Ndk::EntityHandle& LocalLayerEntity::GetEntity() const
	{
		return m_entity;
	}

	inline Nz::UInt32 LocalLayerEntity::GetServerId() const
	{
		return m_serverEntityId;
	}

	inline Nz::Int64 LocalLayerEntity::GetUniqueId() const
	{
		return m_uniqueId;
	}

	inline const LocalLayerEntityHandle& LocalLayerEntity::GetWeaponEntity() const
	{
		return m_weaponEntity;
	}

	inline bool LocalLayerEntity::IsClientside() const
	{
		return m_serverEntityId == ClientsideId;
	}

	inline bool LocalLayerEntity::IsEnabled() const
	{
		return m_entity->IsEnabled();
	}

	inline bool LocalLayerEntity::HasHealth() const
	{
		return m_health.has_value();
	}
	
	inline bool LocalLayerEntity::HasName() const
	{
		return m_name.has_value();
	}
}
