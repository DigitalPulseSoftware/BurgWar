// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/LocalMatch.hpp>
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
		
		visibleLayer->onSoundCreated.Connect(localLayer.OnSoundCreated, [=](LocalLayer*, std::size_t soundIndex, LocalLayerSound& sound)
		{
			CreateSound(visibleLayerPtr, soundIndex, sound);
		});

		visibleLayer->onSoundDelete.Connect(localLayer.OnSoundDelete, [=](LocalLayer*, std::size_t soundIndex, LocalLayerSound& sound)
		{
			DeleteSound(visibleLayerPtr, soundIndex, sound);
		});

		if (localLayer.IsEnabled())
		{
			localLayer.ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
			{
				CreateVisual(visibleLayerPtr, layerEntity);
			});

			localLayer.ForEachLayerSound([&](std::size_t soundIndex, LocalLayerSound& layerSound)
			{
				CreateSound(visibleLayerPtr, soundIndex, layerSound);
			});
		}

		visibleLayer->onCameraMove.Connect(localLayer.GetLocalMatch().GetCamera().OnCameraMove, [=](Camera*, const Nz::Vector2f& newPosition)
		{
			// Parallax factor
			Nz::Vector2f layerPosition = newPosition * (Nz::Vector2f(1.f) - parallaxFactor);
			//layerPosition.x = std::round(layerPosition.x / scale.x) * scale.x;
			//layerPosition.y = std::round(layerPosition.y / scale.y) * scale.y;

			visibleLayerPtr->baseNode.SetPosition(layerPosition);
		});

		m_visibleLayers.emplace_back(std::move(visibleLayer));
	}

	void VisibleLayerComponent::CreateSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& layerSound)
	{
		layer->soundEntities.emplace(soundIndex, SoundEntity(m_renderWorld, layerSound.CreateHandle()));
	}

	void VisibleLayerComponent::CreateVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity)
	{
		if (!layerEntity.IsClientside())
			layer->visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), layer->baseNode, layer->baseRenderOrder));
		else
			layer->localEntities.emplace(layerEntity.GetEntity()->GetId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle(), layer->baseNode, layer->baseRenderOrder));
	}

	void VisibleLayerComponent::DeleteSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& /*layerSound*/)
	{
		layer->soundEntities.erase(soundIndex);
	}

	void VisibleLayerComponent::DeleteVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity)
	{
		if (!layerEntity.IsClientside())
			layer->visualEntities.erase(layerEntity.GetServerId());
		else
			layer->localEntities.erase(layerEntity.GetEntity()->GetId());
	}

	Ndk::ComponentIndex VisibleLayerComponent::componentIndex;
}
