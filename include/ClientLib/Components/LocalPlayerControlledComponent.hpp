// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALPLAYERCONTROLLED_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALPLAYERCONTROLLED_HPP

#include <ClientLib/Export.hpp>
#include <CoreLib/Player.hpp>
#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class ClientMatch;

	class BURGWAR_CLIENTLIB_API LocalPlayerControlledComponent : public Ndk::Component<LocalPlayerControlledComponent>
	{
		public:
			inline LocalPlayerControlledComponent(ClientMatch& clientMatch, Nz::UInt8 localPlayerIndex);
			~LocalPlayerControlledComponent() = default;

			inline ClientMatch& GetClientMatch() const;
			inline Nz::UInt8 GetLocalPlayerIndex() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			ClientMatch& m_clientMatch;
			Nz::UInt8 m_localPlayerIndex;
	};
}

#include <ClientLib/Components/LocalPlayerControlledComponent.inl>

#endif
