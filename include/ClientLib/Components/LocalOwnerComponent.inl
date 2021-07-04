// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalOwnerComponent.hpp>

namespace bw
{
	inline LocalOwnerComponent::LocalOwnerComponent(LocalPlayerHandle owner) :
	m_owner(std::move(owner))
	{
	}

	inline LocalPlayer* LocalOwnerComponent::GetOwner() const
	{
		return m_owner;
	}
}
