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

			void RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor);

			static Ndk::ComponentIndex componentIndex;

		private:
			struct VisibleLayer;

			void CreateSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& layerSound);
			void CreateVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity);

			void DeleteSound(VisibleLayer* layer, std::size_t soundIndex, LocalLayerSound& layerSound);
			void DeleteVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity);

			struct VisibleLayer
			{
				Nz::Node baseNode;
				int baseRenderOrder;

				tsl::hopscotch_map<Nz::UInt32 /*clientId*/, VisualEntity> localEntities;
				tsl::hopscotch_map<Nz::UInt32 /*serverId*/, VisualEntity> visualEntities;
				tsl::hopscotch_map<std::size_t /*sound*/, SoundEntity> soundEntities;

				NazaraSlot(Camera, OnCameraMove, onCameraMove);
				NazaraSlot(LocalLayer, OnDisabled, onDisabled);
				NazaraSlot(LocalLayer, OnEnabled, onEnabled);
				NazaraSlot(LocalLayer, OnEntityCreated, onEntityCreated);
				NazaraSlot(LocalLayer, OnEntityDelete, onEntityDelete);
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
