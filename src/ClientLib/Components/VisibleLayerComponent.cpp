// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	void VisibleLayerComponent::RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor)
	{
		auto& nodeComponent = m_entity->GetComponent<Ndk::NodeComponent>();

		std::shared_ptr<VisibleLayer> visibleLayer = std::make_shared<VisibleLayer>();
		visibleLayer->baseNode.SetParent(nodeComponent);
		visibleLayer->baseNode.SetScale(scale);
		visibleLayer->baseRenderOrder = renderOrder;

		VisibleLayer* visibleLayerPtr = visibleLayer.get();
		
		visibleLayerPtr->onDisabled.Connect(localLayer.OnDisabled, [=](LocalLayer*)
		{
			visibleLayerPtr->visualEntities.clear();
		});
		
		visibleLayerPtr->onEnabled.Connect(localLayer.OnEnabled, [=](LocalLayer* layer)
		{
			assert(visibleLayerPtr->visualEntities.empty());
			layer->ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
			{
				visibleLayerPtr->visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), visibleLayerPtr->baseNode, visibleLayerPtr->baseRenderOrder));
			});
		});

		visibleLayer->onEntityCreated.Connect(localLayer.OnEntityCreated, [=](LocalLayer*, LocalLayerEntity& newEntity)
		{
			visibleLayerPtr->visualEntities.emplace(newEntity.GetServerId(), VisualEntity(m_renderWorld, newEntity.CreateHandle(), visibleLayerPtr->baseNode, visibleLayerPtr->baseRenderOrder));
		});

		visibleLayer->onEntityDelete.Connect(localLayer.OnEntityDelete, [=](LocalLayer*, LocalLayerEntity& newEntity)
		{
			visibleLayerPtr->visualEntities.erase(newEntity.GetServerId());
		});

		if (localLayer.IsEnabled())
		{
			localLayer.ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
			{
				visibleLayerPtr->visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), visibleLayerPtr->baseNode, visibleLayerPtr->baseRenderOrder));
			});
		}

		visibleLayer->onCameraMoved.Connect(localLayer.GetLocalMatch().OnCameraMoved, [=](LocalMatch*, const Nz::Vector2f& newPosition)
		{
			// Parallax factor
			visibleLayerPtr->baseNode.SetPosition(newPosition * (Nz::Vector2f(1.f) - parallaxFactor));
		});

		m_visibleLayers.emplace_back(std::move(visibleLayer));
	}

	Ndk::ComponentIndex VisibleLayerComponent::componentIndex;
}
