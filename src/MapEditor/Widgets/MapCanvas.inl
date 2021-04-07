// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>

namespace bw
{
	template<typename F>
	void MapCanvas::ForEachEntity(F&& func)
	{
		for (const Ndk::EntityHandle& entity : GetWorld().GetEntities())
			func(entity);
	}

	template<typename F>
	void MapCanvas::ForEachMapEntity(F&& func)
	{
		for (auto&& [uniqueId, visualEntityHandle] : m_entitiesByUniqueId)
			func(*visualEntityHandle);
	}

	inline MapCanvasLayer* MapCanvas::GetActiveLayer()
	{
		if (!m_currentLayer)
			return nullptr;

		LayerIndex layerIndex = m_currentLayer.value();
		assert(layerIndex < m_layers.size());
		return &m_layers[layerIndex];
	}

	inline const MapCanvasLayer* MapCanvas::GetActiveLayer() const
	{
		if (!m_currentLayer)
			return nullptr;

		LayerIndex layerIndex = m_currentLayer.value();
		assert(layerIndex < m_layers.size());
		return &m_layers[layerIndex];
	}

	inline const std::shared_ptr<VirtualDirectory>& MapCanvas::GetAssetDirectory()
	{
		return m_assetDirectory;
	}

	inline ScriptingContext& MapCanvas::GetScriptingContext()
	{
		return *m_scriptingContext;
	}

	inline const ScriptingContext& MapCanvas::GetScriptingContext() const
	{
		return *m_scriptingContext;
	}

	inline const std::shared_ptr<VirtualDirectory>& MapCanvas::GetScriptDirectory()
	{
		return m_scriptDirectory;
	}

	inline void MapCanvas::RegisterEntity(EntityId uniqueId, LayerVisualEntityHandle handle)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) == m_entitiesByUniqueId.end());
		m_entitiesByUniqueId.emplace(uniqueId, std::move(handle));
	}

	inline void MapCanvas::UnregisterEntity(EntityId uniqueId)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) != m_entitiesByUniqueId.end());
		m_entitiesByUniqueId.erase(uniqueId);
	}
}
