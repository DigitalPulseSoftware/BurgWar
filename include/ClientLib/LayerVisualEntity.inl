// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LayerVisualEntity.hpp>
#include <cassert>
#include <utility>

namespace bw
{
	inline LayerVisualEntity::LayerVisualEntity(entt::handle entity, LayerIndex layerIndex, EntityId uniqueId) :
	m_entity(entity),
	m_uniqueId(uniqueId),
	m_layerIndex(layerIndex)
	{
		assert(m_entity.GetEntity());
	}

	inline void LayerVisualEntity::Disable()
	{
		return Enable(false);
	}

	template<typename Func>
	void LayerVisualEntity::ForEachRenderable(Func&& func) const
	{
		for (const auto& renderableData : m_attachedRenderables)
			func(std::as_const(renderableData.renderable), std::as_const(renderableData.offsetMatrix), std::as_const(renderableData.renderOrder));
	}

	inline entt::handle LayerVisualEntity::GetEntity() const
	{
		return m_entity.GetEntity();
	}

	inline LayerIndex LayerVisualEntity::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline EntityId LayerVisualEntity::GetUniqueId() const
	{
		return m_uniqueId;
	}

	inline bool LayerVisualEntity::IsEnabled() const
	{
		return true;
		// TODO
		//return m_entity->IsEnabled();
	}
}
