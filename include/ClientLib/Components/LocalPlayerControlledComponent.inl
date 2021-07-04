// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalPlayerControlledComponent.hpp>

namespace bw
{
	inline LocalPlayerControlledComponent::LocalPlayerControlledComponent(LocalMatch& localMatch, Nz::UInt8 localPlayerIndex) :
	m_localMatch(localMatch),
	m_localPlayerIndex(localPlayerIndex)
	{
	}
	
	inline LocalMatch& LocalPlayerControlledComponent::GetLocalMatch() const
	{
		return m_localMatch;
	}
	
	inline Nz::UInt8 LocalPlayerControlledComponent::GetLocalPlayerIndex() const
	{
		return m_localPlayerIndex;
	}
}
