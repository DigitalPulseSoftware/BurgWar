// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/ClientOwnerComponent.hpp>

namespace bw
{
	inline ClientOwnerComponent::ClientOwnerComponent(ClientPlayerHandle owner) :
	m_owner(std::move(owner))
	{
	}

	inline ClientPlayer* ClientOwnerComponent::GetOwner() const
	{
		return m_owner;
	}
}
