// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/MatchComponent.hpp>

namespace bw
{
	inline MatchComponent::MatchComponent(Match& match, LayerIndex layerIndex, EntityId uniqueId) :
	m_match(&match),
	m_uniqueId(uniqueId),
	m_layerIndex(layerIndex)
	{
	}

	inline LayerIndex MatchComponent::GetLayerIndex() const
	{
		return m_layerIndex;
	}

	inline Match& MatchComponent::GetMatch() const
	{
		return *m_match;
	}

	inline EntityId MatchComponent::GetUniqueId() const
	{
		return m_uniqueId;
	}
}
