// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalMatchComponent.hpp>

namespace bw
{
	inline LocalMatchComponent::LocalMatchComponent(LocalMatch& localMatch) :
	m_localMatch(localMatch)
	{
	}

	inline LocalMatch& LocalMatchComponent::GetLocalMatch() const
	{
		return m_localMatch;
	}
}
