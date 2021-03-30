// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP

#include <ClientLib/Camera.hpp>
#include <ClientLib/Export.hpp>
#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/SoundEntity.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/VisualLayer.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Component.hpp>
#include <memory>

namespace bw
{
	class LocalLayerEntity;

	class BURGWAR_CLIENTLIB_API VisibleLayerComponent : public Ndk::Component<VisibleLayerComponent>
	{
		public:
			inline VisibleLayerComponent(Ndk::World& renderWorld);
			~VisibleLayerComponent() = default;

			void Clear();

			void RegisterLocalLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor);
			void RegisterVisibleLayer(Camera& camera, VisualLayer& visualLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor);

			static Ndk::ComponentIndex componentIndex;

		private:
			struct VisibleLayer;

			void CreateSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& layerSound);
			void CreateVisual(VisibleLayer* layer, Nz::Int64 uniqueId, LayerVisualEntity& layerEntity);

			void DeleteSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& layerSound);
			void DeleteVisual(VisibleLayer* layer, Nz::Int64 uniqueId);

			void RegisterLayer(std::shared_ptr<VisibleLayer> visibleLayer, Camera& camera, VisualLayer& visualLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor);

			struct VisibleLayer
			{
				Nz::Node baseNode;
				int baseRenderOrder;

				tsl::hopscotch_map<std::size_t /*sound*/, SoundEntity> soundEntities;
				tsl::hopscotch_map<Nz::Int64 /*uniqueId*/, VisualEntity> visualEntities;

				NazaraSlot(Camera, OnCameraMove, onCameraMove);
				NazaraSlot(VisualLayer, OnDisabled, onDisabled);
				NazaraSlot(VisualLayer, OnEnabled, onEnabled);
				NazaraSlot(VisualLayer, OnEntityVisualCreated, onVisualCreated);
				NazaraSlot(VisualLayer, OnEntityVisualDelete, onVisualDelete);
			};

			struct VisibleLocalLayer : VisibleLayer
			{
				NazaraSlot(LocalLayer, OnSoundCreated, onSoundCreated);
				NazaraSlot(LocalLayer, OnSoundDelete, onSoundDelete);
			};

			//FIXME (shared_ptr => unique_ptr)
			std::vector<std::shared_ptr<VisibleLayer>> m_visibleLayers;
			Ndk::World& m_renderWorld;
	};
}

#include <ClientLib/Components/VisibleLayerComponent.inl>

#endif
