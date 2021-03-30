// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LayerVisualEntity.hpp>
#include <cassert>

namespace bw
{
	inline LayerVisualEntity::LayerVisualEntity(const Ndk::EntityHandle& entity, EntityId uniqueId) :
	m_entity(entity),
	m_uniqueId(uniqueId)
	{
		assert(m_entity);
	}

	inline void LayerVisualEntity::Disable()
	{
		return Enable(false);
	}

	inline const Ndk::EntityHandle& LayerVisualEntity::GetEntity() const
	{
		return m_entity;
	}

	inline EntityId LayerVisualEntity::GetUniqueId() const
	{
		return m_uniqueId;
	}

	inline bool LayerVisualEntity::IsEnabled() const
	{
		return m_entity->IsEnabled();
	}
}
