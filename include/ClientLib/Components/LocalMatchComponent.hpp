// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <ClientLib/Export.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class LocalLayer;
	class LocalMatch;

	class BURGWAR_CLIENTLIB_API LocalMatchComponent : public Ndk::Component<LocalMatchComponent>
	{
		public:
			inline LocalMatchComponent(LocalMatch& localMatch, LayerIndex layerIndex, EntityId uniqueId);
			~LocalMatchComponent() = default;

			LocalLayer& GetLayer();
			const LocalLayer& GetLayer() const;
			inline LayerIndex GetLayerIndex() const;
			inline LocalMatch& GetLocalMatch() const;
			inline EntityId GetUniqueId() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			EntityId m_uniqueId;
			LocalMatch& m_localMatch;
			LayerIndex m_layerIndex;
	};
}

#include <ClientLib/Components/LocalMatchComponent.inl>

#endif
