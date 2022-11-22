// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_CLIENTOWNER_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_CLIENTOWNER_HPP

#include <ClientLib/Export.hpp>
#include <ClientLib/ClientPlayer.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CLIENTLIB_API ClientOwnerComponent
	{
		public:
			ClientOwnerComponent(ClientPlayerHandle owner);
			~ClientOwnerComponent() = default;

			inline ClientPlayer* GetOwner() const;

		private:
			ClientPlayerHandle m_owner;
	};
}

#include <ClientLib/Components/ClientOwnerComponent.inl>

#endif
