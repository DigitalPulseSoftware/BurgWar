// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	void VisibleLayerComponent::Clear()
	{
		m_visibleLayers.clear();
	}

	void VisibleLayerComponent::RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor)
	{
		auto& nodeComponent = m_entity->GetComponent<Ndk::NodeComponent>();

		std::shared_ptr<VisibleLayer> visibleLayer = std::make_shared<VisibleLayer>();
		visibleLayer->baseNode.SetParent(nodeComponent);
		visibleLayer->baseNode.SetInheritPosition(false);
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
				CreateVisual(visibleLayerPtr, layerEntity);
			});
		});

		visibleLayer->onEntityCreated.Connect(localLayer.OnEntityCreated, [=](LocalLayer*, LocalLayerEntity& entity)
		{
			CreateVisual(visibleLayerPtr, entity);
		});

		visibleLayer->onEntityDelete.Connect(localLayer.OnEntityDelete, [=](LocalLayer*, LocalLayerEntity& entity)
		{
			DeleteVisual(visibleLayerPtr, entity);
		});

		if (localLayer.IsEnabled())
		{
			localLayer.ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
			{
				CreateVisual(visibleLayerPtr, layerEntity);
			});
		}

		visibleLayer->onCameraMoved.Connect(localLayer.GetLocalMatch().OnCameraMoved, [=](LocalMatch*, const Nz::Vector2f& newPosition)
		{
			// Parallax factor
			Nz::Vector2f layerPosition = newPosition * (Nz::Vector2f(1.f) - parallaxFactor);
			layerPosition.x = std::floor(layerPosition.x);
			layerPosition.y = std::floor(layerPosition.y);

			visibleLayerPtr->baseNode.SetPosition(layerPosition);
		});

		m_visibleLayers.emplace_back(std::move(visibleLayer));
	}

	void VisibleLayerComponent::CreateVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity)
	{
		if (Nz::UInt32 id = layerEntity.GetServerId(); id != LocalLayerEntity::ClientsideId)
			layer->visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), layer->baseNode, layer->baseRenderOrder));
		else
			layer->localEntities.emplace(layerEntity.GetEntity()->GetId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), layer->baseNode, layer->baseRenderOrder));
	}

	void VisibleLayerComponent::DeleteVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity)
	{
		if (Nz::UInt32 id = layerEntity.GetServerId(); id != LocalLayerEntity::ClientsideId)
			layer->visualEntities.erase(layerEntity.GetServerId());
		else
			layer->localEntities.erase(layerEntity.GetEntity()->GetId());
	}

	Ndk::ComponentIndex VisibleLayerComponent::componentIndex;
}
