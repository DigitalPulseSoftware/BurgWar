// Copyright (C) 2019 Jérôme Leclercq
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

	inline const LocalLayerEntityHandle& LocalLayerEntity::GetWeaponEntity() const
	{
		return m_weaponEntity;
	}

	inline bool LocalLayerEntity::IsPhysical() const
	{
		return m_isPhysical;
	}
}
