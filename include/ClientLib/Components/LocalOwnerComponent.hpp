// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_LOCALOWNER_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_LOCALOWNER_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/LocalPlayer.hpp>
#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CLIENTLIB_API LocalOwnerComponent : public Ndk::Component<LocalOwnerComponent>
	{
		public:
			LocalOwnerComponent(LocalPlayerHandle owner);
			~LocalOwnerComponent() = default;

			inline LocalPlayer* GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			LocalPlayerHandle m_owner;
	};
}

#include <ClientLib/Components/LocalOwnerComponent.inl>

#endif
