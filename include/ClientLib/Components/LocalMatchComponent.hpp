// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP

#include <CoreLib/LayerIndex.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class LocalLayer;
	class LocalMatch;

	class LocalMatchComponent : public Ndk::Component<LocalMatchComponent>
	{
		public:
			inline LocalMatchComponent(LocalMatch& localMatch, LayerIndex layerIndex, Nz::Int64 uniqueId);
			~LocalMatchComponent() = default;

			LocalLayer& GetLayer();
			const LocalLayer& GetLayer() const;
			inline LayerIndex GetLayerIndex() const;
			inline LocalMatch& GetLocalMatch() const;
			inline Nz::Int64 GetUniqueId() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Int64 m_uniqueId;
			LocalMatch& m_localMatch;
			LayerIndex m_layerIndex;
	};
}

#include <ClientLib/Components/LocalMatchComponent.inl>

#endif
