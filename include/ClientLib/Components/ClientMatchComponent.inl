// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/ClientMatchComponent.hpp>

namespace bw
{
	inline ClientMatchComponent::ClientMatchComponent(ClientMatch& clientMatch, LayerIndex layerIndex, EntityId uniqueId) :
	m_uniqueId(uniqueId),
	m_clientMatch(clientMatch),
	m_layerIndex(layerIndex)
	{
	}

	inline LayerIndex ClientMatchComponent::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline ClientMatch& ClientMatchComponent::GetClientMatch() const
	{
		return m_clientMatch;
	}

	inline EntityId ClientMatchComponent::GetUniqueId() const
	{
		return m_uniqueId;
	}
}
