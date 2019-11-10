// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>

namespace bw
{
	void VisibleLayerComponent::RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale)
	{
		std::shared_ptr<VisibleLayer> visibleLayer = std::make_shared<VisibleLayer>();
		VisibleLayer* visibleLayerPtr = visibleLayer.get();
		
		visibleLayer->onEntityCreated.Connect(localLayer.OnEntityCreated, [=](LocalLayer*, LocalLayerEntity& newEntity)
		{
			visibleLayerPtr->visualEntities.emplace(newEntity.GetServerId(), VisualEntity(m_renderWorld, newEntity.CreateHandle()));
		});

		visibleLayer->onEntityDelete.Connect(localLayer.OnEntityDelete, [=](LocalLayer*, LocalLayerEntity& newEntity)
		{
			visibleLayerPtr->visualEntities.erase(newEntity.GetServerId());
		});

		if (localLayer.IsEnabled())
		{
			localLayer.ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
			{
				visibleLayerPtr->visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle()));
			});
		}

		m_visibleLayers.emplace_back(std::move(visibleLayer));
	}

	Ndk::ComponentIndex VisibleLayerComponent::componentIndex;
}
