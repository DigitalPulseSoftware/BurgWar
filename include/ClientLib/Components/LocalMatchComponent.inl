// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalMatchComponent.hpp>

namespace bw
{
	inline LocalMatchComponent::LocalMatchComponent(LocalMatch& localMatch, Nz::UInt16 layerIndex) :
	m_localMatch(localMatch),
	m_layerIndex(layerIndex)
	{
	}

	inline Nz::UInt16 LocalMatchComponent::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline LocalMatch& LocalMatchComponent::GetLocalMatch() const
	{
		return m_localMatch;
	}
}
