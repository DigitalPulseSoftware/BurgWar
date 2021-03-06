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
	class ClientLayer;
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API ClientMatchComponent : public Ndk::Component<ClientMatchComponent>
	{
		public:
			inline ClientMatchComponent(ClientMatch& clientMatch, LayerIndex layerIndex, EntityId uniqueId);
			~ClientMatchComponent() = default;

			ClientLayer& GetLayer();
			const ClientLayer& GetLayer() const;
			inline LayerIndex GetLayerIndex() const;
			inline ClientMatch& GetClientMatch() const;
			inline EntityId GetUniqueId() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			EntityId m_uniqueId;
			ClientMatch& m_clientMatch;
			LayerIndex m_layerIndex;
	};
}

#include <ClientLib/Components/ClientMatchComponent.inl>

#endif
