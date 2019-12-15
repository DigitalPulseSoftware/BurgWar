// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/LocalMatchComponent.hpp>

namespace bw
{
	inline LocalMatchComponent::LocalMatchComponent(LocalMatch& localMatch, LayerIndex layerIndex, Nz::Int64 uniqueId) :
	m_uniqueId(uniqueId),
	m_localMatch(localMatch),
	m_layerIndex(layerIndex)
	{
	}

	inline LayerIndex LocalMatchComponent::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline LocalMatch& LocalMatchComponent::GetLocalMatch() const
	{
		return m_localMatch;
	}

	inline Nz::Int64 LocalMatchComponent::GetUniqueId() const
	{
		return m_uniqueId;
	}
}
