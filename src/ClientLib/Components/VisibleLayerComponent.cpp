// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	void VisibleLayerComponent::Clear()
	{
		m_visibleLayers.clear();
	}

	void VisibleLayerComponent::RegisterLocalLayer(ClientLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor)
	{
		std::shared_ptr<VisibleLocalLayer> visibleLayer = std::make_shared<VisibleLocalLayer>();

		VisibleLayer* visibleLayerPtr = visibleLayer.get();

		visibleLayer->onSoundCreated.Connect(localLayer.OnSoundCreated, [=](ClientLayer*, std::size_t soundIndex, ClientLayerSound& sound)
		{
			CreateSound(visibleLayerPtr, soundIndex, sound);
		});

		visibleLayer->onSoundDelete.Connect(localLayer.OnSoundDelete, [=](ClientLayer*, std::size_t soundIndex, ClientLayerSound& sound)
		{
			DeleteSound(visibleLayerPtr, soundIndex, sound);
		});
		
		if (localLayer.IsEnabled())
		{
			localLayer.ForEachLayerSound([&](std::size_t soundIndex, ClientLayerSound& layerSound)
			{
				CreateSound(visibleLayerPtr, soundIndex, layerSound);
			});
		}

		RegisterLayer(std::move(visibleLayer), localLayer.GetClientMatch().GetCamera(), localLayer, renderOrder, scale, parallaxFactor);
	}

	void VisibleLayerComponent::RegisterVisibleLayer(Camera& camera, VisualLayer& visualLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor)
	{
		RegisterLayer(std::make_shared<VisibleLayer>(), camera, visualLayer, renderOrder, scale, parallaxFactor);
	}

	void VisibleLayerComponent::CreateSound(VisibleLayer* layer, std::size_t soundIndex, ClientLayerSound& layerSound)
	{
		layer->soundEntities.emplace(soundIndex, SoundEntity(m_renderWorld, layerSound.CreateHandle()));
	}

	void VisibleLayerComponent::CreateVisual(VisibleLayer* layer, Nz::Int64 uniqueId, LayerVisualEntity& layerEntity)
	{
		layer->visualEntities.emplace(uniqueId, VisualEntity(m_renderWorld, layerEntity.CreateHandle(), layer->baseNode, layer->baseRenderOrder));
	}

	void VisibleLayerComponent::DeleteSound(VisibleLayer* layer, std::size_t soundIndex, ClientLayerSound& /*layerSound*/)
	{
		layer->soundEntities.erase(soundIndex);
	}

	void VisibleLayerComponent::DeleteVisual(VisibleLayer* layer, Nz::Int64 uniqueId)
	{
		layer->visualEntities.erase(uniqueId);
	}

	void VisibleLayerComponent::RegisterLayer(std::shared_ptr<VisibleLayer> visibleLayer, Camera& camera, VisualLayer& visualLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor)
	{
		assert(visibleLayer);

		auto& nodeComponent = m_entity->GetComponent<Ndk::NodeComponent>();

		visibleLayer->baseNode.SetParent(nodeComponent);
		visibleLayer->baseNode.SetScale(scale);
		visibleLayer->baseRenderOrder = renderOrder;

		VisibleLayer* visibleLayerPtr = visibleLayer.get();
		
		visibleLayerPtr->onDisabled.Connect(visualLayer.OnDisabled, [=](VisualLayer*)
		{
			visibleLayerPtr->soundEntities.clear();
			visibleLayerPtr->visualEntities.clear();
		});

		visibleLayerPtr->onEnabled.Connect(visualLayer.OnEnabled, [=](VisualLayer* layer)
		{
			assert(visibleLayerPtr->visualEntities.empty());
			layer->ForEachVisualEntity([&](LayerVisualEntity& layerEntity)
			{
				CreateVisual(visibleLayerPtr, layerEntity.GetUniqueId(), layerEntity);
			});
		});

		visibleLayer->onVisualCreated.Connect(visualLayer.OnEntityVisualCreated, [=](VisualLayer*, LayerVisualEntity& layerEntity)
		{
			CreateVisual(visibleLayerPtr, layerEntity.GetUniqueId(), layerEntity);
		});

		visibleLayer->onVisualDelete.Connect(visualLayer.OnEntityVisualDelete, [=](VisualLayer*, LayerVisualEntity& layerEntity)
		{
			DeleteVisual(visibleLayerPtr, layerEntity.GetUniqueId());
		});
		
		if (visualLayer.IsEnabled())
		{
			visualLayer.ForEachVisualEntity([&](LayerVisualEntity& layerEntity)
			{
				CreateVisual(visibleLayerPtr, layerEntity.GetUniqueId(), layerEntity);
			});
		}

		visibleLayer->onCameraMove.Connect(camera.OnCameraMove, [=](Camera*, const Nz::Vector2f& newPosition)
		{
			// Parallax factor
			Nz::Vector2f layerPosition = newPosition * (Nz::Vector2f(1.f) - parallaxFactor);
			//layerPosition.x = std::round(layerPosition.x / scale.x) * scale.x;
			//layerPosition.y = std::round(layerPosition.y / scale.y) * scale.y;

			visibleLayerPtr->baseNode.SetPosition(layerPosition);
		});

		m_visibleLayers.emplace_back(std::move(visibleLayer));
	}

	Ndk::ComponentIndex VisibleLayerComponent::componentIndex;
}
