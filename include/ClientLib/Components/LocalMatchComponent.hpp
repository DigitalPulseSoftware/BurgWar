// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALMATCHCOMPONENT_HPP

#include <NDK/Component.hpp>

namespace bw
{
	class LocalMatch;

	class LocalMatchComponent : public Ndk::Component<LocalMatchComponent>
	{
		public:
			inline LocalMatchComponent(LocalMatch& localMatch, Nz::UInt16 layerIndex);
			~LocalMatchComponent() = default;

			inline Nz::UInt16 GetLayerIndex() const;
			inline LocalMatch& GetLocalMatch() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			LocalMatch& m_localMatch;
			Nz::UInt16 m_layerIndex;
	};
}

#include <ClientLib/Components/LocalMatchComponent.inl>

#endif
