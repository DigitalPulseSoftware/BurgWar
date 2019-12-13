// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP

#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Component.hpp>
#include <memory>

namespace bw
{
	class LocalLayerEntity;

	class VisibleLayerComponent : public Ndk::Component<VisibleLayerComponent>
	{
		public:
			inline VisibleLayerComponent(Ndk::World& renderWorld);
			~VisibleLayerComponent() = default;

			void Clear();

			void RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale, const Nz::Vector2f& parallaxFactor);

			static Ndk::ComponentIndex componentIndex;

		private:
			struct VisibleLayer;

			void CreateVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity);
			void DeleteVisual(VisibleLayer* layer, LocalLayerEntity& layerEntity);

			struct VisibleLayer
			{
				Nz::Node baseNode;
				int baseRenderOrder;

				tsl::hopscotch_map<Nz::UInt32 /*clientId*/, VisualEntity> localEntities;
				tsl::hopscotch_map<Nz::UInt32 /*serverId*/, VisualEntity> visualEntities;

				NazaraSlot(LocalLayer, OnDisabled, onDisabled);
				NazaraSlot(LocalLayer, OnEnabled, onEnabled);
				NazaraSlot(LocalLayer, OnEntityCreated, onEntityCreated);
				NazaraSlot(LocalLayer, OnEntityDelete, onEntityDelete);
				NazaraSlot(LocalMatch, OnCameraMoved, onCameraMoved);
			};

			//FIXME (shared_ptr => unique_ptr)
			std::vector<std::shared_ptr<VisibleLayer>> m_visibleLayers;
			Ndk::World& m_renderWorld;
	};
}

#include <ClientLib/Components/VisibleLayerComponent.inl>

#endif
