// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvasLayer.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	MapCanvasLayer::MapCanvasLayer(MapCanvas& mapCanvas, LayerIndex layerIndex) :
	ClientEditorLayer(mapCanvas, layerIndex),
	m_mapCanvas(mapCanvas)
	{
	}

	LayerVisualEntity& MapCanvasLayer::CreateEntity(EntityId uniqueId, const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, PropertyValueMap properties)
	{
		const EditorEntityStore& entityStore = m_mapCanvas.GetEntityStore();

		std::size_t classIndex = entityStore.GetElementIndex(entityClass);

		auto it = m_layerEntities.end();

		try
		{
			if (classIndex == entityStore.InvalidIndex)
				throw std::runtime_error("entity class is not registered");

			auto entityOpt = entityStore.Instantiate(GetLayerIndex(), GetWorld(), classIndex, uniqueId, position, rotation, 1.f, std::move(properties));
			if (!entityOpt)
				throw std::runtime_error("failed to instantiate \"" + entityClass + "\"");

			it = m_layerEntities.emplace(uniqueId, std::move(*entityOpt)).first;
		}
		catch (const std::exception& e)
		{
			bwLog(m_mapCanvas.GetLogger(), LogLevel::Error, "Failed to instantiate entity of type {}: {}", entityClass, e.what());

			entt::entity dummyEntity = GetWorld().CreateEntity();
			dummyEntity->AddComponent<Ndk::NodeComponent>();

			LayerVisualEntity visualEntity(dummyEntity, GetLayerIndex(), uniqueId);
			dummyEntity->AddComponent<CanvasComponent>(m_mapCanvas, GetLayerIndex(), uniqueId);
			dummyEntity->AddComponent<VisualComponent>(visualEntity.CreateHandle());

			it = m_layerEntities.emplace(uniqueId, std::move(visualEntity)).first;
		}

		assert(it != m_layerEntities.end());

		LayerVisualEntity& visualEntity = it.value();
		m_mapCanvas.RegisterEntity(uniqueId, visualEntity.CreateHandle());

		OnEntityVisualCreated(this, visualEntity);

		return visualEntity;
	}

	void MapCanvasLayer::DeleteEntity(EntityId uniqueId)
	{
		auto it = m_layerEntities.find(uniqueId);
		if (it == m_layerEntities.end())
			return;

		OnEntityVisualDelete(this, it.value());

		m_layerEntities.erase(it);
		m_mapCanvas.UnregisterEntity(uniqueId);
	}

	void MapCanvasLayer::ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func)
	{
		for (auto it = m_layerEntities.begin(); it != m_layerEntities.end(); ++it)
			func(it.value());
	}

	bool MapCanvasLayer::IsEnabled() const
	{
		return true;
	}
}
