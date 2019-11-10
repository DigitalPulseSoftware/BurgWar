// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_VISIBLELAYERCOMPONENT_HPP

#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/Component.hpp>
#include <memory>

namespace bw
{
	class VisibleLayerComponent : public Ndk::Component<VisibleLayerComponent>
	{
		public:
			inline VisibleLayerComponent(Ndk::World& renderWorld);
			~VisibleLayerComponent() = default;

			void RegisterVisibleLayer(LocalLayer& localLayer, int renderOrder, const Nz::Vector2f& scale);

			static Ndk::ComponentIndex componentIndex;

		private:
			struct VisibleLayer
			{
				Nz::Node baseNode;

				tsl::hopscotch_map<Nz::UInt32 /*serverId*/, VisualEntity> visualEntities;

				NazaraSlot(LocalLayer, OnEntityCreated, onEntityCreated);
				NazaraSlot(LocalLayer, OnEntityDelete, onEntityDelete);
			};

			//FIXME (shared_ptr => unique_ptr)
			std::vector<std::shared_ptr<VisibleLayer>> m_visibleLayers;
			Ndk::World& m_renderWorld;
	};
}

#include <ClientLib/Components/VisibleLayerComponent.inl>

#endif
