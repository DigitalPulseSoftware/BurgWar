// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerControlledComponent.hpp>

namespace bw
{
	inline PlayerControlledComponent::PlayerControlledComponent(PlayerHandle owner) :
	m_owner(std::move(owner))
	{
	}

	inline Player* PlayerControlledComponent::GetOwner() const
	{
		return m_owner;
	}
}
