// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalPlayerControlledComponent.hpp>

namespace bw
{
	inline LocalPlayerControlledComponent::LocalPlayerControlledComponent(ClientMatch& clientMatch, Nz::UInt8 localPlayerIndex) :
	m_clientMatch(clientMatch),
	m_localPlayerIndex(localPlayerIndex)
	{
	}
	
	inline ClientMatch& LocalPlayerControlledComponent::GetClientMatch() const
	{
		return m_clientMatch;
	}
	
	inline Nz::UInt8 LocalPlayerControlledComponent::GetLocalPlayerIndex() const
	{
		return m_localPlayerIndex;
	}
}
