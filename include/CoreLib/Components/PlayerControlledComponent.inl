// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/PlayerControlledComponent.hpp>

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
